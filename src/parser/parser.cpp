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

      case 60: // program_element


        { delete (yysym.value.IRNode); }

        break;

      case 61: // extern


        { delete (yysym.value.expression); }

        break;

      case 62: // element_type_decl


        { delete (yysym.value.elementType); }

        break;

      case 63: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 64: // field_decl


        { delete (yysym.value.field); }

        break;

      case 65: // procedure


        { delete (yysym.value.function); }

        break;

      case 68: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 69: // function


        { delete (yysym.value.function); }

        break;

      case 72: // function_header


        { delete (yysym.value.function); }

        break;

      case 73: // arguments


        { delete (yysym.value.arguments); }

        break;

      case 74: // argument_list


        { delete (yysym.value.arguments); }

        break;

      case 75: // argument_decl


        { delete (yysym.value.argument); }

        break;

      case 76: // results


        { delete (yysym.value.results); }

        break;

      case 77: // result_list


        { delete (yysym.value.results); }

        break;

      case 78: // result_decl


        { delete (yysym.value.result); }

        break;

      case 79: // stmt_block


        { delete (yysym.value.IRNodes); }

        break;

      case 80: // stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 81: // const_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 82: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 83: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 84: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 85: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 88: // for_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 89: // expr


        { delete (yysym.value.expression); }

        break;

      case 90: // ident_expr


        { delete (yysym.value.expression); }

        break;

      case 91: // paren_expr


        { delete (yysym.value.expression); }

        break;

      case 92: // linear_algebra_expr


        { delete (yysym.value.indexExpr); }

        break;

      case 93: // elwise_binary_op


        {}

        break;

      case 94: // boolean_expr


        { delete (yysym.value.expression); }

        break;

      case 95: // field_read_expr


        { delete (yysym.value.fieldRead); }

        break;

      case 96: // set_read_expr


        { delete (yysym.value.expression); }

        break;

      case 97: // call_or_paren_read_expr


        { delete (yysym.value.expression); }

        break;

      case 98: // call_expr


        { delete (yysym.value.call); }

        break;

      case 99: // expr_list_or_empty


        { delete (yysym.value.expressions); }

        break;

      case 100: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 101: // map_expr


        { delete (yysym.value.expression); }

        break;

      case 105: // write_expr_list


        { delete (yysym.value.writeinfos); }

        break;

      case 106: // write_expr


        { delete (yysym.value.writeinfo); }

        break;

      case 107: // field_write_expr


        { delete (yysym.value.fieldWrite); }

        break;

      case 108: // tensor_write_expr


        { delete (yysym.value.tensorWrite); }

        break;

      case 109: // type


        { delete (yysym.value.type); }

        break;

      case 110: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 111: // set_type


        { delete (yysym.value.setType); }

        break;

      case 112: // endpoints


        { delete (yysym.value.expressions); }

        break;

      case 113: // tuple_type


        { delete (yysym.value.tupleType); }

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

  case 41:

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

  case 42:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 43:

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

  case 44:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 45:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 46:

    {
    (yylhs.value.IRNodes) = NULL;
    delete (yystack_[3].value.expression);
    delete (yystack_[2].value.IRNodes);
  }

    break;

  case 48:

    {
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 50:

    {
    delete (yystack_[1].value.expression);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 51:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 60:

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

  case 61:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = (yystack_[1].value.expression);
  }

    break;

  case 62:

    {
    if ((yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 63:

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

  case 64:

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

  case 65:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 66:

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

  case 67:

    {
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 68:

    {  // Solve
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 69:

    { (yylhs.value.binop) = IndexExpr::ADD; }

    break;

  case 70:

    { (yylhs.value.binop) = IndexExpr::SUB; }

    break;

  case 71:

    { (yylhs.value.binop) = IndexExpr::MUL; }

    break;

  case 72:

    { (yylhs.value.binop) = IndexExpr::DIV; }

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
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 79:

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

  case 83:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto indices =
        unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));

    if (ctx->hasSymbol(name)) {
      const RWExprPair &exprPair = ctx->getSymbol(name);
      if (!exprPair.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      // The parenthesis read can read from a tensor or a tuple.
      auto expr = exprPair.getReadExpr();
      if (expr->getType()->isTensor()) {
        (yylhs.value.expression) = new shared_ptr<Expression>(new TensorRead(expr, *indices));
      }
      else if (expr->getType()->isTuple()) {
        if (indices->size() != 1) {
          REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
        }

        (yylhs.value.expression) = new shared_ptr<Expression>(new TupleRead(expr, (*indices)[0]));
      }
      else {
        REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
      }
    }
    else if (ctx->containsFunction(name)) {
      (yylhs.value.expression) = NULL;
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 84:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 85:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto actuals =
        unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));
    auto call = new Call(name, *actuals);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 86:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 87:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 88:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 89:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
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
    auto elementType = convertAndDelete((yystack_[3].value.elementType));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    auto tupleType = new TupleType(elementType, (yystack_[1].value.num));
    (yylhs.value.tupleType) = new std::shared_ptr<TupleType>(tupleType);
  }

    break;

  case 116:

    {
    (yylhs.value.tensorType) = new shared_ptr<TensorType>(new TensorType((yystack_[0].value.componentType)));
  }

    break;

  case 117:

    {
    (yylhs.value.tensorType) = (yystack_[1].value.tensorType);
  }

    break;

  case 118:

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

  case 119:

    {
    auto tensorType = convertAndDelete((yystack_[1].value.tensorType));
    auto dimensions = tensorType->getDimensions();
    auto componentType = tensorType->getComponentType();
    (yylhs.value.tensorType) = new shared_ptr<TensorType>(new TensorType(componentType, dimensions,
                                                   true /* isColumnVector */));
  }

    break;

  case 120:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 121:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 122:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 123:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(ident);
  }

    break;

  case 124:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 125:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 126:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 127:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 130:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    transposeVector(*(yylhs.value.TensorLiteral));
  }

    break;

  case 132:

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

  case 133:

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

  case 134:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 136:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 137:

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

  case 138:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 139:

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

  case 140:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 141:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 142:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 144:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 145:

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

  case 146:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 147:

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

  case 148:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 149:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 150:

    {
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 151:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 152:

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


  const short int  Parser ::yypact_ninf_ = -186;

  const short int  Parser ::yytable_ninf_ = -151;

  const short int
   Parser ::yypact_[] =
  {
    -186,    11,  -186,  -186,  -186,    78,    14,    24,    62,    85,
     108,   128,    59,   135,   108,    15,  -186,   108,  -186,  -186,
    -186,  -186,   105,  -186,   138,  -186,  -186,  -186,  -186,  -186,
    -186,  -186,   238,   122,   129,  -186,  -186,   146,   139,  -186,
    -186,  -186,   113,  -186,   153,  -186,  -186,  -186,   141,  -186,
    -186,   108,   152,  -186,   145,   148,   157,   177,   171,   278,
     164,   181,  -186,   178,   158,   218,  -186,  -186,    47,   179,
     175,   174,   184,   183,   186,   185,   190,   123,   223,  -186,
     224,  -186,   108,   108,  -186,  -186,  -186,   108,   108,  -186,
    -186,   108,  -186,   108,   108,   108,   108,   108,   108,   226,
     108,   232,     7,   108,   108,  -186,   278,   209,    84,  -186,
      16,   173,    58,  -186,   237,   108,   133,    30,   108,    25,
    -186,    40,    57,  -186,   212,   242,   247,  -186,   220,   254,
     255,  -186,   193,   229,   193,   351,   351,   123,   123,   123,
     298,   338,   338,   351,   351,   278,  -186,   245,   222,  -186,
      19,   129,  -186,    94,    87,   239,  -186,   108,   240,  -186,
    -186,  -186,  -186,   112,    55,  -186,  -186,   243,   290,  -186,
    -186,  -186,  -186,   249,   279,  -186,   274,    56,   318,   215,
    -186,   258,  -186,   285,   276,  -186,  -186,   242,   184,  -186,
     254,   190,  -186,   292,    62,   293,  -186,   108,  -186,  -186,
     278,   173,   173,    30,    25,  -186,   290,   275,   330,   319,
    -186,   108,  -186,   108,   163,  -186,  -186,    68,    69,  -186,
     306,   301,  -186,  -186,    84,    20,   -19,   110,  -186,   299,
     321,   354,  -186,   119,  -186,   278,   193,   278,  -186,  -186,
    -186,   331,    62,  -186,  -186,   329,    30,  -186,   332,   345,
    -186,  -186,  -186,   193,   347,  -186,  -186,   173,  -186,   372,
    -186,   373,    -6,  -186,    90,   340,    92,  -186,  -186,  -186,
     383,    58,  -186,   373,  -186,  -186,  -186
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   150,   151,    60,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    45,     0,     3,     7,
       4,     5,     0,     6,     0,     8,    35,    36,    37,    38,
      39,    40,     0,    52,    53,    55,    56,    57,     0,    58,
      82,    59,     0,    97,   100,   101,    54,   128,   129,   131,
       9,    86,     0,    12,     0,     0,     0,     0,    60,    33,
      53,     0,    42,     0,     0,     0,   148,   140,     0,     0,
     135,   134,   138,     0,   143,   142,   146,    62,     0,    33,
       0,    33,     0,     0,    44,    69,    70,     0,     0,    71,
      72,     0,    66,     0,     0,     0,     0,     0,     0,     0,
      86,     0,     0,     0,     0,   130,    88,     0,     0,   102,
       0,     0,     0,    10,     0,    86,    49,     0,    86,     0,
      61,     0,     0,   132,     0,     0,     0,   133,     0,     0,
       0,    18,    16,     0,    20,    75,    76,    64,    65,    67,
      68,    73,    74,    77,    78,    63,   103,     0,    87,    79,
      99,     0,    98,     0,     0,    83,   104,     0,     0,    11,
      13,   126,   127,     0,     0,   116,   110,     0,     0,    27,
     106,   107,   108,   109,    91,    34,     0,     0,   122,    60,
     125,     0,    33,     0,     0,   136,   144,     0,   139,   141,
       0,   147,   149,     0,    23,     0,    84,     0,    43,   105,
      89,     0,     0,     0,     0,   119,     0,     0,     0,    93,
      46,     0,    33,     0,     0,    85,   152,     0,     0,    17,
       0,    24,    25,    21,     0,     0,     0,     0,   120,     0,
       0,     0,    92,     0,    90,    33,    48,   124,    51,   137,
     145,    28,     0,    14,   117,     0,     0,    41,   111,     0,
      95,    96,    94,    50,     0,    22,    26,     0,   121,     0,
     115,     0,     0,   113,     0,     0,     0,    30,   118,   112,
       0,     0,    29,     0,   114,    32,    31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -184,  -186,  -186,
     120,   -77,   391,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,    -9,  -186,    -1,  -186,  -186,  -186,  -186,  -186,  -186,
     392,   -53,   -48,  -186,  -186,  -186,  -186,  -186,   302,  -186,
    -186,   136,  -156,  -186,  -186,  -186,  -105,  -186,  -185,  -186,
    -186,  -106,  -186,  -186,  -186,   -59,   281,  -186,  -186,   -61,
     280,  -186,  -186
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    18,    19,    20,   110,   160,    21,    22,   193,
      79,    23,    24,   195,    81,   220,   221,    56,   255,   266,
     267,   116,   175,    26,    27,    28,    29,    30,   176,   177,
      31,    32,    33,    60,    35,    98,    36,    37,    38,    39,
      40,   107,   148,    41,   209,   234,   252,    42,    43,    44,
      45,   169,   170,   171,   264,   172,   173,   227,   182,   165,
      46,    47,    48,    69,    70,    71,    72,    73,    74,    75,
      76,    49,    50
  };

  const short int
   Parser ::yytable_[] =
  {
      34,    59,   132,   108,   134,    65,   164,   122,    77,   121,
     222,     2,   207,   184,   244,   150,     3,     4,   228,     5,
      66,    67,    53,     6,   158,     7,     8,   268,   -19,     9,
       3,     4,    54,   205,    10,   178,     4,    11,   179,    14,
      12,    13,   106,    14,   159,    15,   205,   147,     9,    68,
     230,   197,    66,    67,    16,   153,   154,    17,   256,    15,
      52,   258,    14,   243,    15,   183,   166,   161,   162,   163,
      55,   167,   205,   135,   136,   185,    17,   180,   137,   138,
     211,   212,   139,   125,   140,   141,   142,   143,   144,   145,
     168,   106,   186,    57,   106,   106,   225,   226,   229,   204,
     129,   151,    62,   239,   240,   214,   106,   205,   181,   106,
      51,   125,   129,     3,     4,    34,    58,   156,   -99,    52,
     199,    78,   -99,   269,   157,   272,     9,   157,   217,   218,
     270,    34,   273,    34,   157,   236,    61,   198,     3,     4,
      14,     5,    15,    63,   202,   245,   203,     7,   200,   224,
     246,     9,   262,   102,    17,    80,    10,   103,   253,    11,
     109,   -47,    12,   -80,   250,    14,   251,    15,     3,     4,
      99,     5,    89,    90,    91,    92,    16,     7,   100,    17,
     101,     9,   161,   162,   163,   104,    10,   111,   106,    11,
     112,   238,    12,   105,   181,    14,   114,    15,     3,     4,
     113,     5,   235,   115,   237,   -81,    16,     7,   117,    17,
     118,     9,   119,    34,   123,   124,    10,   125,   127,    11,
    -123,  -123,    12,  -123,   126,    14,   128,    15,   129,  -123,
     130,   131,   133,  -123,   146,    34,    16,   181,  -123,    17,
     149,  -123,   155,  -123,  -123,   174,   187,   115,    67,  -123,
    -123,   120,    34,   189,   190,  -123,    82,    83,  -123,    66,
     192,   194,   157,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    82,    83,   196,   206,
     -85,    84,   201,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    82,    83,   166,   208,
     213,   205,   210,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    82,    83,   215,   216,
     219,   223,   231,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    82,    83,   232,   241,
     233,   242,   247,    85,    86,    87,    88,    89,    90,    91,
      92,  -151,    94,    95,    96,    97,  -150,  -150,   248,   249,
    -150,   257,   254,  -150,   259,  -150,  -150,  -150,  -150,  -150,
    -150,  -150,  -150,  -150,  -150,  -150,    82,    83,   260,   261,
     263,   265,   271,    85,    86,    87,    88,    89,    90,    91,
      92,   274,    25,   276,    96,    97,    85,    86,    87,    88,
      89,    90,    91,    92,   152,    64,   188,   275,     0,   191
  };

  const short int
   Parser ::yycheck_[] =
  {
       1,    10,    79,    51,    81,    14,   111,    68,    17,    68,
     194,     0,   168,   119,    33,     8,     5,     6,   203,     8,
       5,     6,     8,    12,     8,    14,    15,    33,    17,    18,
       5,     6,     8,    52,    23,     5,     6,    26,     8,    32,
      29,    30,    51,    32,    28,    34,    52,   100,    18,    34,
     206,    32,     5,     6,    43,   103,   104,    46,   242,    34,
      41,   246,    32,    43,    34,   118,     8,     9,    10,    11,
       8,    13,    52,    82,    83,    35,    46,    47,    87,    88,
      24,    25,    91,    43,    93,    94,    95,    96,    97,    98,
      32,   100,    35,     8,   103,   104,   201,   202,   204,    44,
      43,   102,    43,    35,    35,   182,   115,    52,   117,   118,
      32,    43,    43,     5,     6,   116,     8,    33,    40,    41,
      33,    16,    44,    33,    40,    33,    18,    40,   187,   190,
      40,   132,    40,   134,    40,   212,     8,    43,     5,     6,
      32,     8,    34,     8,    32,    35,    34,    14,   157,   197,
      40,    18,   257,    40,    46,    17,    23,    44,   235,    26,
       8,    28,    29,    41,    45,    32,    47,    34,     5,     6,
      41,     8,    49,    50,    51,    52,    43,    14,    32,    46,
      41,    18,     9,    10,    11,    32,    23,    42,   197,    26,
      42,    28,    29,    52,   203,    32,    19,    34,     5,     6,
      43,     8,   211,    32,   213,    41,    43,    14,    27,    46,
      32,    18,    54,   214,    35,    40,    23,    43,    35,    26,
       5,     6,    29,     8,    40,    32,    40,    34,    43,    14,
      40,     8,     8,    18,     8,   236,    43,   246,    23,    46,
       8,    26,    33,    28,    29,     8,    34,    32,     6,    34,
      35,    33,   253,     6,    34,    40,    38,    39,    43,     5,
       5,    32,    40,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,    33,    36,
      41,    43,    42,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,     8,    20,
      42,    52,    28,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,    33,    43,
      28,    28,    47,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,     8,    33,
      21,    40,    43,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,    37,     5,
      42,    32,    31,    45,    32,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    38,    39,    33,    32,
       8,     8,    42,    45,    46,    47,    48,    49,    50,    51,
      52,     8,     1,   273,    56,    57,    45,    46,    47,    48,
      49,    50,    51,    52,   102,    13,   125,   271,    -1,   129
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     5,     6,     8,    12,    14,    15,    18,
      23,    26,    29,    30,    32,    34,    43,    46,    60,    61,
      62,    65,    66,    69,    70,    80,    81,    82,    83,    84,
      85,    88,    89,    90,    91,    92,    94,    95,    96,    97,
      98,   101,   105,   106,   107,   108,   118,   119,   120,   129,
     130,    32,    41,     8,     8,     8,    75,     8,     8,    89,
      91,     8,    43,     8,    98,    89,     5,     6,    34,   121,
     122,   123,   124,   125,   126,   127,   128,    89,    16,    68,
      17,    72,    38,    39,    43,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    93,    41,
      32,    41,    40,    44,    32,    52,    89,    99,   100,     8,
      63,    42,    42,    43,    19,    32,    79,    27,    32,    54,
      33,   123,   127,    35,    40,    43,    40,    35,    40,    43,
      40,     8,    79,     8,    79,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,     8,    99,   100,     8,
       8,    91,   106,   100,   100,    33,    33,    40,     8,    28,
      64,     9,    10,    11,   114,   117,     8,    13,    32,   109,
     110,   111,   113,   114,     8,    80,    86,    87,     5,     8,
      47,    89,   116,    99,   119,    35,    35,    34,   124,     6,
      34,   128,     5,    67,    32,    71,    33,    32,    43,    33,
      89,    42,    32,    34,    44,    52,    36,   110,    20,   102,
      28,    24,    25,    42,    79,    33,    43,   123,   127,    28,
      73,    74,    75,    28,   100,   114,   114,   115,   116,   119,
     110,    47,     8,    21,   103,    89,    79,    89,    28,    35,
      35,    33,    40,    43,    33,    35,    40,    43,    37,     5,
      45,    47,   104,    79,    31,    76,    75,    32,   116,    32,
      33,    32,   114,     8,   112,     8,    77,    78,    33,    33,
      40,    42,    33,    40,     8,   109,    78
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    63,    64,    66,    67,    65,    68,    70,
      71,    69,    72,    73,    73,    74,    74,    75,    76,    76,
      77,    77,    78,    79,    79,    80,    80,    80,    80,    80,
      80,    81,    82,    83,    84,    84,    85,    86,    86,    87,
      87,    88,    89,    89,    89,    89,    89,    89,    89,    89,
      90,    91,    92,    92,    92,    92,    92,    92,    92,    93,
      93,    93,    93,    94,    94,    94,    94,    94,    94,    95,
      96,    96,    96,    97,    97,    98,    99,    99,   100,   100,
     101,   102,   102,   103,   103,   104,   104,   105,   105,   106,
     106,   106,   107,   107,   108,   108,   109,   109,   109,   109,
     110,   111,   111,   112,   112,   113,   114,   114,   114,   114,
     115,   115,   116,   116,   116,   116,   117,   117,   118,   119,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   126,   126,   127,   127,   128,   128,
     129,   129,   130
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     2,     1,     1,     1,     1,     1,
       1,     7,     2,     4,     2,     1,     5,     0,     3,     0,
       4,     6,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     3,     3,     3,     2,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     1,     4,     4,     4,     0,     1,     1,     3,
       6,     0,     2,     0,     2,     1,     1,     1,     3,     1,
       1,     1,     3,     3,     4,     4,     1,     1,     1,     1,
       1,     4,     7,     1,     3,     5,     1,     4,     7,     2,
       1,     3,     1,     1,     3,     1,     1,     1,     1,     1,
       2,     1,     3,     3,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     5
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
  "\"else\"", "\"for\"", "\"in\"", "\"end\"", "\"return\"", "\"%!\"",
  "\"->\"", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"{\"", "\"}\"", "\"<\"",
  "\">\"", "\",\"", "\".\"", "\":\"", "\";\"", "\"=\"", "\"+\"", "\"-\"",
  "\"*\"", "\"/\"", "\".*\"", "\"./\"", "\"^\"", "\"'\"", "\"\\\\\"",
  "\"==\"", "\"!=\"", "\"<=\"", "\">=\"", "$accept", "program",
  "program_element", "extern", "element_type_decl", "field_decl_list",
  "field_decl", "procedure", "$@1", "$@2", "procedure_header", "function",
  "$@3", "$@4", "function_header", "arguments", "argument_list",
  "argument_decl", "results", "result_list", "result_decl", "stmt_block",
  "stmt", "const_stmt", "return_stmt", "assign_stmt", "expr_stmt",
  "if_stmt", "else_clauses", "elif_clauses", "for_stmt", "expr",
  "ident_expr", "paren_expr", "linear_algebra_expr", "elwise_binary_op",
  "boolean_expr", "field_read_expr", "set_read_expr",
  "call_or_paren_read_expr", "call_expr", "expr_list_or_empty",
  "expr_list", "map_expr", "with", "reduce", "reduction_op",
  "write_expr_list", "write_expr", "field_write_expr", "tensor_write_expr",
  "type", "element_type", "set_type", "endpoints", "tuple_type",
  "tensor_type", "index_sets", "index_set", "component_type",
  "literal_expr", "tensor_literal", "dense_tensor_literal",
  "float_dense_tensor_literal", "float_dense_ndtensor_literal",
  "float_dense_matrix_literal", "float_dense_vector_literal",
  "int_dense_tensor_literal", "int_dense_ndtensor_literal",
  "int_dense_matrix_literal", "int_dense_vector_literal", "scalar_literal",
  "test", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   250,   250,   252,   256,   257,   265,   273,   276,   280,
     287,   301,   315,   318,   326,   346,   346,   346,   354,   376,
     376,   376,   384,   408,   411,   417,   422,   430,   439,   442,
     448,   453,   461,   472,   475,   484,   485,   486,   487,   488,
     489,   493,   523,   529,   606,   609,   615,   621,   623,   627,
     629,   636,   648,   649,   650,   651,   652,   653,   654,   655,
     661,   682,   698,   706,   718,   783,   789,   819,   824,   833,
     834,   835,   836,   842,   848,   854,   860,   866,   872,   887,
     907,   908,   909,   920,   954,   960,   970,   973,   979,   985,
     996,  1004,  1006,  1010,  1012,  1015,  1016,  1064,  1069,  1077,
    1081,  1084,  1090,  1108,  1114,  1132,  1156,  1159,  1162,  1165,
    1171,  1178,  1182,  1192,  1196,  1203,  1216,  1219,  1222,  1262,
    1272,  1277,  1285,  1288,  1292,  1298,  1304,  1307,  1376,  1379,
    1380,  1384,  1387,  1396,  1407,  1414,  1417,  1421,  1434,  1438,
    1452,  1456,  1462,  1469,  1472,  1476,  1489,  1493,  1507,  1511,
    1517,  1522,  1532
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
      55,    56,    57
    };
    const unsigned int user_token_number_max_ = 312;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



