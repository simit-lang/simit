#include "parser.h"
#include "scanner.h"

namespace simit { 
namespace internal {

using namespace simit::ir;

#if 1
void ParserNew::parse(const TokenList &tokens, ProgramContext *ctx, 
                      std::vector<ParseError> *errors) {
  this->tokens = tokens;
  this->ctx = ctx;
  this->errors = errors;
  
  parseProgram();
}

void ParserNew::parseProgram() {
  while (peek().type != TokenType::END) {
    parseProgramElement();
  }
}

void ParserNew::parseProgramElement() {
  switch (peek().type) {
    case TokenType::TEST:
      parseTest();
      break;
    case TokenType::FUNC:
      parseFunction();
      break;
    case TokenType::PROC:
      parseProcedure();
      break;
    case TokenType::ELEMENT:
      parseElementTypeDecl();
      break;
    case TokenType::EXTERN:
      parseExternDecl();
      break;
    case TokenType::CONST:
      parseConstDecl();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseElementTypeDecl() {
  consume(TokenType::ELEMENT);
  
  const Token elementToken = consume(TokenType::IDENT);
  const std::string name = elementToken.str;

  std::vector<Field> fields = parseFieldDeclList();

  consume(TokenType::BLOCKEND);
  
  if (ctx->containsElementType(name)) {
    // TODO: raise error
    //REPORT_ERROR("struct redefinition (" + name + ")", @element_type_decl);
  }

  ctx->addElementType(ElementType::make(name, fields));
}

std::vector<Field> ParserNew::parseFieldDeclList() {
  std::vector<Field> fields;

  while (peek().type == TokenType::IDENT) {
    const Field field = parseFieldDecl();
    fields.push_back(field);
  }

  return fields;
}

Field ParserNew::parseFieldDecl() {
  const Token fieldToken = consume(TokenType::IDENT);
  const std::string name = fieldToken.str;
  
  consume(TokenType::COL);
  
  const Type type = parseTensorType();
  
  consume(TokenType::SEMICOL);

  return Field(name, type);
}

void ParserNew::parseExternDecl() {
  consume(TokenType::EXTERN);
  
  const Argument externVar = parseArgumentDecl();
  
  consume(TokenType::SEMICOL);
  
  ctx->addExtern(externVar.var);
  ctx->addSymbol(externVar.var);
}

void ParserNew::parseFunction() {
  consume(TokenType::FUNC);

  const Token identToken = consume(TokenType::IDENT);
  const std::string name = identToken.str;
 
  ctx->scope();
  const Func header = parseArgsAndResults();
  const Stmt body = parseStmtBlock();
  ctx->unscope();
  
  consume(TokenType::BLOCKEND);

  if (ctx->containsFunction(name)) {
    // TODO: raise error
    //  REPORT_ERROR("function redefinition (" + name + ")", @function);
  }

  const Func newFunc = Func(name, header.getArguments(), 
                                    header.getResults(), body);
  ctx->addFunction(newFunc);
}

void ParserNew::parseProcedure() {
  consume(TokenType::PROC);

  const Token identToken = consume(TokenType::IDENT);
  const std::string name = identToken.str;
  if (ctx->containsFunction(name)) {
    // TODO: raise error
    //  REPORT_ERROR("function redefinition (" + name + ")", @function);
  }

  std::vector<Var> arguments;
  std::vector<Var> results;

  ctx->scope();
  if (peek().type == TokenType::LP) {
    switch (peek(1).type) {
      case TokenType::VAR:
      case TokenType::INOUT:
      {
        const Func header = parseArgsAndResults();
        arguments = header.getArguments();
        results = header.getResults();
        break;
      }
      default:
        break;
    }
  }

  const Stmt body = parseStmtBlock();
  ctx->unscope();

  consume(TokenType::BLOCKEND);
    
  for (auto &extPair : ctx->getExterns()) {
    Var ext = ctx->getExtern(extPair.first);
    arguments.push_back(ext);
  }

  const Func newProc = Func(name, arguments, results, body);
  ctx->addFunction(newProc);
}

Func ParserNew::parseArgsAndResults() {
  consume(TokenType::LP);
  
  const std::vector<Argument> arguments = parseArguments();
  std::vector<Var> args;
  for (const Argument &arg : arguments) {
    const Var var = arg.var;
    auto access = arg.isInout ? Symbol::ReadWrite : Symbol::Read;
    ctx->addSymbol(var.getName(), var, access);
    args.push_back(var);
  }

  consume(TokenType::RP);
  
  const std::vector<Var> results = parseResults();
  for (const Var &res : results) {
    ctx->addSymbol(res.getName(), res, Symbol::ReadWrite);
  }
    
  return Func("", args, results, Stmt());
}

std::vector<ParserNew::Argument> ParserNew::parseArguments() {
  std::vector<Argument> arguments;

  const Argument argument = parseArgumentDecl();
  arguments.push_back(argument);
  
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);

    const Argument argument = parseArgumentDecl();
    arguments.push_back(argument);
  }

  return arguments;
}

ParserNew::Argument ParserNew::parseArgumentDecl() {
  bool isInout = false;

  if (peek().type == TokenType::INOUT) {
    consume(TokenType::INOUT);
    isInout = true;
  }

  Var var = parseIdentDecl();
  return Argument(var, isInout); 
}

std::vector<Var> ParserNew::parseResults() {
  std::vector<Var> results;

  if (peek().type == TokenType::RARROW) {
    consume(TokenType::RARROW);
    consume(TokenType::LP);
    
    const Var result = parseIdentDecl();
    results.push_back(result);

    while (peek().type == TokenType::COMMA) {
      consume(TokenType::COMMA);
      
      const Var result = parseIdentDecl();
      results.push_back(result);
    }

    consume(TokenType::RP);
  }

  return results;
}

Stmt ParserNew::parseStmtBlock() {
  while (true) {
    switch (peek().type) {
      case TokenType::BLOCKEND:
      case TokenType::UNTIL:
      case TokenType::ELIF:
      case TokenType::ELSE:
      case TokenType::END:
      {
        const std::vector<Stmt> *stmts = ctx->getStatements();
        const Stmt stmtBlock = (stmts->size() == 0) ? Pass::make() : 
                                   Block::make(*stmts);
        return stmtBlock;
      }
      default:
        parseStmt();
        break;
    }
  }
}

void ParserNew::parseStmt() {
  switch (peek().type) {
    case TokenType::VAR:
      parseVarDecl();
      break;
    case TokenType::CONST:
      parseConstDecl();
      break;
    case TokenType::IF:
      parseIfStmt();
      break;
    case TokenType::WHILE:
      parseWhileStmt();
      break;
    case TokenType::DO:
      parseDoUntilStmt();
      break;
    case TokenType::FOR:
      parseForStmt();
      break;
    case TokenType::PRINT:
      parsePrintStmt();
      break;
    default:
      parseExprOrAssignStmt();
      break;
  }
}

void ParserNew::parseVarDecl() {
  consume(TokenType::VAR);
  
  const Var var = parseIdentDecl();

  CallMap calls;
  Expr initVal;
  if (peek().type == TokenType::ASSIGN) {
    consume(TokenType::ASSIGN);
    
    initVal = parseExpr(calls);
  }

  consume(TokenType::SEMICOL);

  if (ctx->hasSymbol(var.getName())) {
    // TODO: raise error
    //REPORT_ERROR("variable redeclaration", @var_decl);
  }

  ctx->addSymbol(var.getName(), var, Symbol::ReadWrite);
  ctx->addStatement(VarDecl::make(var));

  if (initVal.defined()) {
    emitAssign({VarExpr::make(var)}, initVal, calls);
  }
}

void ParserNew::parseConstDecl() {
  consume(TokenType::CONST);

  const Var var = parseIdentDecl();
  
  consume(TokenType::ASSIGN);
  
  // TODO: support initialization of constants at runtime?
  const Expr literalExpr = parseTensorLiteral();
  
  consume(TokenType::SEMICOL);

  if (ctx->hasSymbol(var.getName())) {
    // TODO: raise error?
    //REPORT_ERROR("variable redeclaration", @var_decl);
  }

  const Type type = var.getType();
  const auto *tensorType = type.toTensor();

  // TODO: raise error?
  //iassert(literalExpr.type().isTensor())
  //    << "Only tensor literals are currently supported";
  auto litType = literalExpr.type();

  // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
  // cast $tensor_literal to a 1xn matrix.
  const auto *litTensorType = litType.toTensor();
  if (tensorType->order() == 2 && litTensorType->order() == 1) {
    const_cast<Literal*>(to<Literal>(literalExpr))->cast(type);
  }

  // TODO: check type equality
  // Typecheck: value and literal types must be equivalent.
  //CHECK_TYPE_EQUALITY(type, literalExpr.type(), @2);

  ctx->addConstant(var, literalExpr);
  ctx->addSymbol(var);
}

Var ParserNew::parseIdentDecl() {
  const Token varToken = consume(TokenType::IDENT);
  const std::string name = varToken.str;

  consume(TokenType::COL);
  
  const Type type = parseType();
  return Var(name, type);
}

void ParserNew::parseWhileStmt() {
  consume(TokenType::WHILE);
  
  CallMap calls;
  const Expr cond = parseExpr(calls);
  
  ctx->scope();
  const Stmt body = parseStmtBlock();
  ctx->unscope();

  consume(TokenType::BLOCKEND);
  
  checkValidSubexpr(cond, calls);
  checkValidReadExpr(cond);
  if (!isScalar(cond.type()) ||
      !cond.type().toTensor()->componentType.isBoolean()) {
    // TODO: raise error
    //REPORT_ERROR("conditional expression is not boolean", @expr);
  }

  const std::vector<Stmt> callStmts = liftCallsAndMaps(calls);
  const Stmt newBody = Block::make(body, Block::make(callStmts));
  ctx->addStatement(While::make(cond, newBody));
}

void ParserNew::parseDoUntilStmt() {
  // TODO: implement
  //consume(TokenType::DO);
  //parseStmtBlock();
  //consume(TokenType::UNTIL);
  //parseExpr();
}

void ParserNew::parseIfStmt() {
  consume(TokenType::IF);

  CallMap calls;
  const Expr cond = parseExpr(calls);
  
  ctx->scope();
  const Stmt ifBody = parseStmtBlock();
  ctx->unscope();

  const Stmt elseBody = parseElseClause();

  consume(TokenType::BLOCKEND);
  
  checkValidSubexpr(cond, calls);
  checkValidReadExpr(cond);

  liftCallsAndMaps(calls);
  ctx->addStatement(IfThenElse::make(cond, ifBody, elseBody));
}

Stmt ParserNew::parseElseClause() {
  ir::Stmt elseClause = Pass::make();

  switch (peek().type) {
    case TokenType::ELSE:
    {
      consume(TokenType::ELSE);
      
      ctx->scope();
      elseClause = parseStmtBlock();
      ctx->unscope();

      break;
    }
    case TokenType::ELIF:
    {
      consume(TokenType::ELIF);
      ctx->scope();

      CallMap calls;
      const Expr cond = parseExpr(calls);
  
      ctx->scope();
      const Stmt ifBody = parseStmtBlock();
      ctx->unscope();
      
      const Stmt elseBody = parseElseClause();

      checkValidSubexpr(cond, calls);
      checkValidReadExpr(cond);
      if (!isScalar(cond.type()) ||
          !cond.type().toTensor()->componentType.isBoolean()) {
        // TODO: raise error
        //  REPORT_ERROR("conditional expression is not boolean", @expr);
      }

      liftCallsAndMaps(calls);
      ctx->addStatement(IfThenElse::make(cond, ifBody, elseBody));
      const std::vector<Stmt> *stmts = ctx->getStatements();
      elseClause = Block::make(*stmts);

      ctx->unscope();
      break;
    }
    default:
      break;
  }

  return elseClause;
}

void ParserNew::parseForStmt() {
  consume(TokenType::FOR); 
  ctx->scope();

  const Token identToken = consume(TokenType::IDENT);
  const std::string loopVarName = identToken.str;
  const Var loopVar = Var(loopVarName, ir::Int);
  
  // If we need to write to loop variables, then that should be added as a
  // separate loop structure (that can't be vectorized easily)
  ctx->addSymbol(loopVarName, loopVar, Symbol::Read);

  consume(TokenType::IN);

  const Stmt range = parseForStmtRange();
  const Stmt body = parseStmtBlock();

  ctx->unscope();
  consume(TokenType::BLOCKEND);

  if (isa<For>(range)) {
    const For *forStmtRange = to<For>(range);
    const Stmt forDomain = For::make(loopVar, forStmtRange->domain, body);
    ctx->addStatement(forDomain);
  } else {
    //iassert(isa<ForRange>(range));
    const ForRange *forStmtRange = to<ForRange>(range); 
    const Stmt forRange = ForRange::make(loopVar, forStmtRange->start, 
                                         forStmtRange->end, body);
    ctx->addStatement(forRange);
  }
}

Stmt ParserNew::parseForStmtRange() {
  // index_set
  switch (peek().type) {
    case TokenType::INT_LITERAL:
    case TokenType::IDENT:
      if (peek(1).type == TokenType::COL) {
        break;
      }
    case TokenType::STAR:
    {
      IndexSet domain = parseIndexSet();
      if (domain.getKind() != IndexSet::Set) {
        // TODO: raise error?
      }

      const Stmt forRange = For::make(Var(), domain, Stmt());
      return forRange;
    }
    default:
      break;
  }

  CallMap lowerCalls;
  const Expr lower = parseExpr(lowerCalls);

  consume(TokenType::COL);

  CallMap upperCalls;
  const Expr upper = parseExpr(upperCalls);
  
  // TODO: check range is valid?
  checkValidSubexpr(lower, lowerCalls);
  checkValidSubexpr(upper, upperCalls);
  checkValidReadExpr(lower);
  checkValidReadExpr(upper);

  liftCallsAndMaps(lowerCalls);
  liftCallsAndMaps(upperCalls);
  const Stmt forRange = ForRange::make(Var(), lower, upper, Stmt());
  return forRange;
}

void ParserNew::parsePrintStmt() {
  consume(TokenType::PRINT);
  
  CallMap calls;
  const Expr expr = parseExpr(calls);

  checkValidSubexpr(expr, calls);
  checkValidReadExpr(expr);

  liftCallsAndMaps(calls);
  const Stmt printStmt = Print::make(expr);
  ctx->addStatement(printStmt);
}

void ParserNew::parseExprOrAssignStmt() {
  std::cout << "peek = " << peek() << std::endl;
  if (peek().type != TokenType::SEMICOL) {
    CallMap calls;
    std::vector<Expr> lhs;

    Expr expr = parseExpr(calls);
    switch (peek().type) {
      case TokenType::COMMA:
      case TokenType::ASSIGN:
      {
        lhs.push_back(expr);

        while (peek().type == TokenType::COMMA) {
          consume(TokenType::COMMA);
          
          expr = parseExpr(calls);
          lhs.push_back(expr);
        }

        consume(TokenType::ASSIGN);
        
        //iassert(calls.empty());
        expr = parseExpr(calls);
        break;
      }
      default:
        break;
    }

    emitAssign(lhs, expr, calls);
  }

  std::cout << "end = " << peek() << std::endl;
  consume(TokenType::SEMICOL);
}

Expr ParserNew::parseExpr(CallMap &calls) {
  if (peek().type == TokenType::MAP) {
    return parseMapExpr(calls);
  }

  return parseOrExpr(calls);
}

Expr ParserNew::parseMapExpr(CallMap &calls) {
  consume(TokenType::MAP);
  
  const Token funcToken = consume(TokenType::IDENT);
  const std::string funcName = funcToken.str;
 
  std::vector<Expr> partialActuals;
  if (peek().type == TokenType::LP) {
    partialActuals = parseOptionalExprList(calls); 
  }

  consume(TokenType::TO);
  
  const Token targetToken = consume(TokenType::IDENT);
  const std::string targetName = targetToken.str;
  
  ReductionOperator reduction = ReductionOperator::Undefined;
  if (peek().type == TokenType::REDUCE) {
    consume(TokenType::REDUCE);
    consume(TokenType::PLUS);

    reduction = ReductionOperator::Sum;
  }
    
  if (!ctx->containsFunction(funcName)) {
    // TODO: raise error
    //REPORT_ERROR("undefined function '" + funcName + "'", @func);
  }
  const Func func = ctx->getFunction(funcName);
  const std::vector<Var> results = func.getResults();

  if (!ctx->hasSymbol(targetName)) {
    // TODO: raise error
    //REPORT_ERROR("undefined set '" + targetName + "'", @target);
  }
  const Expr target = ctx->getSymbol(targetName).getExpr();

  if (!target.type().isSet()) {
    // TODO: raise error
    //REPORT_ERROR("maps can only be applied to sets", @target);
  }

  // Get endpoints
  std::vector<Expr> endpoints;
  for (Expr *endpoint : target.type().toSet()->endpointSets) {
    endpoints.push_back(*endpoint);
  }

  const size_t numFormals = func.getArguments().size();
  const size_t numActuals = partialActuals.size() + 1 + 
                            (endpoints.size() >0 ? 1 : 0);
  if (!(numFormals == numActuals ||
        (endpoints.size() > 0 && numFormals == numActuals - 1))) {
    // TODO: raise error
    //REPORT_ERROR("the number of actuals (" + to_string(numActuals) +
    //             ") does not match the number of formals accepted by " +
    //             func.getName() + " (" +
    //             to_string(func.getArguments().size()) + ")", @1);
  }

  if (target.type().toSet()->elementType != 
      func.getArguments()[partialActuals.size()].getType()){
    // TODO: raise error
    //REPORT_ERROR("the mapped set's element type is different from the mapped "
    //             "function's target argument", @target);
  }

  // We assume the edge set is homogeneous for now
  const Expr endpoint = (endpoints.size() > 0) ? endpoints[0] : Expr();

  const Type type = (results.size() == 1) ? 
                        results[0].getType() : Type();
  const Var tmp = ctx->getBuilder()->temporary(type);;
  const CallMap::Entry callEntry = CallMap::Entry::make(func, partialActuals, 
                                                        target, endpoint,
                                                        reduction);
  calls.add(tmp, callEntry);
  return Expr(VarExpr::make(tmp)); 
}

Expr ParserNew::parseOrExpr(CallMap &calls) {
  Expr expr = parseAndExpr(calls);

  while (peek().type == TokenType::OR) {
    consume(TokenType::OR);
    
    const Expr rhs = parseAndExpr(calls);

    checkValidSubexpr(expr, calls);
    checkValidSubexpr(rhs, calls);
    // TODO: undeclared var use probably needs to be checked here as well
    // TODO: check operands are booleans

    expr = Expr(Or::make(expr, rhs));
  }

  return expr;
}

Expr ParserNew::parseAndExpr(CallMap &calls) {
  Expr expr = parseXorExpr(calls);

  while (peek().type == TokenType::AND) {
    consume(TokenType::AND);
    
    const Expr rhs = parseXorExpr(calls);
    
    checkValidSubexpr(expr, calls);
    checkValidSubexpr(rhs, calls);
    // TODO: undeclared var use probably needs to be checked here as well
    // TODO: check operands are booleans

    expr = Expr(And::make(expr, rhs));
  }

  return expr;
}

Expr ParserNew::parseXorExpr(CallMap &calls) {
  Expr expr = parseEqExpr(calls);

  while (peek().type == TokenType::XOR) {
    consume(TokenType::XOR);
    
    const Expr rhs = parseEqExpr(calls);
    
    checkValidSubexpr(expr, calls);
    checkValidSubexpr(rhs, calls);
    // TODO: undeclared var use probably needs to be checked here as well
    // TODO: check operands are booleans

    expr = Expr(Xor::make(expr, rhs));
  }

  return expr;
}

Expr ParserNew::parseEqExpr(CallMap &calls) {
  Expr expr = parseIneqExpr(calls);

  switch (peek().type) {
    case TokenType::EQ:
    {
      consume(TokenType::EQ);
      
      const Expr rhs = parseIneqExpr(calls);
    
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Eq::make(expr, rhs));
      break;
    }
    case TokenType::NE:
    {
      consume(TokenType::NE);
      
      const Expr rhs = parseIneqExpr(calls);
      
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Ne::make(expr, rhs));
      break;
    }
    default:
      break;
  }

