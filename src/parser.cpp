#include <memory>

#include "parser.h"
#include "scanner.h"
#include "hir.h"

namespace simit { 
namespace internal {

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
  
  const Token elementToken = consume(TokenType::ELEMENT);
  elemTypeDecl->setBeginLoc(elementToken);
  
  elemTypeDecl->name = parseIdent(); 
  elemTypeDecl->fields = parseFieldDeclList();

  const Token endToken = consume(TokenType::BLOCKEND);
  elemTypeDecl->setEndLoc(endToken);
  
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
  
  fieldDecl->name = parseIdent();
  consume(TokenType::COL);
  fieldDecl->type = parseTensorType();
  
  const Token endToken = consume(TokenType::SEMICOL);
  fieldDecl->setEndLoc(endToken);

  return fieldDecl;
}

hir::ExternDecl::Ptr ParserNew::parseExternDecl() {
  auto externDecl = std::make_shared<hir::ExternDecl>();
  
  const Token externToken = consume(TokenType::EXTERN);
  externDecl->setBeginLoc(externToken);
  
  externDecl->var = parseArgumentDecl();
  
  const Token endToken = consume(TokenType::SEMICOL);
  externDecl->setEndLoc(endToken);
 
  return externDecl;
}

hir::FuncDecl::Ptr ParserNew::parseFunction() {
  auto funcDecl = std::make_shared<hir::FuncDecl>();

  const Token funcToken = consume(TokenType::FUNC);
  funcDecl->setBeginLoc(funcToken);

  funcDecl->name = parseIdent();
  funcDecl->args = parseArguments();
  funcDecl->results = parseResults();
  funcDecl->body = parseStmtBlock();
  
  const Token endToken = consume(TokenType::BLOCKEND);
  funcDecl->setEndLoc(endToken);

  return funcDecl;
}

hir::ProcDecl::Ptr ParserNew::parseProcedure() {
  auto procDecl = std::make_shared<hir::ProcDecl>();

  const Token procToken = consume(TokenType::PROC);
  procDecl->setBeginLoc(procToken);

  procDecl->name = parseIdent();
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
  
  const Token endToken = consume(TokenType::BLOCKEND);
  procDecl->setEndLoc(endToken);

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
  if (tryconsume(TokenType::INOUT)) {
    argDecl->inout = true;
  }

  const hir::IdentDecl::Ptr var = parseIdentDecl();
  argDecl->name = var->name;
  argDecl->type = var->type;
  
  return argDecl;
}

std::vector<hir::IdentDecl::Ptr> ParserNew::parseResults() {
  std::vector<hir::IdentDecl::Ptr> results;

  if (tryconsume(TokenType::RARROW)) {
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
    auto varDecl = std::make_shared<hir::VarDecl>();
    
    const Token varToken = consume(TokenType::VAR);
    varDecl->setBeginLoc(varToken);
    
    varDecl->var = parseIdentDecl();
    if (tryconsume(TokenType::ASSIGN)) {
      varDecl->initVal = parseExpr();
    }
  
    const Token endToken = consume(TokenType::SEMICOL);
    varDecl->setEndLoc(endToken);

    return varDecl;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::VarDecl::Ptr();
  }
}

hir::ConstDecl::Ptr ParserNew::parseConstDecl() {
  try {
    auto constDecl = std::make_shared<hir::ConstDecl>();
    
    const Token constToken = consume(TokenType::CONST);
    constDecl->setBeginLoc(constToken);
    
    constDecl->var = parseIdentDecl();
    consume(TokenType::ASSIGN);
    constDecl->initVal = parseExpr();
  
    const Token endToken = consume(TokenType::SEMICOL);
    constDecl->setEndLoc(endToken);

    return constDecl;
  } catch (const SyntaxError &) {
    skipTo({TokenType::SEMICOL});
    return hir::ConstDecl::Ptr();
  }
}

hir::IdentDecl::Ptr ParserNew::parseIdentDecl() {
  auto identDecl = std::make_shared<hir::IdentDecl>();

  identDecl->name = parseIdent();
  consume(TokenType::COL);
  identDecl->type = parseType();
  
  return identDecl;
}

hir::WhileStmt::Ptr ParserNew::parseWhileStmt() {
  try {
    auto whileStmt = std::make_shared<hir::WhileStmt>();
    
    const Token whileToken = consume(TokenType::WHILE);
    whileStmt->setBeginLoc(whileToken);
    
    whileStmt->cond = parseExpr();
    whileStmt->body = parseStmtBlock();

    const Token endToken = consume(TokenType::BLOCKEND);
    whileStmt->setEndLoc(endToken);

    return whileStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND});
    return hir::WhileStmt::Ptr();
  }
}

