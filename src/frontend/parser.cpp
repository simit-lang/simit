#include <memory>
#include <string>

#include "complex_types.h"
#include "fir.h"
#include "intrinsics.h"
#include "parser.h"
#include "scanner.h"

namespace simit { 
namespace internal {

// Simit language grammar is documented here in EBNF. Note that '{}' is used 
// here to denote zero or more instances of the enclosing term, while '[]' is 
// used to denote zero or one instance of the enclosing term.

fir::Program::Ptr Parser::parse(const TokenStream &tokens) {
  this->tokens = tokens;
  
  decls = SymbolTable();
  for (const auto kv : ir::intrinsics::byNames()) {
    decls.insert(kv.first, IdentType::FUNCTION);
  }

  return parseProgram();
}

// program: {program_element}
fir::Program::Ptr Parser::parseProgram() {
  auto program = std::make_shared<fir::Program>();
  
  while (peek().type != Token::Type::END) {
    const fir::FIRNode::Ptr element = parseProgramElement();
    if (element) {
      program->elems.push_back(element);
    }
  }
 
  return program;
}

// program_element: element_type_decl | extern_decl | const_decl
//                | func_decl | proc_decl | test
fir::FIRNode::Ptr Parser::parseProgramElement() {
  try {
    switch (peek().type) {
      case Token::Type::TEST:
        return parseTest();
      case Token::Type::EXPORT:
      case Token::Type::FUNC:
        return parseFuncDecl();
      case Token::Type::PROC:
        return parseProcDecl();
      case Token::Type::ELEMENT:
        return parseElementTypeDecl();
      case Token::Type::EXTERN:
        return parseExternFuncOrDecl();
      case Token::Type::CONST:
        return parseConstDecl();
      default:
        reportError(peek(), "a program element");
        throw SyntaxError();
        break;
    }
  } catch (const SyntaxError &) {
    skipTo({Token::Type::TEST, Token::Type::FUNC, Token::Type::PROC, 
            Token::Type::EXPORT, Token::Type::ELEMENT, Token::Type::EXTERN, 
            Token::Type::CONST});
    
    return fir::FIRNode::Ptr();
  }
}

// element_type_decl: 'element' ident field_decl_list 'end'
fir::ElementTypeDecl::Ptr Parser::parseElementTypeDecl() {
  auto elemTypeDecl = std::make_shared<fir::ElementTypeDecl>();
  
  const Token elementToken = consume(Token::Type::ELEMENT);
  elemTypeDecl->setBeginLoc(elementToken);
  
  elemTypeDecl->name = parseIdent(); 
  elemTypeDecl->fields = parseFieldDeclList();

  const Token endToken = consume(Token::Type::BLOCKEND);
  elemTypeDecl->setEndLoc(endToken);
  
  return elemTypeDecl;
}

// field_decl_list: {field_decl}
std::vector<fir::FieldDecl::Ptr> Parser::parseFieldDeclList() {
  std::vector<fir::FieldDecl::Ptr> fields;

  while (peek().type == Token::Type::IDENT) {
    const fir::FieldDecl::Ptr field = parseFieldDecl();
    fields.push_back(field);
  }

  return fields;
}

// field_decl: tensor_decl ';'
fir::FieldDecl::Ptr Parser::parseFieldDecl() {
  auto fieldDecl = std::make_shared<fir::FieldDecl>();

  const auto tensorDecl = parseTensorDecl();
  fieldDecl->name = tensorDecl->name;
  fieldDecl->type = tensorDecl->type;
  
  const Token endToken = consume(Token::Type::SEMICOL);
  fieldDecl->setEndLoc(endToken);

  return fieldDecl;
}

// extern_func_or_decl: extern_decl | extern_func_decl
fir::FIRNode::Ptr Parser::parseExternFuncOrDecl() {
  auto tokenAfterExtern = peek(1);

  if (tokenAfterExtern.type == Token::Type::FUNC)
    return parseExternFuncDecl();
  else
    return parseExternDecl();
}

// extern_decl: 'extern' ident_decl ';'
fir::ExternDecl::Ptr Parser::parseExternDecl() {
  auto externDecl = std::make_shared<fir::ExternDecl>();
  
  const Token externToken = consume(Token::Type::EXTERN);
  externDecl->setBeginLoc(externToken);
 
  const auto identDecl = parseIdentDecl();
  externDecl->name = identDecl->name;
  externDecl->type = identDecl->type;
  
  const Token endToken = consume(Token::Type::SEMICOL);
  externDecl->setEndLoc(endToken);
 
  return externDecl;
}

// extern_func_decl: 'extern' 'func' ident generic_params arguments results ';'
fir::FuncDecl::Ptr Parser::parseExternFuncDecl() {
  const unsigned originalLevel = decls.levels();

  try {
    auto externFuncDecl = std::make_shared<fir::FuncDecl>();
    externFuncDecl->type = fir::FuncDecl::Type::EXTERNAL;
    
    const Token externToken = consume(Token::Type::EXTERN);
    externFuncDecl->setBeginLoc(externToken);
    
    consume(Token::Type::FUNC);
    externFuncDecl->name = parseIdent();
    
    decls.insert(externFuncDecl->name->ident, IdentType::FUNCTION);
    decls.scope();
    
    externFuncDecl->genericParams = parseGenericParams();
    externFuncDecl->args = parseArguments();
    externFuncDecl->results = parseResults();
    
    decls.unscope();
    
    const Token endToken = consume(Token::Type::SEMICOL);
    externFuncDecl->setEndLoc(endToken);
    
    return externFuncDecl;
  } catch (const SyntaxError &) {
    while (decls.levels() != originalLevel) {
      decls.unscope();
    }

    throw;
  }
}

// func_decl: ['export'] 'func' ident generic_params
//            arguments results stmt_block 'end'
fir::FuncDecl::Ptr Parser::parseFuncDecl() {
  const unsigned originalLevel = decls.levels();

  try {
    auto funcDecl = std::make_shared<fir::FuncDecl>();
  
    const Token funcToken = peek();
    funcDecl->setBeginLoc(funcToken);
    funcDecl->type = (funcToken.type == Token::Type::EXPORT) ? 
                     fir::FuncDecl::Type::EXPORTED : 
                     fir::FuncDecl::Type::INTERNAL;
    
    tryconsume(Token::Type::EXPORT);
    consume(Token::Type::FUNC);
    
    funcDecl->name = parseIdent();
    
    decls.insert(funcDecl->name->ident, IdentType::FUNCTION);
    decls.scope();
    
    funcDecl->genericParams = parseGenericParams();
    funcDecl->args = parseArguments();
    funcDecl->results = parseResults();

    decls.scope();
    funcDecl->body = parseStmtBlock();
    decls.unscope();
    
    decls.unscope();
  
    const Token endToken = consume(Token::Type::BLOCKEND);
    funcDecl->setEndLoc(endToken);
  
    return funcDecl;
  } catch (const SyntaxError &) {
    while (decls.levels() != originalLevel) {
      decls.unscope();
    }

    throw;
  }
}

// proc_decl: 'proc' ident [arguments results] stmt_block 'end'
fir::FuncDecl::Ptr Parser::parseProcDecl() {
  const unsigned originalLevel = decls.levels();

  try {
    auto procDecl = std::make_shared<fir::FuncDecl>();
    procDecl->type = fir::FuncDecl::Type::EXPORTED;
  
    const Token procToken = consume(Token::Type::PROC);
    procDecl->setBeginLoc(procToken);
  
    procDecl->name = parseIdent();
    
    decls.insert(procDecl->name->ident, IdentType::FUNCTION);
    decls.scope();
    
    if (peek().type == Token::Type::LP) {
      switch (peek(1).type) {
        case Token::Type::IDENT:
          if (peek(2).type != Token::Type::COL) {
            break;
          }
        case Token::Type::RP:
        case Token::Type::INOUT:
          procDecl->genericParams = parseGenericParams();
          procDecl->args = parseArguments();
          procDecl->results = parseResults();
          break;
        default:
          break;
      }
    }

    decls.scope();
    procDecl->body = parseStmtBlock();
    decls.unscope();
    
    decls.unscope();
    
    const Token endToken = consume(Token::Type::BLOCKEND);
    procDecl->setEndLoc(endToken);
  
    return procDecl;
  } catch (const SyntaxError &) {
    while (decls.levels() != originalLevel) {
      decls.unscope();
    }

    throw;
  }
}

// generic_params: ['<' generic_param {',' generic_param} '>']
std::vector<fir::GenericParam::Ptr> Parser::parseGenericParams() {
  std::vector<fir::GenericParam::Ptr> genericParams;

  if (tryconsume(Token::Type::LA)) {
    do {
      const fir::GenericParam::Ptr genericParam = parseGenericParam();
      genericParams.push_back(genericParam);
    } while (tryconsume(Token::Type::COMMA));
    consume(Token::Type::RA);
  }

  return genericParams;
}

// generic_param: [INT_LITERAL(0) ':'] ident
fir::GenericParam::Ptr Parser::parseGenericParam() {
  auto genericParam = std::make_shared<fir::GenericParam>();
  genericParam->type = fir::GenericParam::Type::UNKNOWN;

  if (peek().type == Token::Type::INT_LITERAL && peek().num == 0) {
    consume(Token::Type::INT_LITERAL);
    consume(Token::Type::COL);
    
    genericParam->type = fir::GenericParam::Type::RANGE;
  }

  const Token identToken = consume(Token::Type::IDENT);
  genericParam->setLoc(identToken);
  genericParam->name = identToken.str;

  const auto type = (genericParam->type == fir::GenericParam::Type::RANGE) ?
                    IdentType::RANGE_GENERIC_PARAM : IdentType::GENERIC_PARAM;
  decls.insert(genericParam->name, type); 

  return genericParam;
}

// arguments: '(' [argument_decl {',' argument_decl}] ')'
std::vector<fir::Argument::Ptr> Parser::parseArguments() {
  std::vector<fir::Argument::Ptr> arguments;
  
  consume(Token::Type::LP);
  if (peek().type != Token::Type::RP) {
    do {
      const fir::Argument::Ptr argument = parseArgumentDecl();
      arguments.push_back(argument);
    } while (tryconsume(Token::Type::COMMA));
  }
  consume(Token::Type::RP);

  return arguments;
}

// argument_decl: ['inout'] ident_decl
fir::Argument::Ptr Parser::parseArgumentDecl() {
  auto argDecl = std::make_shared<fir::Argument>();
  
  if (peek().type == Token::Type::INOUT) {
    const Token inoutToken = consume(Token::Type::INOUT);

    argDecl = std::make_shared<fir::InOutArgument>();
    argDecl->setBeginLoc(inoutToken);
  }

  const auto identDecl = parseIdentDecl();
  argDecl->name = identDecl->name;
  argDecl->type = identDecl->type;
  
  return argDecl;
}

// results: ['->' '(' ident_decl {',' ident_decl} ')']
std::vector<fir::IdentDecl::Ptr> Parser::parseResults() {
  std::vector<fir::IdentDecl::Ptr> results;

  if (tryconsume(Token::Type::RARROW)) {
    consume(Token::Type::LP);
    do {
      const fir::IdentDecl::Ptr result = parseIdentDecl();
      results.push_back(result);
    } while (tryconsume(Token::Type::COMMA));
    consume(Token::Type::RP);
  }

  return results;
}

// stmt_block: {stmt}
fir::StmtBlock::Ptr Parser::parseStmtBlock() {
  auto stmtBlock = std::make_shared<fir::StmtBlock>();

  while (true) {
    switch (peek().type) {
      case Token::Type::BLOCKEND:
      case Token::Type::ELIF:
      case Token::Type::ELSE:
      case Token::Type::END:
        return stmtBlock;
      default:
      {
        const fir::Stmt::Ptr stmt = parseStmt();
        if (stmt) {
          stmtBlock->stmts.push_back(stmt);
        }
        break;
      }
    }
  }
}

// stmt: var_decl | const_decl | if_stmt | while_stmt | do_while_stmt 
//     | for_stmt | print_stmt | apply_stmt | expr_or_assign_stmt
fir::Stmt::Ptr Parser::parseStmt() {
  switch (peek().type) {
    case Token::Type::VAR:
      return parseVarDecl();
    case Token::Type::CONST:
      return parseConstDecl();
    case Token::Type::IF:
      return parseIfStmt();
    case Token::Type::WHILE:
      return parseWhileStmt();
    case Token::Type::DO:
      return parseDoWhileStmt();
    case Token::Type::FOR:
      return parseForStmt();
    case Token::Type::PRINT:
    case Token::Type::PRINTLN:
      return parsePrintStmt();
    case Token::Type::APPLY:
      return parseApplyStmt();
    default:
      return parseExprOrAssignStmt();
  }
}

// var_decl: 'var' ident (('=' expr) | (':' tensor_type ['=' expr])) ';'
fir::VarDecl::Ptr Parser::parseVarDecl() {
  try {
    auto varDecl = std::make_shared<fir::VarDecl>();
    
    const Token varToken = consume(Token::Type::VAR);
    varDecl->setBeginLoc(varToken);
  
    varDecl->name = parseIdent();
    if (tryconsume(Token::Type::COL)) {
      varDecl->type = parseTensorType();
      if (tryconsume(Token::Type::ASSIGN)) {
        varDecl->initVal = parseExpr();
      }
    } else {
      consume(Token::Type::ASSIGN);
      varDecl->initVal = parseExpr();
    }
  
    const Token endToken = consume(Token::Type::SEMICOL);
    varDecl->setEndLoc(endToken);

    decls.insert(varDecl->name->ident, IdentType::OTHER);

    return varDecl;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL});
    consume(Token::Type::SEMICOL);