  return expr;
}

Expr ParserNew::parseIneqExpr(CallMap &calls) {
  Expr expr = parseBooleanFactor(calls);

  switch (peek().type) {
    case TokenType::RA:
    {
      consume(TokenType::RA);

      const Expr rhs = parseBooleanFactor(calls);
      
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Gt::make(expr, rhs));
      break;
    }
    case TokenType::LA:
    {
      consume(TokenType::LA);
      
      const Expr rhs = parseBooleanFactor(calls);
      
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Lt::make(expr, rhs));
      break;
    }
    case TokenType::LE:
    {
      consume(TokenType::LE);
      
      const Expr rhs = parseBooleanFactor(calls);
      
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Le::make(expr, rhs));
      break;
    }
    case TokenType::GE:
    {
      consume(TokenType::GE);
      
      const Expr rhs = parseBooleanFactor(calls);
      
      checkValidSubexpr(expr, calls);
      checkValidSubexpr(rhs, calls);
      // TODO: undeclared var use probably needs to be checked here as well
      // TODO: type equality needs to be checked here

      expr = Expr(Ge::make(expr, rhs));
      break;
    }
    default:
      break;
  }

  return expr;
}

Expr ParserNew::parseBooleanFactor(CallMap &calls) {
  if (peek().type == TokenType::NOT) {
    consume(TokenType::NOT);
    
    const Expr expr = parseBooleanFactor(calls);

    checkValidSubexpr(expr, calls);
    // TODO: undeclared var use probably needs to be checked here as well
    // TODO: check operand is boolean
    
    return Expr(Not::make(expr));
  }

  return parseSolveExpr(calls);
}

