#include <memory>

#include "parser.h"
#include "scanner.h"
#include "hir.h"

namespace simit { 
namespace internal {

hir::Program::Ptr ParserNew::parse(const TokenStream &tokens, 
                                 std::vector<ParseError> *errors) {
  this->tokens = tokens;
  this->errors = errors;
  return parseProgram();
}

hir::Program::Ptr ParserNew::parseProgram() {
  auto program = std::make_shared<hir::Program>();
  
  while (peek().type != TokenType::END) {
    const hir::HIRNode::Ptr element = parseProgramElement();
    if (element) {
      program->elems.push_back(element);
    }
  }
  
  return program;
}

hir::HIRNode::Ptr ParserNew::parseProgramElement() {
  try {
    switch (peek().type) {
      case TokenType::TEST:
        return parseTest();
        break;
      case TokenType::FUNC:
        return parseFunction();
        break;
      case TokenType::PROC:
        return parseProcedure();
        break;
      case TokenType::ELEMENT:
        return parseElementTypeDecl();
        break;
      case TokenType::EXTERN:
        return parseExternDecl();
        break;
      case TokenType::CONST:
        return parseConstDecl();
        break;
      default:
        reportError(peek(), "unexpected symbol: "); // TODO: print symbol
        throw SyntaxError();
        break;
    }
  } catch (const SyntaxError &) {
    skipTo({TokenType::TEST, TokenType::FUNC, TokenType::PROC, 
            TokenType::ELEMENT, TokenType::EXTERN, TokenType::CONST});
    return hir::HIRNode::Ptr();
  }
}

hir::ElementTypeDecl::Ptr ParserNew::parseElementTypeDecl() {
  auto elemTypeDecl = std::make_shared<hir::ElementTypeDecl>();
  
  consume(TokenType::ELEMENT);
  
  const Token nameToken = consume(TokenType::IDENT);
  elemTypeDecl->setLoc(nameToken);
  elemTypeDecl->ident = nameToken.str;
  
  elemTypeDecl->fields = parseFieldDeclList();

  consume(TokenType::BLOCKEND);
  
  return elemTypeDecl;
}

std::vector<hir::Field::Ptr> ParserNew::parseFieldDeclList() {
  std::vector<hir::Field::Ptr> fields;

  while (peek().type == TokenType::IDENT) {
    const hir::Field::Ptr field = parseFieldDecl();
    fields.push_back(field);
  }

  return fields;
}

hir::Field::Ptr ParserNew::parseFieldDecl() {
  auto fieldDecl = std::make_shared<hir::Field>();

  const Token fieldToken = consume(TokenType::IDENT);
  fieldDecl->setLoc(fieldToken);
  fieldDecl->name = fieldToken.str;
  
  consume(TokenType::COL);
  
  fieldDecl->type = parseTensorType();
  
  consume(TokenType::SEMICOL);

  return fieldDecl;
}

hir::ExternDecl::Ptr ParserNew::parseExternDecl() {
  auto externDecl = std::make_shared<hir::ExternDecl>();

  consume(TokenType::EXTERN);
  
  externDecl->var = parseArgumentDecl();
  externDecl->setLoc(externDecl->var);
  
  consume(TokenType::SEMICOL);
 
  return externDecl;
}

hir::FuncDecl::Ptr ParserNew::parseFunction() {
  auto funcDecl = std::make_shared<hir::FuncDecl>();

  consume(TokenType::FUNC);

  const Token nameToken = consume(TokenType::IDENT);
  funcDecl->setLoc(nameToken);
  funcDecl->name = nameToken.str;

  funcDecl->args = parseArguments();
  funcDecl->results = parseResults();
  funcDecl->body = parseStmtBlock();
  
  consume(TokenType::BLOCKEND);

  return funcDecl;
}

hir::ProcDecl::Ptr ParserNew::parseProcedure() {
  auto procDecl = std::make_shared<hir::ProcDecl>();

  consume(TokenType::PROC);

  const Token nameToken = consume(TokenType::IDENT);
  procDecl->setLoc(nameToken);
  procDecl->name = nameToken.str;

  if (peek().type == TokenType::LP) {
    switch (peek(1).type) {
      case TokenType::IDENT:
        if (peek(2).type != TokenType::COL) {
          break;
        }
      case TokenType::INOUT:
        procDecl->args = parseArguments();
        procDecl->results = parseResults();
        break;
      default:
        break;
    }
  }

  procDecl->body = parseStmtBlock();
  
  consume(TokenType::BLOCKEND);

  return procDecl;
}

std::vector<hir::Argument::Ptr> ParserNew::parseArguments() {
  std::vector<hir::Argument::Ptr> arguments;
  
  consume(TokenType::LP);

  if (peek().type != TokenType::RP) {
    do {
      const hir::Argument::Ptr argument = parseArgumentDecl();
      arguments.push_back(argument);
    } while (tryconsume(TokenType::COMMA));
  }

  consume(TokenType::RP);

  return arguments;
}

hir::Argument::Ptr ParserNew::parseArgumentDecl() {
  auto argDecl = std::make_shared<hir::Argument>();
  argDecl->inout = false;

  if (peek().type == TokenType::INOUT) {
    consume(TokenType::INOUT);
    argDecl->inout = true;
  }

  const hir::IdentDecl::Ptr var = parseIdentDecl();
  argDecl->setLoc(var);
  argDecl->ident = var->ident;
  argDecl->type = var->type;
  
  return argDecl;
}

std::vector<hir::IdentDecl::Ptr> ParserNew::parseResults() {
  std::vector<hir::IdentDecl::Ptr> results;

  if (peek().type == TokenType::RARROW) {
    consume(TokenType::RARROW);
    consume(TokenType::LP);
    
    do {
      const hir::IdentDecl::Ptr result = parseIdentDecl();
      results.push_back(result);
    } while (tryconsume(TokenType::COMMA));

    consume(TokenType::RP);
  }

  return results;
}

hir::StmtBlock::Ptr ParserNew::parseStmtBlock() {
  auto stmtBlock = std::make_shared<hir::StmtBlock>();
  stmtBlock->setLoc(peek());

  while (true) {
    switch (peek().type) {
      case TokenType::BLOCKEND:
      case TokenType::ELIF:
      case TokenType::ELSE:
      case TokenType::END:
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

hir::Stmt::Ptr ParserNew::parseStmt() {
  switch (peek().type) {
    case TokenType::VAR:
      return parseVarDecl();
    case TokenType::CONST:
      return parseConstDecl();
    case TokenType::IF:
      return parseIfStmt();
    case TokenType::WHILE:
      return parseWhileStmt();
    case TokenType::DO:
      return parseDoWhileStmt();
    case TokenType::FOR:
      return parseForStmt();
    case TokenType::PRINT:
      return parsePrintStmt();
    default:
      return parseExprOrAssignStmt();
  }
}

hir::VarDecl::Ptr ParserNew::parseVarDecl() {
  try {
    consume(TokenType::VAR);
    
    auto varDecl = std::make_shared<hir::VarDecl>();
    varDecl->var = parseIdentDecl();
  
    if (peek().type == TokenType::ASSIGN) {
      consume(TokenType::ASSIGN);
      varDecl->initVal = parseExpr();
    }
  
    consume(TokenType::SEMICOL);

    return varDecl;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::VarDecl::Ptr();
  }
}

hir::ConstDecl::Ptr ParserNew::parseConstDecl() {
  try {
    consume(TokenType::CONST);
    
    auto constDecl = std::make_shared<hir::ConstDecl>();
    constDecl->var = parseIdentDecl();
  
    consume(TokenType::ASSIGN);
    constDecl->initVal = parseExpr();
  
    consume(TokenType::SEMICOL);

    return constDecl;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::ConstDecl::Ptr();
  }
}

hir::IdentDecl::Ptr ParserNew::parseIdentDecl() {
  auto identDecl = std::make_shared<hir::IdentDecl>();

  const Token identToken = consume(TokenType::IDENT);
  identDecl->setLoc(identToken);
  identDecl->ident = identToken.str;

  consume(TokenType::COL);
  
  identDecl->type = parseType();
  
  return identDecl;
}

hir::WhileStmt::Ptr ParserNew::parseWhileStmt() {
  try {
    consume(TokenType::WHILE);
    
    auto whileStmt = std::make_shared<hir::WhileStmt>();
    whileStmt->cond = parseExpr();
    whileStmt->body = parseStmtBlock();

    consume(TokenType::BLOCKEND);

    return whileStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND});
    return hir::WhileStmt::Ptr();
  }
}

hir::DoWhileStmt::Ptr ParserNew::parseDoWhileStmt() {
  try {
    consume(TokenType::DO);
    
    auto doWhileStmt = std::make_shared<hir::DoWhileStmt>();
    doWhileStmt->body = parseStmtBlock();

    consume(TokenType::BLOCKEND);
    consume(TokenType::WHILE);

    doWhileStmt->cond = parseExpr();

    return doWhileStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND, TokenType::ELIF, TokenType::ELSE, 
            TokenType::END, TokenType::VAR, TokenType::CONST, TokenType::IF,
            TokenType::WHILE, TokenType::DO, TokenType::FOR, TokenType::PRINT});
    return hir::DoWhileStmt::Ptr();
  }
}

hir::IfStmt::Ptr ParserNew::parseIfStmt() {
  try {
    consume(TokenType::IF);
    
    auto ifStmt = std::make_shared<hir::IfStmt>();
    ifStmt->cond = parseExpr();
    ifStmt->ifBody = parseStmtBlock();
    ifStmt->elseBody = parseElseClause();

    consume(TokenType::BLOCKEND);

    return ifStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND});
    return hir::IfStmt::Ptr();
  }
}