    return fir::VarDecl::Ptr();
  }
}

// const_decl: 'const' ident [':' tensor_type] '=' expr ';'
fir::ConstDecl::Ptr Parser::parseConstDecl() {
  try {
    auto constDecl = std::make_shared<fir::ConstDecl>();
    
    const Token constToken = consume(Token::Type::CONST);
    constDecl->setBeginLoc(constToken);
    
    constDecl->name = parseIdent();
    if (tryconsume(Token::Type::COL)) {
      constDecl->type = parseTensorType();
    }
    consume(Token::Type::ASSIGN);
    constDecl->initVal = parseExpr();
  
    const Token endToken = consume(Token::Type::SEMICOL);
    constDecl->setEndLoc(endToken);

    decls.insert(constDecl->name->ident, IdentType::OTHER);

    return constDecl;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL}); 
    consume(Token::Type::SEMICOL);

    return fir::ConstDecl::Ptr();
  }
}

// ident_decl: ident ':' type
fir::IdentDecl::Ptr Parser::parseIdentDecl() {
  auto identDecl = std::make_shared<fir::IdentDecl>();

  identDecl->name = parseIdent();
  consume(Token::Type::COL);
  identDecl->type = parseType();
  
  const auto type = fir::isa<fir::TupleType>(identDecl->type) ? 
                    IdentType::TUPLE : IdentType::OTHER;
  decls.insert(identDecl->name->ident, type);
  
  return identDecl;
}

// tensor_decl: ident ':' tensor_type
// This rule is needed to prohibit declaration of non-tensor variables and 
// fields, which are currently unsupported. Probably want to replace with 
// ident_decl rule at some point in the future.
fir::IdentDecl::Ptr Parser::parseTensorDecl() {
  auto tensorDecl = std::make_shared<fir::IdentDecl>();

  tensorDecl->name = parseIdent();
  consume(Token::Type::COL);
  tensorDecl->type = parseTensorType();
  
  decls.insert(tensorDecl->name->ident, IdentType::OTHER);

  return tensorDecl;
}