Expr ParserNew::parseSolveExpr(CallMap &calls) {
  Expr expr = parseAddExpr(calls);

  while (peek().type == TokenType::BACKSLASH) {
    // TODO: raise unsupported error?
    consume(TokenType::BACKSLASH);
    parseAddExpr(calls);
  }

  return expr;
}

Expr ParserNew::parseAddExpr(CallMap &calls) {
  std::cout << "first = " << peek() << std::endl;
  Expr expr = parseMulExpr(calls);
  std::cout << "next' = " << peek() << std::endl;

  while (true) {
    switch (peek().type) {
      case TokenType::PLUS:
      {
        consume(TokenType::PLUS);
       
        std::cout << "next = " << peek() << std::endl;
        const Expr rhs = parseMulExpr(calls);
      
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);

        expr = ctx->getBuilder()->binaryElwiseExpr(expr, IRBuilder::Add, rhs);
        break;
      }
      case TokenType::MINUS:
      {
        consume(TokenType::MINUS);
        
        const Expr rhs = parseMulExpr(calls);
      
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);

        expr = ctx->getBuilder()->binaryElwiseExpr(expr, IRBuilder::Sub, rhs);
        break;
      }
      default:
        return expr;
    }
  }
}

Expr ParserNew::parseMulExpr(CallMap &calls) {
  Expr expr = parseNegExpr(calls);
  IRBuilder *builder = ctx->getBuilder();

  while (true) {
    switch (peek().type) {
      case TokenType::STAR:
      {
        consume(TokenType::STAR);
    
        const Expr rhs = parseNegExpr(calls);
        
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);
    
        const auto ltype = expr.type().toTensor();
        const auto rtype = rhs.type().toTensor();
        const auto ldimensions = ltype->getDimensions();
        const auto rdimensions = rtype->getDimensions();
    
        if (ltype->order() == 0 || rtype->order() == 0) {
          // Scale
          expr = builder->binaryElwiseExpr(expr, IRBuilder::Mul, rhs);
        } else if (ltype->order() == 1 && rtype->order() == 1) {
          // Vector-Vector Multiplication (inner and outer product)
          if (!ltype->isColumnVector) {
            // Inner product
            if (!rtype->isColumnVector) {
              // TODO: raise error
              //REPORT_ERROR("cannot multiply two row vectors", @2);
            } else if (expr.type() != rhs.type()) {
              // TODO: raise error
              //REPORT_TYPE_MISSMATCH(l.type(), r.type(), @2);
            }
            
            expr = builder->innerProduct(expr, rhs);
          } else {
            // Outer product (l is a column vector)
            if (rtype->isColumnVector) {
              // TODO: raise error
              //REPORT_ERROR("cannot multiply two column vectors", @2);
            } else if (expr.type() != rhs.type()) {
              // TODO: raise error
              //REPORT_TYPE_MISSMATCH(l.type(), r.type(), @2);
            }

            expr = builder->outerProduct(expr, rhs);
          }
        } else if (ltype->order() == 2 && rtype->order() == 1) {
          // Matrix-Vector
          // TODO: Figure out how column vectors should be handled here
          if (ldimensions[1] != rdimensions[0]){
            // TODO: raise error
            //REPORT_TYPE_MISSMATCH(l.type(), r.type(), @2);
          }

          expr = builder->gemv(expr, rhs);
        } else if (ltype->order() == 1 && rtype->order() == 2) {
          // Vector-Matrix
          // TODO: Figure out how column vectors should be handled here
          if (ldimensions[0] != rdimensions[0]) {
            // TODO: raise error
            //REPORT_TYPE_MISSMATCH(l.type(), r.type(), @2);
          }

          expr = builder->gevm(expr, rhs);
        } else if (ltype->order() == 2 && rtype->order() == 2) {
          // Matrix-Matrix
          if (ldimensions[1] != rdimensions[0]){
            // TODO: raise error
            //REPORT_TYPE_MISSMATCH(l.type(), r.type(), @2);
          }

          expr = builder->gemm(expr, rhs);
        } else {
          // TODO: raise error
          // REPORT_ERROR("cannot multiply >2-order tensors using *", @2);
        }

        break;
      }
      case TokenType::SLASH:
      {
        consume(TokenType::SLASH);
        
        const Expr rhs = parseNegExpr(calls);
      
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);
        const auto ltype = expr.type().toTensor();
        const auto rtype = rhs.type().toTensor();
        if (ltype->order() > 0 && rtype->order() > 0) {
          // TODO: raise error
        }

        expr = builder->binaryElwiseExpr(expr, IRBuilder::Div, rhs);
        break;
      }
      case TokenType::DOTSTAR:
      {
        consume(TokenType::DOTSTAR);
        
        const Expr rhs = parseNegExpr(calls);
      
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);

        expr = builder->binaryElwiseExpr(expr, IRBuilder::Mul, rhs);
        break;
      }
      case TokenType::DOTSLASH:
      {
        consume(TokenType::DOTSLASH);
        
        const Expr rhs = parseNegExpr(calls);
      
        checkValidSubexpr(expr, calls);
        checkValidSubexpr(rhs, calls);
        // TODO: undeclared var use probably needs to be checked here as well
        //CHECK_IS_TENSOR(l, @1);
        //CHECK_IS_TENSOR(r, @3);

        expr = builder->binaryElwiseExpr(expr, IRBuilder::Div, rhs);
        break;
      }
      default:
        return expr;
    }
  }
}

