#include <memory>
#include <string>

#include "parser.h"
#include "scanner.h"
#include "hir.h"

namespace simit { 
namespace internal {

// Simit language grammar is documented here in EBNF. Note that '{}' is used 
// here to denote zero or more instances of the enclosing term, while '[]' is 
// used to denote zero or one instance of the enclosing term.

hir::Program::Ptr Parser::parse(const TokenStream &tokens) {
  this->tokens = tokens;
  return parseProgram();
}

// program: {program_element}
hir::Program::Ptr Parser::parseProgram() {
  auto program = std::make_shared<hir::Program>();
  
  while (peek().type != Token::Type::END) {
    const hir::HIRNode::Ptr element = parseProgramElement();
    if (element) {
      program->elems.push_back(element);
    }
  }
  
  return program;
}

// program_element: element_type_decl | extern_decl | const_decl
//                | func_decl | proc_decl | test
hir::HIRNode::Ptr Parser::parseProgramElement() {
  try {
    switch (peek().type) {
      case Token::Type::TEST:
        return parseTest();
        break;
      case Token::Type::FUNC:
        return parseFuncDecl();
        break;
      case Token::Type::PROC:
      case Token::Type::EXPORT:
        return parseProcDecl();
        break;
      case Token::Type::ELEMENT:
        return parseElementTypeDecl();
        break;
      case Token::Type::EXTERN:
        return parseExternDecl();
        break;
      case Token::Type::CONST:
        return parseConstDecl();
        break;
      default:
        reportError(peek(), "a program element");
        throw SyntaxError();
        break;
    }
  } catch (const SyntaxError &) {
    skipTo({Token::Type::TEST, Token::Type::FUNC, Token::Type::PROC, 
            Token::Type::EXPORT, Token::Type::ELEMENT, Token::Type::EXTERN, 
            Token::Type::CONST});
    return hir::HIRNode::Ptr();
  }
}

// element_type_decl: 'element' ident field_decl_list 'end'
hir::ElementTypeDecl::Ptr Parser::parseElementTypeDecl() {
  auto elemTypeDecl = std::make_shared<hir::ElementTypeDecl>();
  
  const Token elementToken = consume(Token::Type::ELEMENT);
  elemTypeDecl->setBeginLoc(elementToken);
  
  elemTypeDecl->name = parseIdent(); 
  elemTypeDecl->fields = parseFieldDeclList();

  const Token endToken = consume(Token::Type::BLOCKEND);
  elemTypeDecl->setEndLoc(endToken);
  
  return elemTypeDecl;
}

// field_decl_list: {field_decl}
std::vector<hir::Field::Ptr> Parser::parseFieldDeclList() {
  std::vector<hir::Field::Ptr> fields;

  while (peek().type == Token::Type::IDENT) {
    const hir::Field::Ptr field = parseFieldDecl();
    fields.push_back(field);
  }

  return fields;
}

// field_decl: tensor_decl ';'
hir::Field::Ptr Parser::parseFieldDecl() {
  auto fieldDecl = std::make_shared<hir::Field>();
  
  fieldDecl->field = parseTensorDecl();
  
  const Token endToken = consume(Token::Type::SEMICOL);
  fieldDecl->setEndLoc(endToken);

  return fieldDecl;
}

// extern_decl: 'extern' argument_decl ';'
hir::ExternDecl::Ptr Parser::parseExternDecl() {
  auto externDecl = std::make_shared<hir::ExternDecl>();
  
  const Token externToken = consume(Token::Type::EXTERN);
  externDecl->setBeginLoc(externToken);
  
  externDecl->var = parseArgumentDecl();
  
  const Token endToken = consume(Token::Type::SEMICOL);
  externDecl->setEndLoc(endToken);
 
  return externDecl;
}

// func_decl: 'func' ident arguments results stmt_block 'end'
hir::FuncDecl::Ptr Parser::parseFuncDecl() {
  auto funcDecl = std::make_shared<hir::FuncDecl>();

  const Token funcToken = consume(Token::Type::FUNC);
  funcDecl->setBeginLoc(funcToken);

  funcDecl->name = parseIdent();
  funcDecl->args = parseArguments();
  funcDecl->results = parseResults();
  funcDecl->body = parseStmtBlock();
  
  const Token endToken = consume(Token::Type::BLOCKEND);
  funcDecl->setEndLoc(endToken);

  return funcDecl;
}

// proc_decl: 
//     ('proc' | ('export' 'func')) ident [arguments results] stmt_block 'end'
hir::ProcDecl::Ptr Parser::parseProcDecl() {
  auto procDecl = std::make_shared<hir::ProcDecl>();

  procDecl->setBeginLoc(peek());
  if (!tryconsume(Token::Type::PROC)) {
    consume(Token::Type::EXPORT);
    consume(Token::Type::FUNC);
  }

  procDecl->name = parseIdent();
  if (peek().type == Token::Type::LP) {
    switch (peek(1).type) {
      case Token::Type::IDENT:
        if (peek(2).type != Token::Type::COL) {
          break;
        }
      case Token::Type::INOUT:
        procDecl->args = parseArguments();
        procDecl->results = parseResults();
        break;
      default:
        break;
    }
  }
  procDecl->body = parseStmtBlock();
  
  const Token endToken = consume(Token::Type::BLOCKEND);
  procDecl->setEndLoc(endToken);

  return procDecl;
}

// arguments: '(' [argument_decl {',' argument_decl}] ')'
std::vector<hir::Argument::Ptr> Parser::parseArguments() {
  std::vector<hir::Argument::Ptr> arguments;
  
  consume(Token::Type::LP);
  if (peek().type != Token::Type::RP) {
    do {
      const hir::Argument::Ptr argument = parseArgumentDecl();
      arguments.push_back(argument);
    } while (tryconsume(Token::Type::COMMA));
  }
  consume(Token::Type::RP);

  return arguments;
}

// argument_decl: ['inout'] ident_decl
hir::Argument::Ptr Parser::parseArgumentDecl() {
  auto argDecl = std::make_shared<hir::Argument>();
  
  argDecl->inout = false;
  if (tryconsume(Token::Type::INOUT)) {
    argDecl->inout = true;
  }

  const hir::IdentDecl::Ptr var = parseIdentDecl();
  argDecl->name = var->name;
  argDecl->type = var->type;
  
  return argDecl;
}

// results: ['->' '(' ident_decl {',' ident_decl} ')']
std::vector<hir::IdentDecl::Ptr> Parser::parseResults() {
  std::vector<hir::IdentDecl::Ptr> results;

  if (tryconsume(Token::Type::RARROW)) {
    consume(Token::Type::LP);
    do {
      const hir::IdentDecl::Ptr result = parseIdentDecl();
      results.push_back(result);
    } while (tryconsume(Token::Type::COMMA));
    consume(Token::Type::RP);
  }

  return results;
}

// stmt_block: {stmt}
hir::StmtBlock::Ptr Parser::parseStmtBlock() {
  auto stmtBlock = std::make_shared<hir::StmtBlock>();

  while (true) {
    switch (peek().type) {
      case Token::Type::BLOCKEND:
      case Token::Type::ELIF:
      case Token::Type::ELSE:
      case Token::Type::END:
        return stmtBlock;
      default:
      {
        const hir::Stmt::Ptr stmt = parseStmt();
        if (stmt) {
          stmtBlock->stmts.push_back(stmt);
        }
        break;
      }
    }
  }
}

// stmt: var_decl | const_decl | if_stmt | while_stmt | do_while_stmt 
//     | for_stmt | print_stmt | expr_or_assign_stmt
hir::Stmt::Ptr Parser::parseStmt() {
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
      return parsePrintStmt();
    default:
      return parseExprOrAssignStmt();
  }
}

// var_decl: 'var' tensor_decl ['=' expr] ';'
hir::VarDecl::Ptr Parser::parseVarDecl() {
  try {
    auto varDecl = std::make_shared<hir::VarDecl>();
    
    const Token varToken = consume(Token::Type::VAR);
    varDecl->setBeginLoc(varToken);
    
    varDecl->var = parseTensorDecl();
    if (tryconsume(Token::Type::ASSIGN)) {
      varDecl->initVal = parseExpr();
    }
  
    const Token endToken = consume(Token::Type::SEMICOL);
    varDecl->setEndLoc(endToken);

    return varDecl;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL});
    
