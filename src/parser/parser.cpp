// A Bison parser, made by GNU Bison 3.0.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2013 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.h"

// User implementation prologue.


// Unqualified %code blocks.


  #include <stdlib.h>
  #include <cassert>
  #include <iostream>
  #include <map>
  #include <set>
  #include <algorithm>

  #include "program_context.h"
  #include "scanner.h"
  #include "ir_codegen.h"
  using namespace std;
  using namespace simit::internal;
  using namespace simit::ir;

  std::string typeString(const Type &type, ProgramContext *ctx) {
    std::stringstream ss;
    ss << type;
    std::string str = ss.str();
    if (ctx->isColumnVector(type)) {
      str += "'";
    }
    return str;
  }

  #define REPORT_ERROR(msg, loc)  \
    do {                          \
      error((loc), (msg));        \
      YYERROR;                    \
    } while (0)

  #define REPORT_TYPE_MISSMATCH(t1, t2, loc)           \
    do {                                               \
      std::stringstream errorStr;                      \
      errorStr << "type missmatch ("                   \
               << typeString(t1, ctx) << " and "       \
               << typeString(t2, ctx) << ")";          \
      REPORT_ERROR(errorStr.str(), loc);               \
    } while (0)

  #define REPORT_INDEX_VARIABLE_MISSMATCH(numIndexVars, order, loc) \
    do {                                                            \
      REPORT_ERROR("wrong number of index variables (" +            \
                    to_string(numIndexVars) +                       \
                    " index variables, but tensor order is " +      \
                    to_string(order), loc);                         \
      } while (0)

  void Parser::error(const Parser::location_type &loc, const std::string &msg) {
    errors->push_back(Error(loc.begin.line, loc.begin.column,
                            loc.end.line, loc.end.column, msg));
  }

  #undef yylex
  #define yylex scanner->lex

  inline std::string convertAndFree(const char *str) {
    std::string result = std::string(str);
    free((void*)str);
    return result;
  }

  template <typename T>
  inline T convertAndDelete(T *obj) {
    auto result = T(*obj);
    delete obj;
    return result;
  }

  bool compare(const Type &l, const Type &r, ProgramContext *ctx) {
    if (l.getKind() != r.getKind()) {
      return false;
    }

    if (l.isTensor()) {
      if (ctx->isColumnVector(l) != ctx->isColumnVector(r)) {
        return false;
      }
    }

    if (l != r) {
      return false;
    }
    return true;
  }

  #define CHECK_IS_TENSOR(expr, loc)                    \
    do {                                                \
      assert(expr->getType());                          \
      if (!expr->getType()->isTensor()) {               \
        std::stringstream errorStr;                     \
        errorStr << "expected tensor";                  \
        REPORT_ERROR(errorStr.str(), loc);              \
      }                                                 \
    } while (0)

  #define CHECK_IS_SET(expr, loc)                       \
    do {                                                \
      assert(expr->getType());                          \
      if (!expr->getType()->isSet()) {                  \
        std::stringstream errorStr;                     \
        errorStr << "expected element";                 \
        REPORT_ERROR(errorStr.str(), loc);              \
      }                                                 \
    } while (0)

  #define CHECK_TYPE_EQUALITY(t1, t2, loc)              \
    do {                                                \
      if (!compare(t1, t2, ctx)) {                      \
        REPORT_TYPE_MISSMATCH(t1, t2, loc);             \
      }                                                 \
    } while (0)


  #define BINARY_ELWISE_TYPE_CHECK(lt, rt, loc)         \
    do {                                                \
      TensorType *ltt = tensorTypePtr(lt);              \
      TensorType *rtt = tensorTypePtr(rt);              \
      if (ltt->getOrder() > 0 && rtt->getOrder() > 0) { \
        CHECK_TYPE_EQUALITY(*ltt, *rtt, loc);           \
      }                                                 \
    }                                                   \
    while (0)