Expr ParserNew::parseNegExpr(CallMap &calls) {
  bool neg = false;
  bool checkType = false;

  while (true) {
    switch (peek().type) {
      case TokenType::MINUS:
        consume(TokenType::MINUS);
        neg = !neg;
        checkType = true;
        break;
      case TokenType::PLUS:
        consume(TokenType::PLUS);
        checkType = true;
        break;
      default:
      {
        Expr expr = parseExpExpr(calls);

        if (checkType) {
          // TODO: CHECK_IS_TENSOR(expr, @2);
        }

        if (neg) {
          expr = ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr);
        }
        return expr;
      }
    }
  }
}

Expr ParserNew::parseExpExpr(CallMap &calls) {
  Expr expr = parseTransposeExpr(calls);

  while (peek().type == TokenType::EXP) {
    // TODO: raise unsupported exception?
    consume(TokenType::EXP);
    parseExpExpr(calls);
  }

  return expr;
}

Expr ParserNew::parseTransposeExpr(CallMap &calls) {
  IRBuilder *builder = ctx->getBuilder();
  Expr expr = parseCallOrReadExpr(calls);

  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);
    
    // TODO: CHECK_IS_TENSOR(expr, @2);
    
    const auto type = expr.type().toTensor();
    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        expr = builder->unaryElwiseExpr(IRBuilder::None, expr);
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        expr = builder->unaryElwiseExpr(IRBuilder::None, expr);
        if (!type->isColumnVector) {
          const Type transposedVector = ir::TensorType::make(
            type->componentType, type->getDimensions(), !type->isColumnVector);
          const_cast<ExprNodeBase*>(to<ExprNodeBase>(expr))->type = 
            transposedVector;
        }
        break;
      case 2:
        expr = builder->transposedMatrix(expr);
        break;
      default:
        // TODO: raise error
        //REPORT_ERROR("cannot transpose >2-order tensors using '", @1);
        break;
    }
  }

  return expr;
}