hir::Stmt::Ptr ParserNew::parseElseClause() {
  try {
    switch (peek().type) {
      case TokenType::ELSE:
        consume(TokenType::ELSE);
        return parseStmtBlock();
      case TokenType::ELIF:
      {
        consume(TokenType::ELIF);
    
        auto elifClause = std::make_shared<hir::IfStmt>();
        elifClause->cond = parseExpr();
        elifClause->ifBody = parseStmtBlock();
        elifClause->elseBody = parseElseClause();

        return elifClause;
      }
      default:
        return hir::Stmt::Ptr();
    }
  } catch (const SyntaxError &) {
    skipTo({TokenType::ELIF, TokenType::ELSE, TokenType::BLOCKEND});
    return std::make_shared<hir::StmtBlock>();
  }
}

hir::ForStmt::Ptr ParserNew::parseForStmt() {
  try {
    consume(TokenType::FOR);

    auto forStmt = std::make_shared<hir::ForStmt>();

    const Token loopVarToken = consume(TokenType::IDENT);
    forStmt->loopVarName = loopVarToken.str;

    consume(TokenType::IN);

    forStmt->domain = parseForDomain();
    forStmt->body = parseStmtBlock();

    consume(TokenType::BLOCKEND);

    return forStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND});
    return hir::ForStmt::Ptr();
  }
}