// while_stmt: 'while' expr stmt_block 'end'
fir::WhileStmt::Ptr Parser::parseWhileStmt() {
  try {
    auto whileStmt = std::make_shared<fir::WhileStmt>();
    
    const Token whileToken = consume(Token::Type::WHILE);
    whileStmt->setBeginLoc(whileToken);
    
    whileStmt->cond = parseExpr();
    
    decls.scope();
    whileStmt->body = parseStmtBlock();
    decls.unscope();

    const Token endToken = consume(Token::Type::BLOCKEND);
    whileStmt->setEndLoc(endToken);

    return whileStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND});
    consume(Token::Type::BLOCKEND);

    return fir::WhileStmt::Ptr();
  }
}

// do_while_stmt: 'do' stmt_block 'end' 'while' expr
fir::DoWhileStmt::Ptr Parser::parseDoWhileStmt() {
  try {
    auto doWhileStmt = std::make_shared<fir::DoWhileStmt>();
    
    const Token doToken = consume(Token::Type::DO);
    doWhileStmt->setBeginLoc(doToken);
   
    decls.scope();
    doWhileStmt->body = parseStmtBlock();
    decls.unscope();

    consume(Token::Type::BLOCKEND);
    consume(Token::Type::WHILE);
    doWhileStmt->cond = parseExpr();

    return doWhileStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND, Token::Type::ELIF, Token::Type::ELSE, 
            Token::Type::END, Token::Type::VAR, Token::Type::CONST, 
            Token::Type::IF, Token::Type::WHILE, Token::Type::DO, 
            Token::Type::FOR, Token::Type::PRINT, Token::Type::PRINTLN});
    
    return fir::DoWhileStmt::Ptr();
  }
}

// if_stmt: 'if' expr stmt_block else_clause 'end'
fir::IfStmt::Ptr Parser::parseIfStmt() {
  try {
    auto ifStmt = std::make_shared<fir::IfStmt>();
    
    const Token ifToken = consume(Token::Type::IF);
    ifStmt->setBeginLoc(ifToken);
    
    ifStmt->cond = parseExpr();

    decls.scope();
    ifStmt->ifBody = parseStmtBlock();
    decls.unscope();

    decls.scope();
    ifStmt->elseBody = parseElseClause();
    decls.unscope();

    const Token endToken = consume(Token::Type::BLOCKEND);
    ifStmt->setEndLoc(endToken);

    return ifStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::ELIF, Token::Type::ELSE, Token::Type::BLOCKEND});
    
    parseElseClause();
    consume(Token::Type::BLOCKEND);
    
    return fir::IfStmt::Ptr();
  }
}

// else_clause: [('else' stmt_block) | ('elif' expr stmt_block else_clause)]
fir::Stmt::Ptr Parser::parseElseClause() {
  switch (peek().type) {
    case Token::Type::ELSE:
      consume(Token::Type::ELSE);
      return parseStmtBlock();
    case Token::Type::ELIF:
    {
      try {
        auto elifClause = std::make_shared<fir::IfStmt>();
        
        const Token elifToken = consume(Token::Type::ELIF);
        elifClause->setBeginLoc(elifToken);
 
        elifClause->cond = parseExpr();

        decls.scope();
        elifClause->ifBody = parseStmtBlock();
        decls.unscope();

        decls.scope();
        elifClause->elseBody = parseElseClause();
        decls.unscope();

        return elifClause;
      } catch (const SyntaxError &) {
        skipTo({Token::Type::ELIF, Token::Type::ELSE, Token::Type::BLOCKEND});
        
        decls.scope();
        parseElseClause();
        decls.unscope();

        return fir::Stmt::Ptr();
      }
    }
    default:
      return fir::Stmt::Ptr();
  }
}

// for_stmt: 'for' ident 'in' for_domain stmt_block 'end'
fir::ForStmt::Ptr Parser::parseForStmt() {
  try {
    auto forStmt = std::make_shared<fir::ForStmt>();

    const Token forToken = consume(Token::Type::FOR);
    forStmt->setBeginLoc(forToken);
   
    forStmt->loopVar = parseIdent();
    consume(Token::Type::IN);
    forStmt->domain = parseForDomain();
  
    decls.scope();
    decls.insert(forStmt->loopVar->ident, IdentType::OTHER);
    
    forStmt->body = parseStmtBlock(); 
    decls.unscope();

    const Token endToken = consume(Token::Type::BLOCKEND);
    forStmt->setEndLoc(endToken);

    return forStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND});    
    consume(Token::Type::BLOCKEND);

    return fir::ForStmt::Ptr();
  }
}

// for_domain: set_index_set | (expr ':' expr)
fir::ForDomain::Ptr Parser::parseForDomain() {
  if (peek().type == Token::Type::IDENT && peek(1).type != Token::Type::COL) {
    auto indexSetDomain = std::make_shared<fir::IndexSetDomain>();
    indexSetDomain->set = parseSetIndexSet();
    
    return indexSetDomain;
  }

  auto rangeDomain = std::make_shared<fir::RangeDomain>();

  rangeDomain->lower = parseExpr();
  consume(Token::Type::COL);
  rangeDomain->upper = parseExpr();
  
  return rangeDomain;
}

// print_stmt: ('print' | 'println') expr {',' expr} ';'
fir::PrintStmt::Ptr Parser::parsePrintStmt() {
  try {
    auto printStmt = std::make_shared<fir::PrintStmt>();

    if (peek().type == Token::Type::PRINT) {
      const Token printToken = consume(Token::Type::PRINT);
      printStmt->setBeginLoc(printToken);
      printStmt->printNewline = false;
    } else {
      const Token printlnToken = consume(Token::Type::PRINTLN);
      printStmt->setBeginLoc(printlnToken);
      printStmt->printNewline = true;
    }
    
    do {
      const fir::Expr::Ptr arg = parseExpr();
      printStmt->args.push_back(arg);
    } while (tryconsume(Token::Type::COMMA));
   
    const Token endToken = consume(Token::Type::SEMICOL);
    printStmt->setEndLoc(endToken);

    return printStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL}); 
    consume(Token::Type::SEMICOL);

    return fir::PrintStmt::Ptr();
  }
}

// apply_stmt: apply ident ['<' endpoints '>'] ['(' [expr_params] ')'] 
//             'to' set_index_set ';'
fir::ApplyStmt::Ptr Parser::parseApplyStmt() {
  try {
    auto applyStmt = std::make_shared<fir::ApplyStmt>();
    applyStmt->map = std::make_shared<fir::UnreducedMapExpr>();

    const Token applyToken = consume(Token::Type::APPLY);
    applyStmt->map->setBeginLoc(applyToken);
 
    applyStmt->map->func = parseIdent();
  
    if (tryconsume(Token::Type::LA)) {
      applyStmt->map->genericArgs = parseIndexSets();
      consume(Token::Type::RA);
    }

    if (tryconsume(Token::Type::LP)) {
      if (!tryconsume(Token::Type::RP)) {
        applyStmt->map->partialActuals = parseExprParams();
        consume(Token::Type::RP);
      }
    }
    
    consume(Token::Type::TO);
    applyStmt->map->target = parseSetIndexSet();
    
    const Token endToken = consume(Token::Type::SEMICOL);
    applyStmt->setEndLoc(endToken);

    return applyStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL}); 
    consume(Token::Type::SEMICOL);

    return fir::ApplyStmt::Ptr();
  }
}