Expr ParserNew::parseCallOrReadExpr(CallMap &calls) {
  Expr expr;
  
  if (peek().type == TokenType::IDENT && ctx->containsFunction(peek().str)) {
    const Token funcToken = consume(TokenType::IDENT);
    const Func func = ctx->getFunction(funcToken.str);
    const std::vector<Var> results = func.getResults();

    const std::vector<Expr> arguments = parseOptionalExprList(calls);

    const Type type = (results.size() == 1) ? results[0].getType() : Type();
    const Var tmp = ctx->getBuilder()->temporary(type);
    const CallMap::Entry callEntry = CallMap::Entry::make(func, arguments);
    calls.add(tmp, callEntry);
    expr = Expr(VarExpr::make(tmp)); 
  } else {
    expr = parseFactor(calls);
  }

  while (true) {
    switch (peek().type) {
      case TokenType::LP:
      {
        checkValidSubexpr(expr, calls);
        
        const std::vector<Expr> indices = parseOptionalExprList(calls);

        // The parenthesis read can be a read from a tensor or a tuple.
        if (expr.type().isTensor()) {
          // check if we need to turn this into an index expression
         
          bool containsSlices = false;
          for (auto arg : indices) {
            if (isa<VarExpr>(arg) && 
                to<VarExpr>(arg)->var.getName() == ":") {
              containsSlices = true;
            }
          }

          if (containsSlices) {
            // We will construct an index expression.
            // first, we build IndexVars
            std::vector<IndexVar> allivars;
            std::vector<IndexVar> freeVars;
            std::vector<IndexDomain> dimensions =
                expr.type().toTensor()->getDimensions();

            unsigned int i = 0;
            for (auto &arg : indices) {
              if (isa<VarExpr>(arg) && 
                  to<VarExpr>(arg)->var.getName() == ":") {
                auto iv = IndexVar("tmpfree" + std::to_string(i),
                                       dimensions[i]);
                allivars.push_back(iv);
                freeVars.push_back(iv);
              } else {
                auto iv = IndexVar("tmpfixed" + std::to_string(i),
                                       dimensions[i], new Expr(arg));
                allivars.push_back(iv);
              }
              ++i;
            }

            // now construct an index expression
            expr = Expr(IndexExpr::make(freeVars,
                    IndexedTensor::make(expr, allivars)));
          } else {
            expr = Expr(TensorRead::make(expr, indices));
          }
        } else if (expr.type().isTuple()) {
          if (indices.size() != 1) {
            // TODO: raise error
            //REPORT_ERROR("reading a tuple requires exactly one index", @3);
          }
          expr = Expr(TupleRead::make(expr, indices[0]));
        } else {
          // TODO: raise error
          //REPORT_ERROR("can only access components in tensors and tuples", @1);
        }
        break;
      }
      case TokenType::PERIOD:
      {
        checkValidSubexpr(expr, calls);

        const Type type = expr.type();
        const ElementType *elemType = nullptr;
        if (expr.type().isElement()) {
          elemType = expr.type().toElement();
        } else if (expr.type().isSet()) {
          const SetType *setType = expr.type().toSet();
          elemType = setType->elementType.toElement();
        } else {
          // TODO: raise error
          //std::stringstream errorStr;
          //errorStr << "only elements and sets have fields";
          //REPORT_ERROR(errorStr.str(), @1);
        }

        consume(TokenType::PERIOD);
        const Token fieldToken = consume(TokenType::IDENT);
        const std::string fieldName = fieldToken.str;
        if (!elemType->hasField(fieldName)) {
          // TODO: raise error
          //REPORT_ERROR("undefined field '" + toString(elemOrSet)+"."+fieldName+ "'",
          //         @fieldName);
        }

        expr = Expr(FieldRead::make(expr, fieldName));
        break;
      }
      default:
        return expr;
    }
  }
}

Expr ParserNew::parseFactor(CallMap &calls) {
  Expr factor;

  switch (peek().type) {
    case TokenType::LP:
      consume(TokenType::LP);
      factor = parseExpr(calls);
      consume(TokenType::RP);
      break;
    case TokenType::IDENT:
    {
      const Token identToken = consume(TokenType::IDENT);
      const std::string ident = identToken.str;
      
      factor = ctx->hasSymbol(ident) ? 
               Expr(ctx->getSymbol(ident).getExpr()) :
               VarExpr::make(Var(ident, Type()));
      // TODO: only check after expression has been fully compiled, at use
      //if (!ctx->hasSymbol(ident)) {
        // TODO: raise error
        //REPORT_ERROR(ident + " is not defined in scope", @1);
      //}
      //if (!symbol.isReadable()) {
        // TODO: raise error
        //REPORT_ERROR(ident + " is not readable", @1);
      //}
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
      throw ParseException();
      break;
  }

  return factor;
}

std::vector<Expr> ParserNew::parseOptionalExprList(CallMap &calls) {
  if (peek(1).type == TokenType::RP) {
    consume(TokenType::LP);
    consume(TokenType::RP);
    
    return std::vector<Expr>();
  }

  consume(TokenType::LP);

  const std::vector<Expr> exprs = parseExprList(calls);
  
  consume(TokenType::RP);
  
  return exprs;
}

std::vector<Expr> ParserNew::parseExprList(CallMap &calls) {
  std::vector<Expr> exprs;

  const Expr expr = parseExprListElement(calls);
  exprs.push_back(expr);

  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    
    const Expr expr = parseExprListElement(calls);
    exprs.push_back(expr);
  }

  return exprs;
}