    consume(Token::Type::SEMICOL);
    return hir::VarDecl::Ptr();
  }
}

// const_decl: 'const' tensor_decl '=' expr ';'
hir::ConstDecl::Ptr Parser::parseConstDecl() {
  try {
    auto constDecl = std::make_shared<hir::ConstDecl>();
    
    const Token constToken = consume(Token::Type::CONST);
    constDecl->setBeginLoc(constToken);
    
    constDecl->var = parseTensorDecl();
    consume(Token::Type::ASSIGN);
    constDecl->initVal = parseExpr();
  
    const Token endToken = consume(Token::Type::SEMICOL);
    constDecl->setEndLoc(endToken);

    return constDecl;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL});
    
    consume(Token::Type::SEMICOL);
    return hir::ConstDecl::Ptr();
  }
}

// ident_decl: ident ':' type
hir::IdentDecl::Ptr Parser::parseIdentDecl() {
  auto identDecl = std::make_shared<hir::IdentDecl>();

  identDecl->name = parseIdent();
  consume(Token::Type::COL);
  identDecl->type = parseType();
  
  return identDecl;
}

// tensor_decl: ident ':' tensor_type
// This rule is needed to prohibit declaration of non-tensor variables and 
// fields, which are currently unsupported. Probably want to replace with 
// ident_decl rule at some point in the future.
hir::IdentDecl::Ptr Parser::parseTensorDecl() {
  auto tensorDecl = std::make_shared<hir::IdentDecl>();

  tensorDecl->name = parseIdent();
  consume(Token::Type::COL);
  tensorDecl->type = parseTensorType();
  
  return tensorDecl;
}