// expr_or_assign_stmt: [[expr {',' expr} '='] expr] ';'
fir::ExprStmt::Ptr Parser::parseExprOrAssignStmt() {
  try {
    fir::ExprStmt::Ptr stmt;

    if (peek().type != Token::Type::SEMICOL) {
      const fir::Expr::Ptr expr = parseExpr();
      
      switch (peek().type) {
        case Token::Type::COMMA:
        case Token::Type::ASSIGN:
        {
          auto assignStmt = std::make_shared<fir::AssignStmt>();
          
          assignStmt->lhs.push_back(expr);
          while (tryconsume(Token::Type::COMMA)) {
            const fir::Expr::Ptr expr = parseExpr();
            assignStmt->lhs.push_back(expr);
          }
          
          consume(Token::Type::ASSIGN);
          assignStmt->expr = parseExpr();
          
          for (const auto lhs : assignStmt->lhs) {
            if (fir::isa<fir::VarExpr>(lhs)) {
              const std::string varName = fir::to<fir::VarExpr>(lhs)->ident;
              
              if (!decls.contains(varName)) {
                decls.insert(varName, IdentType::OTHER);
              }
            }
          }

          stmt = assignStmt;
          break;
        }
        default:
          stmt = std::make_shared<fir::ExprStmt>();
          stmt->expr = expr;
          break;
      }
    }
    
    const Token endToken = consume(Token::Type::SEMICOL);
    if (stmt) {
      stmt->setEndLoc(endToken);
    }

    return stmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL});
    consume(Token::Type::SEMICOL);

    return fir::ExprStmt::Ptr();
  }
}

// expr: map_expr | or_expr
fir::Expr::Ptr Parser::parseExpr() {
  return (peek().type == Token::Type::MAP) ? parseMapExpr() : parseOrExpr();
}

// map_expr: 'map' ident ['<' endpoints '>'] ['(' [expr_params] ')'] 
//           'to' set_index_set ['through' set_index_set] ['reduce' '+']
fir::MapExpr::Ptr Parser::parseMapExpr() {
  const Token mapToken = consume(Token::Type::MAP);
  const fir::Identifier::Ptr func = parseIdent();
 
  std::vector<fir::IndexSet::Ptr> genericArgs;
  if (tryconsume(Token::Type::LA)) {
    genericArgs = parseIndexSets();
    consume(Token::Type::RA);
  }
    
  std::vector<fir::Expr::Ptr> partialActuals;
  if (tryconsume(Token::Type::LP)) {
    if (!tryconsume(Token::Type::RP)) {
      partialActuals = parseExprParams();
      consume(Token::Type::RP);
    }
  }
  
  consume(Token::Type::TO);
  const fir::SetIndexSet::Ptr target = parseSetIndexSet();

  fir::SetIndexSet::Ptr through;

  if (peek().type == Token::Type::THROUGH) {
    consume(Token::Type::THROUGH);
    through = parseSetIndexSet();
  }
 
  if (peek().type == Token::Type::REDUCE) {
    consume(Token::Type::REDUCE);
    
    const auto mapExpr = std::make_shared<fir::ReducedMapExpr>();
    mapExpr->setBeginLoc(mapToken);
    
    mapExpr->func = func;
    mapExpr->genericArgs = genericArgs;
    mapExpr->partialActuals = partialActuals;
    mapExpr->target = target;
    mapExpr->through = through;
    mapExpr->op = fir::MapExpr::ReductionOp::SUM;

    const Token plusToken = consume(Token::Type::PLUS);
    mapExpr->setEndLoc(plusToken);

    return mapExpr;
  }
  
  const auto mapExpr = std::make_shared<fir::UnreducedMapExpr>();
  mapExpr->setBeginLoc(mapToken);

  mapExpr->func = func;
  mapExpr->genericArgs = genericArgs;
  mapExpr->partialActuals = partialActuals;
  mapExpr->target = target;
  mapExpr->through = through;

  return mapExpr;
}

// or_expr: and_expr {'or' and_expr}
fir::Expr::Ptr Parser::parseOrExpr() {
  fir::Expr::Ptr expr = parseAndExpr(); 

  while (tryconsume(Token::Type::OR)) {
    auto orExpr = std::make_shared<fir::OrExpr>();
    
    orExpr->lhs = expr;
    orExpr->rhs = parseAndExpr();
    
    expr = orExpr;
  }

  return expr;
}

// and_expr: xor_expr {'and' xor_expr}
fir::Expr::Ptr Parser::parseAndExpr() {
  fir::Expr::Ptr expr = parseXorExpr(); 

  while (tryconsume(Token::Type::AND)) {
    auto andExpr = std::make_shared<fir::AndExpr>();
    
    andExpr->lhs = expr;
    andExpr->rhs = parseXorExpr();
    
    expr = andExpr;
  }

  return expr;
}

// xor_expr: eq_expr {'xor' eq_expr}
fir::Expr::Ptr Parser::parseXorExpr() {
  fir::Expr::Ptr expr = parseEqExpr(); 

  while (tryconsume(Token::Type::XOR)) {
    auto xorExpr = std::make_shared<fir::XorExpr>();
    
    xorExpr->lhs = expr;
    xorExpr->rhs = parseEqExpr();
    
    expr = xorExpr;
  }

  return expr;
}

// eq_expr: term {('==' | '!=' | '>' | '<' | '>=' | '<=') term}
fir::Expr::Ptr Parser::parseEqExpr() {
  auto expr = std::make_shared<fir::EqExpr>();

  const fir::Expr::Ptr operand = parseTerm();
  expr->operands.push_back(operand);
  
  while (true) {
    switch (peek().type) {
      case Token::Type::EQ:
        consume(Token::Type::EQ);
        expr->ops.push_back(fir::EqExpr::Op::EQ);
        break;
      case Token::Type::NE:
        consume(Token::Type::NE);
        expr->ops.push_back(fir::EqExpr::Op::NE);
        break;
      case Token::Type::RA:
        consume(Token::Type::RA);
        expr->ops.push_back(fir::EqExpr::Op::GT);
        break;
      case Token::Type::LA:
        consume(Token::Type::LA);
        expr->ops.push_back(fir::EqExpr::Op::LT);
        break;
      case Token::Type::GE:
        consume(Token::Type::GE);
        expr->ops.push_back(fir::EqExpr::Op::GE);
        break;
      case Token::Type::LE:
        consume(Token::Type::LE);
        expr->ops.push_back(fir::EqExpr::Op::LE);
        break;
      default:
        return (expr->operands.size() == 1) ? expr->operands[0] : expr;
    }
   
    const fir::Expr::Ptr operand = parseTerm();
    expr->operands.push_back(operand);
  }
}

// term: ('not' term) | add_expr
fir::Expr::Ptr Parser::parseTerm() {
  if (peek().type == Token::Type::NOT) {
    auto notExpr = std::make_shared<fir::NotExpr>();

    const Token notToken = consume(Token::Type::NOT);
    notExpr->setBeginLoc(notToken);
    notExpr->operand = parseTerm();

    return notExpr;
  }

  return parseAddExpr();
}

// add_expr : mul_expr {('+' | '-') mul_expr}
fir::Expr::Ptr Parser::parseAddExpr() {
  fir::Expr::Ptr expr = parseMulExpr();
  
  while (true) {
    fir::BinaryExpr::Ptr addExpr;
    switch (peek().type) {
      case Token::Type::PLUS:
        consume(Token::Type::PLUS);
        addExpr = std::make_shared<fir::AddExpr>();
        break;
      case Token::Type::MINUS:
        consume(Token::Type::MINUS);
        addExpr = std::make_shared<fir::SubExpr>();
        break;
      default:
        return expr;
    }
    
    addExpr->lhs = expr;
    addExpr->rhs = parseMulExpr();
    
    expr = addExpr;
  }
}

// mul_expr: neg_expr {('*' | '/' | '\' | '.*' | './') neg_expr}
fir::Expr::Ptr Parser::parseMulExpr() {
  fir::Expr::Ptr expr = parseNegExpr();
  
  while (true) {
    fir::BinaryExpr::Ptr mulExpr;
    switch (peek().type) {
      case Token::Type::STAR:
        consume(Token::Type::STAR);
        mulExpr = std::make_shared<fir::MulExpr>();
        break;
      case Token::Type::SLASH:
        consume(Token::Type::SLASH);
        mulExpr = std::make_shared<fir::DivExpr>();
        break;
      case Token::Type::BACKSLASH:
        consume(Token::Type::BACKSLASH);
        mulExpr = std::make_shared<fir::LeftDivExpr>();
        break;
      case Token::Type::DOTSTAR:
        consume(Token::Type::DOTSTAR);
        mulExpr = std::make_shared<fir::ElwiseMulExpr>();
        break;
      case Token::Type::DOTSLASH:
        consume(Token::Type::DOTSLASH);
        mulExpr = std::make_shared<fir::ElwiseDivExpr>();
        break;
      default:
        return expr;
    }
        
    mulExpr->lhs = expr;
    mulExpr->rhs = parseNegExpr();
    
    expr = mulExpr;
  }
}