Expr ParserNew::parseExprListElement(CallMap &calls) {
  if (peek().type == TokenType::COL) {
    consume(TokenType::COL);

    return Expr(VarExpr::make(Var(":", Type())));
  }

  return parseExpr(calls);
}

Type ParserNew::parseType() {
  Type type;

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
      throw ParseException();
      break;
  }

  return type;
}

Type ParserNew::parseElementType() {
  const Token typeToken = consume(TokenType::IDENT);
  const std::string name = typeToken.str;

  if (!ctx->containsElementType(name)) {
    // TODO: raise error
    //REPORT_ERROR("undefined element type '" + name + "'" , @1);
  }

  return Type(ctx->getElementType(name));
}

Type ParserNew::parseSetType() {
  consume(TokenType::SET);
  consume(TokenType::LC);
  
  const Type elementType = parseElementType();
  
  consume(TokenType::RC);

  std::vector<Expr> endpoints;
  if (peek().type == TokenType::LP) {
    consume(TokenType::LP);
    
    endpoints = parseEndpoints();
    
    consume(TokenType::RP);
  }

  // TODO: Add endpoint information to set type
  return Type(SetType::make(elementType, endpoints));
}

std::vector<Expr> ParserNew::parseEndpoints() {
  std::vector<Expr> endpoints;
  
  const Token endpointToken = consume(TokenType::IDENT);
  const std::string name = endpointToken.str;
  if (!ctx->hasSymbol(name)) {
    // TODO: raise error
    //REPORT_ERROR("undefined set type '" + name + "'" , @1);
  }
  endpoints.push_back(ctx->getSymbol(name).getExpr());

  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    
    const Token endpointToken = consume(TokenType::IDENT);
    const std::string name = endpointToken.str;
    if (!ctx->hasSymbol(name)) {
      // TODO: raise error
      //REPORT_ERROR("undefined set type '" + name + "'" , @1);
    }
    endpoints.push_back(ctx->getSymbol(name).getExpr());
  }

  return endpoints;
}

Type ParserNew::parseTupleType() {
  consume(TokenType::LP);
  
  const Type elementType = parseElementType();
  
  consume(TokenType::STAR);
  
  const Token intToken = consume(TokenType::INT_LITERAL);
  
  consume(TokenType::RP);
  
  const int size = intToken.num;
  if (size < 1) {
    // TODO: raise error
    //REPORT_ERROR("Must be 1 or greater", @3);
  }

  return Type(TupleType::make(elementType, size));
}

Type ParserNew::parseTensorType() {
  Type type = parseTensorTypeStart();

  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);

    const auto tensorType = type.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->componentType;
    type = Type(ir::TensorType::make(componentType, dimensions, true));
  }

  return type;
}

Type ParserNew::parseTensorTypeStart() {
  ScalarType componentType;

  switch (peek().type) {
    case TokenType::INT:
      consume(TokenType::INT);
      componentType = ScalarType(ScalarType::Int);
      break;
    case TokenType::FLOAT:
      consume(TokenType::FLOAT);
      componentType = ScalarType(ScalarType::Float);
      break;
    case TokenType::BOOL:
      consume(TokenType::BOOL);
      componentType = ScalarType(ScalarType::Boolean);
      break;
    case TokenType::STRING:
      consume(TokenType::STRING);
      // TODO: raise error?
      return Type();
    case TokenType::TENSOR:
    {
      consume(TokenType::TENSOR);
      
      std::vector<IndexSet> indexSets;
      if (peek().type == TokenType::LB) {
        consume(TokenType::LB);
        indexSets = parseIndexSets();
        consume(TokenType::RB);
      }

      consume(TokenType::LP);
      const Type blockType = parseTensorType();
      consume(TokenType::RP);

      if (indexSets.empty()) {
        return blockType;
      } else {
        const auto blockTensorType = blockType.toTensor();
        const auto componentType = blockTensorType->componentType;
        const auto blockDimensions = blockTensorType->getDimensions();
    
        std::vector<IndexDomain> dimensions;
        if (blockTensorType->order() == 0) {
          for (size_t i = 0; i< indexSets.size(); ++i) {
            dimensions.push_back(IndexDomain(indexSets[i]));
          }
        } else {
          // TODO: Handle the following cases where there there are more inner than
          //       outer dimensions (e.g. a vector of matrixes) and where there are
          //       more outer than inner dimensions (e.g. a matrix of vectors)
          //       gracefully by padding with '1'-dimensions.
          // TODO: Handle case where there are more block than outer dimensions
          // TODO: Handle case where there are more outer than block dimensions
          // TODO: Remove below error
    //      if (blockType->order() != outerDimensions->size()) {
    //        REPORT_ERROR("Blocktype order (" + to_string(blockType->order()) +
    //                     ") differ from number of dimensions", @index_sets);
    //      }
    
    //      iassert(blockDimensions.size() == outerDimensions->size());
          for (size_t i = 0; i< indexSets.size(); ++i) {
            std::vector<IndexSet> dimension;
            dimension.push_back(indexSets[i]);
            dimension.insert(dimension.end(),
                             blockDimensions[i].getIndexSets().begin(),
                             blockDimensions[i].getIndexSets().end());
    
            dimensions.push_back(IndexDomain(dimension));
          }
        }
    
        return Type(ir::TensorType::make(componentType, dimensions));
      }
    }
    default:
      throw ParseException();
      return Type();
  }

  return Type(ir::TensorType::make(componentType));
}

std::vector<IndexSet> ParserNew::parseIndexSets() {
  std::vector<IndexSet> indexSets;

  const IndexSet indexSet = parseIndexSet();
  indexSets.push_back(indexSet);

  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    
    const IndexSet indexSet = parseIndexSet();
    indexSets.push_back(indexSet);
  }

  return indexSets;
}

IndexSet ParserNew::parseIndexSet() {
  switch (peek().type) {
    case TokenType::INT_LITERAL:
    {
      const Token intToken = consume(TokenType::INT_LITERAL);
      return IndexSet(intToken.num);
    }
    case TokenType::IDENT:
    {
      const Token identToken = consume(TokenType::IDENT);
      const std::string setName = identToken.str;
      if (!ctx->hasSymbol(setName)) {
        // TODO: raise error
        //REPORT_ERROR("the set has not been declared", @1);
      }

      const Expr set = ctx->getSymbol(setName).getExpr();
      if (!set.type().isSet()) {
        // TODO: raise error
        //REPORT_ERROR("an index set must be a set, a range or dynamic (*)", @1);
      }

      return IndexSet(set);
    }
    case TokenType::STAR:
      consume(TokenType::STAR);
      return IndexSet();
    default:
      throw ParseException();
      return IndexSet();
  }
}