// while_stmt: 'while' expr stmt_block 'end'
hir::WhileStmt::Ptr Parser::parseWhileStmt() {
  try {
    auto whileStmt = std::make_shared<hir::WhileStmt>();
    
    const Token whileToken = consume(Token::Type::WHILE);
    whileStmt->setBeginLoc(whileToken);
    
    whileStmt->cond = parseExpr();
    whileStmt->body = parseStmtBlock();

    const Token endToken = consume(Token::Type::BLOCKEND);
    whileStmt->setEndLoc(endToken);

    return whileStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND});
    
    consume(Token::Type::BLOCKEND);
    return hir::WhileStmt::Ptr();
  }
}

// do_while_stmt: 'do' stmt_block 'end' 'while' expr
hir::DoWhileStmt::Ptr Parser::parseDoWhileStmt() {
  try {
    auto doWhileStmt = std::make_shared<hir::DoWhileStmt>();
    
    const Token doToken = consume(Token::Type::DO);
    doWhileStmt->setBeginLoc(doToken);
    
    doWhileStmt->body = parseStmtBlock();
    consume(Token::Type::BLOCKEND);
    consume(Token::Type::WHILE);
    doWhileStmt->cond = parseExpr();

    return doWhileStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND, Token::Type::ELIF, Token::Type::ELSE, 
            Token::Type::END, Token::Type::VAR, Token::Type::CONST, 
            Token::Type::IF, Token::Type::WHILE, Token::Type::DO, 
            Token::Type::FOR, Token::Type::PRINT});
    return hir::DoWhileStmt::Ptr();
  }
}

// if_stmt: 'if' expr stmt_block else_clause 'end'
hir::IfStmt::Ptr Parser::parseIfStmt() {
  try {
    auto ifStmt = std::make_shared<hir::IfStmt>();
    
    const Token ifToken = consume(Token::Type::IF);
    ifStmt->setBeginLoc(ifToken);
    
    ifStmt->cond = parseExpr();
    ifStmt->ifBody = parseStmtBlock();
    ifStmt->elseBody = parseElseClause();

    const Token endToken = consume(Token::Type::BLOCKEND);
    ifStmt->setEndLoc(endToken);

    return ifStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::ELIF, Token::Type::ELSE, Token::Type::BLOCKEND});
    
    parseElseClause();
    consume(Token::Type::BLOCKEND);
    
    return hir::IfStmt::Ptr();
  }
}

// else_clause: [('else' stmt_block) | ('elif' expr stmt_block else_clause)]
hir::Stmt::Ptr Parser::parseElseClause() {
  switch (peek().type) {
    case Token::Type::ELSE:
      consume(Token::Type::ELSE);
      return parseStmtBlock();
    case Token::Type::ELIF:
      try {
        auto elifClause = std::make_shared<hir::IfStmt>();
        
        const Token elifToken = consume(Token::Type::ELIF);
        elifClause->setBeginLoc(elifToken);
    
        elifClause->cond = parseExpr();
        elifClause->ifBody = parseStmtBlock();
        elifClause->elseBody = parseElseClause();

        return elifClause;
      } catch (const SyntaxError &) {
        skipTo({Token::Type::ELIF, Token::Type::ELSE, Token::Type::BLOCKEND});
        
        parseElseClause();
        return hir::Stmt::Ptr();
      }
    default:
      return hir::Stmt::Ptr();
  }
}

// for_stmt: 'for' ident 'in' for_domain stmt_block 'end'
hir::ForStmt::Ptr Parser::parseForStmt() {
  try {
    auto forStmt = std::make_shared<hir::ForStmt>();

    const Token forToken = consume(Token::Type::FOR);
    forStmt->setBeginLoc(forToken);
    
    forStmt->loopVar = parseIdent();
    consume(Token::Type::IN);
    forStmt->domain = parseForDomain();
    forStmt->body = parseStmtBlock();

    const Token endToken = consume(Token::Type::BLOCKEND);
    forStmt->setEndLoc(endToken);

    return forStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::BLOCKEND});
    
    consume(Token::Type::BLOCKEND);
    return hir::ForStmt::Ptr();
  }
}