// neg_expr: (('+' | '-') neg_expr) | exp_expr
fir::Expr::Ptr Parser::parseNegExpr() {
  auto negExpr = std::make_shared<fir::NegExpr>();

  switch (peek().type) {
    case Token::Type::MINUS:
    {
      const Token minusToken = consume(Token::Type::MINUS);
      negExpr->setBeginLoc(minusToken);
      negExpr->negate = true;
      break;
    }
    case Token::Type::PLUS:
    {
      const Token plusToken = consume(Token::Type::PLUS);
      negExpr->setBeginLoc(plusToken);
      negExpr->negate = false;
      break;
    }
    default:
      return parseExpExpr();
  }
  negExpr->operand = parseNegExpr();
  
  return negExpr;
}

// exp_expr: transpose_expr ['^' exp_expr]
fir::Expr::Ptr Parser::parseExpExpr() {
  fir::Expr::Ptr expr = parseTransposeExpr();

  if (tryconsume(Token::Type::EXP)) {
    auto expExpr = std::make_shared<fir::ExpExpr>();
    
    expExpr->lhs = expr;
    expExpr->rhs = parseExpExpr();
    
    expr = expExpr;
  }

  return expr;
}

// transpose_expr: call_or_read_expr {'''}
fir::Expr::Ptr Parser::parseTransposeExpr() {
  fir::Expr::Ptr expr = parseCallOrReadExpr();

  while (peek().type == Token::Type::TRANSPOSE) {
    auto transposeExpr = std::make_shared<fir::TransposeExpr>();
    
    const Token transposeToken = consume(Token::Type::TRANSPOSE);
    transposeExpr->setEndLoc(transposeToken);
    transposeExpr->operand = expr;

    expr = transposeExpr; 
  }

  return expr;
}

// call_or_read_expr: factor {('(' [read_params] ')') | ('[' [expr_params] ']')
//                            | ('.' ident)}
fir::Expr::Ptr Parser::parseCallOrReadExpr() {
  fir::Expr::Ptr expr = parseFactor();

  while (true) {
    switch (peek().type) {
      case Token::Type::LP:
      {
        auto tensorRead = std::make_shared<fir::TensorReadExpr>();
        tensorRead->tensor = expr;
        
        consume(Token::Type::LP);
        if (peek().type != Token::Type::RP) {
          tensorRead->indices = parseReadParams();
        }
        
        const Token rightParenToken = consume(Token::Type::RP);
        tensorRead->setEndLoc(rightParenToken);

        expr = tensorRead;
        break;
      }
      case Token::Type::LB:
      {
        auto setRead = std::make_shared<fir::SetReadExpr>();
        setRead->set = expr;

        consume(Token::Type::LB);
        if (peek().type != Token::Type::RB) {
          setRead->indices = parseExprParams();
        }
        if (tryconsume(Token::Type::SEMICOL)) {
          auto sink = parseExprParams();
          std::copy(sink.begin(), sink.end(),
                    std::back_inserter(setRead->indices));
        }

        const Token rightBracketToken = consume(Token::Type::RB);
        setRead->setEndLoc(rightBracketToken);

        expr = setRead;
        break;
      }
      case Token::Type::PERIOD:
      {
        auto fieldRead = std::make_shared<fir::FieldReadExpr>();
        fieldRead->setOrElem = expr;
        
        consume(Token::Type::PERIOD);
        fieldRead->field = parseIdent();
        
        expr = fieldRead;
        break;
      }
      default:
        return expr;
    }
  }
}

// factor: ('(' expr ')') | call_expr | tuple_read_expr | range_const 
//       | var_expr | tensor_literal
fir::Expr::Ptr Parser::parseFactor() {
  switch (peek().type) {
    case Token::Type::LP:
    {
      auto parenExpr = std::make_shared<fir::ParenExpr>();

      const Token leftParenToken = consume(Token::Type::LP);
      parenExpr->setBeginLoc(leftParenToken);
      
      parenExpr->expr = parseExpr();
      
      const Token rightParenToken = consume(Token::Type::RP);
      parenExpr->setEndLoc(rightParenToken);

      return parenExpr;
    }
    case Token::Type::IDENT:
    {
      const std::string identStr = peek().str;

      if (decls.contains(identStr)) {
        switch (decls.get(identStr)) {
          case IdentType::FUNCTION:
            return parseCallExpr();
          case IdentType::RANGE_GENERIC_PARAM:
            return parseRangeConst();
          case IdentType::TUPLE:
            if (peek(1).type == Token::Type::LP) {
              return parseTupleReadExpr();
            }
            break;
          default:
            break;
        }
      }

      return parseVarExpr();
    }
    case Token::Type::INT_LITERAL:
    case Token::Type::FLOAT_LITERAL:
    case Token::Type::STRING_LITERAL:
    case Token::Type::TRUE:
    case Token::Type::FALSE:
    case Token::Type::LB:
    case Token::Type::LA:
      return parseTensorLiteral();
    default:
      reportError(peek(), "an expression");
      throw SyntaxError();
      break;
  }

  return fir::Expr::Ptr();
}

// var_expr: ident
fir::VarExpr::Ptr Parser::parseVarExpr() {
  auto varExpr = std::make_shared<fir::VarExpr>(); 
  
  const Token identToken = consume(Token::Type::IDENT);
  varExpr->setLoc(identToken);
  varExpr->ident = identToken.str;
 
  return varExpr;
}

// range_const: ident
fir::RangeConst::Ptr Parser::parseRangeConst() {
  auto rangeConst = std::make_shared<fir::RangeConst>(); 
  
  const Token identToken = consume(Token::Type::IDENT);
  rangeConst->setLoc(identToken);
  rangeConst->ident = identToken.str;
 
  return rangeConst;
}

// call_expr: ident ['<' endpoints '>'] '(' [expr_params] ')'
fir::CallExpr::Ptr Parser::parseCallExpr() {
  auto call = std::make_shared<fir::CallExpr>();
  
  call->func = parseIdent();

  if (tryconsume(Token::Type::LA)) {
    call->genericArgs = parseIndexSets();
    consume(Token::Type::RA);
  }

  consume(Token::Type::LP);

  if (peek().type != Token::Type::RP) {
    call->args = parseExprParams();
  }
  
  const Token endToken = consume(Token::Type::RP);
  call->setEndLoc(endToken);
  
  return call;
}

// tuple_read_expr: var_expr '(' expr ')'
fir::TupleReadExpr::Ptr Parser::parseTupleReadExpr() {
  auto tupleRead = std::make_shared<fir::TupleReadExpr>();
  
  tupleRead->tuple = parseVarExpr();
  consume(Token::Type::LP);

  tupleRead->index = parseExpr();

  const Token endToken = consume(Token::Type::RP);
  tupleRead->setEndLoc(endToken);
  
  return tupleRead;
}

// ident: IDENT
fir::Identifier::Ptr Parser::parseIdent() {
  auto ident = std::make_shared<fir::Identifier>();
  
  const Token identToken = consume(Token::Type::IDENT);
  ident->setLoc(identToken);
  ident->ident = identToken.str;
  
  return ident;
}

// read_params: read_param {',' read_param}
std::vector<fir::ReadParam::Ptr> Parser::parseReadParams() {
  std::vector<fir::ReadParam::Ptr> readParams;

  do {
    const fir::ReadParam::Ptr param = parseReadParam();
    readParams.push_back(param);
  } while (tryconsume(Token::Type::COMMA));

  return readParams;
}