hir::ForDomain::Ptr ParserNew::parseForDomain() {
  switch (peek().type) {
    case TokenType::INT_LITERAL:
    case TokenType::IDENT:
      if (peek(1).type == TokenType::COL) {
        break;
      }
    case TokenType::STAR:
    {
      auto indexSetDomain = std::make_shared<hir::IndexSetDomain>();
      indexSetDomain->domain = parseIndexSet();
      return indexSetDomain;
    }
    default:
      break;
  }

  auto rangeDomain = std::make_shared<hir::RangeDomain>();
  rangeDomain->lower = parseExpr();

  consume(TokenType::COL);

  rangeDomain->upper = parseExpr();
  return rangeDomain;
}

hir::PrintStmt::Ptr ParserNew::parsePrintStmt() {
  try {
    consume(TokenType::PRINT);
    
    auto printStmt = std::make_shared<hir::PrintStmt>();
    printStmt->expr = parseExpr();

    return printStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::PrintStmt::Ptr();
  }
}

hir::ExprStmt::Ptr ParserNew::parseExprOrAssignStmt() {
  try {
    hir::ExprStmt::Ptr stmt;

    if (peek().type != TokenType::SEMICOL) {
      hir::Expr::Ptr expr = parseExpr();
      
      switch (peek().type) {
        case TokenType::COMMA:
        case TokenType::ASSIGN:
        {
          auto assignStmt = std::make_shared<hir::AssignStmt>();
          assignStmt->lhs.push_back(expr);
  
          while (peek().type == TokenType::COMMA) {
            consume(TokenType::COMMA);
  
            expr = parseExpr();
            assignStmt->lhs.push_back(expr);
          }
  
          consume(TokenType::ASSIGN);
          
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
    
    consume(TokenType::SEMICOL);

    return stmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::ExprStmt::Ptr();
  }
}

hir::Expr::Ptr ParserNew::parseExpr() {
  return (peek().type == TokenType::MAP) ? parseMapExpr() : parseOrExpr();
}

hir::Expr::Ptr ParserNew::parseMapExpr() {
  auto mapExpr = std::make_shared<hir::MapExpr>();
  mapExpr->setLoc(peek());

  consume(TokenType::MAP);
  
  const Token funcToken = consume(TokenType::IDENT);
  mapExpr->setFuncNameLoc(funcToken);
  mapExpr->funcName = funcToken.str;
 
  if (peek().type == TokenType::LP) {
    mapExpr->partialActuals = parseCallParams();
  }

  consume(TokenType::TO);
  
  const Token targetToken = consume(TokenType::IDENT);
  mapExpr->setTargetNameLoc(targetToken);
  mapExpr->targetName = targetToken.str;
  
  if (peek().type == TokenType::REDUCE) {
    consume(TokenType::REDUCE);
    consume(TokenType::PLUS);

    mapExpr->op = hir::MapExpr::ReductionOp::SUM;
  } else {
    mapExpr->op = hir::MapExpr::ReductionOp::NONE;
  }

  return mapExpr;
}

hir::Expr::Ptr ParserNew::parseOrExpr() {
  hir::Expr::Ptr expr = parseAndExpr(); 

  while (peek().type == TokenType::OR) {
    consume(TokenType::OR);

    auto orExpr = std::make_shared<hir::OrExpr>();
    orExpr->setLoc(expr);
    orExpr->lhs = expr;
    orExpr->rhs = parseAndExpr();
    expr = orExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseAndExpr() {
  hir::Expr::Ptr expr = parseXorExpr(); 

  while (peek().type == TokenType::AND) {
    consume(TokenType::AND);

    auto andExpr = std::make_shared<hir::AndExpr>();
    andExpr->setLoc(expr);
    andExpr->lhs = expr;
    andExpr->rhs = parseXorExpr();
    expr = andExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseXorExpr() {
  hir::Expr::Ptr expr = parseEqExpr(); 

  while (peek().type == TokenType::XOR) {
    consume(TokenType::XOR);

    auto xorExpr = std::make_shared<hir::XorExpr>();
    xorExpr->setLoc(expr);
    xorExpr->lhs = expr;
    xorExpr->rhs = parseEqExpr();
    expr = xorExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseEqExpr() {
  auto expr = std::make_shared<hir::EqExpr>();
  expr->setLoc(peek());

  const hir::Expr::Ptr operand = parseTerm();
  expr->operands.push_back(operand);

  while (true) {
    switch (peek().type) {
      case TokenType::EQ:
        consume(TokenType::EQ);
        expr->ops.push_back(hir::EqExpr::Op::EQ);
        break;
      case TokenType::NE:
        consume(TokenType::NE);
        expr->ops.push_back(hir::EqExpr::Op::NE);
        break;
      case TokenType::RA:
        consume(TokenType::RA);
        expr->ops.push_back(hir::EqExpr::Op::GT);
        break;
      case TokenType::LA:
        consume(TokenType::LA);
        expr->ops.push_back(hir::EqExpr::Op::LT);
        break;
      case TokenType::GE:
        consume(TokenType::GE);
        expr->ops.push_back(hir::EqExpr::Op::GE);
        break;
      case TokenType::LE:
        consume(TokenType::LE);
        expr->ops.push_back(hir::EqExpr::Op::LE);
        break;
      default:
        return (expr->operands.size() == 1) ? expr->operands[0] : expr;
    }
   
    const hir::Expr::Ptr operand = parseTerm();
    expr->operands.push_back(operand);
  }
}

hir::Expr::Ptr ParserNew::parseTerm() {
  if (peek().type == TokenType::NOT) {
    auto notExpr = std::make_shared<hir::NotExpr>();
    notExpr->setLoc(peek());

    consume(TokenType::NOT);
   
    notExpr->operand = parseTerm();
    return notExpr;
  }

  return parseSolveExpr();
}

hir::Expr::Ptr ParserNew::parseSolveExpr() {
  hir::Expr::Ptr expr = parseAddExpr();

  //while (peek().type == TokenType::BACKSLASH) {
    // TODO: raise unsupported error?
  //  consume(TokenType::BACKSLASH);
  //  parseAddExpr(calls);
  //}

  return expr;
}

hir::Expr::Ptr ParserNew::parseAddExpr() {
  hir::Expr::Ptr expr = parseMulExpr();

  while (true) {
    hir::BinaryExpr::Ptr addExpr;

    switch (peek().type) {
      case TokenType::PLUS:
        consume(TokenType::PLUS);
        addExpr = std::make_shared<hir::AddExpr>();
        break;
      case TokenType::MINUS:
        consume(TokenType::MINUS);
        addExpr = std::make_shared<hir::SubExpr>();
        break;
      default:
        return expr;
    }
    
    addExpr->setLoc(expr);
    addExpr->lhs = expr;
    addExpr->rhs = parseMulExpr();
    expr = addExpr;
  }
}

hir::Expr::Ptr ParserNew::parseMulExpr() {
  hir::Expr::Ptr expr = parseNegExpr();

  while (true) {
    hir::BinaryExpr::Ptr mulExpr;

    switch (peek().type) {
      case TokenType::STAR:
        consume(TokenType::STAR);
        mulExpr = std::make_shared<hir::MulExpr>();
        break;
      case TokenType::SLASH:
        consume(TokenType::SLASH);
        mulExpr = std::make_shared<hir::DivExpr>();
        break;
      case TokenType::DOTSTAR:
        consume(TokenType::DOTSTAR);
        mulExpr = std::make_shared<hir::ElwiseMulExpr>();
        break;
      case TokenType::DOTSLASH:
        consume(TokenType::DOTSLASH);
        mulExpr = std::make_shared<hir::ElwiseDivExpr>();
        break;
      default:
        return expr;
    }
        
    mulExpr->setLoc(expr);
    mulExpr->lhs = expr;
    mulExpr->rhs = parseNegExpr();
    expr = mulExpr;
  }
}

hir::Expr::Ptr ParserNew::parseNegExpr() {
  auto negExpr = std::make_shared<hir::NegExpr>();
  negExpr->setLoc(peek());

  switch (peek().type) {
    case TokenType::MINUS:
      consume(TokenType::MINUS);
      negExpr->negate = true;
      break;
    case TokenType::PLUS:
      consume(TokenType::PLUS);
      negExpr->negate = false;
      break;
    default:
      return parseExpExpr();
  }

  negExpr->operand = parseExpExpr();
  return negExpr;
}

hir::Expr::Ptr ParserNew::parseExpExpr() {
  hir::Expr::Ptr expr = parseTransposeExpr();

  if (peek().type == TokenType::EXP) {
    consume(TokenType::EXP);

    auto expExpr = std::make_shared<hir::ExpExpr>();
    expExpr->setLoc(expr);
    expExpr->lhs = expr;
    expExpr->rhs = parseExpExpr();
    expr = expExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseTransposeExpr() {
  hir::Expr::Ptr expr = parseCallOrReadExpr();

  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);
    
    auto transposeExpr = std::make_shared<hir::TransposeExpr>();
    transposeExpr->setLoc(expr);
    transposeExpr->operand = expr;
    expr = transposeExpr; 
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseCallOrReadExpr() {
  hir::Expr::Ptr expr = parseFactor();

  while (true) {
    switch (peek().type) {
      case TokenType::LP:
      {
        auto tensorRead = std::make_shared<hir::TensorReadExpr>();
        tensorRead->setLoc(expr);
        tensorRead->tensor = expr;
        tensorRead->indices = parseReadParams();
        expr = tensorRead;
        break;
      }
      case TokenType::PERIOD:
      {
        consume(TokenType::PERIOD);
        const Token fieldToken = consume(TokenType::IDENT);
        
        auto fieldRead = std::make_shared<hir::FieldReadExpr>();
        fieldRead->setLoc(fieldToken);
        fieldRead->setOrElem = expr;
        fieldRead->fieldName = fieldToken.str;
        expr = fieldRead;
        break;
      }
      default:
        return expr;
    }
  }
}

hir::Expr::Ptr ParserNew::parseFactor() {
  hir::Expr::Ptr factor;

  switch (peek().type) {
    case TokenType::LP:
    {
      const Token leftParen = consume(TokenType::LP);
      
      factor = parseExpr();
      factor->setLoc(leftParen);
      
      consume(TokenType::RP);
      break;
    }
    case TokenType::IDENT:
    {
      const Token identToken = consume(TokenType::IDENT);
      auto var = std::make_shared<hir::VarExpr>(); 
      var->setLoc(identToken);
      var->ident = identToken.str;
      factor = var;
      break;
    }
    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::STRING_LITERAL:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::LB:
      factor = parseTensorLiteral();
      break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return factor;
}

std::vector<hir::ReadParam::Ptr> ParserNew::parseReadParams() {
  std::vector<hir::ReadParam::Ptr> readParams;

  consume(TokenType::LP);

  if (peek().type != TokenType::RP) {
    do {
      const hir::ReadParam::Ptr param = parseReadParam();
      readParams.push_back(param);
    } while (tryconsume(TokenType::COMMA));
  }
  
  consume(TokenType::RP);

  return readParams;
}

hir::ReadParam::Ptr ParserNew::parseReadParam() {
  if (peek().type == TokenType::COL) {
    const Token colToken = consume(TokenType::COL);
    auto slice = std::make_shared<hir::Slice>();
    slice->setLoc(colToken);
    return slice;
  }

  auto param = std::make_shared<hir::ExprParam>();
  param->setLoc(peek());
  param->expr = parseExpr();
  
  return param;
}

std::vector<hir::Expr::Ptr> ParserNew::parseCallParams() {
  std::vector<hir::Expr::Ptr> callParams;

  consume(TokenType::LP);

  if (peek().type != TokenType::RP) {
    do {
      const hir::Expr::Ptr param = parseExpr();
      callParams.push_back(param);
    } while (tryconsume(TokenType::COMMA));
  }
  
  consume(TokenType::RP);

  return callParams;
}

hir::Type::Ptr ParserNew::parseType() {
  hir::Type::Ptr type;

  switch (peek().type) {
    case TokenType::IDENT:
      type = parseElementType();
      break;
    case TokenType::SET:
      type = parseSetType();
      break;
    case TokenType::LP:
      type = parseTupleType();
      break;
    case TokenType::INT:
    case TokenType::FLOAT:
    case TokenType::BOOL:
    case TokenType::STRING:
    case TokenType::TENSOR:
      type = parseTensorType();
      break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return type;
}

hir::ElementType::Ptr ParserNew::parseElementType() {
  auto elementType = std::make_shared<hir::ElementType>();

  const Token typeToken = consume(TokenType::IDENT);
  elementType->setLoc(typeToken);
  elementType->ident = typeToken.str;

  return elementType;
}

hir::SetType::Ptr ParserNew::parseSetType() {
  auto setType = std::make_shared<hir::SetType>();
  setType->setLoc(peek());

  consume(TokenType::SET);
  consume(TokenType::LC);
  
  setType->element = parseElementType();
  
  consume(TokenType::RC);

  if (peek().type == TokenType::LP) {
    consume(TokenType::LP);
    setType->endpoints = parseEndpoints();
    consume(TokenType::RP);
  }

  return setType;
}

std::vector<hir::Endpoint::Ptr> ParserNew::parseEndpoints() {
  std::vector<hir::Endpoint::Ptr> endpoints;
  
  do {
    const Token endpointToken = consume(TokenType::IDENT);
    auto endpoint = std::make_shared<hir::Endpoint>();
    endpoint->setLoc(endpointToken);
    endpoint->setName = endpointToken.str;
    endpoints.push_back(endpoint);
  } while (tryconsume(TokenType::COMMA));

  return endpoints;
}

hir::TupleType::Ptr ParserNew::parseTupleType() {
  auto tupleType = std::make_shared<hir::TupleType>();

  consume(TokenType::LP);
  
  tupleType->element = parseElementType();
  
  consume(TokenType::STAR);
  
  const Token intToken = consume(TokenType::INT_LITERAL);
  tupleType->setLoc(intToken);
  tupleType->length = intToken.num;
  
  consume(TokenType::RP);
  
  return tupleType;
}

hir::TensorType::Ptr ParserNew::parseTensorType() {
  hir::TensorType::Ptr tensorType;
  
  if (peek().type == TokenType::TENSOR) {
    tensorType = std::make_shared<hir::NonScalarTensorType>();
  } else {
    tensorType = std::make_shared<hir::ScalarTensorType>();
  }
  tensorType->setLoc(peek());

  switch (peek().type) {
    case TokenType::INT:
      consume(TokenType::INT);
      hir::to<hir::ScalarTensorType>(tensorType)->type = 
        hir::ScalarTensorType::Type::INT;
      break;
    case TokenType::FLOAT:
      consume(TokenType::FLOAT);
      hir::to<hir::ScalarTensorType>(tensorType)->type = 
        hir::ScalarTensorType::Type::FLOAT;
      break;
    case TokenType::BOOL:
      consume(TokenType::BOOL);
      hir::to<hir::ScalarTensorType>(tensorType)->type = 
        hir::ScalarTensorType::Type::BOOL;
      break;
    //case TokenType::STRING:
    //  consume(TokenType::STRING);
    //  // TODO: raise error?
    //  return Type();
    case TokenType::TENSOR:
    {
      const auto nonScalarTensorType = 
        hir::to<hir::NonScalarTensorType>(tensorType);
      
      consume(TokenType::TENSOR);
      
      if (peek().type == TokenType::LB) {
        consume(TokenType::LB);
        nonScalarTensorType->indexSets = parseIndexSets();
        consume(TokenType::RB);
      }

      consume(TokenType::LP);
      nonScalarTensorType->blockType = parseTensorType();
      consume(TokenType::RP);
  
      if (peek().type == TokenType::TRANSPOSE) {
        consume(TokenType::TRANSPOSE);
        nonScalarTensorType->transposed = true;
      } else {
        nonScalarTensorType->transposed = false;
      }

      break;
    }
    default:
      reportError(peek(), "unexpected token"); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return tensorType;
}

std::vector<hir::IndexSet::Ptr> ParserNew::parseIndexSets() {
  std::vector<hir::IndexSet::Ptr> indexSets;

  do {
    const hir::IndexSet::Ptr indexSet = parseIndexSet();
    indexSets.push_back(indexSet);
  } while (tryconsume(TokenType::COMMA));

  return indexSets;
}

hir::IndexSet::Ptr ParserNew::parseIndexSet() {
  hir::IndexSet::Ptr indexSet;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
    {
      const Token intToken = consume(TokenType::INT_LITERAL);
      auto rangeIndexSet = std::make_shared<hir::RangeIndexSet>();
      rangeIndexSet->range = intToken.num;
      rangeIndexSet->setLoc(intToken);
      indexSet = rangeIndexSet;
      break;
    }
    case TokenType::IDENT:
    {
      const Token identToken = consume(TokenType::IDENT);
      auto setIndexSet = std::make_shared<hir::SetIndexSet>();
      setIndexSet->setName = identToken.str;
      setIndexSet->setLoc(identToken);
      indexSet = setIndexSet;
      break;
    }
    case TokenType::STAR:
    {
      const Token starToken = consume(TokenType::STAR);
      indexSet = std::make_shared<hir::DynamicIndexSet>();
      indexSet->setLoc(starToken);
      break;
    }
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return indexSet;
}

hir::Expr::Ptr ParserNew::parseTensorLiteral() {
  hir::Expr::Ptr literal;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::PLUS:
    case TokenType::MINUS:
      literal = parseSignedNumLiteral();
      break;
    case TokenType::TRUE:
    case TokenType::FALSE:
    {
      const bool boolVal = tryconsume(TokenType::TRUE);
      if (!boolVal) {
        consume(TokenType::FALSE);
      }

      auto boolLiteral = std::make_shared<hir::BoolLiteral>();
      boolLiteral->val = boolVal;
      literal = boolLiteral;
      
      break;
    }
    case TokenType::LB:
    {
      literal = parseDenseTensorLiteral();

      if (peek().type == TokenType::TRANSPOSE) {
        consume(TokenType::TRANSPOSE);
        
        auto transposedLiteral = std::make_shared<hir::TransposeExpr>();
        transposedLiteral->setLoc(literal);
        transposedLiteral->operand = literal;
        literal = transposedLiteral;
      }

      break;
    }
    case TokenType::STRING_LITERAL:
      //consume(TokenType::STRING_LITERAL);
      //break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return literal;
}

hir::TensorLiteral::Ptr ParserNew::parseSignedNumLiteral() {
  hir::TensorLiteral::Ptr literal;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
    {
      auto intLiteral = std::make_shared<hir::IntLiteral>();
      intLiteral->setLoc(peek());
      intLiteral->val = parseSignedIntLiteral();
      literal = intLiteral;
      break;
    }
    case TokenType::FLOAT_LITERAL:
    {
      auto floatLiteral = std::make_shared<hir::FloatLiteral>();
      floatLiteral->setLoc(peek());
      floatLiteral->val = parseSignedFloatLiteral();
      literal = floatLiteral;
      break;
    }
    case TokenType::PLUS:
    case TokenType::MINUS:
      switch (peek(1).type) {
        case TokenType::INT_LITERAL:
        {
          auto intLiteral = std::make_shared<hir::IntLiteral>();
          intLiteral->setLoc(peek());
          intLiteral->val = parseSignedIntLiteral();
          literal = intLiteral;
          break;
        }
        case TokenType::FLOAT_LITERAL:
        {
          auto floatLiteral = std::make_shared<hir::FloatLiteral>();
          floatLiteral->setLoc(peek());
          floatLiteral->val = parseSignedFloatLiteral();
          literal = floatLiteral;
          break;
        }
        default:
          reportError(peek(), "unexpected symbol: "); // TODO: print symbol
          throw SyntaxError();
          break;
      }
      break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return literal;
}

hir::DenseTensorLiteral::Ptr ParserNew::parseDenseTensorLiteral() {
  const Token bracketToken = consume(TokenType::LB);
  
  hir::DenseTensorLiteral::Ptr tensor = parseDenseTensorLiteralInner();
  tensor->setLoc(bracketToken);
  
  consume(TokenType::RB);
  
  return tensor;
}

hir::DenseTensorLiteral::Ptr ParserNew::parseDenseTensorLiteralInner() {
  if (peek().type == TokenType::LB) {
    auto tensor = std::make_shared<hir::DenseNDTensorLiteral>();
    tensor->setLoc(peek());

    hir::DenseTensorLiteral::Ptr elem = parseDenseTensorLiteral();
    tensor->elems.push_back(elem);
    
    while (true) {
      switch (peek().type) {
        case TokenType::COMMA:
          consume(TokenType::COMMA);
        case TokenType::LB:
          elem = parseDenseTensorLiteral();
          tensor->elems.push_back(elem);
          break;
        default:
          return (tensor->elems.size() == 1) ? tensor->elems[0] : tensor;
      }
    }
  }

  return parseDenseMatrixLiteral();
}

hir::DenseTensorLiteral::Ptr ParserNew::parseDenseMatrixLiteral() {
  auto mat = std::make_shared<hir::DenseNDTensorLiteral>();
  mat->setLoc(peek());
  
  do {
    const hir::DenseTensorLiteral::Ptr vec = parseDenseVectorLiteral();
    mat->elems.push_back(vec);
  } while (tryconsume(TokenType::SEMICOL));

  return (mat->elems.size() == 1) ? mat->elems[0] : mat;
}

hir::DenseTensorLiteral::Ptr ParserNew::parseDenseVectorLiteral() {
  hir::DenseTensorLiteral::Ptr vec;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
      vec = parseDenseIntVectorLiteral();
      break;
    case TokenType::FLOAT_LITERAL:
      vec = parseDenseFloatVectorLiteral();
      break;
    case TokenType::PLUS:
    case TokenType::MINUS:
      switch (peek(1).type) {
        case TokenType::INT_LITERAL:
          vec = parseDenseIntVectorLiteral();
          break;
        case TokenType::FLOAT_LITERAL:
          vec = parseDenseFloatVectorLiteral();
          break;
        default:
          reportError(peek(), "unexpected symbol: "); // TODO: print symbol
          throw SyntaxError();
          break;
      }
      break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return vec;
}

hir::DenseIntVectorLiteral::Ptr ParserNew::parseDenseIntVectorLiteral() {
  auto vec = std::make_shared<hir::DenseIntVectorLiteral>();
  vec->setLoc(peek());

  int elem = parseSignedIntLiteral();
  vec->vals.push_back(elem);

  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::INT_LITERAL:
        elem = parseSignedIntLiteral();
        vec->vals.push_back(elem);
        break;
      default:
        return vec;
    }
  }
}

hir::DenseFloatVectorLiteral::Ptr ParserNew::parseDenseFloatVectorLiteral() {
  auto vec = std::make_shared<hir::DenseFloatVectorLiteral>();
  vec->setLoc(peek());

  double elem = parseSignedFloatLiteral();
  vec->vals.push_back(elem);

  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::FLOAT_LITERAL:
        elem = parseSignedFloatLiteral();
        vec->vals.push_back(elem);
        break;
      default:
        return vec;
    }
  }
}

int ParserNew::parseSignedIntLiteral() {
  int coeff = 1;

  switch (peek().type) {
    case TokenType::PLUS:
      consume(TokenType::PLUS);
      break;
    case TokenType::MINUS:
      consume(TokenType::MINUS);
      coeff = -1;
      break;
    default:
      break;
  }

  return (coeff * consume(TokenType::INT_LITERAL).num);
}

double ParserNew::parseSignedFloatLiteral() {
  double coeff = 1.0;

  switch (peek().type) {
    case TokenType::PLUS:
      consume(TokenType::PLUS);
      break;
    case TokenType::MINUS:
      consume(TokenType::MINUS);
      coeff = -1.0;
      break;
    default:
      break;
  }

  return (coeff * consume(TokenType::FLOAT_LITERAL).fnum);
}

hir::Test::Ptr ParserNew::parseTest() {
  auto test = std::make_shared<hir::Test>();

  consume(TokenType::TEST);

  const Token funcToken = consume(TokenType::IDENT);
  test->setLoc(funcToken);
  test->funcName = funcToken.str;

  switch (peek().type) {
    case TokenType::LP:
    {
      test->args = parseCallParams();
      
      consume(TokenType::EQ);
      test->expected = parseTensorLiteral();
      
      consume(TokenType::SEMICOL);
      break;
    }
    case TokenType::ASSIGN:
//    consume(TokenType::ASSIGN);
//    parseSystemGenerator();
//    consume(TokenType::COL);
//    parseExternAssert();
//    break;
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return test;
}

//void ParserNew::parseSystemGenerator() {
//  switch (peek().type) {
//    case TokenType::IDENT:
//      consume(TokenType::IDENT);
//      break;
//    case TokenType::INT_LITERAL:
//      consume(TokenType::INT_LITERAL);
//      break;
//    default:
//      throw ParseException();
//      break;
//  }
//}
//
//void ParserNew::parseExternAssert() {
//  consume(TokenType::IDENT);
//  consume(TokenType::PERIOD);
//  consume(TokenType::IDENT);
//  consume(TokenType::ASSIGN);
//  parseTensorLiteral();
//  consume(TokenType::RARROW);
//  parseTensorLiteral();
//}

}
}