#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyempty = true)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace  simit { namespace internal  {


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
   Parser ::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
   Parser :: Parser  (Scanner *scanner_yyarg, ProgramContext *ctx_yyarg, std::vector<Error> *errors_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      ctx (ctx_yyarg),
      errors (errors_yyarg)
  {}

   Parser ::~ Parser  ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  inline
   Parser ::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  inline
   Parser ::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  inline
   Parser ::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    value = other.value;
  }


  template <typename Base>
  inline
   Parser ::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  inline
   Parser ::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  inline
   Parser ::basic_symbol<Base>::~basic_symbol ()
  {
  }

  template <typename Base>
  inline
  void
   Parser ::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move(s);
    value = s.value;
    location = s.location;
  }

  // by_type.
  inline
   Parser ::by_type::by_type ()
     : type (empty)
  {}

  inline
   Parser ::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  inline
   Parser ::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  inline
  void
   Parser ::by_type::move (by_type& that)
  {
    type = that.type;
    that.type = empty;
  }

  inline
  int
   Parser ::by_type::type_get () const
  {
    return type;
  }


  // by_state.
  inline
   Parser ::by_state::by_state ()
    : state (empty)
  {}

  inline
   Parser ::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
   Parser ::by_state::move (by_state& that)
  {
    state = that.state;
    that.state = empty;
  }

  inline
   Parser ::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
   Parser ::symbol_number_type
   Parser ::by_state::type_get () const
  {
    return state == empty ? 0 : yystos_[state];
  }

  inline
   Parser ::stack_symbol_type::stack_symbol_type ()
  {}


  inline
   Parser ::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    value = that.value;
    // that is emptied.
    that.type = empty;
  }

  inline
   Parser ::stack_symbol_type&
   Parser ::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
   Parser ::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.type_get ())
    {
            case 5: // "int literal"


        {}

        break;

      case 6: // "float literal"


        {}

        break;

      case 7: // "string literal"


        { free((void*)((yysym.value.string))); }

        break;

      case 8: // "identifier"


        { free((void*)((yysym.value.string))); }

        break;

      case 57: // program_element


        { delete (yysym.value.IRNode); }

        break;

      case 60: // element_type_decl


        { delete (yysym.value.elementType); }

        break;

      case 61: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 62: // field_decl


        { delete (yysym.value.field); }

        break;

      case 63: // procedure


        { delete (yysym.value.function); }

        break;

      case 64: // function


        { delete (yysym.value.function); }

        break;

      case 65: // function_header


        { delete (yysym.value.function); }

        break;

      case 66: // arguments


        { delete (yysym.value.Arguments); }

        break;

      case 67: // results


        { delete (yysym.value.Results); }

        break;

      case 68: // formal_list


        { delete (yysym.value.Formals); }

        break;

      case 69: // formal_decl


        { delete (yysym.value.Formal); }

        break;

      case 70: // stmt_block


        { delete (yysym.value.IRNodes); }

        break;

      case 71: // stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 72: // const_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 73: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 74: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 75: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 76: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 79: // expr


        { delete (yysym.value.expression); }

        break;

      case 80: // ident_expr


        { delete (yysym.value.expression); }

        break;

      case 81: // paren_expr


        { delete (yysym.value.expression); }

        break;

      case 82: // linear_algebra_expr


        { delete (yysym.value.indexExpr); }

        break;

      case 83: // elwise_binary_op


        {}

        break;

      case 84: // boolean_expr


        { delete (yysym.value.expression); }

        break;

      case 85: // field_read_expr


        { delete (yysym.value.fieldRead); }

        break;

      case 86: // set_expr


        { delete (yysym.value.expression); }

        break;

      case 87: // call_or_tensor_read_expr


        { delete (yysym.value.expression); }

        break;

      case 88: // call_expr


        { delete (yysym.value.call); }

        break;

      case 89: // actual_list


        { delete (yysym.value.expressions); }

        break;

      case 90: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 91: // range_expr


        { delete (yysym.value.expression); }

        break;

      case 92: // map_expr


        { delete (yysym.value.expression); }

        break;

      case 96: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 97: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 98: // type


        { delete (yysym.value.type); }

        break;

      case 99: // set_type


        { delete (yysym.value.setType); }

        break;

      case 100: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 101: // tensor_type


        { delete (yysym.value.tensorType); }

        break;

      case 102: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 103: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 104: // dimension


        { delete (yysym.value.indexSet); }

        break;

      case 105: // component_type


        {}

        break;

      case 107: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 108: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 109: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 110: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 111: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 112: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 113: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 114: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 115: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 116: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 117: // scalar_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 118: // test


        { delete (yysym.value.test); }

        break;


      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
   Parser ::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
   Parser ::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
   Parser ::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
   Parser ::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
   Parser ::debug_stream () const
  {
    return *yycdebug_;
  }

  void
   Parser ::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


   Parser ::debug_level_type
   Parser ::debug_level () const
  {
    return yydebug_;
  }

  void
   Parser ::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline  Parser ::state_type
   Parser ::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
   Parser ::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
   Parser ::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
   Parser ::parse ()
  {
    /// Whether yyla contains a lookahead.
    bool yyempty = true;

    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyempty)
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
        yyempty = false;
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Discard the token being shifted.
    yyempty = true;

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 5:

    {
    (yylhs.value.IRNode) = NULL;
  }

    break;

  case 6:

    {
    std::unique_ptr<Function> function((yystack_[0].value.function));

    std::string name = function->getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("function redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(function.release());
  }

    break;

  case 7:

    {
    (yylhs.value.IRNode) = NULL;
  }

    break;

  case 8:

    {
    (yylhs.value.IRNode) = NULL;
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 9:

    {
    ctx->addTest((yystack_[0].value.test));
  }

    break;

  case 10:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.type);
  }

    break;

  case 11:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.type);
  }

    break;

  case 12:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 13:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 14:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    std::unique_ptr<ir::ElementType::FieldsMapType> fields((yystack_[1].value.fields));

    if (ctx->containsElementType(name)) {
      REPORT_ERROR("struct redefinition (" + name + ")", yylhs.location);
    }

    ElementType *elementType = new ElementType(name, *fields);
    ctx->addElementType(std::shared_ptr<ElementType>(elementType));
  }

    break;

  case 15:

    {
    (yylhs.value.fields) = new ElementType::FieldsMapType();
  }

    break;

  case 16:

    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yylhs.value.fields)->insert(*(yystack_[0].value.field));
    delete (yystack_[0].value.field);
  }

    break;

  case 17:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    std::shared_ptr<TensorType> tensorType((yystack_[1].value.tensorType));
    (yylhs.value.field) = new pair<string,shared_ptr<ir::TensorType>>(name, tensorType);
  }

    break;

  case 18:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.IRNodes);
  }

    break;

  case 19:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[1].value.IRNodes));
    (yylhs.value.function) = (yystack_[2].value.function);
    (yylhs.value.function)->addStatements(*statements);
    ctx->unscope();
  }

    break;

  case 20:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    auto arguments = unique_ptr<vector<shared_ptr<Argument>>>((yystack_[1].value.Arguments));
    auto results = unique_ptr<vector<shared_ptr<Result>>>((yystack_[0].value.Results));

    (yylhs.value.function) = new Function(ident, *arguments, *results);

    ctx->scope();
    for (auto argument : *arguments) {
      ctx->addSymbol(argument->getName(), argument);
    }

    for (auto result : *results) {
      ctx->addSymbol(result->getName(), result);
    }
  }

    break;

  case 21:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
  }

    break;

  case 22:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
    for (auto formal : *(yystack_[1].value.Formals)) {
      auto result = new Argument(formal->name, formal->type);
      (yylhs.value.Arguments)->push_back(shared_ptr<Argument>(result));
    }
    delete (yystack_[1].value.Formals);
 }

    break;

  case 23:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    (yylhs.value.Results)->push_back(shared_ptr<Result>(new Result("asd", NULL)));
  }

    break;

  case 24:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    for (auto formal : *(yystack_[1].value.Formals)) {
      auto result = new Result(formal->name, formal->type);
      (yylhs.value.Results)->push_back(shared_ptr<Result>(result));
    }
    delete (yystack_[1].value.Formals);
  }

    break;

  case 25:

    {
    (yylhs.value.Formals) = new simit::util::OwnershipVector<FormalData *>();
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 26:

    {
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 27:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    (yylhs.value.Formal) = new FormalData(ident, std::shared_ptr<Type>((yystack_[0].value.type)));
  }

    break;

  case 28:

    {
    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
  }

    break;

  case 29:

    {
    (yylhs.value.IRNodes) = (yystack_[1].value.IRNodes);
    if ((yystack_[0].value.IRNodes) == NULL) break;  // TODO: Remove check
    (yylhs.value.IRNodes)->insert((yylhs.value.IRNodes)->end(), (yystack_[0].value.IRNodes)->begin(), (yystack_[0].value.IRNodes)->end());
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 35:

    {
    std::string ident = convertAndFree((yystack_[5].value.string));
    auto tensorType = std::shared_ptr<TensorType>((yystack_[3].value.tensorType));

    auto literal = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[1].value.TensorLiteral);

    assert(literal->getType()->isTensor() && "Set literals not supported yet");
    auto literalType = literal->getType();

    literal->setName(ident);

    // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
    // cast $tensor_literal to a 1xn matrix.
    TensorType *literalTensorType = tensorTypePtr(literalType);
    if (tensorType->getOrder() == 2 && literalTensorType->getOrder() == 1) {
      literal->cast(tensorType);
    }

    // Typecheck: value and literal types must be equivalent.
    CHECK_TYPE_EQUALITY(*tensorType, *literal->getType(), yystack_[5].location);

    ctx->addSymbol(literal->getName(), literal);

    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
    (yylhs.value.IRNodes)->push_back(literal);
  }

    break;

  case 36:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 37:

    {
    auto lhsList = unique_ptr<VariableAccessVector>((yystack_[3].value.VarAccesses));
    auto rhsList = unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));

    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();

    if (lhsList->size() > rhsList->size()) {
      // TODO: Handle maps and then reintroduce this error
      // REPORT_ERROR("too few expressions assigned to too many variables", @2);
      break;
    }
    else if (lhsList->size() < rhsList->size()) {
      REPORT_ERROR("too many expressions assigned to too few variables", yystack_[2].location);
    }

    auto lhsIter = lhsList->begin();
    auto rhsIter = rhsList->begin();
    for (; lhsIter != lhsList->end(); ++lhsIter, ++rhsIter) {
      VariableAccess *lhs = *lhsIter;
      std::shared_ptr<Expression> &rhs = *rhsIter;

      // TODO: Remove these checks
      if (rhs == NULL) continue;
      if (!ctx->hasSymbol(lhs->name)) continue;

      assert(ctx->hasSymbol(lhs->name));
      shared_ptr<Expression> lhsTensor = ctx->getSymbol(lhs->name);
      if (auto result = dynamic_pointer_cast<Result>(lhsTensor)) {
        CHECK_TYPE_EQUALITY(*result->getType(), *rhs->getType(), yystack_[2].location);
        rhs->setName(result->getName());
        result->setValue(rhs);
        (yylhs.value.IRNodes)->push_back(rhs);
      }
      else {
        NOT_SUPPORTED_YET;
      }
    }
  }

    break;

  case 38:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 39:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 40:

    {
    (yylhs.value.IRNodes) = NULL;
    delete (yystack_[3].value.expression);
    delete (yystack_[2].value.IRNodes);
  }

    break;

  case 42:

    {
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 44:

    {
    delete (yystack_[1].value.expression);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 54:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    if (!ctx->hasSymbol(ident)) {
      // TODO: reintroduce error
      // REPORT_ERROR(ident + " is not defined in scope", @1);
      (yylhs.value.expression) = NULL;
      break;
    }
    (yylhs.value.expression) = new shared_ptr<Expression>(ctx->getSymbol(ident));
  }

    break;

  case 55:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = (yystack_[1].value.expression);
  }

    break;

  case 56:

    {
    if ((yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 57:

    {  // + - .* ./
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> l = convertAndDelete((yystack_[2].value.expression));
    std::shared_ptr<Expression> r = convertAndDelete((yystack_[0].value.expression));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }

    break;

  case 58:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> l = convertAndDelete((yystack_[2].value.expression));
    std::shared_ptr<Expression> r = convertAndDelete((yystack_[0].value.expression));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    TensorType *ltype = tensorTypePtr(l->getType());
    TensorType *rtype = tensorTypePtr(r->getType());

    // Scale
    if (ltype->getOrder()==0 || rtype->getOrder()==0) {
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(binaryElwiseExpr(l, IndexExpr::MUL, r));
    }
    // Vector-Vector Multiplication (inner and outer product)
    else if (ltype->getOrder() == 1 && rtype->getOrder() == 1) {
      // Inner product
      if (!ctx->isColumnVector(*l->getType())) {
        if (!ctx->isColumnVector(*r->getType())) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(*l->getType(), *r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (ctx->isColumnVector(*r->getType())) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(*l->getType(), *r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (ltype->getOrder() == 2 && rtype->getOrder() == 1) {
      if (ltype->getDimensions()[1] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*l->getType(), *rtype, yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->getOrder() == 1 && rtype->getOrder() == 2) {
      if (ltype->getDimensions()[0] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*ltype, *rtype, yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->getOrder() == 2 && rtype->getOrder() == 2) {
      if (ltype->getDimensions()[1] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*ltype, *rtype, yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.indexExpr) = NULL;
    }
  }

    break;

  case 59:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 60:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    auto expr = shared_ptr<Expression>(*(yystack_[1].value.expression));
    delete (yystack_[1].value.expression);

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    TensorType *type = tensorTypePtr(expr->getType());

    switch (type->getOrder()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NONE, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NONE, expr));
        if (!ctx->isColumnVector(*expr->getType())) {
          ctx->toggleColumnVector(*(*(yylhs.value.indexExpr))->getType());
        }
        break;
      case 2:
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(transposeMatrix(expr));
        break;
      default:
        REPORT_ERROR("cannot transpose >2-order tensors using '", yystack_[1].location);
        (yylhs.value.indexExpr) = NULL;
    }
  }

    break;

  case 61:

    {
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 62:

    {  // Solve
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 63:

    {
    (yylhs.value.binop) = IndexExpr::ADD;
  }

    break;

  case 64:

    {
    (yylhs.value.binop) = IndexExpr::SUB;
  }

    break;

  case 65:

    {
    (yylhs.value.binop) = IndexExpr::MUL;
  }

    break;

  case 66:

    {
    (yylhs.value.binop) = IndexExpr::DIV;
  }

    break;

  case 67:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 68:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 69:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 70:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 71:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 72:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 73:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.string) == NULL) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check
    if ((*(yystack_[2].value.expression))->getType() == NULL) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> set = convertAndDelete((yystack_[2].value.expression));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    CHECK_IS_SET(set, yystack_[2].location);
    
    (yylhs.value.fieldRead) = new shared_ptr<FieldRead>(new FieldRead(set, fieldName));
  }

    break;

  case 77:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 78:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 79:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.expressions));
    delete (yystack_[1].value.expressions);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 80:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 81:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 82:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 83:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 84:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 85:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 87:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 92:

    {
    (yylhs.value.VarAccesses) = new VariableAccessVector();
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 93:

    {
    (yylhs.value.VarAccesses) = (yystack_[2].value.VarAccesses);
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 94:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto variableStore = new VariableAccess;
    variableStore->name = name;
    (yylhs.value.VarAccess) = variableStore;
  }

    break;

  case 95:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    std::string field = convertAndFree((yystack_[0].value.string));
    (yylhs.value.VarAccess) = NULL;
  }

    break;

  case 96:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.VarAccess) = NULL;
    delete (yystack_[1].value.expressions);
  }

    break;

  case 97:

    {
    (yylhs.value.type) = (yystack_[0].value.setType);
  }

    break;

  case 98:

    {
    // If we define an element as a one-item set, then we don't need this rule.
    (yylhs.value.type) = NULL;
  }

    break;

  case 99:

    {
    (yylhs.value.type) = (yystack_[0].value.tensorType);
  }

    break;

  case 100:

    {
    (yylhs.value.setType) = new SetType(std::shared_ptr<ElementType>(*(yystack_[2].value.elementType)));
    delete (yystack_[2].value.elementType);
  }

    break;

  case 101:

    {
    (yylhs.value.setType) = NULL;
  }

    break;

  case 102:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.elementType) = new std::shared_ptr<ElementType>(ctx->getElementType(name));
  }

    break;

  case 103:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[0].value.componentType));
  }

    break;

  case 104:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[1].value.componentType), *(yystack_[3].value.IndexSetProducts));
    delete (yystack_[3].value.IndexSetProducts);
  }

    break;

  case 105:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[2].value.componentType), *(yystack_[4].value.IndexSetProducts));
    ctx->toggleColumnVector(*(yylhs.value.tensorType));
    delete (yystack_[4].value.IndexSetProducts);
  }

    break;

  case 106:

    {
    (yylhs.value.IndexSetProducts) = new std::vector<IndexSetProduct>();
  }

    break;

  case 107:

    {
    (yylhs.value.IndexSetProducts) = (yystack_[3].value.IndexSetProducts);

    auto parentDims = (yylhs.value.IndexSetProducts);
    auto childDims = unique_ptr<std::vector<IndexSet>>((yystack_[1].value.IndexSets));

    // If there are no previous dimensions then create IndexSetProducts
    if (parentDims->size() == 0) {
      for (auto &dim : *childDims) {
        UNUSED(dim);
        parentDims->push_back(IndexSetProduct());
      }
    }

    // Handle case where there are more child than parent dimensions
    if (childDims->size() > parentDims->size()) {
      for (size_t i=0; i < childDims->size() - parentDims->size(); ++i) {
        size_t numNestings = (*parentDims)[0].getFactors().size();
        std::vector<IndexSet> indexSets(numNestings, IndexSet(1));
        parentDims->push_back(IndexSetProduct(indexSets));
      }
    }

    // Handle case where there are more parent than child dimensions
    if (parentDims->size() > childDims->size()) {
      for (size_t i=0; i < parentDims->size() - childDims->size(); ++i) {
        childDims->push_back(IndexSet(1));
      }
    }

    // Multiply each dimension with the corresponding dimension in the shape.
    assert(childDims->size() == parentDims->size());
    for (size_t i=0; i<(yylhs.value.IndexSetProducts)->size(); ++i) {
      (*parentDims)[i] = (*parentDims)[i] * (*childDims)[i];
    }
  }

    break;

  case 108:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 109:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 110:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 111:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 112:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 113:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 114:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 117:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    ctx->toggleColumnVector(*(*(yylhs.value.TensorLiteral))->getType());
  }

    break;

  case 119:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::FLOAT, isps);
    auto literal = new Literal(shared_ptr<TensorType>(type), // TODO: <Type>
                               values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 120:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::INT, isps);
    auto literal = new Literal(shared_ptr<TensorType>(type), // TODO: <Type>
                               values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 121:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 123:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 124:

    {
    auto  left = unique_ptr<TensorValues<double>>((yystack_[4].value.TensorDoubleValues));
    auto right = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));

    string errorStr;
    if(!left->dimensionsMatch(*right, &errorStr)) {
      REPORT_ERROR(errorStr, yystack_[3].location);
    }
    left->merge(*right);
    (yylhs.value.TensorDoubleValues) = left.release();
  }

    break;

  case 125:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 126:

    {
    auto  left = unique_ptr<TensorValues<double>>((yystack_[2].value.TensorDoubleValues));
    auto right = unique_ptr<TensorValues<double>>((yystack_[0].value.TensorDoubleValues));

    string errorStr;
    if(!left->dimensionsMatch(*right, &errorStr)) {
      REPORT_ERROR(errorStr, yystack_[1].location);
    }

    left->merge(*right);
    (yylhs.value.TensorDoubleValues) = left.release();
  }

    break;

  case 127:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 128:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 129:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 131:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 132:

    {
    auto  left = unique_ptr<TensorValues<int>>((yystack_[4].value.TensorIntValues));
    auto right = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));

    string errorStr;
    if(!left->dimensionsMatch(*right, &errorStr)) {
      REPORT_ERROR(errorStr, yystack_[3].location);
    }
    left->merge(*right);
    (yylhs.value.TensorIntValues) = left.release();
  }

    break;

  case 133:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 134:

    {
    auto  left = unique_ptr<TensorValues<int>>((yystack_[2].value.TensorIntValues));
    auto right = unique_ptr<TensorValues<int>>((yystack_[0].value.TensorIntValues));

    string errorStr;
    if(!left->dimensionsMatch(*right, &errorStr)) {
      REPORT_ERROR(errorStr, yystack_[1].location);
    }

    left->merge(*right);
    (yylhs.value.TensorIntValues) = left.release();
  }

    break;

  case 135:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 136:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 137:

    {
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 138:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 139:

    {
    auto call = shared_ptr<Call>(*(yystack_[3].value.call));
    auto expected = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[3].value.call);
    delete (yystack_[1].value.TensorLiteral);

    std::vector<std::shared_ptr<Literal>> literalArgs;
    literalArgs.reserve(call->getArguments().size());
    for (auto &arg : call->getArguments()) {
      std::shared_ptr<Literal> litarg = dynamic_pointer_cast<Literal>(arg);
      if (!litarg) {
        REPORT_ERROR("function calls in tests must have literal arguments", yystack_[4].location);
      }
      literalArgs.push_back(litarg);
    }
    assert(literalArgs.size() == call->getArguments().size());

    std::vector<std::shared_ptr<Literal>> expecteds;
    expecteds.push_back(expected);
    (yylhs.value.test) = new Test(call->getName(), literalArgs, expecteds);
  }

    break;



            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state,
                                           yyempty ? yyempty_ : yyla.type_get ()));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyempty)
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyempty = true;
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyempty)
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyempty)
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
   Parser ::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
   Parser ::yysyntax_error_ (state_type yystate, symbol_number_type yytoken) const
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char  Parser ::yypact_ninf_ = -107;

  const signed char  Parser ::yytable_ninf_ = -95;

  const short int
   Parser ::yypact_[] =
  {
    -107,    12,  -107,  -107,  -107,    22,    11,    24,    65,    76,
      88,   109,    25,    83,   125,    25,    16,  -107,    25,  -107,
    -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,
    -107,   193,    67,   102,  -107,  -107,   115,   118,  -107,  -107,
    -107,  -107,    94,  -107,  -107,  -107,   110,  -107,  -107,    25,
     150,  -107,   123,    78,  -107,   135,   147,   138,   213,  -107,
     139,   119,   173,  -107,  -107,    52,   137,   144,   142,   148,
     152,   154,   146,   157,   277,    87,    25,    25,    25,  -107,
    -107,  -107,    25,    25,  -107,  -107,    25,  -107,    25,    25,
      25,    25,    25,    25,    25,   179,   187,    25,  -107,   213,
     166,   -21,  -107,     3,    91,  -107,  -107,  -107,  -107,   158,
    -107,   103,  -107,  -107,   120,    37,   176,   191,    25,   149,
      25,    30,  -107,    57,    58,  -107,   170,   199,   200,  -107,
     182,   202,   205,  -107,  -107,   266,   266,   213,   277,   277,
     277,   233,   253,   253,   266,   266,   213,   201,   197,  -107,
      15,  -107,    35,   192,  -107,    25,   211,  -107,  -107,   210,
      17,  -107,   245,   220,  -107,   231,  -107,    32,  -107,   242,
    -107,   254,   249,   124,   260,   251,  -107,  -107,   199,   148,
    -107,   202,   157,  -107,  -107,    25,  -107,   213,    91,    30,
     143,     5,  -107,    82,  -107,    78,  -107,   285,   285,   286,
     283,  -107,    25,  -107,  -107,  -107,    70,    71,   -21,   264,
     278,   287,  -107,  -107,  -107,    92,  -107,   288,   311,  -107,
    -107,    85,  -107,    97,  -107,   213,   171,  -107,  -107,  -107,
    -107,   271,  -107,     5,   293,  -107,  -107,  -107,  -107,  -107,
     171,  -107,  -107,  -107
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   137,   138,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    39,     0,     3,
       7,     4,     5,     6,    28,     8,    30,    31,    32,    33,
      34,     0,    45,    46,    48,    49,    50,     0,    51,    76,
      52,    53,     0,    92,    47,   115,   116,   118,     9,    80,
       0,    15,     0,     0,    28,     0,     0,    54,    28,    36,
       0,     0,     0,   135,   127,     0,     0,   122,   121,   125,
       0,   130,   129,   133,    56,     0,     0,     0,     0,    38,
      63,    64,     0,     0,    65,    66,     0,    60,     0,     0,
       0,     0,     0,     0,    80,     0,     0,     0,   117,    82,
       0,     0,    95,     0,     0,   102,   113,   114,   106,     0,
      97,    98,    99,   103,     0,     0,    23,     0,    80,    43,
      80,     0,    55,     0,     0,   119,     0,     0,     0,   120,
       0,     0,     0,    19,    29,    69,    70,    84,    58,    59,
      61,    62,    67,    68,    71,    72,    57,     0,    81,    73,
      94,    93,     0,    77,    96,     0,     0,    14,    16,     0,
       0,    10,     0,     0,    18,     0,    21,     0,    25,     0,
      20,    86,     0,     0,     0,     0,   123,   131,     0,   126,
     128,     0,   134,   136,    78,     0,    37,    83,     0,     0,
       0,     0,    12,     0,   100,     0,    22,     0,     0,     0,
      88,    40,     0,    28,    79,   139,     0,     0,     0,     0,
       0,     0,   110,   111,   112,     0,   108,     0,     0,    27,
      26,     0,    87,     0,    85,    28,    42,   124,   132,    17,
      35,   104,   107,     0,     0,    13,    24,    90,    91,    89,
      44,   105,   109,   101
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,  -107,
    -107,  -107,  -107,   130,   132,   -53,   321,  -107,  -107,  -107,
    -107,  -107,  -107,  -107,   -12,  -107,  -107,  -107,  -107,  -107,
    -107,  -107,  -107,   316,   -80,   -47,  -107,  -107,  -107,  -107,
    -107,  -107,   235,   140,  -107,  -107,   -97,  -107,  -107,    99,
     151,  -107,  -106,  -107,  -107,  -107,   -57,   206,  -107,  -107,
     -61,   203,  -107,  -107
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    19,    20,   193,    21,   103,   158,    22,    23,
      24,   116,   170,   167,   168,    75,   134,    26,    27,    28,
      29,    30,   172,   173,    31,    32,    33,    34,    93,    35,
      36,    37,    38,    39,   100,   148,    40,    41,   200,   224,
     239,    42,    43,   109,   110,   111,   112,   160,   215,   216,
     113,    44,    45,    46,    66,    67,    68,    69,    70,    71,
      72,    73,    47,    48
  };

  const short int
   Parser ::yytable_[] =
  {
      58,   114,   101,    62,   124,   119,    74,   159,   123,   154,
     212,   156,     2,   213,   147,   175,   155,     3,     4,    51,
       5,    63,    64,     6,     7,     8,     9,    10,   157,    11,
       3,     4,    52,    57,    12,     3,     4,    99,    13,    14,
     174,    15,    11,    16,   185,   165,   190,    65,   191,   214,
     152,    49,    17,    50,    15,    18,    16,    63,    64,   -94,
      50,    16,   196,   -94,   135,   136,   137,   166,    18,   197,
     138,   139,   155,    53,   140,   186,   141,   142,   143,   144,
     145,   146,    99,   210,    54,    99,   105,   106,   107,   176,
     177,   209,     3,     4,   108,     5,    55,   127,   131,     7,
     106,   107,   227,   228,    11,   -74,    99,   108,    99,    12,
     127,   131,   133,    13,   217,   236,    15,    56,    16,   218,
     207,   206,   197,    59,   232,     3,     4,    17,     5,   233,
      18,    96,     7,    60,   162,    97,   163,    11,   208,   237,
     -75,   238,    12,   187,    94,   164,    13,   202,   203,    15,
     226,    16,   106,   107,     3,     4,    95,     5,   102,    98,
      17,     7,   104,    18,   115,   117,    11,   118,   120,   125,
     121,    12,   240,    99,   -41,    13,     3,     4,    15,     5,
      16,   126,   127,     7,   129,   128,   131,   149,    11,    17,
     225,   130,    18,    12,   132,   150,   153,    13,   161,   171,
      15,   178,    16,   122,   169,    64,   180,    63,    76,    77,
     183,    17,    78,   181,    18,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    76,    77,
     -79,   184,    78,    79,   155,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    76,    77,
     188,   189,    78,   192,   194,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    76,    77,
     195,   198,    78,   199,   201,    80,    81,    82,    83,    84,
      85,    86,    87,   -95,    89,    90,    91,    92,    76,    77,
     204,   205,    78,   165,   222,    80,    81,    82,    83,    84,
      85,    86,    87,   223,   229,    78,    91,    92,    80,    81,
      82,    83,    84,    85,    86,    87,    78,   231,   230,   235,
     241,   234,    25,    84,    85,    86,    87,   243,   221,   220,
      61,   151,   242,   179,   182,   219,     0,     0,     0,     0,
       0,   211
  };

  const short int
   Parser ::yycheck_[] =
  {
      12,    54,    49,    15,    65,    58,    18,   104,    65,    30,
       5,     8,     0,     8,    94,   121,    37,     5,     6,     8,
       8,     5,     6,    11,    12,    13,    14,    15,    25,    17,
       5,     6,     8,     8,    22,     5,     6,    49,    26,    27,
     120,    29,    17,    31,    29,     8,    29,    31,    31,    44,
      97,    29,    40,    38,    29,    43,    31,     5,     6,    37,
      38,    31,    30,    41,    76,    77,    78,    30,    43,    37,
      82,    83,    37,     8,    86,    40,    88,    89,    90,    91,
      92,    93,    94,   189,     8,    97,     8,     9,    10,    32,
      32,   188,     5,     6,    16,     8,     8,    40,    40,    12,
       9,    10,    32,    32,    17,    38,   118,    16,   120,    22,
      40,    40,    25,    26,    32,    30,    29,     8,    31,    37,
     181,   178,    37,    40,    32,     5,     6,    40,     8,    37,
      43,    37,    12,     8,    31,    41,    33,    17,   185,    42,
      38,    44,    22,   155,    29,    25,    26,    23,    24,    29,
     203,    31,     9,    10,     5,     6,    38,     8,     8,    49,
      40,    12,    39,    43,    29,    18,    17,    29,    29,    32,
      51,    22,   225,   185,    25,    26,     5,     6,    29,     8,
      31,    37,    40,    12,    32,    37,    40,     8,    17,    40,
     202,    37,    43,    22,    37,     8,    30,    26,    40,     8,
      29,    31,    31,    30,    28,     6,     6,     5,    35,    36,
       5,    40,    39,    31,    43,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    35,    36,
      38,    30,    39,    40,    37,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    35,    36,
      39,    41,    39,     8,    34,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    35,    36,
      39,    29,    39,    19,    25,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    35,    36,
      30,    40,    39,     8,     8,    42,    43,    44,    45,    46,
      47,    48,    49,    20,    40,    39,    53,    54,    42,    43,
      44,    45,    46,    47,    48,    49,    39,    30,    40,     8,
      49,    33,     1,    46,    47,    48,    49,    34,   198,   197,
      14,    96,   233,   127,   131,   195,    -1,    -1,    -1,    -1,
      -1,   190
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    56,     0,     5,     6,     8,    11,    12,    13,    14,
      15,    17,    22,    26,    27,    29,    31,    40,    43,    57,
      58,    60,    63,    64,    65,    71,    72,    73,    74,    75,
      76,    79,    80,    81,    82,    84,    85,    86,    87,    88,
      91,    92,    96,    97,   106,   107,   108,   117,   118,    29,
      38,     8,     8,     8,     8,     8,     8,     8,    79,    40,
       8,    88,    79,     5,     6,    31,   109,   110,   111,   112,
     113,   114,   115,   116,    79,    70,    35,    36,    39,    40,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    83,    29,    38,    37,    41,    49,    79,
      89,    90,     8,    61,    39,     8,     9,    10,    16,    98,
      99,   100,   101,   105,    70,    29,    66,    18,    29,    70,
      29,    51,    30,   111,   115,    32,    37,    40,    37,    32,
      37,    40,    37,    25,    71,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    89,    90,     8,
       8,    97,    90,    30,    30,    37,     8,    25,    62,   101,
     102,    40,    31,    33,    25,     8,    30,    68,    69,    28,
      67,     8,    77,    78,    89,   107,    32,    32,    31,   112,
       6,    31,   116,     5,    30,    29,    40,    79,    39,    41,
      29,    31,     8,    59,    34,    39,    30,    37,    29,    19,
      93,    25,    23,    24,    30,    40,   111,   115,    90,   101,
     107,   105,     5,     8,    44,   103,   104,    32,    37,    98,
      69,    68,     8,    20,    94,    79,    70,    32,    32,    40,
      40,    30,    32,    37,    33,     8,    30,    42,    44,    95,
      70,    49,   104,    34
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    55,    56,    56,    57,    57,    57,    57,    57,    57,
      58,    58,    59,    59,    60,    61,    61,    62,    63,    64,
      65,    66,    66,    67,    67,    68,    68,    69,    70,    70,
      71,    71,    71,    71,    71,    72,    73,    74,    75,    75,
      76,    77,    77,    78,    78,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    80,    81,    82,    82,    82,    82,
      82,    82,    82,    83,    83,    83,    83,    84,    84,    84,
      84,    84,    84,    85,    86,    86,    86,    87,    87,    88,
      89,    89,    90,    90,    91,    92,    93,    93,    94,    94,
      95,    95,    96,    96,    97,    97,    97,    98,    98,    98,
      99,    99,   100,   101,   101,   101,   102,   102,   103,   103,
     104,   104,   104,   105,   105,   106,   107,   107,   107,   108,
     108,   109,   109,   110,   110,   111,   111,   112,   112,   113,
     113,   114,   114,   115,   115,   116,   116,   117,   117,   118
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       4,     4,     1,     3,     4,     0,     2,     4,     4,     3,
       4,     2,     3,     0,     4,     1,     3,     3,     0,     2,
       1,     1,     1,     1,     1,     7,     2,     4,     2,     1,
       5,     0,     3,     0,     4,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     3,     3,     3,
       2,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     4,     4,     4,
       0,     1,     1,     3,     3,     6,     0,     2,     0,     2,
       1,     1,     1,     3,     1,     3,     4,     1,     1,     1,
       3,     6,     1,     1,     5,     6,     0,     4,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     3,
       3,     1,     1,     3,     5,     1,     3,     1,     3,     1,
       1,     3,     5,     1,     3,     1,     3,     1,     1,     5
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "NEG",
  "\"int literal\"", "\"float literal\"", "\"string literal\"",
  "\"identifier\"", "\"int\"", "\"float\"", "\"struct\"", "\"const\"",
  "\"extern\"", "\"proc\"", "\"func\"", "\"Tensor\"", "\"map\"", "\"to\"",
  "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"", "\"else\"",
  "\"end\"", "\"return\"", "\"%!\"", "\"->\"", "\"(\"", "\")\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\",\"", "\".\"", "\":\"",
  "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\".*\"", "\"./\"",
  "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"",
  "$accept", "program", "program_element", "extern", "endpoints",
  "element_type_decl", "field_decl_list", "field_decl", "procedure",
  "function", "function_header", "arguments", "results", "formal_list",
  "formal_decl", "stmt_block", "stmt", "const_stmt", "return_stmt",
  "assign_stmt", "expr_stmt", "if_stmt", "else_clauses", "elif_clauses",
  "expr", "ident_expr", "paren_expr", "linear_algebra_expr",
  "elwise_binary_op", "boolean_expr", "field_read_expr", "set_expr",
  "call_or_tensor_read_expr", "call_expr", "actual_list", "expr_list",
  "range_expr", "map_expr", "with", "reduce", "reduction_op",
  "lhs_expr_list", "lhs_expr", "type", "set_type", "element_type",
  "tensor_type", "nested_dimensions", "dimensions", "dimension",
  "component_type", "literal_expr", "tensor_literal",
  "dense_tensor_literal", "float_dense_tensor_literal",
  "float_dense_ndtensor_literal", "float_dense_matrix_literal",
  "float_dense_vector_literal", "int_dense_tensor_literal",
  "int_dense_ndtensor_literal", "int_dense_matrix_literal",
  "int_dense_vector_literal", "scalar_literal", "test", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   246,   246,   248,   252,   253,   256,   265,   268,   272,
     279,   283,   289,   292,   306,   320,   323,   331,   344,   351,
     360,   398,   401,   412,   416,   427,   431,   437,   445,   448,
     457,   458,   459,   460,   461,   465,   495,   501,   542,   545,
     551,   557,   559,   563,   565,   579,   580,   581,   582,   583,
     584,   585,   586,   587,   593,   608,   624,   632,   644,   709,
     715,   745,   750,   759,   762,   765,   768,   776,   782,   788,
     794,   800,   806,   821,   835,   836,   837,   848,   852,   858,
     867,   870,   876,   882,   893,   904,   912,   914,   918,   920,
     923,   924,   945,   950,   958,   964,   969,  1002,  1005,  1009,
    1014,  1018,  1023,  1029,  1032,  1036,  1043,  1046,  1084,  1089,
    1096,  1099,  1103,  1108,  1111,  1180,  1183,  1184,  1188,  1191,
    1200,  1211,  1218,  1221,  1225,  1238,  1242,  1256,  1260,  1266,
    1273,  1276,  1280,  1293,  1297,  1311,  1315,  1321,  1326,  1336
  };

  // Print the state stack on the debug stream.
  void
   Parser ::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
   Parser ::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  inline
   Parser ::token_number_type
   Parser ::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
    };
    const unsigned int user_token_number_max_ = 309;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