// read_param: ':' | expr
fir::ReadParam::Ptr Parser::parseReadParam() {
  if (peek().type == Token::Type::COL) {
    auto slice = std::make_shared<fir::Slice>();
    
    const Token colToken = consume(Token::Type::COL);
    slice->setLoc(colToken);
    
    return slice;
  }

  auto param = std::make_shared<fir::ExprParam>();
  param->expr = parseExpr();
  
  return param;
}

// expr_params: expr {',' expr}
std::vector<fir::Expr::Ptr> Parser::parseExprParams() {
  std::vector<fir::Expr::Ptr> exprParams;

  do {
    const fir::Expr::Ptr param = parseExpr();
    exprParams.push_back(param);
  } while (tryconsume(Token::Type::COMMA));

  return exprParams;
}

// type: element_type | unstructured_set_type | lattice_link_set_type
//     | tuple_type | tensor_type
fir::Type::Ptr Parser::parseType() {
  fir::Type::Ptr type;
  switch (peek().type) {
    case Token::Type::IDENT:
      type = parseElementType();
      break;
    case Token::Type::SET:
      type = parseUnstructuredSetType();
      break;
    case Token::Type::LATTICE:
      type = parseLatticeLinkSetType();
      break;
    case Token::Type::LP:
      type = parseTupleType();
      break;
    case Token::Type::INT:
    case Token::Type::FLOAT:
    case Token::Type::BOOL:
    case Token::Type::COMPLEX:
    case Token::Type::STRING:
    case Token::Type::TENSOR:
    case Token::Type::MATRIX:
    case Token::Type::VECTOR:
      type = parseTensorType();
      break;
    case Token::Type::OPAQUE:
      consume(Token::Type::OPAQUE);
      type = std::make_shared<fir::OpaqueType>();
      break;
    default:
      reportError(peek(), "a type identifier");
      throw SyntaxError();
      break;
  }

  return type;
}

// element_type: ident
fir::ElementType::Ptr Parser::parseElementType() {
  auto elementType = std::make_shared<fir::ElementType>();

  const Token typeToken = consume(Token::Type::IDENT);
  elementType->setLoc(typeToken);
  elementType->ident = typeToken.str;

  return elementType;
}

// unstructured_set_type: 'set' '{' element_type '}' ['(' endpoints ')']
fir::SetType::Ptr Parser::parseUnstructuredSetType() {
  auto setType = std::make_shared<fir::UnstructuredSetType>();

  const Token setToken = consume(Token::Type::SET);
  setType->setBeginLoc(setToken);

  consume(Token::Type::LC);
  setType->element = parseElementType();
  
  const Token rightCurlyToken = consume(Token::Type::RC);
  setType->setEndLoc(rightCurlyToken);
  
  if (tryconsume(Token::Type::LP)) {
    setType->endpoints = parseEndpoints();

    const Token rightParenToken = consume(Token::Type::RP);
    setType->setEndLoc(rightParenToken);
  }

  return setType;
}

// lattice_link_set_type: 'lattice' '[' INT_LITERAL ']' 
//                        '{' element_type '}' '(' IDENT ')'
fir::SetType::Ptr Parser::parseLatticeLinkSetType() {
  auto setType = std::make_shared<fir::LatticeLinkSetType>();

  const Token latticeToken = consume(Token::Type::LATTICE);
  setType->setBeginLoc(latticeToken);

  consume(Token::Type::LB);
  const Token dimsToken = consume(Token::Type::INT_LITERAL);
  setType->dimensions = dimsToken.num;
  consume(Token::Type::RB);

  consume(Token::Type::LC);
  setType->element = parseElementType();
  consume(Token::Type::RC);

  consume(Token::Type::LP);
  auto latticePointSet = std::make_shared<fir::Endpoint>();
  latticePointSet->set = parseSetIndexSet();
  setType->latticePointSet = latticePointSet;
  
  const Token endToken = consume(Token::Type::RP);
  setType->setEndLoc(endToken);

  return setType;
}

// endpoints: set_index_set {',' set_index_set}
std::vector<fir::Endpoint::Ptr> Parser::parseEndpoints() {
  std::vector<fir::Endpoint::Ptr> endpoints;
  
  do {
    auto endpoint = std::make_shared<fir::Endpoint>();
    endpoint->set = parseSetIndexSet();
    endpoints.push_back(endpoint);
  } while (tryconsume(Token::Type::COMMA));

  return endpoints;
}

// tuple_length: INT_LITERAL
fir::TupleLength::Ptr Parser::parseTupleLength() {
  auto tupleLength = std::make_shared<fir::TupleLength>();

  const Token intToken = consume(Token::Type::INT_LITERAL);
  tupleLength->setLoc(intToken);
  tupleLength->val = intToken.num;

  return tupleLength;
}

// tuple_type: '(' element_type '*' tuple_length ')'
fir::TupleType::Ptr Parser::parseTupleType() {
  auto tupleType = std::make_shared<fir::TupleType>();

  const Token leftParenToken = consume(Token::Type::LP);
  tupleType->setBeginLoc(leftParenToken);
  
  tupleType->element = parseElementType();
  consume(Token::Type::STAR);
  tupleType->length = parseTupleLength();
  
  const Token rightParenToken = consume(Token::Type::RP);
  tupleType->setEndLoc(rightParenToken);
  
  return tupleType;
}

// tensor_type: scalar_type
//            | matrix_block_type
//            | (vector_block_type | tensor_block_type) [''']
fir::TensorType::Ptr Parser::parseTensorType() {
  fir::NDTensorType::Ptr tensorType;
  switch (peek().type) {
    case Token::Type::INT:
    case Token::Type::FLOAT:
    case Token::Type::BOOL:
    case Token::Type::COMPLEX:
    case Token::Type::STRING:
      return parseScalarType();
    case Token::Type::MATRIX:
      return parseMatrixBlockType();
    case Token::Type::VECTOR:
      tensorType = parseVectorBlockType();
      break;
    default:
      tensorType = parseTensorBlockType();
      break;
  }
  
  if (peek().type == Token::Type::TRANSPOSE) {
    const Token transposeToken = consume(Token::Type::TRANSPOSE);
    tensorType->setEndLoc(transposeToken);
    tensorType->transposed = true;
  }

  return tensorType;
}

// vector_block_type:
//     'vector' ['[' index_set ']'] 
//     '(' (vector_block_type | tensor_component_type) ')'
fir::NDTensorType::Ptr Parser::parseVectorBlockType() {
  auto tensorType = std::make_shared<fir::NDTensorType>();
  tensorType->transposed = false;

  const Token tensorToken = consume(Token::Type::VECTOR);
  tensorType->setBeginLoc(tensorToken);

  if (tryconsume(Token::Type::LB)) {
    const fir::IndexSet::Ptr indexSet = parseIndexSet();
    tensorType->indexSets.push_back(indexSet);
    consume(Token::Type::RB);
  }

  consume(Token::Type::LP);
  if (peek().type == Token::Type::VECTOR) {
    tensorType->blockType = parseVectorBlockType();
  } else {
    tensorType->blockType = parseScalarType();
  }
      
  const Token rightParenToken = consume(Token::Type::RP);
  tensorType->setEndLoc(rightParenToken);

  return tensorType;
}

// matrix_block_type:
//     'matrix' ['[' index_set ',' index_set ']'] 
//     '(' (matrix_block_type | tensor_component_type) ')'
fir::NDTensorType::Ptr Parser::parseMatrixBlockType() {
  auto tensorType = std::make_shared<fir::NDTensorType>();
  tensorType->transposed = false;

  const Token tensorToken = consume(Token::Type::MATRIX);
  tensorType->setBeginLoc(tensorToken);

  if (tryconsume(Token::Type::LB)) {
    fir::IndexSet::Ptr indexSet = parseIndexSet();
    tensorType->indexSets.push_back(indexSet);
    consume(Token::Type::COMMA);

    indexSet = parseIndexSet();
    tensorType->indexSets.push_back(indexSet);
    consume(Token::Type::RB);
  }

  consume(Token::Type::LP);
  if (peek().type == Token::Type::MATRIX) {
    tensorType->blockType = parseMatrixBlockType();
  } else {
    tensorType->blockType = parseScalarType();
  }

  const Token rightParenToken = consume(Token::Type::RP);
  tensorType->setEndLoc(rightParenToken);

  return tensorType;
}