// for_domain: ident | (expr ':' expr)
hir::ForDomain::Ptr Parser::parseForDomain() {
  if (peek().type == Token::Type::IDENT && peek(1).type != Token::Type::COL) {
    auto setIndexSet = std::make_shared<hir::SetIndexSet>();
    
    const Token identToken = consume(Token::Type::IDENT);
    setIndexSet->setLoc(identToken);
    setIndexSet->setName = identToken.str;
    
    auto indexSetDomain = std::make_shared<hir::IndexSetDomain>();
    indexSetDomain->set = setIndexSet;
    
    return indexSetDomain;
  }

  auto rangeDomain = std::make_shared<hir::RangeDomain>();

  rangeDomain->lower = parseExpr();
  consume(Token::Type::COL);
  rangeDomain->upper = parseExpr();
  
  return rangeDomain;
}

// print_stmt: 'print' expr ';'
hir::PrintStmt::Ptr Parser::parsePrintStmt() {
  try {
    auto printStmt = std::make_shared<hir::PrintStmt>();
    
    const Token printToken = consume(Token::Type::PRINT);
    printStmt->setBeginLoc(printToken);
    
    printStmt->expr = parseExpr();

    const Token endToken = consume(Token::Type::SEMICOL);
    printStmt->setEndLoc(endToken);

    return printStmt;
  } catch (const SyntaxError &) {
    skipTo({Token::Type::SEMICOL});
    
    consume(Token::Type::SEMICOL);
    return hir::PrintStmt::Ptr();
  }
}