hir::DoWhileStmt::Ptr ParserNew::parseDoWhileStmt() {
  try {
    auto doWhileStmt = std::make_shared<hir::DoWhileStmt>();
    
    const Token doToken = consume(TokenType::DO);
    doWhileStmt->setBeginLoc(doToken);
    
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
    auto ifStmt = std::make_shared<hir::IfStmt>();
    
    const Token ifToken = consume(TokenType::IF);
    ifStmt->setBeginLoc(ifToken);
    
    ifStmt->cond = parseExpr();
    ifStmt->ifBody = parseStmtBlock();
    ifStmt->elseBody = parseElseClause();

    const Token endToken = consume(TokenType::BLOCKEND);
    ifStmt->setEndLoc(endToken);

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
        auto elifClause = std::make_shared<hir::IfStmt>();
        
        const Token elifToken = consume(TokenType::ELIF);
        elifClause->setBeginLoc(elifToken);
    
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
    auto forStmt = std::make_shared<hir::ForStmt>();

    const Token forToken = consume(TokenType::FOR);
    forStmt->setBeginLoc(forToken);
    
    forStmt->loopVar = parseIdent();
    consume(TokenType::IN);
    forStmt->domain = parseForDomain();
    forStmt->body = parseStmtBlock();

    const Token endToken = consume(TokenType::BLOCKEND);
    forStmt->setEndLoc(endToken);

    return forStmt;
  } catch (const SyntaxError &) {
    skipTo({TokenType::BLOCKEND});
    return hir::ForStmt::Ptr();
  }
}

hir::ForDomain::Ptr ParserNew::parseForDomain() {
  if (peek().type == TokenType::IDENT && peek(1).type != TokenType::COL) {
    auto setIndexSet = std::make_shared<hir::SetIndexSet>();
    
    const Token identToken = consume(TokenType::IDENT);
    setIndexSet->setLoc(identToken);
    setIndexSet->setName = identToken.str;
    
    auto indexSetDomain = std::make_shared<hir::IndexSetDomain>();
    indexSetDomain->set = setIndexSet;
    
    return indexSetDomain;
  }

  auto rangeDomain = std::make_shared<hir::RangeDomain>();

  rangeDomain->lower = parseExpr();
  consume(TokenType::COL);
  rangeDomain->upper = parseExpr();
  
  return rangeDomain;
}