// tensor_block_type:
//     'tensor' ['[' index_sets ']'] 
//     '(' (tensor_block_type | tensor_component_type) ')'
fir::NDTensorType::Ptr Parser::parseTensorBlockType() {
  auto tensorType = std::make_shared<fir::NDTensorType>();
  tensorType->transposed = false;

  const Token tensorToken = consume(Token::Type::TENSOR);
  tensorType->setBeginLoc(tensorToken);

  if (tryconsume(Token::Type::LB)) {
    tensorType->indexSets = parseIndexSets();
    consume(Token::Type::RB);
  }

  consume(Token::Type::LP);
  if (peek().type == Token::Type::TENSOR) {
    tensorType->blockType = parseTensorBlockType();
  } else {
    tensorType->blockType = parseScalarType();
  }
 
  const Token rightParenToken = consume(Token::Type::RP);
  tensorType->setEndLoc(rightParenToken);

  return tensorType;
}

// tensor_component_type: 'int' | 'float' | 'bool' | 'complex'
fir::ScalarType::Ptr Parser::parseTensorComponentType() {
  auto scalarType = std::make_shared<fir::ScalarType>();

  const Token typeToken = peek();
  scalarType->setLoc(typeToken);

  switch (peek().type) {
    case Token::Type::INT:
      consume(Token::Type::INT);
      scalarType->type = fir::ScalarType::Type::INT;
      break;
    case Token::Type::FLOAT:
      consume(Token::Type::FLOAT);
      scalarType->type = fir::ScalarType::Type::FLOAT;
      break;
    case Token::Type::BOOL:
      consume(Token::Type::BOOL);
      scalarType->type = fir::ScalarType::Type::BOOL;
      break;
    case Token::Type::COMPLEX:
      consume(Token::Type::COMPLEX);
      scalarType->type = fir::ScalarType::Type::COMPLEX;
      break;
    case Token::Type::STRING:
      // TODO: Implement.
    default:
      reportError(peek(), "a tensor component type identifier");
      throw SyntaxError();
      break;
  }

  return scalarType;
}

// scalar_type: 'string' | tensor_component_type
fir::ScalarType::Ptr Parser::parseScalarType() {
  if (peek().type == Token::Type::STRING) {
    auto stringType = std::make_shared<fir::ScalarType>();
    
    const Token stringToken = consume(Token::Type::STRING);
    stringType->setLoc(stringToken);
    stringType->type = fir::ScalarType::Type::STRING;
    
    return stringType;
  }

  return parseTensorComponentType();
}

// index_sets: index_set {',' index_set}
std::vector<fir::IndexSet::Ptr> Parser::parseIndexSets() {
  std::vector<fir::IndexSet::Ptr> indexSets;

  do {
    const fir::IndexSet::Ptr indexSet = parseIndexSet();
    indexSets.push_back(indexSet);
  } while (tryconsume(Token::Type::COMMA));

  return indexSets;
}

// index_set: INT_LITERAL | set_index_set
fir::IndexSet::Ptr Parser::parseIndexSet() {
  fir::IndexSet::Ptr indexSet;
  switch (peek().type) {
    case Token::Type::INT_LITERAL:
    {
      auto rangeIndexSet = std::make_shared<fir::RangeIndexSet>();
      
      const Token intToken = consume(Token::Type::INT_LITERAL);
      rangeIndexSet->setLoc(intToken);
      rangeIndexSet->range = intToken.num;
      
      indexSet = rangeIndexSet;
      break;
    }
    case Token::Type::IDENT:
      indexSet = parseSetIndexSet();
      break;
    default:
      reportError(peek(), "an index set");
      throw SyntaxError();
      break;
  }

  return indexSet;
}

// set_index_set: ident
fir::SetIndexSet::Ptr Parser::parseSetIndexSet() {
  const Token identToken = consume(Token::Type::IDENT);
  const std::string identStr = identToken.str;

  fir::SetIndexSet::Ptr setIndexSet;
  
  if (decls.contains(identStr)) {
    switch (decls.get(identStr)) {
      case IdentType::GENERIC_PARAM:
      {
        auto genericIndexSet = std::make_shared<fir::GenericIndexSet>();
        genericIndexSet->type = fir::GenericIndexSet::Type::UNKNOWN;

        setIndexSet = genericIndexSet;
        break;
      }
      case IdentType::RANGE_GENERIC_PARAM:
      {
        auto genericIndexSet = std::make_shared<fir::GenericIndexSet>();
        genericIndexSet->type = fir::GenericIndexSet::Type::RANGE;

        setIndexSet = genericIndexSet;
        break;
      }
      default:
        break;
    }
  }

  if (!setIndexSet) {
    setIndexSet = std::make_shared<fir::SetIndexSet>();
  }
  
  setIndexSet->setLoc(identToken);
  setIndexSet->setName = identStr;

  return setIndexSet;
}

// tensor_literal: INT_LITERAL | FLOAT_LITERAL | 'true' | 'false'
//               | STRING_LITERAL | complex_literal | dense_tensor_literal
fir::Expr::Ptr Parser::parseTensorLiteral() {
  fir::Expr::Ptr literal;
  switch (peek().type) {
    case Token::Type::INT_LITERAL:
    {
      auto intLiteral = std::make_shared<fir::IntLiteral>();
      
      const Token intToken = consume(Token::Type::INT_LITERAL);
      intLiteral->setLoc(intToken);
      intLiteral->val = intToken.num;
      
      literal = intLiteral;
      break;
    }
    case Token::Type::FLOAT_LITERAL:
    {
      auto floatLiteral = std::make_shared<fir::FloatLiteral>();
      
      const Token floatToken = consume(Token::Type::FLOAT_LITERAL);
      floatLiteral->setLoc(floatToken);
      floatLiteral->val = floatToken.fnum;
      
      literal = floatLiteral;
      break;
    }
    case Token::Type::TRUE:
    {
      auto trueLiteral = std::make_shared<fir::BoolLiteral>();
      
      const Token trueToken = consume(Token::Type::TRUE);
      trueLiteral->setLoc(trueToken);
      trueLiteral->val = true;
      
      literal = trueLiteral;
      break;
    }
    case Token::Type::FALSE:
    {
      auto falseLiteral = std::make_shared<fir::BoolLiteral>();
      
      const Token falseToken = consume(Token::Type::FALSE);
      falseLiteral->setLoc(falseToken);
      falseLiteral->val = false;
      
      literal = falseLiteral;
      break;
    }
    case Token::Type::STRING_LITERAL:
    {
      auto stringLiteral = std::make_shared<fir::StringLiteral>();

      const Token stringToken = consume(Token::Type::STRING_LITERAL);
      stringLiteral->setLoc(stringToken);
      stringLiteral->val = stringToken.str;

      literal = stringLiteral;
      break;
    }
    case Token::Type::LA:
    {
      const Token laToken = peek();
      
      auto complexLiteral = std::make_shared<fir::ComplexLiteral>();
      double_complex complexVal = parseComplexLiteral();
      complexLiteral->setLoc(laToken);
      complexLiteral->val = complexVal;

      literal = complexLiteral;
      break;
    }
    case Token::Type::LB:
      literal = parseDenseTensorLiteral();
      break;
    default:
      reportError(peek(), "a tensor literal");
      throw SyntaxError();
      break;
  }

  return literal;
}