// expr_or_assign_stmt: [[expr {',' expr} '='] expr] ';'
hir::ExprStmt::Ptr Parser::parseExprOrAssignStmt() {
  try {
    hir::ExprStmt::Ptr stmt;

    if (peek().type != Token::Type::SEMICOL) {
      const hir::Expr::Ptr expr = parseExpr();
      
      switch (peek().type) {
        case Token::Type::COMMA:
        case Token::Type::ASSIGN:
        {
          auto assignStmt = std::make_shared<hir::AssignStmt>();
          
          assignStmt->lhs.push_back(expr);
          while (tryconsume(Token::Type::COMMA)) {
            const hir::Expr::Ptr expr = parseExpr();
            assignStmt->lhs.push_back(expr);
          }
          consume(Token::Type::ASSIGN);
          assignStmt->expr = parseExpr();
          
          stmt = assignStmt;
          break;
        }
        default:
          stmt = std::make_shared<hir::ExprStmt>();
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
    return hir::ExprStmt::Ptr();
  }
}

// expr: map_expr | or_expr
hir::Expr::Ptr Parser::parseExpr() {
  return (peek().type == Token::Type::MAP) ? parseMapExpr() : parseOrExpr();
}

// map_expr: 'map' ident [call_params] 'to' ident ['reduce' '+']
hir::Expr::Ptr Parser::parseMapExpr() {
  auto mapExpr = std::make_shared<hir::MapExpr>();

  const Token mapToken = consume(Token::Type::MAP);
  mapExpr->setBeginLoc(mapToken);
 
  mapExpr->func = parseIdent();
  if (peek().type == Token::Type::LP) {
    mapExpr->partialActuals = parseCallParams();
  }
  
  consume(Token::Type::TO);
  mapExpr->target = parseIdent();
  
  if (peek().type == Token::Type::REDUCE) {
    consume(Token::Type::REDUCE);
    
    const Token plusToken = consume(Token::Type::PLUS);
    mapExpr->setEndLoc(plusToken);

    mapExpr->op = hir::MapExpr::ReductionOp::SUM;
  } else {
    mapExpr->op = hir::MapExpr::ReductionOp::NONE;
  }

  return mapExpr;
}

// or_expr: and_expr {'or' and_expr}
hir::Expr::Ptr Parser::parseOrExpr() {
  hir::Expr::Ptr expr = parseAndExpr(); 

  while (tryconsume(Token::Type::OR)) {
    auto orExpr = std::make_shared<hir::OrExpr>();
    
    orExpr->lhs = expr;
    orExpr->rhs = parseAndExpr();
    
    expr = orExpr;
  }

  return expr;
}

// and_expr: xor_expr {'and' xor_expr}
hir::Expr::Ptr Parser::parseAndExpr() {
  hir::Expr::Ptr expr = parseXorExpr(); 

  while (tryconsume(Token::Type::AND)) {
    auto andExpr = std::make_shared<hir::AndExpr>();
    
    andExpr->lhs = expr;
    andExpr->rhs = parseXorExpr();
    
    expr = andExpr;
  }

  return expr;
}

// xor_expr: eq_expr {'xor' eq_expr}
hir::Expr::Ptr Parser::parseXorExpr() {
  hir::Expr::Ptr expr = parseEqExpr(); 

  while (tryconsume(Token::Type::XOR)) {
    auto xorExpr = std::make_shared<hir::XorExpr>();
    
    xorExpr->lhs = expr;
    xorExpr->rhs = parseEqExpr();
    
    expr = xorExpr;
  }

  return expr;
}

// eq_expr: term {('==' | '!=' | '>' | '<' | '>=' | '<=') term}
hir::Expr::Ptr Parser::parseEqExpr() {
  auto expr = std::make_shared<hir::EqExpr>();

  const hir::Expr::Ptr operand = parseTerm();
  expr->operands.push_back(operand);
  
  while (true) {
    switch (peek().type) {
      case Token::Type::EQ:
        consume(Token::Type::EQ);
        expr->ops.push_back(hir::EqExpr::Op::EQ);
        break;
      case Token::Type::NE:
        consume(Token::Type::NE);
        expr->ops.push_back(hir::EqExpr::Op::NE);
        break;
      case Token::Type::RA:
        consume(Token::Type::RA);
        expr->ops.push_back(hir::EqExpr::Op::GT);
        break;
      case Token::Type::LA:
        consume(Token::Type::LA);
        expr->ops.push_back(hir::EqExpr::Op::LT);
        break;
      case Token::Type::GE:
        consume(Token::Type::GE);
        expr->ops.push_back(hir::EqExpr::Op::GE);
        break;
      case Token::Type::LE:
        consume(Token::Type::LE);
        expr->ops.push_back(hir::EqExpr::Op::LE);
        break;
      default:
        return (expr->operands.size() == 1) ? expr->operands[0] : expr;
    }
   
    const hir::Expr::Ptr operand = parseTerm();
    expr->operands.push_back(operand);
  }
}

// term: ('not' term) | add_expr
hir::Expr::Ptr Parser::parseTerm() {
  if (peek().type == Token::Type::NOT) {
    auto notExpr = std::make_shared<hir::NotExpr>();

    const Token notToken = consume(Token::Type::NOT);
    notExpr->setBeginLoc(notToken);
    notExpr->operand = parseTerm();

    return notExpr;
  }

  return parseAddExpr();
}

// add_expr : mul_expr {('+' | '-') mul_expr}
hir::Expr::Ptr Parser::parseAddExpr() {
  hir::Expr::Ptr expr = parseMulExpr();
  
  while (true) {
    hir::BinaryExpr::Ptr addExpr;
    switch (peek().type) {
      case Token::Type::PLUS:
        consume(Token::Type::PLUS);
        addExpr = std::make_shared<hir::AddExpr>();
        break;
      case Token::Type::MINUS:
        consume(Token::Type::MINUS);
        addExpr = std::make_shared<hir::SubExpr>();
        break;
      default:
        return expr;
    }
    
    addExpr->lhs = expr;
    addExpr->rhs = parseMulExpr();
    
    expr = addExpr;
  }
}

// mul_expr: neg_expr {('*' | '/' | '.*' | './') neg_expr}
hir::Expr::Ptr Parser::parseMulExpr() {
  hir::Expr::Ptr expr = parseNegExpr();
  
  while (true) {
    hir::BinaryExpr::Ptr mulExpr;
    switch (peek().type) {
      case Token::Type::STAR:
        consume(Token::Type::STAR);
        mulExpr = std::make_shared<hir::MulExpr>();
        break;
      case Token::Type::SLASH:
        consume(Token::Type::SLASH);
        mulExpr = std::make_shared<hir::DivExpr>();
        break;
      case Token::Type::DOTSTAR:
        consume(Token::Type::DOTSTAR);
        mulExpr = std::make_shared<hir::ElwiseMulExpr>();
        break;
      case Token::Type::DOTSLASH:
        consume(Token::Type::DOTSLASH);
        mulExpr = std::make_shared<hir::ElwiseDivExpr>();
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
hir::Expr::Ptr Parser::parseNegExpr() {
  auto negExpr = std::make_shared<hir::NegExpr>();

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
hir::Expr::Ptr Parser::parseExpExpr() {
  hir::Expr::Ptr expr = parseTransposeExpr();

  if (tryconsume(Token::Type::EXP)) {
    auto expExpr = std::make_shared<hir::ExpExpr>();
    
    expExpr->lhs = expr;
    expExpr->rhs = parseExpExpr();
    
    expr = expExpr;
  }

  return expr;
}

// transpose_expr: call_or_read_expr {'''}
hir::Expr::Ptr Parser::parseTransposeExpr() {
  hir::Expr::Ptr expr = parseCallOrReadExpr();

  while (peek().type == Token::Type::TRANSPOSE) {
    auto transposeExpr = std::make_shared<hir::TransposeExpr>();
    
    const Token transposeToken = consume(Token::Type::TRANSPOSE);
    transposeExpr->setEndLoc(transposeToken);
    transposeExpr->operand = expr;

    expr = transposeExpr; 
  }

  return expr;
}

// call_or_read_expr: factor {('(' [read_params] ')') | ('.' ident)}
hir::Expr::Ptr Parser::parseCallOrReadExpr() {
  hir::Expr::Ptr expr = parseFactor();

  while (true) {
    switch (peek().type) {
      case Token::Type::LP:
      {
        auto tensorRead = std::make_shared<hir::TensorReadExpr>();
        
        consume(Token::Type::LP);
        tensorRead->tensor = expr;
        if (peek().type != Token::Type::RP) {
          tensorRead->indices = parseReadParams();
        }
        
        const Token rightParenToken = consume(Token::Type::RP);
        tensorRead->setEndLoc(rightParenToken);

        expr = tensorRead;
        break;
      }
      case Token::Type::PERIOD:
      {
        auto fieldRead = std::make_shared<hir::FieldReadExpr>();
        
        consume(Token::Type::PERIOD);
        fieldRead->setOrElem = expr;
        fieldRead->field = parseIdent();
        
        expr = fieldRead;
        break;
      }
      default:
        return expr;
    }
  }
}

// factor: ('(' expr ')') | ident | tensor_literal
hir::Expr::Ptr Parser::parseFactor() {
  hir::Expr::Ptr factor;
  switch (peek().type) {
    case Token::Type::LP:
    {
      auto parenExpr = std::make_shared<hir::ParenExpr>();

      const Token leftParenToken = consume(Token::Type::LP);
      parenExpr->setBeginLoc(leftParenToken);
      
      parenExpr->expr = parseExpr();
      
      const Token rightParenToken = consume(Token::Type::RP);
      parenExpr->setEndLoc(rightParenToken);

      factor = parenExpr;
      break;
    }
    case Token::Type::IDENT:
    {
      auto var = std::make_shared<hir::VarExpr>(); 
      
      const Token identToken = consume(Token::Type::IDENT);
      var->setLoc(identToken);
      var->ident = identToken.str;
      
      factor = var;
      break;
    }
    case Token::Type::INT_LITERAL:
    case Token::Type::FLOAT_LITERAL:
    case Token::Type::STRING_LITERAL:
    case Token::Type::TRUE:
    case Token::Type::FALSE:
    case Token::Type::LB:
      factor = parseTensorLiteral();
      break;
    default:
      reportError(peek(), "an expression");
      throw SyntaxError();
      break;
  }

  return factor;
}

// ident: IDENT
hir::Identifier::Ptr Parser::parseIdent() {
  auto ident = std::make_shared<hir::Identifier>();
  
  const Token identToken = consume(Token::Type::IDENT);
  ident->setLoc(identToken);
  ident->ident = identToken.str;
  
  return ident;
}

// read_params: read_param {',' read_param}
std::vector<hir::ReadParam::Ptr> Parser::parseReadParams() {
  std::vector<hir::ReadParam::Ptr> readParams;

  do {
    const hir::ReadParam::Ptr param = parseReadParam();
    readParams.push_back(param);
  } while (tryconsume(Token::Type::COMMA));

  return readParams;
}

// read_param: ':' | expr
hir::ReadParam::Ptr Parser::parseReadParam() {
  if (peek().type == Token::Type::COL) {
    auto slice = std::make_shared<hir::Slice>();
    
    const Token colToken = consume(Token::Type::COL);
    slice->setLoc(colToken);
    
    return slice;
  }

  auto param = std::make_shared<hir::ExprParam>();
  param->expr = parseExpr();
  
  return param;
}

// call_params: '(' [expr {',' expr}] ')'
std::vector<hir::Expr::Ptr> Parser::parseCallParams() {
  std::vector<hir::Expr::Ptr> callParams;

  consume(Token::Type::LP);
  if (peek().type != Token::Type::RP) {
    do {
      const hir::Expr::Ptr param = parseExpr();
      callParams.push_back(param);
    } while (tryconsume(Token::Type::COMMA));
  }
  consume(Token::Type::RP);

  return callParams;
}

// type: element_type | set_type | tuple_type | tensor_type
hir::Type::Ptr Parser::parseType() {
  hir::Type::Ptr type;
  switch (peek().type) {
    case Token::Type::IDENT:
      type = parseElementType();
      break;
    case Token::Type::SET:
      type = parseSetType();
      break;
    case Token::Type::LP:
      type = parseTupleType();
      break;
    case Token::Type::INT:
    case Token::Type::FLOAT:
    case Token::Type::BOOL:
    case Token::Type::STRING:
    case Token::Type::TENSOR:
      type = parseTensorType();
      break;
    default:
      reportError(peek(), "a type identifier");
      throw SyntaxError();
      break;
  }

  return type;
}

// element_type: ident
hir::ElementType::Ptr Parser::parseElementType() {
  auto elementType = std::make_shared<hir::ElementType>();

  const Token typeToken = consume(Token::Type::IDENT);
  elementType->setLoc(typeToken);
  elementType->ident = typeToken.str;

  return elementType;
}

// set_type: 'set' '{' element_type '}' ['(' endpoints ')']
hir::SetType::Ptr Parser::parseSetType() {
  auto setType = std::make_shared<hir::SetType>();

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

// endpoints: ident {',' ident}
std::vector<hir::Endpoint::Ptr> Parser::parseEndpoints() {
  std::vector<hir::Endpoint::Ptr> endpoints;
  
  do {
    auto endpoint = std::make_shared<hir::Endpoint>();
    
    const Token endpointToken = consume(Token::Type::IDENT);
    endpoint->setLoc(endpointToken);
    endpoint->setName = endpointToken.str;
    
    endpoints.push_back(endpoint);
  } while (tryconsume(Token::Type::COMMA));

  return endpoints;
}

// tuple_length: INT_LITERAL
hir::TupleLength::Ptr Parser::parseTupleLength() {
  auto tupleLength = std::make_shared<hir::TupleLength>();

  const Token intToken = consume(Token::Type::INT_LITERAL);
  tupleLength->setLoc(intToken);
  tupleLength->val = intToken.num;

  return tupleLength;
}

// tuple_type: '(' element_type '*' tuple_length ')'
hir::TupleType::Ptr Parser::parseTupleType() {
  auto tupleType = std::make_shared<hir::TupleType>();

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
hir::TensorType::Ptr Parser::parseTensorType() {
  hir::NDTensorType::Ptr tensorType;
  switch (peek().type) {
    case Token::Type::INT:
    case Token::Type::FLOAT:
    case Token::Type::BOOL:
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
//     'vector' ['[' index_set ']'] '(' (vector_block_type | scalar_type) ')'
hir::NDTensorType::Ptr Parser::parseVectorBlockType() {
  auto tensorType = std::make_shared<hir::NDTensorType>();
  tensorType->transposed = false;

  const Token tensorToken = consume(Token::Type::VECTOR);
  tensorType->setBeginLoc(tensorToken);

  if (tryconsume(Token::Type::LB)) {
    const hir::IndexSet::Ptr indexSet = parseIndexSet();
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
//     '(' (matrix_block_type | scalar_type) ')'
hir::NDTensorType::Ptr Parser::parseMatrixBlockType() {
  auto tensorType = std::make_shared<hir::NDTensorType>();
  tensorType->transposed = false;

  const Token tensorToken = consume(Token::Type::MATRIX);
  tensorType->setBeginLoc(tensorToken);

  if (tryconsume(Token::Type::LB)) {
    hir::IndexSet::Ptr indexSet = parseIndexSet();
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
//     'tensor' ['[' index_sets ']'] '(' (tensor_block_type | scalar_type) ')'
hir::NDTensorType::Ptr Parser::parseTensorBlockType() {
  auto tensorType = std::make_shared<hir::NDTensorType>();
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

// scalar_type: 'int' | 'float' | 'bool'
hir::ScalarType::Ptr Parser::parseScalarType() {
  auto scalarType = std::make_shared<hir::ScalarType>();

  scalarType->setLoc(peek());
  switch (peek().type) {
    case Token::Type::INT:
      consume(Token::Type::INT);
      scalarType->type = hir::ScalarType::Type::INT;
      break;
    case Token::Type::FLOAT:
      consume(Token::Type::FLOAT);
      scalarType->type = hir::ScalarType::Type::FLOAT;
      break;
    case Token::Type::BOOL:
      consume(Token::Type::BOOL);
      scalarType->type = hir::ScalarType::Type::BOOL;
      break;
    case Token::Type::STRING:
      // TODO: Implement.
    default:
      reportError(peek(), "a tensor type identifier");
      throw SyntaxError();
      break;
  }

  return scalarType;
}

// index_sets: index_set {',' index_set}
std::vector<hir::IndexSet::Ptr> Parser::parseIndexSets() {
  std::vector<hir::IndexSet::Ptr> indexSets;

  do {
    const hir::IndexSet::Ptr indexSet = parseIndexSet();
    indexSets.push_back(indexSet);
  } while (tryconsume(Token::Type::COMMA));

  return indexSets;
}

// index_set: INT_LITERAL | IDENT | '*'
hir::IndexSet::Ptr Parser::parseIndexSet() {
  hir::IndexSet::Ptr indexSet;
  switch (peek().type) {
    case Token::Type::INT_LITERAL:
    {
      auto rangeIndexSet = std::make_shared<hir::RangeIndexSet>();
      
      const Token intToken = consume(Token::Type::INT_LITERAL);
      rangeIndexSet->setLoc(intToken);
      rangeIndexSet->range = intToken.num;
      
      indexSet = rangeIndexSet;
      break;
    }
    case Token::Type::IDENT:
    {
      auto setIndexSet = std::make_shared<hir::SetIndexSet>();
      
      const Token identToken = consume(Token::Type::IDENT);
      setIndexSet->setLoc(identToken);
      setIndexSet->setName = identToken.str;
      
      indexSet = setIndexSet;
      break;
    }
    case Token::Type::STAR:
    {
      indexSet = std::make_shared<hir::DynamicIndexSet>();
      
      const Token starToken = consume(Token::Type::STAR);
      indexSet->setLoc(starToken);
      break;
    }
    default:
      reportError(peek(), "an index set");
      throw SyntaxError();
      break;
  }

  return indexSet;
}

// tensor_literal: INT_LITERAL | FLOAT_LITERAL | 'true' | 'false'
//               | dense_tensor_literal
hir::Expr::Ptr Parser::parseTensorLiteral() {
  hir::Expr::Ptr literal;
  switch (peek().type) {
    case Token::Type::INT_LITERAL:
    {
      auto intLiteral = std::make_shared<hir::IntLiteral>();
      
      const Token intToken = consume(Token::Type::INT_LITERAL);
      intLiteral->setLoc(intToken);
      intLiteral->val = intToken.num;
      
      literal = intLiteral;
      break;
    }
    case Token::Type::FLOAT_LITERAL:
    {
      auto floatLiteral = std::make_shared<hir::FloatLiteral>();
      
      const Token floatToken = consume(Token::Type::FLOAT_LITERAL);
      floatLiteral->setLoc(floatToken);
      floatLiteral->val = floatToken.fnum;
      
      literal = floatLiteral;
      break;
    }
    case Token::Type::TRUE:
    {
      auto trueLiteral = std::make_shared<hir::BoolLiteral>();
      
      const Token trueToken = consume(Token::Type::TRUE);
      trueLiteral->setLoc(trueToken);
      trueLiteral->val = true;
      
      literal = trueLiteral;
      break;
    }
    case Token::Type::FALSE:
    {
      auto falseLiteral = std::make_shared<hir::BoolLiteral>();
      
      const Token falseToken = consume(Token::Type::FALSE);
      falseLiteral->setLoc(falseToken);
      falseLiteral->val = false;
      
      literal = falseLiteral;
      break;
    }
    case Token::Type::LB:
      literal = parseDenseTensorLiteral();
      break;
    case Token::Type::STRING_LITERAL:
      // TODO: Implement.
    default:
      reportError(peek(), "a tensor literal");
      throw SyntaxError();
      break;
  }

  return literal;
}

// dense_tensor_literal: '[' dense_tensor_literal_inner ']'
hir::DenseTensorLiteral::Ptr Parser::parseDenseTensorLiteral() {
  const Token leftBracketToken = consume(Token::Type::LB);
  hir::DenseTensorLiteral::Ptr tensor = parseDenseTensorLiteralInner();
  const Token rightBracketToken = consume(Token::Type::RB);
  
  tensor->setBeginLoc(leftBracketToken);
  tensor->setEndLoc(rightBracketToken);

  return tensor;
}

// dense_tensor_literal_inner: dense_tensor_literal {[','] dense_tensor_literal}
//                           | dense_matrix_literal
hir::DenseTensorLiteral::Ptr Parser::parseDenseTensorLiteralInner() {
  if (peek().type == Token::Type::LB) {
    auto tensor = std::make_shared<hir::NDTensorLiteral>();
    tensor->transposed = false;

    hir::DenseTensorLiteral::Ptr elem = parseDenseTensorLiteral();
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
hir::DenseTensorLiteral::Ptr Parser::parseDenseMatrixLiteral() {
  auto mat = std::make_shared<hir::NDTensorLiteral>();
  mat->transposed = false;
  
  do {
    const hir::DenseTensorLiteral::Ptr vec = parseDenseVectorLiteral();
    mat->elems.push_back(vec);
  } while (tryconsume(Token::Type::SEMICOL));

  return (mat->elems.size() == 1) ? mat->elems[0] : mat;
}

// dense_vector_literal: dense_int_vector_literal | dense_float_vector_literal
hir::DenseTensorLiteral::Ptr Parser::parseDenseVectorLiteral() {
  hir::DenseTensorLiteral::Ptr vec;
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
    default:
      reportError(peek(), "a vector literal");
      throw SyntaxError();
      break;
  }

  return vec;
}

// dense_int_vector_literal: signed_int_literal {[','] signed_int_literal}
hir::IntVectorLiteral::Ptr Parser::parseDenseIntVectorLiteral() {
  auto vec = std::make_shared<hir::IntVectorLiteral>();
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
hir::FloatVectorLiteral::Ptr Parser::parseDenseFloatVectorLiteral() {
  auto vec = std::make_shared<hir::FloatVectorLiteral>();
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

// '%!' ident call_params '==' expr ';'
hir::Test::Ptr Parser::parseTest() {
  auto test = std::make_shared<hir::Test>();

  const Token testToken = consume(Token::Type::TEST);
  test->setBeginLoc(testToken);

  test->func = parseIdent();
  switch (peek().type) {
    case Token::Type::LP:
    {
      test->args = parseCallParams();
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