hir::PrintStmt::Ptr ParserNew::parsePrintStmt() {
  try {
    auto printStmt = std::make_shared<hir::PrintStmt>();
    
    const Token printToken = consume(TokenType::PRINT);
    printStmt->setBeginLoc(printToken);
    
    printStmt->expr = parseExpr();

    const Token endToken = consume(TokenType::SEMICOL);
    printStmt->setEndLoc(endToken);

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
      const hir::Expr::Ptr expr = parseExpr();
      
      switch (peek().type) {
        case TokenType::COMMA:
        case TokenType::ASSIGN:
        {
          auto assignStmt = std::make_shared<hir::AssignStmt>();
          
          assignStmt->lhs.push_back(expr);
          while (tryconsume(TokenType::COMMA)) {
            const hir::Expr::Ptr expr = parseExpr();
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
    
    const Token endToken = consume(TokenType::SEMICOL);
    if (stmt) {
      stmt->setEndLoc(endToken);
    }

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

  const Token mapToken = consume(TokenType::MAP);
  mapExpr->setBeginLoc(mapToken);
 
  mapExpr->func = parseIdent();
  if (peek().type == TokenType::LP) {
    mapExpr->partialActuals = parseCallParams();
  }
  
  consume(TokenType::TO);
  mapExpr->target = parseIdent();
  
  if (peek().type == TokenType::REDUCE) {
    consume(TokenType::REDUCE);
    
    const Token plusToken = consume(TokenType::PLUS);
    mapExpr->setEndLoc(plusToken);

    mapExpr->op = hir::MapExpr::ReductionOp::SUM;
  } else {
    mapExpr->op = hir::MapExpr::ReductionOp::NONE;
  }

  return mapExpr;
}

hir::Expr::Ptr ParserNew::parseOrExpr() {
  hir::Expr::Ptr expr = parseAndExpr(); 

  while (tryconsume(TokenType::OR)) {
    auto orExpr = std::make_shared<hir::OrExpr>();
    
    orExpr->lhs = expr;
    orExpr->rhs = parseAndExpr();
    
    expr = orExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseAndExpr() {
  hir::Expr::Ptr expr = parseXorExpr(); 

  while (tryconsume(TokenType::AND)) {
    auto andExpr = std::make_shared<hir::AndExpr>();
    
    andExpr->lhs = expr;
    andExpr->rhs = parseXorExpr();
    
    expr = andExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseXorExpr() {
  hir::Expr::Ptr expr = parseEqExpr(); 

  while (tryconsume(TokenType::XOR)) {
    auto xorExpr = std::make_shared<hir::XorExpr>();
    
    xorExpr->lhs = expr;
    xorExpr->rhs = parseEqExpr();
    
    expr = xorExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseEqExpr() {
  auto expr = std::make_shared<hir::EqExpr>();

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

    const Token notToken = consume(TokenType::NOT);
    notExpr->setBeginLoc(notToken);
    notExpr->operand = parseTerm();

    return notExpr;
  }

  return parseSolveExpr();
}

hir::Expr::Ptr ParserNew::parseSolveExpr() {
  hir::Expr::Ptr expr = parseAddExpr();
  // TODO: Implement.
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
        
    mulExpr->lhs = expr;
    mulExpr->rhs = parseNegExpr();
    
    expr = mulExpr;
  }
}

hir::Expr::Ptr ParserNew::parseNegExpr() {
  auto negExpr = std::make_shared<hir::NegExpr>();

  switch (peek().type) {
    case TokenType::MINUS:
    {
      const Token minusToken = consume(TokenType::MINUS);
      negExpr->setBeginLoc(minusToken);
      negExpr->negate = true;
      break;
    }
    case TokenType::PLUS:
    {
      const Token plusToken = consume(TokenType::PLUS);
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

hir::Expr::Ptr ParserNew::parseExpExpr() {
  hir::Expr::Ptr expr = parseTransposeExpr();

  if (tryconsume(TokenType::EXP)) {
    auto expExpr = std::make_shared<hir::ExpExpr>();
    
    expExpr->lhs = expr;
    expExpr->rhs = parseExpExpr();
    
    expr = expExpr;
  }

  return expr;
}

hir::Expr::Ptr ParserNew::parseTransposeExpr() {
  hir::Expr::Ptr expr = parseCallOrReadExpr();

  while (peek().type == TokenType::TRANSPOSE) {
    auto transposeExpr = std::make_shared<hir::TransposeExpr>();
    
    const Token transposeToken = consume(TokenType::TRANSPOSE);
    transposeExpr->setEndLoc(transposeToken);
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
        
        consume(TokenType::LP);
        tensorRead->tensor = expr;
        tensorRead->indices = parseReadParams();
        
        const Token rightParenToken = consume(TokenType::RP);
        tensorRead->setEndLoc(rightParenToken);

        expr = tensorRead;
        break;
      }
      case TokenType::PERIOD:
      {
        auto fieldRead = std::make_shared<hir::FieldReadExpr>();
        
        consume(TokenType::PERIOD);
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

hir::Expr::Ptr ParserNew::parseFactor() {
  hir::Expr::Ptr factor;
  switch (peek().type) {
    case TokenType::LP:
    {
      auto parenExpr = std::make_shared<hir::ParenExpr>();

      const Token leftParenToken = consume(TokenType::LP);
      parenExpr->setBeginLoc(leftParenToken);
      
      parenExpr->expr = parseExpr();
      
      const Token rightParenToken = consume(TokenType::RP);
      parenExpr->setEndLoc(rightParenToken);

      factor = parenExpr;
      break;
    }
    case TokenType::IDENT:
    {
      auto var = std::make_shared<hir::VarExpr>(); 
      
      const Token identToken = consume(TokenType::IDENT);
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

  do {
    const hir::ReadParam::Ptr param = parseReadParam();
    readParams.push_back(param);
  } while (tryconsume(TokenType::COMMA));

  return readParams;
}

hir::ReadParam::Ptr ParserNew::parseReadParam() {
  if (peek().type == TokenType::COL) {
    auto slice = std::make_shared<hir::Slice>();
    
    const Token colToken = consume(TokenType::COL);
    slice->setLoc(colToken);
    
    return slice;
  }

  auto param = std::make_shared<hir::ExprParam>();
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

  const Token setToken = consume(TokenType::SET);
  setType->setBeginLoc(setToken);

  consume(TokenType::LC);
  setType->element = parseElementType();
  
  const Token rightCurlyToken = consume(TokenType::RC);
  setType->setEndLoc(rightCurlyToken);
  
  if (tryconsume(TokenType::LP)) {
    setType->endpoints = parseEndpoints();

    const Token rightParenToken = consume(TokenType::RP);
    setType->setEndLoc(rightParenToken);
  }

  return setType;
}

std::vector<hir::Endpoint::Ptr> ParserNew::parseEndpoints() {
  std::vector<hir::Endpoint::Ptr> endpoints;
  
  do {
    auto endpoint = std::make_shared<hir::Endpoint>();
    
    const Token endpointToken = consume(TokenType::IDENT);
    endpoint->setLoc(endpointToken);
    endpoint->setName = endpointToken.str;
    
    endpoints.push_back(endpoint);
  } while (tryconsume(TokenType::COMMA));

  return endpoints;
}

hir::TupleLength::Ptr ParserNew::parseTupleLength() {
  auto tupleLength = std::make_shared<hir::TupleLength>();

  const Token intToken = consume(TokenType::INT_LITERAL);
  tupleLength->setLoc(intToken);
  tupleLength->val = intToken.num;

  return tupleLength;
}

hir::TupleType::Ptr ParserNew::parseTupleType() {
  auto tupleType = std::make_shared<hir::TupleType>();

  const Token leftParenToken = consume(TokenType::LP);
  tupleType->setBeginLoc(leftParenToken);
  
  tupleType->element = parseElementType();
  consume(TokenType::STAR);
  tupleType->length = parseTupleLength();
  
  const Token rightParenToken = consume(TokenType::RP);
  tupleType->setEndLoc(rightParenToken);
  
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
    case TokenType::TENSOR:
    {
      const auto nonScalarTensorType = 
        hir::to<hir::NonScalarTensorType>(tensorType);
      
      consume(TokenType::TENSOR);
      if (tryconsume(TokenType::LB)) {
        nonScalarTensorType->indexSets = parseIndexSets();
        consume(TokenType::RB);
      }
      consume(TokenType::LP);
      nonScalarTensorType->blockType = parseTensorType();
      
      const Token rightParenToken = consume(TokenType::RP);
      nonScalarTensorType->setEndLoc(rightParenToken);
  
      if (peek().type == TokenType::TRANSPOSE) {
        const Token transposeToken = consume(TokenType::TRANSPOSE);
        nonScalarTensorType->setEndLoc(transposeToken);

        nonScalarTensorType->transposed = true;
      } else {
        nonScalarTensorType->transposed = false;
      }
      break;
    }
    case TokenType::STRING:
      // TODO: Implement.
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
      auto rangeIndexSet = std::make_shared<hir::RangeIndexSet>();
      
      const Token intToken = consume(TokenType::INT_LITERAL);
      rangeIndexSet->setLoc(intToken);
      rangeIndexSet->range = intToken.num;
      
      indexSet = rangeIndexSet;
      break;
    }
    case TokenType::IDENT:
    {
      auto setIndexSet = std::make_shared<hir::SetIndexSet>();
      
      const Token identToken = consume(TokenType::IDENT);
      setIndexSet->setLoc(identToken);
      setIndexSet->setName = identToken.str;
      
      indexSet = setIndexSet;
      break;
    }
    case TokenType::STAR:
    {
      indexSet = std::make_shared<hir::DynamicIndexSet>();
      
      const Token starToken = consume(TokenType::STAR);
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
    {
      auto intLiteral = std::make_shared<hir::IntLiteral>();
      
      const Token intToken = consume(TokenType::INT_LITERAL);
      intLiteral->setLoc(intToken);
      intLiteral->val = intToken.num;
      
      literal = intLiteral;
      break;
    }
    case TokenType::FLOAT_LITERAL:
    {
      auto floatLiteral = std::make_shared<hir::FloatLiteral>();
      
      const Token floatToken = consume(TokenType::FLOAT_LITERAL);
      floatLiteral->setLoc(floatToken);
      floatLiteral->val = floatToken.fnum;
      
      literal = floatLiteral;
      break;
    }
    case TokenType::TRUE:
    {
      auto trueLiteral = std::make_shared<hir::BoolLiteral>();
      
      const Token trueToken = consume(TokenType::TRUE);
      trueLiteral->setLoc(trueToken);
      trueLiteral->val = true;
      
      literal = trueLiteral;
      break;
    }
    case TokenType::FALSE:
    {
      auto falseLiteral = std::make_shared<hir::BoolLiteral>();
      
      const Token falseToken = consume(TokenType::FALSE);
      falseLiteral->setLoc(falseToken);
      falseLiteral->val = false;
      
      literal = falseLiteral;
      break;
    }
    case TokenType::LB:
    {
      const auto tensorLiteral = std::make_shared<hir::DenseTensorLiteral>();
      
      tensorLiteral->tensor = parseDenseTensorLiteral();
      tensorLiteral->transposed = false;
      
      literal = tensorLiteral;
      break;
    }
    case TokenType::STRING_LITERAL:
      // TODO: Implement.
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return literal;
}

hir::DenseTensorElement::Ptr ParserNew::parseDenseTensorLiteral() {
  const Token leftBracketToken = consume(TokenType::LB);
  hir::DenseTensorElement::Ptr tensor = parseDenseTensorLiteralInner();
  const Token rightBracketToken = consume(TokenType::RB);
  
  tensor->setBeginLoc(leftBracketToken);
  tensor->setEndLoc(rightBracketToken);

  return tensor;
}

hir::DenseTensorElement::Ptr ParserNew::parseDenseTensorLiteralInner() {
  if (peek().type == TokenType::LB) {
    auto tensor = std::make_shared<hir::DenseNDTensor>();

    hir::DenseTensorElement::Ptr elem = parseDenseTensorLiteral();
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

hir::DenseTensorElement::Ptr ParserNew::parseDenseMatrixLiteral() {
  auto mat = std::make_shared<hir::DenseNDTensor>();
  
  do {
    const hir::DenseTensorElement::Ptr vec = parseDenseVectorLiteral();
    mat->elems.push_back(vec);
  } while (tryconsume(TokenType::SEMICOL));

  return (mat->elems.size() == 1) ? mat->elems[0] : mat;
}

hir::DenseTensorElement::Ptr ParserNew::parseDenseVectorLiteral() {
  hir::DenseTensorElement::Ptr vec;
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

hir::DenseIntVector::Ptr ParserNew::parseDenseIntVectorLiteral() {
  auto vec = std::make_shared<hir::DenseIntVector>();

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

hir::DenseFloatVector::Ptr ParserNew::parseDenseFloatVectorLiteral() {
  auto vec = std::make_shared<hir::DenseFloatVector>();

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

  const Token testToken = consume(TokenType::TEST);
  test->setBeginLoc(testToken);

  test->func = parseIdent();
  switch (peek().type) {
    case TokenType::LP:
    {
      test->args = parseCallParams();
      consume(TokenType::EQ);
      test->expected = parseExpr();
      
      const Token endToken = consume(TokenType::SEMICOL);
      test->setEndLoc(endToken);
      break;
    }
    case TokenType::ASSIGN:
      // TODO: Implement.
    default:
      reportError(peek(), "unexpected symbol: "); // TODO: print symbol
      throw SyntaxError();
      break;
  }

  return test;
}

hir::Identifier::Ptr ParserNew::parseIdent() {
  auto ident = std::make_shared<hir::Identifier>();
  
  const Token identToken = consume(TokenType::IDENT);
  ident->setLoc(identToken);
  ident->ident = identToken.str;
  
  return ident;
}

}
}