// dense_tensor_literal: '[' dense_tensor_literal_inner ']'
fir::DenseTensorLiteral::Ptr Parser::parseDenseTensorLiteral() {
  const Token leftBracketToken = consume(Token::Type::LB);
  fir::DenseTensorLiteral::Ptr tensor = parseDenseTensorLiteralInner();
  const Token rightBracketToken = consume(Token::Type::RB);
  
  tensor->setBeginLoc(leftBracketToken);
  tensor->setEndLoc(rightBracketToken);

  return tensor;
}

// dense_tensor_literal_inner: dense_tensor_literal {[','] dense_tensor_literal}
//                           | dense_matrix_literal
fir::DenseTensorLiteral::Ptr Parser::parseDenseTensorLiteralInner() {
  if (peek().type == Token::Type::LB) {
    auto tensor = std::make_shared<fir::NDTensorLiteral>();
    tensor->transposed = false;

    fir::DenseTensorLiteral::Ptr elem = parseDenseTensorLiteral();
    tensor->elems.push_back(elem);
    
    while (true) {
      switch (peek().type) {
        case Token::Type::COMMA:
          consume(Token::Type::COMMA);
        case Token::Type::LB:
          elem = parseDenseTensorLiteral();
          tensor->elems.push_back(elem);
          break;
        default:
          return tensor;
      }
    }
  }

  return parseDenseMatrixLiteral();
}

// dense_matrix_literal: dense_vector_literal {';' dense_vector_literal}
fir::DenseTensorLiteral::Ptr Parser::parseDenseMatrixLiteral() {
  auto mat = std::make_shared<fir::NDTensorLiteral>();
  mat->transposed = false;
  
  do {
    const fir::DenseTensorLiteral::Ptr vec = parseDenseVectorLiteral();
    mat->elems.push_back(vec);
  } while (tryconsume(Token::Type::SEMICOL));

  return (mat->elems.size() == 1) ? mat->elems[0] : mat;
}

// dense_vector_literal: dense_int_vector_literal | dense_float_vector_literal
//                     | dense_complex_vector_literal
fir::DenseTensorLiteral::Ptr Parser::parseDenseVectorLiteral() {
  fir::DenseTensorLiteral::Ptr vec;
  switch (peek().type) {
    case Token::Type::INT_LITERAL:
      vec = parseDenseIntVectorLiteral();
      break;
    case Token::Type::FLOAT_LITERAL:
      vec = parseDenseFloatVectorLiteral();
      break;
    case Token::Type::PLUS:
    case Token::Type::MINUS:
      switch (peek(1).type) {
        case Token::Type::INT_LITERAL:
          vec = parseDenseIntVectorLiteral();
          break;
        case Token::Type::FLOAT_LITERAL:
          vec = parseDenseFloatVectorLiteral();
          break;
        default:
          reportError(peek(), "a vector literal");
          throw SyntaxError();
          break;
      }
      break;
    case Token::Type::LA:
      vec = parseDenseComplexVectorLiteral();
      break;
    default:
      reportError(peek(), "a vector literal");
      throw SyntaxError();
      break;
  }

  return vec;
}

// dense_int_vector_literal: signed_int_literal {[','] signed_int_literal}
fir::IntVectorLiteral::Ptr Parser::parseDenseIntVectorLiteral() {
  auto vec = std::make_shared<fir::IntVectorLiteral>();
  vec->transposed = false;

  int elem = parseSignedIntLiteral();
  vec->vals.push_back(elem);

  while (true) {
    switch (peek().type) {
      case Token::Type::COMMA:
        consume(Token::Type::COMMA);
      case Token::Type::PLUS:
      case Token::Type::MINUS:
      case Token::Type::INT_LITERAL:
        elem = parseSignedIntLiteral();
        vec->vals.push_back(elem);
        break;
      default:
        return vec;
    }
  }
}

// dense_float_vector_literal: signed_float_literal {[','] signed_float_literal}
fir::FloatVectorLiteral::Ptr Parser::parseDenseFloatVectorLiteral() {
  auto vec = std::make_shared<fir::FloatVectorLiteral>();
  vec->transposed = false;

  double elem = parseSignedFloatLiteral();
  vec->vals.push_back(elem);

  while (true) {
    switch (peek().type) {
      case Token::Type::COMMA:
        consume(Token::Type::COMMA);
      case Token::Type::PLUS:
      case Token::Type::MINUS:
      case Token::Type::FLOAT_LITERAL:
        elem = parseSignedFloatLiteral();
        vec->vals.push_back(elem);
        break;
      default:
        return vec;
    }
  }
}

// dense_complex_vector_literal: complex_literal {[','] complex_literal}
fir::ComplexVectorLiteral::Ptr Parser::parseDenseComplexVectorLiteral() {
  auto vec = std::make_shared<fir::ComplexVectorLiteral>();
  vec->transposed = false;

  double_complex elem = parseComplexLiteral();
  vec->vals.push_back(elem);

  while (true) {
    switch(peek().type) {
      case Token::Type::COMMA:
        consume(Token::Type::COMMA);
      case Token::Type::LA:
        elem = parseComplexLiteral();
        vec->vals.push_back(elem);
        break;
      default:
        return vec;
    }
  }
}

// signed_int_literal: ['+' | '-'] INT_LITERAL
int Parser::parseSignedIntLiteral() {
  int coeff = 1;
  switch (peek().type) {
    case Token::Type::PLUS:
      consume(Token::Type::PLUS);
      break;
    case Token::Type::MINUS:
      consume(Token::Type::MINUS);
      coeff = -1;
      break;
    default:
      break;
  }

  return (coeff * consume(Token::Type::INT_LITERAL).num);
}

// signed_float_literal: ['+' | '-'] FLOAT_LITERAL
double Parser::parseSignedFloatLiteral() {
  double coeff = 1.0;
  switch (peek().type) {
    case Token::Type::PLUS:
      consume(Token::Type::PLUS);
      break;
    case Token::Type::MINUS:
      consume(Token::Type::MINUS);
      coeff = -1.0;
      break;
    default:
      break;
  }

  return (coeff * consume(Token::Type::FLOAT_LITERAL).fnum);
}

// complex_literal: '<' signed_float_literal ',' signed_float_literal '>'
double_complex Parser::parseComplexLiteral() {
  consume(Token::Type::LA); 
  double real = parseSignedFloatLiteral();

  consume(Token::Type::COMMA);
  
  double imag = parseSignedFloatLiteral();
  consume(Token::Type::RA);
  
  return double_complex(real, imag);
}

// '%!' ident '(' [expr_params] ')' '==' expr ';'
fir::Test::Ptr Parser::parseTest() {
  auto test = std::make_shared<fir::Test>();

  const Token testToken = consume(Token::Type::TEST);
  test->setBeginLoc(testToken);

  test->func = parseIdent();
  switch (peek().type) {
    case Token::Type::LP:
    {
      consume(Token::Type::LP);

      if (!tryconsume(Token::Type::RP)) {
        test->args = parseExprParams();
        consume(Token::Type::RP);
      }

      consume(Token::Type::EQ);
      test->expected = parseExpr();
      
      const Token endToken = consume(Token::Type::SEMICOL);
      test->setEndLoc(endToken);
      break;
    }
    case Token::Type::ASSIGN:
      // TODO: Implement.
    default:
      reportError(peek(), "a test");
      throw SyntaxError();
      break;
  }

  return test;
}
  
void Parser::reportError(const Token &token, std::string expected) {
  std::stringstream errMsg;
  errMsg << "expected " << expected << " but got " << token.toString();

  const auto err = ParseError(token.lineBegin, token.colBegin, 
                              token.lineEnd, token.colEnd, errMsg.str());
  errors->push_back(err);
}
  
void Parser::skipTo(std::vector<Token::Type> types) {
  while (peek().type != Token::Type::END) {
    for (const auto type : types) {
      if (peek().type == type) {
        return;
      }
    }
    tokens.skip();
  }
}
  
Token Parser::consume(Token::Type type) { 
  const Token token = peek();
  
  if (!tokens.consume(type)) {
    reportError(token, Token::tokenTypeString(type));
    throw SyntaxError();
  }

  return token;
}

}
}