Expr ParserNew::parseTensorLiteral() {
  Expr literal;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::PLUS:
    case TokenType::MINUS:
      literal = parseSignedNumLiteral();
      break;
    case TokenType::TRUE:
      consume(TokenType::TRUE);
      literal = Literal::make(true);
      break;
    case TokenType::FALSE:
      consume(TokenType::FALSE);
      literal = Literal::make(false);
      break;
    case TokenType::LB:
    {
      const TensorValues values = parseDenseTensorLiteral();
      const auto idoms = std::vector<IndexDomain>(values.dimSizes.rbegin(),
                                                  values.dimSizes.rend());
      const ScalarType elemType = (values.type == TensorValues::Type::INT) ?
                                  ScalarType::Int : ScalarType::Float;
      const Type type = ir::TensorType::make(elemType, idoms);
      const void *data = (values.type == TensorValues::Type::INT) ? 
                         static_cast<const void*>(values.intVals.data()) :
                         static_cast<const void*>(values.floatVals.data());
      literal = Literal::make(type, const_cast<void*>(data));

      if (peek().type == TokenType::TRANSPOSE) {
        consume(TokenType::TRANSPOSE);
    
        //iassert(vec.type().isTensor());
        const auto ttype = literal.type().toTensor();
        //iassert(ttype->order() == 1);
        const Type transposedVector = ir::TensorType::make(
          ttype->componentType, ttype->getDimensions(), !ttype->isColumnVector);
        const_cast<ExprNodeBase*>(to<ExprNodeBase>(literal))->type = 
          transposedVector;
      }
      break;
    }
    case TokenType::STRING_LITERAL:
      //consume(TokenType::STRING_LITERAL);
      //break;
    default:
      throw ParseException();
      break;
  }

  return literal;
}

Expr ParserNew::parseSignedNumLiteral() {
  Expr literal;

  switch (peek().type) {
    case TokenType::INT_LITERAL:
    {
      const int val = parseSignedIntLiteral();
      literal = Literal::make(val);
      break;
    }
    case TokenType::FLOAT_LITERAL:
    {
      const double val = parseSignedFloatLiteral();
      literal = Literal::make(val);
      break;
    }
    case TokenType::PLUS:
    case TokenType::MINUS:
      switch (peek(1).type) {
        case TokenType::INT_LITERAL:
        {
          const int val = parseSignedIntLiteral();
          literal = Literal::make(val);
          break;
        }
        case TokenType::FLOAT_LITERAL:
        {
          const double val = parseSignedFloatLiteral();
          literal = Literal::make(val);
          break;
        }
        default:
          throw ParseException();
          break;
      }
    default:
      throw ParseException();
      break;
  }

  return literal;
}

ParserNew::TensorValues ParserNew::parseDenseTensorLiteral() {
  consume(TokenType::LB);

  TensorValues tensor = parseDenseTensorLiteralInner();
  
  consume(TokenType::RB);
  
  //if (peek().type == TokenType::TRANSPOSE) {
  //  consume(TokenType::TRANSPOSE);
  //}
  return tensor;
}

ParserNew::TensorValues ParserNew::parseDenseTensorLiteralInner() {
  if (peek().type == TokenType::LB) {
    TensorValues tensor = parseDenseTensorLiteral();
    tensor.addDimension();
    
    while (true) {
      switch (peek().type) {
        case TokenType::COMMA:
          consume(TokenType::COMMA);
        case TokenType::LB:
        {
          const TensorValues right = parseDenseTensorLiteral();
          tensor.merge(right);
          break;
        }
        default:
          return tensor;
      }
    }
  }

  return parseDenseMatrixLiteral();
}

ParserNew::TensorValues ParserNew::parseDenseMatrixLiteral() {
  TensorValues mat = parseDenseVectorLiteral();
  mat.addDimension();
  
  while (peek().type == TokenType::SEMICOL) {
    consume(TokenType::SEMICOL);
    const TensorValues right = parseDenseVectorLiteral();
    mat.merge(right);
  }

  return mat;
}

ParserNew::TensorValues ParserNew::parseDenseVectorLiteral() {
  TensorValues vec;

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
          throw ParseException();
          break;
      }
    default:
      throw ParseException();
      break;
  }

  return vec;
}

ParserNew::TensorValues ParserNew::parseDenseIntVectorLiteral() {
  TensorValues vec;

  int elem = parseSignedIntLiteral();
  vec.addIntValue(elem);

  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::INT_LITERAL:
        elem = parseSignedIntLiteral();
        vec.addIntValue(elem);
        break;
      default:
        return vec;
    }
  }
}

