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

  std::string typeString(const Type &type) {
    std::stringstream ss;
    ss << type;
    std::string str = ss.str();
    if (type.isTensor() && tensorTypePtr(&type)->isColumnVector()) {
      str += "'";
    }
    return str;
  }

  #define REPORT_ERROR(msg, loc)  \
    do {                          \
      error((loc), (msg));        \
      YYERROR;                    \
    } while (0)

  #define REPORT_TYPE_MISSMATCH(t1, t2, loc)                          \
    do {                                                              \
      std::stringstream errorStr;                                     \
      errorStr << "type missmatch (" <<                               \
                  typeString(t1) << " and " << typeString(t2) << ")"; \
      REPORT_ERROR(errorStr.str(), loc);                              \
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

  void transposeVector(const std::shared_ptr<Expression> &vec) {
    assert(vec->getType()->isTensor());
    TensorType *ttype = tensorTypePtr(vec->getType());
    assert(ttype->getOrder() == 1);

    auto transposedVector = new TensorType(ttype->getComponentType(),
                                           ttype->getDimensions(),
                                           !ttype->isColumnVector());
    vec->setType(std::shared_ptr<TensorType>(transposedVector));
  }

  bool compare(const Type &l, const Type &r, ProgramContext *ctx) {
    if (l.getKind() != r.getKind()) {
      return false;
    }

    if (l.isTensor()) {
      if (tensorTypePtr(&l)->isColumnVector() !=
          tensorTypePtr(&r)->isColumnVector()) {
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

  #define CHECK_TYPE_EQUALITY(t1, t2, loc)              \
    do {                                                \
      if (!compare(t1, t2, ctx)) {                      \
        REPORT_TYPE_MISSMATCH(t1, t2, loc);             \
      }                                                 \
    } while (0)


  #define BINARY_ELWISE_TYPE_CHECK(lt, rt, loc)         \
    do {                                                \
      const TensorType *ltt = tensorTypePtr(lt);        \
      const TensorType *rtt = tensorTypePtr(rt);        \
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

      case 58: // program_element


        { delete (yysym.value.IRNode); }

        break;

      case 59: // extern


        { delete (yysym.value.expression); }

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

      case 66: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 67: // function


        { delete (yysym.value.function); }

        break;

      case 70: // function_header


        { delete (yysym.value.function); }

        break;

      case 71: // arguments


        { delete (yysym.value.arguments); }

        break;

      case 72: // argument_list


        { delete (yysym.value.arguments); }

        break;

      case 73: // argument_decl


        { delete (yysym.value.argument); }

        break;

      case 74: // results


        { delete (yysym.value.results); }

        break;

      case 75: // result_list


        { delete (yysym.value.results); }

        break;

      case 76: // result_decl


        { delete (yysym.value.result); }

        break;

      case 77: // stmt_block


        { delete (yysym.value.IRNodes); }

        break;

      case 78: // stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 79: // const_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 80: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 81: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 82: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 83: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 86: // expr


        { delete (yysym.value.expression); }

        break;

      case 87: // ident_expr


        { delete (yysym.value.expression); }

        break;

      case 88: // paren_expr


        { delete (yysym.value.expression); }

        break;

      case 89: // linear_algebra_expr


        { delete (yysym.value.indexExpr); }

        break;

      case 90: // elwise_binary_op


        {}

        break;

      case 91: // boolean_expr


        { delete (yysym.value.expression); }

        break;

      case 92: // field_read_expr


        { delete (yysym.value.fieldRead); }

        break;

      case 93: // set_read_expr


        { delete (yysym.value.expression); }

        break;

      case 94: // call_or_tensor_read_expr


        { delete (yysym.value.expression); }

        break;

      case 95: // call_expr


        { delete (yysym.value.call); }

        break;

      case 96: // expr_list_or_empty


        { delete (yysym.value.expressions); }

        break;

      case 97: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 98: // range_expr


        { delete (yysym.value.expression); }

        break;

      case 99: // map_expr


        { delete (yysym.value.expression); }

        break;

      case 103: // write_expr_list


        { delete (yysym.value.writeinfos); }

        break;

      case 104: // write_expr


        { delete (yysym.value.writeinfo); }

        break;

      case 105: // field_write_expr


        { delete (yysym.value.fieldWrite); }

        break;

      case 106: // tensor_write_expr


        { delete (yysym.value.tensorWrite); }

        break;

      case 107: // type


        { delete (yysym.value.type); }

        break;

      case 108: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 109: // set_type


        { delete (yysym.value.setType); }

        break;

      case 110: // endpoints


        { delete (yysym.value.expressions); }

        break;

      case 111: // tuple_type


        { delete (yysym.value.tupleType); }

        break;

      case 112: // tuple_element_types


        { delete (yysym.value.elementTypes); }

        break;

      case 113: // tuple_element_type


        { delete (yysym.value.elementTypes); }

        break;

      case 114: // tensor_type


        { delete (yysym.value.tensorType); }

        break;

      case 115: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 116: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 117: // component_type


        {}

        break;

      case 119: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 120: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 121: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 122: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 123: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 124: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 125: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 126: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 127: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 128: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 129: // scalar_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 130: // test


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
    std::unique_ptr<Function> function((yystack_[0].value.function));
    std::string name = function->getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("procedure redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(function.release());
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
    auto externArgument = convertAndDelete((yystack_[1].value.argument));
    ctx->addExtern(externArgument);
  }

    break;

  case 11:

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

  case 12:

    {
    (yylhs.value.fields) = new ElementType::FieldsMapType();
  }

    break;

  case 13:

    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yylhs.value.fields)->insert(*(yystack_[0].value.field));
    delete (yystack_[0].value.field);
  }

    break;

  case 14:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    std::shared_ptr<TensorType> tensorType = convertAndDelete((yystack_[1].value.tensorType));
    (yylhs.value.field) = new pair<string,shared_ptr<ir::TensorType>>(name, tensorType);
  }

    break;

  case 15:

    {ctx->scope();}

    break;

  case 16:

    {ctx->unscope();}

    break;

  case 17:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[2].value.IRNodes));
    (yylhs.value.function) = (yystack_[3].value.function);
    (yylhs.value.function)->addStatements(*statements);
  }

    break;

  case 18:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto arguments = vector<shared_ptr<Argument>>();
    auto results = vector<shared_ptr<Result>>();

    for (auto &extPair : ctx->getExterns()) {
      auto ext = extPair.second;

      // TODO: Replace extResult with mutable parameters
      auto tmp = new Result(ext->getName(), ext->getType());
      auto extResult = std::shared_ptr<Result>(tmp);
      results.push_back(extResult);

      arguments.push_back(ext);
      ctx->addSymbol(ext->getName(), ext, extResult);
    }

    (yylhs.value.function) = new Function(name, arguments, results);
  }

    break;

  case 19:

    {ctx->scope();}

    break;

  case 20:

    {ctx->unscope();}

    break;

  case 21:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[2].value.IRNodes));
    (yylhs.value.function) = (yystack_[3].value.function);
    (yylhs.value.function)->addStatements(*statements);
  }

    break;

  case 22:

    {
    std::string name = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<shared_ptr<Argument>>>((yystack_[2].value.arguments));
    auto results = unique_ptr<vector<shared_ptr<Result>>>((yystack_[0].value.results));
    (yylhs.value.function) = new Function(name, *arguments, *results);

    std::set<std::string> argumentNames;
    for (auto argument : *arguments) {
      ctx->addSymbol(argument->getName(), argument, NULL);
      argumentNames.insert(argument->getName());
    }

    for (auto result : *results) {
      std::shared_ptr<Expression> readExpr(NULL);
      if (argumentNames.find(result->getName()) != argumentNames.end()) {
        readExpr = ctx->getSymbol(result->getName()).getReadExpr();
      }

      ctx->addSymbol(result->getName(), readExpr, result);
    }
  }

    break;

  case 23:

    {
    (yylhs.value.arguments) = new vector<shared_ptr<Argument>>;
  }

    break;

  case 24:

    {
    (yylhs.value.arguments) = (yystack_[0].value.arguments);
 }

    break;

  case 25:

    {
    auto argument = convertAndDelete((yystack_[0].value.argument));
    (yylhs.value.arguments) = new vector<shared_ptr<Argument>>;
    (yylhs.value.arguments)->push_back(argument);
  }

    break;

  case 26:

    {
    auto argument = convertAndDelete((yystack_[0].value.argument));
    (yylhs.value.arguments) = (yystack_[2].value.arguments);
    (yylhs.value.arguments)->push_back(argument);
  }

    break;

  case 27:

    {
    std::string name = convertAndFree((yystack_[2].value.string));

    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.argument) = new shared_ptr<Argument>(new Argument(name, type));
  }

    break;

  case 28:

    {
    (yylhs.value.results) = new vector<shared_ptr<Result>>();
  }

    break;

  case 29:

    {
    (yylhs.value.results) = (yystack_[1].value.results);
  }

    break;

  case 30:

    {
    auto result = convertAndDelete((yystack_[0].value.result));
    (yylhs.value.results) = new vector<shared_ptr<Result>>;
    (yylhs.value.results)->push_back(result);
  }

    break;

  case 31:

    {
    auto result = convertAndDelete((yystack_[0].value.result));
    (yylhs.value.results) = (yystack_[2].value.results);
    (yylhs.value.results)->push_back(result);
  }

    break;

  case 32:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    auto type = convertAndDelete((yystack_[0].value.type));
    auto result = new Result(name, type);
    (yylhs.value.result) = new shared_ptr<Result>(result);
  }

    break;

  case 33:

    {
    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
  }

    break;

  case 34:

    {
    (yylhs.value.IRNodes) = (yystack_[1].value.IRNodes);
    if ((yystack_[0].value.IRNodes) == NULL) break;  // TODO: Remove check
    (yylhs.value.IRNodes)->insert((yylhs.value.IRNodes)->end(), (yystack_[0].value.IRNodes)->begin(), (yystack_[0].value.IRNodes)->end());
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 40:

    {
    std::string ident = convertAndFree((yystack_[5].value.string));
    std::shared_ptr<TensorType> tensorType = convertAndDelete((yystack_[3].value.tensorType));

    auto literal = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[1].value.TensorLiteral);

    assert(literal->getType()->isTensor() && "Set literals not supported yet");
    auto literalType = literal->getType();

    literal->setName(ident);

    // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
    // cast $tensor_literal to a 1xn matrix.
    const TensorType *literalTensorType = tensorTypePtr(literalType);
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

  case 41:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 42:

    {
    auto lhsList = unique_ptr<vector<unique_ptr<WriteInfo>>>((yystack_[3].value.writeinfos));
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
      const unique_ptr<WriteInfo>  &lhs = *lhsIter;
      const shared_ptr<Expression> &rhs = *rhsIter;

      if (rhs == NULL) continue;

      switch (lhs->kind) {
        case WriteInfo::Kind::VARIABLE: {
          std::string variableName = *lhs->variableName;

          // TODO: Remove check
          if (!ctx->hasSymbol(variableName)) continue;

          assert(ctx->hasSymbol(variableName));

          const RWExprPair &varExprPair = ctx->getSymbol(variableName);
          assert(varExprPair.isWritable());

          shared_ptr<Expression> lhsTensor = varExprPair.getWriteExpr();
          if (auto result = dynamic_pointer_cast<Result>(lhsTensor)) {
            CHECK_TYPE_EQUALITY(*result->getType(), *rhs->getType(), yystack_[2].location);
            rhs->setName(result->getName());
            result->addValue(rhs);
            (yylhs.value.IRNodes)->push_back(rhs);
          }
          else {
            NOT_SUPPORTED_YET;
          }
          break;
        }
        case WriteInfo::Kind::FIELD: {
          std::shared_ptr<FieldWrite> fieldWrite(*lhs->fieldWrite);
          fieldWrite->setValue(rhs);

          auto result = dynamic_pointer_cast<Result>(fieldWrite->getTarget());
          if (result) {
            result->addValue(fieldWrite);
            (yylhs.value.IRNodes)->push_back(fieldWrite);
          }

          break;
        }
        case WriteInfo::Kind::TENSOR: {
          std::shared_ptr<TensorWrite> tensorWrite(*lhs->tensorWrite);
          tensorWrite->setValue(rhs);

          auto result = dynamic_pointer_cast<Result>(tensorWrite->getTensor());
          if (result){
            result->addValue(tensorWrite);
            (yylhs.value.IRNodes)->push_back(tensorWrite);
          }

          break;
        }
      }
    }
  }

    break;

  case 43:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 44:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 45:

    {
    (yylhs.value.IRNodes) = NULL;
    delete (yystack_[3].value.expression);
    delete (yystack_[2].value.IRNodes);
  }

    break;

  case 47:

    {
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 49:

    {
    delete (yystack_[1].value.expression);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 59:

    {
    string ident = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(ident)) { (yylhs.value.expression)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(ident)) {
      REPORT_ERROR(ident + " is not defined in scope", yystack_[0].location);
    }

    const RWExprPair &rwExprPair = ctx->getSymbol(ident);
    if (!rwExprPair.isReadable()) {
      REPORT_ERROR(ident + " is not readable", yystack_[0].location);
    }

    (yylhs.value.expression) = new shared_ptr<Expression>(rwExprPair.getReadExpr());
  }

    break;

  case 60:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = (yystack_[1].value.expression);
  }

    break;

  case 61:

    {
    if ((yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 62:

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

  case 63:

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
      if (!ltype->isColumnVector()) {
        if (!rtype->isColumnVector()) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(*l->getType(), *r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (rtype->isColumnVector()) {
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

  case 64:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 65:

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
        if (!type->isColumnVector()) {
          transposeVector(*(yylhs.value.indexExpr));
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

  case 66:

    {
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 67:

    {  // Solve
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 68:

    { (yylhs.value.binop) = IndexExpr::ADD; }

    break;

  case 69:

    { (yylhs.value.binop) = IndexExpr::SUB; }

    break;

  case 70:

    { (yylhs.value.binop) = IndexExpr::MUL; }

    break;

  case 71:

    { (yylhs.value.binop) = IndexExpr::DIV; }

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
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 74:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 75:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 76:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 77:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 78:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.string) == NULL) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check
    if ((*(yystack_[2].value.expression))->getType() == NULL) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> elemOrSet = convertAndDelete((yystack_[2].value.expression));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    auto type = elemOrSet->getType();
    assert(type);
    if (!(type->isElement() || type->isSet())) {
      std::stringstream errorStr;
      errorStr << "only elements and sets have fields";
      REPORT_ERROR(errorStr.str(), yystack_[2].location);
    }

    (yylhs.value.fieldRead) = new shared_ptr<FieldRead>(new FieldRead(elemOrSet, fieldName));
  }

    break;

  case 82:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto indices =
        unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));

    if (ctx->hasSymbol(name)) {
      const RWExprPair &exprPair = ctx->getSymbol(name);
      if (!exprPair.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      auto tensorExpr = exprPair.getReadExpr();
      (yylhs.value.expression) = new shared_ptr<Expression>(new TensorRead(tensorExpr, *indices));
    }
    else if (ctx->containsFunction(name)) {
      (yylhs.value.expression) = NULL;
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 83:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 84:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto actuals =
        unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));
    auto call = new Call(name, *actuals);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 85:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 86:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 87:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 88:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 89:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 90:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 92:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 97:

    {
    (yylhs.value.writeinfos) = new vector<unique_ptr<WriteInfo>>;
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back(unique_ptr<WriteInfo>((yystack_[0].value.writeinfo)));
  }

    break;

  case 98:

    {
    (yylhs.value.writeinfos) = (yystack_[2].value.writeinfos);
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back(unique_ptr<WriteInfo>((yystack_[0].value.writeinfo)));
  }

    break;

  case 99:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.writeinfo) = new WriteInfo(name);
  }

    break;

  case 100:

    {
    (yylhs.value.writeinfo) = new WriteInfo((yystack_[0].value.fieldWrite));
  }

    break;

  case 101:

    {
    (yylhs.value.writeinfo) = new WriteInfo((yystack_[0].value.tensorWrite));
  }

    break;

  case 102:

    {
    string setName = convertAndFree((yystack_[2].value.string));
    string fieldName = convertAndFree((yystack_[0].value.string));

    if(!ctx->hasSymbol(setName)) { (yylhs.value.fieldWrite)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(setName)) {
      REPORT_ERROR(setName + " is not defined in scope", yystack_[2].location);
    }

    const RWExprPair &setExprPair = ctx->getSymbol(setName);
    if (!setExprPair.isWritable()) {
      REPORT_ERROR(setName + " is not writable", yystack_[2].location);
    }

    auto setExpr = setExprPair.getWriteExpr();
    (yylhs.value.fieldWrite) = new shared_ptr<FieldWrite>(new FieldWrite(setExpr, fieldName));
  }

    break;

  case 103:

    {
    (yylhs.value.fieldWrite) = NULL;
  }

    break;

  case 104:

    {
    std::string tensorName = convertAndFree((yystack_[3].value.string));
    auto indices = unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));

    if(!ctx->hasSymbol(tensorName)) { (yylhs.value.tensorWrite)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(tensorName)) {
      REPORT_ERROR(tensorName + " is not defined in scope", yystack_[3].location);
    }

    const RWExprPair &tensorExprPair = ctx->getSymbol(tensorName);
    if (!tensorExprPair.isWritable()) {
      REPORT_ERROR(tensorName + " is not writable", yystack_[3].location);
    }

    auto tensorExpr = tensorExprPair.getWriteExpr();
    (yylhs.value.tensorWrite) = new shared_ptr<TensorWrite>(new TensorWrite(tensorExpr, *indices));
  }

    break;

  case 105:

    {
    // TODO
    (yylhs.value.tensorWrite) = NULL;
  }

    break;

  case 106:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.elementType));
  }

    break;

  case 107:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.setType));
  }

    break;

  case 108:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.tupleType));
  }

    break;

  case 109:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.tensorType));
  }

    break;

  case 110:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.elementType) = new std::shared_ptr<ElementType>(ctx->getElementType(name));
  }

    break;

  case 111:

    {
    std::shared_ptr<ElementType> elementType = convertAndDelete((yystack_[1].value.elementType));
    (yylhs.value.setType) = new std::shared_ptr<SetType>(new SetType(elementType));
  }

    break;

  case 112:

    {
    std::shared_ptr<ElementType> elementType = convertAndDelete((yystack_[4].value.elementType));
    auto eps = convertAndDelete((yystack_[1].value.expressions));

    // TODO: Add endpoint information to set type
    (yylhs.value.setType) = new std::shared_ptr<SetType>(new SetType(elementType));
  }

    break;

  case 113:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>;
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 114:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 115:

    {
    (yylhs.value.tupleType) = new std::shared_ptr<TupleType>(new TupleType(*(yystack_[1].value.elementTypes)));
    delete (yystack_[1].value.elementTypes);
  }

    break;

  case 116:

    {
    (yylhs.value.elementTypes) = (yystack_[0].value.elementTypes);
  }

    break;

  case 117:

    {
    (yylhs.value.elementTypes) = (yystack_[2].value.elementTypes);
    (yylhs.value.elementTypes)->insert((yylhs.value.elementTypes)->begin(), (yystack_[0].value.elementTypes)->begin(), (yystack_[0].value.elementTypes)->end());
    delete (yystack_[0].value.elementTypes);
  }

    break;

  case 118:

    {
    auto elementType = convertAndDelete((yystack_[0].value.elementType));
    (yylhs.value.elementTypes) = new vector<shared_ptr<ElementType>>;
    (yylhs.value.elementTypes)->push_back(elementType);
  }

    break;

  case 119:

    {
    auto elementType = convertAndDelete((yystack_[2].value.elementType));

    if ((yystack_[0].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[0].location);
    }

    (yylhs.value.elementTypes) = new vector<shared_ptr<ElementType>>;
    for (int i=0; i < (yystack_[0].value.num); ++i) {
      (yylhs.value.elementTypes)->push_back(elementType);
    }
  }

    break;

  case 120:

    {
    (yylhs.value.tensorType) = new shared_ptr<TensorType>(new TensorType((yystack_[0].value.componentType)));
  }

    break;

  case 121:

    {
    (yylhs.value.tensorType) = (yystack_[1].value.tensorType);
  }

    break;

  case 122:

    {
    auto blockType = convertAndDelete((yystack_[1].value.tensorType));
    auto componentType = blockType->getComponentType();

    auto outerDimensions = unique_ptr<vector<IndexSet>>((yystack_[4].value.indexSets));
    auto blockDimensions = blockType->getDimensions();

    vector<IndexDomain> dimensions;
    if (blockType->getOrder() == 0) {
      for (size_t i=0; i<outerDimensions->size(); ++i) {
        dimensions.push_back(IndexDomain((*outerDimensions)[i]));
      }
    }
    else {
      // TODO: Handle the following cases where there there are more inner than
      //       outer dimensions (e.g. a vector of matrixes) and where there are
      //       more outer than inner dimensions (e.g. a matrix of vectors)
      //       gracefully by padding with '1'-dimensions.
      // TODO: Handle case where there are more block than outer dimensions
      // TODO: Handle case where there are more outer than block dimensions
      // TODO: Remove below error
      if (blockType->getOrder() != outerDimensions->size()) {
        REPORT_ERROR("Blocktype order (" + to_string(blockType->getOrder()) +
                     ") differ from number of dimensions", yystack_[4].location);
      }

      assert(blockDimensions.size() == outerDimensions->size());
      for (size_t i=0; i<outerDimensions->size(); ++i) {
        vector<IndexSet> dimension;
        dimension.push_back((*outerDimensions)[i]);
        dimension.insert(dimension.begin(),
                         blockDimensions[i].getFactors().begin(),
                         blockDimensions[i].getFactors().end());

        dimensions.push_back(IndexDomain(dimension));
      }
    }

    (yylhs.value.tensorType) = new shared_ptr<TensorType>(new TensorType(componentType, dimensions));
  }

    break;

  case 123:

    {
    auto tensorType = convertAndDelete((yystack_[1].value.tensorType));
    auto dimensions = tensorType->getDimensions();
    auto componentType = tensorType->getComponentType();
    (yylhs.value.tensorType) = new shared_ptr<TensorType>(new TensorType(componentType, dimensions,
                                                   true /* isColumnVector */));
  }

    break;

  case 124:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 125:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 126:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 127:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(ident);
  }

    break;

  case 128:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 129:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 130:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 133:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    transposeVector(*(yylhs.value.TensorLiteral));
  }

    break;

  case 135:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::FLOAT, isps);
    auto literal = new Literal(shared_ptr<TensorType>(type), // TODO: <Type>
                               values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 136:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::INT, isps);
    auto literal = new Literal(shared_ptr<TensorType>(type), // TODO: <Type>
                               values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 137:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 139:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 140:

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

  case 141:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 142:

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

  case 143:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 144:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 145:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 147:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 148:

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

  case 149:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 150:

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

  case 151:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 152:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 153:

    {
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 154:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 155:

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


  const short int  Parser ::yypact_ninf_ = -179;

  const signed char  Parser ::yytable_ninf_ = -100;

  const short int
   Parser ::yypact_[] =
  {
    -179,    16,  -179,  -179,  -179,    67,    45,    65,    82,    96,
      32,    77,   114,    32,    13,  -179,    32,  -179,  -179,  -179,
    -179,   124,  -179,   119,  -179,  -179,  -179,  -179,  -179,  -179,
     171,   118,   126,  -179,  -179,   115,   137,  -179,  -179,  -179,
    -179,   -15,  -179,   147,  -179,  -179,  -179,   133,  -179,  -179,
      32,   174,  -179,   139,   145,   143,   169,   161,   193,   170,
    -179,   180,   175,   150,  -179,  -179,    53,   195,   194,   172,
     211,   198,   217,   213,   219,    63,   250,  -179,   266,  -179,
      32,    32,    32,  -179,  -179,  -179,    32,    32,  -179,  -179,
      32,  -179,    32,    32,    32,    32,    32,    32,   267,    32,
     269,    12,    32,    32,  -179,   193,   247,    46,  -179,    21,
     163,    57,  -179,   279,    32,   120,    32,    20,  -179,    41,
      42,  -179,   256,   294,   295,  -179,   270,   298,   299,  -179,
     148,   275,   148,   249,   249,   193,    63,    63,    63,   216,
     236,   236,   249,   249,   193,  -179,   276,   268,  -179,    33,
     126,  -179,    79,   104,   271,  -179,    32,   272,  -179,  -179,
    -179,  -179,   107,    58,  -179,  -179,   274,   301,  -179,  -179,
    -179,  -179,   261,   293,  -179,   288,   144,   284,   277,  -179,
    -179,   294,   211,  -179,   298,   219,  -179,   290,    82,   291,
    -179,    32,  -179,  -179,   193,   163,   163,     6,    20,  -179,
     301,   278,   110,  -179,   311,   300,  -179,    32,  -179,  -179,
    -179,    83,    86,  -179,   289,   286,  -179,  -179,    46,    28,
     -14,  -179,  -179,  -179,    69,  -179,   281,   292,   320,  -179,
     301,  -179,   106,  -179,   193,   148,  -179,  -179,   297,    82,
    -179,  -179,   302,     6,  -179,   303,  -179,  -179,  -179,  -179,
    -179,   148,   304,  -179,  -179,   163,  -179,   321,   322,     4,
    -179,   129,   296,   132,  -179,  -179,  -179,   323,    57,  -179,
     322,  -179,  -179,  -179
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   153,   154,    59,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    44,     0,     3,     7,     4,
       5,     0,     6,     0,     8,    35,    36,    37,    38,    39,
       0,    50,    51,    53,    54,    55,     0,    56,    81,    57,
      58,     0,    97,   100,   101,    52,   131,   132,   134,     9,
      85,     0,    12,     0,     0,     0,     0,    59,    33,    51,
      41,     0,     0,     0,   151,   143,     0,     0,   138,   137,
     141,     0,   146,   145,   149,    61,     0,    33,     0,    33,
       0,     0,     0,    43,    68,    69,     0,     0,    70,    71,
       0,    65,     0,     0,     0,     0,     0,     0,     0,    85,
       0,     0,     0,     0,   133,    87,     0,     0,   102,     0,
       0,     0,    10,     0,    85,    48,    85,     0,    60,     0,
       0,   135,     0,     0,     0,   136,     0,     0,     0,    18,
      16,     0,    20,    74,    75,    89,    63,    64,    66,    67,
      72,    73,    76,    77,    62,   103,     0,    86,    78,    99,
       0,    98,     0,     0,    82,   104,     0,     0,    11,    13,
     129,   130,     0,     0,   120,   110,     0,     0,    27,   106,
     107,   108,   109,    91,    34,     0,     0,     0,     0,   139,
     147,     0,   142,   144,     0,   150,   152,     0,    23,     0,
      83,     0,    42,   105,    88,     0,     0,     0,     0,   123,
       0,   118,     0,   116,     0,    93,    45,     0,    33,    84,
     155,     0,     0,    17,     0,    24,    25,    21,     0,     0,
       0,   126,   127,   128,     0,   124,     0,     0,     0,   115,
       0,    92,     0,    90,    33,    47,   140,   148,    28,     0,
      14,   121,     0,     0,    40,   111,   119,   117,    95,    96,
      94,    49,     0,    22,    26,     0,   125,     0,     0,     0,
     113,     0,     0,     0,    30,   122,   112,     0,     0,    29,
       0,   114,    32,    31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
    -179,  -179,  -179,  -179,  -179,  -179,  -179,  -178,  -179,  -179,
      68,   -75,   327,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
      -1,  -179,     0,  -179,  -179,  -179,  -179,  -179,  -179,   325,
     -67,   -47,  -179,  -179,  -179,  -179,  -179,  -179,   234,  -179,
    -179,    71,  -159,  -179,  -179,  -179,  -179,   111,   -97,  -179,
      97,  -179,  -179,  -110,  -179,  -179,  -179,   -60,   220,  -179,
    -179,   -61,   215,  -179,  -179
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    17,    18,    19,   109,   159,    20,    21,   187,
      77,    22,    23,   189,    79,   214,   215,    55,   253,   263,
     264,   115,   174,    25,    26,    27,    28,    29,   175,   176,
     105,    31,    59,    33,    97,    34,    35,    36,    37,    38,
     106,   147,    39,    40,   205,   233,   250,    41,    42,    43,
      44,   168,   169,   170,   261,   171,   202,   203,   172,   224,
     225,   164,    45,    46,    47,    67,    68,    69,    70,    71,
      72,    73,    74,    48,    49
  };

  const short int
   Parser ::yytable_[] =
  {
      30,    32,   130,   107,   132,   120,   119,   178,   201,    58,
     216,   221,    63,   163,   222,    75,     2,   241,    64,    65,
     149,     3,     4,   101,     5,     3,     4,   102,     6,   157,
       7,     8,   146,   -19,     9,   265,   199,     3,     4,    10,
      57,   227,    13,    11,    12,    66,    13,   158,    14,   177,
       9,   223,    14,    52,   199,   152,   153,    15,    64,    65,
      16,   254,    13,   191,    14,   165,   160,   161,   162,   240,
     166,   201,    51,    53,   179,   180,    16,   155,   199,   133,
     134,   135,   123,   127,   156,   136,   137,   167,   226,   138,
      54,   139,   140,   141,   142,   143,   144,    50,   219,   220,
     198,   150,   242,    82,    56,   -99,    51,   243,   199,   -99,
      88,    89,    90,    91,    30,    32,   236,   156,    60,   237,
     192,   211,    61,   212,   123,     3,     4,   127,     5,    30,
      32,    30,    32,   235,     7,   193,    78,   196,     9,   197,
      76,   229,   156,    10,   218,    99,   -46,    11,   230,   248,
      13,   249,    14,     3,     4,   194,     5,   -79,   259,   251,
     266,    15,     7,   269,    16,    98,     9,   267,   207,   208,
     270,    10,   160,   161,   162,    11,   100,   103,    13,   110,
      14,   118,   108,   104,   112,   111,    80,    81,   113,    15,
      82,   114,    16,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,   234,    80,    81,   -80,
     116,    82,    83,   123,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,   117,   121,    80,
      81,   125,   122,    82,    30,    32,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,   124,
      30,    32,    80,    81,   127,   126,    82,   128,   129,    84,
      85,    86,    87,    88,    89,    90,    91,  -100,    93,    94,
      95,    96,    80,    81,   131,   145,    82,   148,   154,    84,
      85,    86,    87,    88,    89,    90,    91,   173,   181,    82,
      95,    96,    84,    85,    86,    87,    88,    89,    90,    91,
      65,   183,   184,    64,   186,   188,   156,   190,   200,   165,
     -84,   199,   195,   204,   206,   209,   213,   217,   210,   231,
     238,   232,   244,   228,   239,   246,   252,   245,    24,   260,
     262,   271,   255,   257,   258,   151,   268,    62,   273,   272,
     256,   247,   185,   182
  };

  const unsigned short int
   Parser ::yycheck_[] =
  {
       1,     1,    77,    50,    79,    66,    66,   117,   167,    10,
     188,     5,    13,   110,     8,    16,     0,    31,     5,     6,
       8,     5,     6,    38,     8,     5,     6,    42,    12,     8,
      14,    15,    99,    17,    18,    31,    50,     5,     6,    23,
       8,   200,    30,    27,    28,    32,    30,    26,    32,   116,
      18,    45,    32,     8,    50,   102,   103,    41,     5,     6,
      44,   239,    30,    30,    32,     8,     9,    10,    11,    41,
      13,   230,    39,     8,    33,    33,    44,    31,    50,    80,
      81,    82,    41,    41,    38,    86,    87,    30,   198,    90,
       8,    92,    93,    94,    95,    96,    97,    30,   195,   196,
      42,   101,    33,    40,     8,    38,    39,    38,    50,    42,
      47,    48,    49,    50,   115,   115,    33,    38,    41,    33,
      41,   181,     8,   184,    41,     5,     6,    41,     8,   130,
     130,   132,   132,   208,    14,    31,    17,    30,    18,    32,
      16,    31,    38,    23,   191,    30,    26,    27,    38,    43,
      30,    45,    32,     5,     6,   156,     8,    39,   255,   234,
      31,    41,    14,    31,    44,    39,    18,    38,    24,    25,
      38,    23,     9,    10,    11,    27,    39,    30,    30,    40,
      32,    31,     8,    50,    41,    40,    36,    37,    19,    41,
      40,    30,    44,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   207,    36,    37,    39,
      30,    40,    41,    41,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    52,    33,    36,
      37,    33,    38,    40,   235,   235,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    38,
     251,   251,    36,    37,    41,    38,    40,    38,     8,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    36,    37,     8,     8,    40,     8,    31,    43,
      44,    45,    46,    47,    48,    49,    50,     8,    32,    40,
      54,    55,    43,    44,    45,    46,    47,    48,    49,    50,
       6,     6,    32,     5,     5,    30,    38,    31,    34,     8,
      39,    50,    40,    20,    26,    31,    26,    26,    41,     8,
      31,    21,    41,    45,    38,     5,    29,    35,     1,     8,
       8,     8,    30,    30,    30,   101,    40,    12,   270,   268,
     243,   230,   127,   123
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    57,     0,     5,     6,     8,    12,    14,    15,    18,
      23,    27,    28,    30,    32,    41,    44,    58,    59,    60,
      63,    64,    67,    68,    78,    79,    80,    81,    82,    83,
      86,    87,    88,    89,    91,    92,    93,    94,    95,    98,
      99,   103,   104,   105,   106,   118,   119,   120,   129,   130,
      30,    39,     8,     8,     8,    73,     8,     8,    86,    88,
      41,     8,    95,    86,     5,     6,    32,   121,   122,   123,
     124,   125,   126,   127,   128,    86,    16,    66,    17,    70,
      36,    37,    40,    41,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    90,    39,    30,
      39,    38,    42,    30,    50,    86,    96,    97,     8,    61,
      40,    40,    41,    19,    30,    77,    30,    52,    31,   123,
     127,    33,    38,    41,    38,    33,    38,    41,    38,     8,
      77,     8,    77,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,     8,    96,    97,     8,     8,
      88,   104,    97,    97,    31,    31,    38,     8,    26,    62,
       9,    10,    11,   114,   117,     8,    13,    30,   107,   108,
     109,   111,   114,     8,    78,    84,    85,    96,   119,    33,
      33,    32,   124,     6,    32,   128,     5,    65,    30,    69,
      31,    30,    41,    31,    86,    40,    30,    32,    42,    50,
      34,   108,   112,   113,    20,   100,    26,    24,    25,    31,
      41,   123,   127,    26,    71,    72,    73,    26,    97,   114,
     114,     5,     8,    45,   115,   116,   119,   108,    45,    31,
      38,     8,    21,   101,    86,    77,    33,    33,    31,    38,
      41,    31,    33,    38,    41,    35,     5,   113,    43,    45,
     102,    77,    29,    74,    73,    30,   116,    30,    30,   114,
       8,   110,     8,    75,    76,    31,    31,    38,    40,    31,
      38,     8,   107,    76
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    56,    57,    57,    58,    58,    58,    58,    58,    58,
      59,    60,    61,    61,    62,    64,    65,    63,    66,    68,
      69,    67,    70,    71,    71,    72,    72,    73,    74,    74,
      75,    75,    76,    77,    77,    78,    78,    78,    78,    78,
      79,    80,    81,    82,    82,    83,    84,    84,    85,    85,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    87,
      88,    89,    89,    89,    89,    89,    89,    89,    90,    90,
      90,    90,    91,    91,    91,    91,    91,    91,    92,    93,
      93,    93,    94,    94,    95,    96,    96,    97,    97,    98,
      99,   100,   100,   101,   101,   102,   102,   103,   103,   104,
     104,   104,   105,   105,   106,   106,   107,   107,   107,   107,
     108,   109,   109,   110,   110,   111,   112,   112,   113,   113,
     114,   114,   114,   114,   115,   115,   116,   116,   116,   117,
     117,   118,   119,   119,   119,   120,   120,   121,   121,   122,
     122,   123,   123,   124,   124,   125,   125,   126,   126,   127,
     127,   128,   128,   129,   129,   130
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     2,     1,     1,     1,     1,     1,
       7,     2,     4,     2,     1,     5,     0,     3,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     3,     3,     3,     2,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     4,     4,     4,     0,     1,     1,     3,     3,
       6,     0,     2,     0,     2,     1,     1,     1,     3,     1,
       1,     1,     3,     3,     4,     4,     1,     1,     1,     1,
       1,     4,     7,     1,     3,     3,     1,     3,     1,     3,
       1,     4,     7,     2,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     3,     3,     1,     1,     3,
       5,     1,     3,     1,     3,     1,     1,     3,     5,     1,
       3,     1,     3,     1,     1,     5
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "NEG",
  "\"int literal\"", "\"float literal\"", "\"string literal\"",
  "\"identifier\"", "\"int\"", "\"float\"", "\"tensor\"", "\"element\"",
  "\"set\"", "\"const\"", "\"extern\"", "\"proc\"", "\"func\"", "\"map\"",
  "\"to\"", "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"",
  "\"else\"", "\"end\"", "\"return\"", "\"%!\"", "\"->\"", "\"(\"",
  "\")\"", "\"[\"", "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\",\"",
  "\".\"", "\":\"", "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"",
  "\".*\"", "\"./\"", "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"", "\"!=\"",
  "\"<=\"", "\">=\"", "$accept", "program", "program_element", "extern",
  "element_type_decl", "field_decl_list", "field_decl", "procedure", "$@1",
  "$@2", "procedure_header", "function", "$@3", "$@4", "function_header",
  "arguments", "argument_list", "argument_decl", "results", "result_list",
  "result_decl", "stmt_block", "stmt", "const_stmt", "return_stmt",
  "assign_stmt", "expr_stmt", "if_stmt", "else_clauses", "elif_clauses",
  "expr", "ident_expr", "paren_expr", "linear_algebra_expr",
  "elwise_binary_op", "boolean_expr", "field_read_expr", "set_read_expr",
  "call_or_tensor_read_expr", "call_expr", "expr_list_or_empty",
  "expr_list", "range_expr", "map_expr", "with", "reduce", "reduction_op",
  "write_expr_list", "write_expr", "field_write_expr", "tensor_write_expr",
  "type", "element_type", "set_type", "endpoints", "tuple_type",
  "tuple_element_types", "tuple_element_type", "tensor_type", "index_sets",
  "index_set", "component_type", "literal_expr", "tensor_literal",
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
       0,   248,   248,   250,   254,   255,   263,   271,   274,   278,
     285,   299,   313,   316,   324,   344,   344,   344,   352,   374,
     374,   374,   382,   406,   409,   415,   420,   428,   437,   440,
     446,   451,   459,   470,   473,   482,   483,   484,   485,   486,
     490,   520,   526,   603,   606,   612,   618,   620,   624,   626,
     640,   641,   642,   643,   644,   645,   646,   647,   648,   654,
     675,   691,   699,   711,   776,   782,   812,   817,   826,   827,
     828,   829,   835,   841,   847,   853,   859,   865,   880,   900,
     901,   902,   913,   934,   940,   950,   953,   959,   965,   976,
     987,   995,   997,  1001,  1003,  1006,  1007,  1055,  1060,  1068,
    1072,  1075,  1081,  1099,  1105,  1123,  1147,  1150,  1153,  1156,
    1162,  1169,  1173,  1183,  1187,  1194,  1201,  1204,  1212,  1217,
    1232,  1235,  1238,  1278,  1288,  1293,  1301,  1304,  1308,  1314,
    1317,  1386,  1389,  1390,  1394,  1397,  1406,  1417,  1424,  1427,
    1431,  1444,  1448,  1462,  1466,  1472,  1479,  1482,  1486,  1499,
    1503,  1517,  1521,  1527,  1532,  1542
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55
    };
    const unsigned int user_token_number_max_ = 310;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