ParserNew::TensorValues ParserNew::parseDenseFloatVectorLiteral() {
  TensorValues vec;

  double elem = parseSignedFloatLiteral();
  vec.addFloatValue(elem);

  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::FLOAT_LITERAL:
        elem = parseSignedFloatLiteral();
        vec.addFloatValue(elem);
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

void ParserNew::parseTest() {
  //consume(TokenType::TEST);
  //consume(TokenType::IDENT);
  //switch (peek().type) {
  //  case TokenType::LP:
  //    parseOptionalExprList();
  //    consume(TokenType::EQ);
  //    parseTensorLiteral();
  //    consume(TokenType::SEMICOL);
  //    break;
  //  case TokenType::ASSIGN:
  //    consume(TokenType::ASSIGN);
  //    parseSystemGenerator();
  //    consume(TokenType::COL);
  //    parseExternAssert();
  //    break;
  //  default:
  //    throw ParseException();
  //    break;
  //}
}

void ParserNew::parseSystemGenerator() {
  switch (peek().type) {
    case TokenType::IDENT:
      consume(TokenType::IDENT);
      break;
    case TokenType::INT_LITERAL:
      consume(TokenType::INT_LITERAL);
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseExternAssert() {
  consume(TokenType::IDENT);
  consume(TokenType::PERIOD);
  consume(TokenType::IDENT);
  consume(TokenType::ASSIGN);
  parseTensorLiteral();
  consume(TokenType::RARROW);
  parseTensorLiteral();
}

void ParserNew::checkValidSubexpr(const Expr expr, const CallMap &calls) {
  if (!isa<VarExpr>(expr)) return;
  if (!calls.exists(to<VarExpr>(expr)->var)) return;

  //const Stmt stmt = calls.get(to<VarExpr>(expr)->var);
  //if (isa<CallStmt>(stmt)) {
    //const CallStmt *callStmt = to<CallStmt>(stmt);
    // TODO: raise error if # results != 1
  //} else if (isa<Map>(stmt)) {
    //const Map *mapStmt = to<Map>(stmt);
    // TODO: raise error if # results != 1
  //}
}

void ParserNew::checkValidReadExpr(const Expr expr) {
  // TODO: check symbols readable
}

void ParserNew::checkValidWriteExpr(const Expr expr) {
           // if (!symbol.isWritable()) {
              // TODO: raise error
              //REPORT_ERROR(varName + " is constant", @idents);
           // }
}

void ParserNew::emitAssign(const std::vector<Expr> &lhs, Expr expr, 
                           const CallMap &calls) {
  const bool exprIsCallOrMap = isa<VarExpr>(expr) && 
                               calls.exists(to<VarExpr>(expr)->var);

  if (lhs.size() > 0 || exprIsCallOrMap) {
    liftCallsAndMaps(calls, expr);
  } else {
    liftCallsAndMaps(calls);
  }

  checkValidReadExpr(expr);
  for (unsigned int i = 0; i < lhs.size(); ++i) {
    checkValidWriteExpr(lhs[i]);
  }

  if (exprIsCallOrMap) {
    const CallMap::Entry callEntry = calls.get(to<VarExpr>(expr)->var);
    const std::vector<Var> retVals = callEntry.func.getResults(); 
   
    if (lhs.size() != retVals.size()) {
      // TODO: raise error
    }

    std::vector<Var> results;
    for (unsigned int i = 0; i < lhs.size(); ++i) {
      if (isa<VarExpr>(lhs[i])) {
        Var var = to<VarExpr>(lhs[i])->var;
        const std::string varName = var.getName();

        if (ctx->hasSymbol(varName)) {
          Symbol symbol = ctx->getSymbol(varName);
          // TODO: check type equality
          // CHECK_TYPE_EQUALITY(symbol.getVar().getType(), value.type(), @2);
        } else {
          var = Var(varName, retVals[i].getType());
          ctx->addSymbol(varName, var, Symbol::ReadWrite);
          ctx->addStatement(VarDecl::make(var));
        }

        results.push_back(var);
      } else {
        const Var tmp = ctx->getBuilder()->temporary(retVals[i].getType());
        ctx->addSymbol(tmp.getName(), tmp, Symbol::ReadWrite);
        ctx->addStatement(VarDecl::make(tmp));
        results.push_back(tmp);
      }
    }
      
    switch (callEntry.callType) {
      case CallMap::Entry::Type::FUNC:
      {
        const Stmt newCallStmt = CallStmt::make(results, callEntry.func, 
                                                callEntry.actuals);
        ctx->addStatement(newCallStmt);
        break;
      }
      case CallMap::Entry::Type::MAP:
      {
        const Stmt newMapStmt = Map::make(results, callEntry.func, 
                                          callEntry.actuals, callEntry.target,
                                          callEntry.neighbors, 
                                          callEntry.reduction);
        ctx->addStatement(newMapStmt);
        break;
      }
      default:
        break;
    }
    
    for (unsigned int i = 0; i < lhs.size(); ++i) {
      const Expr tmpVarExpr = VarExpr::make(results[i]);
      if (isa<FieldRead>(lhs[i])) {
        const FieldRead *fieldRead = to<FieldRead>(lhs[i]);
        // TODO: check equal types
        const Stmt fieldWrite = FieldWrite::make(fieldRead->elementOrSet, 
                                                 fieldRead->fieldName, 
                                                 tmpVarExpr);
        ctx->addStatement(fieldWrite);
      } else if (isa<TensorRead>(lhs[i])) {
        const TensorRead *tensorRead = to<TensorRead>(lhs[i]);
        // TODO: check equal types
        const Stmt tensorWrite = TensorWrite::make(tensorRead->tensor,
                                                   tensorRead->indices,
                                                   tmpVarExpr);
        ctx->addStatement(tensorWrite);
      }
    }
  } else {
    switch (lhs.size()) {
      case 0:
        break;
      case 1:
        if (isa<FieldRead>(lhs[0])) {
          const FieldRead *fieldRead = to<FieldRead>(lhs[0]);
          // TODO: check equal types
          const Stmt fieldWrite = FieldWrite::make(fieldRead->elementOrSet, 
                                                   fieldRead->fieldName, 
                                                   expr);
          ctx->addStatement(fieldWrite);
        } else if (isa<TensorRead>(lhs[0])) {
          const TensorRead *tensorRead = to<TensorRead>(lhs[0]);
          // TODO: check equal types
          const Stmt tensorWrite = TensorWrite::make(tensorRead->tensor, 
                                                     tensorRead->indices, 
                                                     expr);
          ctx->addStatement(tensorWrite);
        } else {
          // iassert(isa<VarExpr>(lhs[0]);
          Var var = to<VarExpr>(lhs[0])->var;
          const std::string varName = var.getName();
      
          if (ctx->hasSymbol(varName)) {
            Symbol symbol = ctx->getSymbol(varName);
            // TODO: check type equality
            //CHECK_TYPE_EQUALITY(symbol.getVar().getType(), value.type(), @expr);
          } else {
            var = Var(varName, expr.type());
            ctx->addSymbol(varName, var, Symbol::ReadWrite);
          }
      
          // TODO: This should be dealt with inside the ident_expr rule?
          if (isa<VarExpr>(expr) && expr.type().isTensor()) {
            // The statement assign a tensor to a tensor, so we change it to an
            // assignment index expr
            expr = ctx->getBuilder()->unaryElwiseExpr(IRBuilder::None, expr);
          }
      
          ctx->addStatement(AssignStmt::make(var, expr));
        }
        break;
      default:
        // TODO: raise error
        //REPORT_ERROR("can only assign the results of calls and maps to "
        //           "multiple variables", @idents);
        break;
    }
  }
}

std::vector<Stmt> ParserNew::liftCallsAndMaps(const CallMap &calls, 
                                                  const Expr expr) {
  std::vector<Stmt> stmts;

  for (auto it = calls.getAllInOrder().cbegin(); 
      it != calls.getAllInOrder().cend(); ++it) {
    const Var tmp = it->first;
    if (isa<VarExpr>(expr) && to<VarExpr>(expr)->var == tmp) continue;

    const CallMap::Entry callEntry = it->second;
    switch (callEntry.callType) {
      case CallMap::Entry::Type::FUNC:
      {
        ctx->addSymbol(tmp.getName(), tmp, Symbol::ReadWrite);
        ctx->addStatement(VarDecl::make(tmp));
        
        const Stmt newCallStmt = CallStmt::make({tmp}, callEntry.func, 
                                                callEntry.actuals);
        ctx->addStatement(newCallStmt);
        stmts.push_back(newCallStmt);
        break;
      }
      case CallMap::Entry::Type::MAP:
      {
        ctx->addSymbol(tmp.getName(), tmp, Symbol::ReadWrite);
        ctx->addStatement(VarDecl::make(tmp));
        
        const Stmt newMapStmt = Map::make({tmp}, callEntry.func, 
                                          callEntry.actuals, callEntry.target,
                                          callEntry.neighbors, 
                                          callEntry.reduction);
        ctx->addStatement(newMapStmt);
        stmts.push_back(newMapStmt);
        break;
      }
      default:
        break;
    }
  }

  return stmts;
}

#endif

}
}

