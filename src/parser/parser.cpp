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
  using namespace std;
  using namespace simit::internal;
  using namespace simit::ir;

  std::string typeString(const Type &type) {
    std::stringstream ss;
    ss << type;
    std::string str = ss.str();
    if (type.isTensor() && type.toTensor()->isColumnVector) {
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

  void transposeVector(Expr vec) {
    assert(vec.type().isTensor());
    const TensorType *ttype = vec.type().toTensor();
    assert(ttype->order() == 1);

    Type transposedVector = TensorType::make(ttype->componentType,
                                             ttype->dimensions,
                                             !ttype->isColumnVector);

    const_cast<ExprNodeBase*>(vec.expr())->type = transposedVector;
  }

  bool compare(const Type &l, const Type &r, ProgramContext *ctx) {
    if (l.getKind() != r.getKind()) {
      return false;
    }

    if (l.isTensor()) {
      if (l.toTensor()->isColumnVector != r.toTensor()->isColumnVector) {
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
      if (!expr.type().isTensor()) {                \
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


  #define BINARY_ELWISE_TYPE_CHECK(lt, rt, loc)   \
    do {                                          \
      assert(lt.isTensor() && rt.isTensor());     \
      const TensorType *ltt = lt.toTensor();      \
      const TensorType *rtt = rt.toTensor();      \
      if (ltt->order() > 0 && rtt->order() > 0) { \
        CHECK_TYPE_EQUALITY(lt, rt, loc);         \
      }                                           \
    }                                             \
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


        { delete (yysym.value.expr); }

        break;

      case 61: // extern


        { delete (yysym.value.expr); }

        break;

      case 62: // element_type_decl


        { delete (yysym.value.type); }

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


        { delete (yysym.value.exprs); }

        break;

      case 74: // argument_list


        { delete (yysym.value.exprs); }

        break;

      case 75: // argument_decl


        { delete (yysym.value.expr); }

        break;

      case 76: // results


        { delete (yysym.value.exprs); }

        break;

      case 77: // result_list


        { delete (yysym.value.exprs); }

        break;

      case 78: // result_decl


        { delete (yysym.value.expr); }

        break;

      case 79: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 80: // stmts


        { delete (yysym.value.stmts); }

        break;

      case 81: // stmt


        { delete (yysym.value.stmt); }

        break;

      case 82: // assign_stmt


        { delete (yysym.value.stmt); }

        break;

      case 83: // value_name_list


        { delete (yysym.value.strings); }

        break;

      case 84: // field_write_stmt


        { delete (yysym.value.stmt); }

        break;

      case 85: // tensor_write_stmt


        { delete (yysym.value.stmt); }

        break;

      case 86: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 87: // if_stmt


        { delete (yysym.value.stmt); }

        break;

      case 90: // for_stmt


        { delete (yysym.value.stmt); }

        break;

      case 91: // const_stmt


        { delete (yysym.value.stmt); }

        break;

      case 92: // expr


        { delete (yysym.value.expr); }

        break;

      case 93: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 94: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 95: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 96: // elwise_binary_op


        {}

        break;

      case 97: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 98: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 99: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 100: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 101: // call_expr


        { delete (yysym.value.expr); }

        break;

      case 102: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 103: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 104: // map_expr


        { delete (yysym.value.expr); }

        break;

      case 108: // type


        { delete (yysym.value.type); }

        break;

      case 109: // element_type


        { delete (yysym.value.type); }

        break;

      case 110: // set_type


        { delete (yysym.value.type); }

        break;

      case 111: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 112: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 113: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 114: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 115: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 116: // component_type


        { delete (yysym.value.type); }

        break;

      case 118: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 119: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 120: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 121: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 122: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 123: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 124: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 125: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 126: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 127: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 128: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 129: // test


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
    (yylhs.value.expr) = NULL;
  }

    break;

  case 8:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[0].value.stmt);
  }

    break;

  case 9:

    {
    ctx->addTest((yystack_[0].value.test));
  }

    break;

  case 10:

    {
    Expr externVariable = convertAndDelete((yystack_[1].value.expr));
    ctx->addExtern(externVariable);
  }

    break;

  case 11:

    {
    string name = convertAndFree((yystack_[2].value.string));
    unique_ptr<std::map<string,Type>> fields((yystack_[1].value.fields));

    if (ctx->containsElementType(name)) {
      REPORT_ERROR("struct redefinition (" + name + ")", yylhs.location);
    }

    ctx->addElementType(ElementType::make(name, *fields));
  }

    break;

  case 12:

    {
    (yylhs.value.fields) = new map<string,Type>;
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
    auto tensorType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.field) = new pair<string,Type>(name, tensorType);
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
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = (yystack_[3].value.function);
    (yylhs.value.function)->setBody(body);
  }

    break;

  case 18:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto arguments = vector<Expr>();
    auto results = vector<Expr>();

    for (auto &extPair : ctx->getExterns()) {
      Expr ext = extPair.second;

      // TODO: Replace extResult with mutable parameters
      results.push_back(ext);

      arguments.push_back(ext);
      ctx->addSymbol(toVariable(ext)->name, ext, ext);
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
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = (yystack_[3].value.function);
    (yylhs.value.function)->setBody(body);
  }

    break;

  case 22:

    {
    std::string name = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<Expr>>((yystack_[2].value.exprs));
    auto results = unique_ptr<vector<Expr>>((yystack_[0].value.exprs));
    (yylhs.value.function) = new Function(name, *arguments, *results);

    std::set<std::string> argumentNames;
    for (Expr &argument : *arguments) {
      std::string argumentName = toVariable(argument)->name;
      ctx->addSymbol(argumentName, argument, Expr());
      argumentNames.insert(argumentName);
    }

    for (auto result : *results) {
      Expr readExpr;
      std::string resultName = toVariable(result)->name;
      if (argumentNames.find(resultName) != argumentNames.end()) {
        readExpr = ctx->getSymbol(resultName).getReadExpr();
      }

      ctx->addSymbol(resultName, readExpr, result);
    }
  }

    break;

  case 23:

    {
    (yylhs.value.exprs) = new vector<Expr>;
  }

    break;

  case 24:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
 }

    break;

  case 25:

    {
    auto argument = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.exprs) = new vector<Expr>;
    (yylhs.value.exprs)->push_back(argument);
  }

    break;

  case 26:

    {
    auto argument = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(argument);
  }

    break;

  case 27:

    {
    std::string name = convertAndFree((yystack_[2].value.string));

    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.expr) = new Expr(Variable::make(name, type));
  }

    break;

  case 28:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 29:

    {
    (yylhs.value.exprs) = (yystack_[1].value.exprs);
  }

    break;

  case 30:

    {
    auto result = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.exprs) = new vector<Expr>;
    (yylhs.value.exprs)->push_back(result);
  }

    break;

  case 31:

    {
    auto result = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(result);
  }

    break;

  case 32:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.expr) = new Expr(Variable::make(name, type));
  }

    break;

  case 33:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 34:

    {
    vector<Stmt> stmts = convertAndDelete((yystack_[0].value.stmts));
    if (stmts.size() == 0) {(yylhs.value.stmt) = new Stmt(Pass::make()); break;} // TODO: remove
    (yylhs.value.stmt) = new Stmt((stmts.size() == 1) ? stmts[0] : Block::make(stmts));
  }

    break;

  case 35:

    {
    (yylhs.value.stmts) = new vector<Stmt>();
    if ((yystack_[0].value.stmt) == NULL) break;  // TODO: Remove check
    Stmt stmt = convertAndDelete((yystack_[0].value.stmt));
    (yylhs.value.stmts)->push_back(stmt);
  }

    break;

  case 36:

    {
    (yylhs.value.stmts) = (yystack_[1].value.stmts);
    if ((yystack_[0].value.stmt) == NULL) break;  // TODO: Remove check
    Stmt stmt = convertAndDelete((yystack_[0].value.stmt));
    (yylhs.value.stmts)->push_back(stmt);
  }

    break;

  case 44:

    {
    (yylhs.value.stmt) = NULL;  // TODO: Remove this

    vector<string> valueNames = convertAndDelete((yystack_[3].value.strings));
    if ((yystack_[1].value.expr) == nullptr) { break; } // TODO: Remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    // TODO: Check valueNames.size() matches number of values produced by expr

    (yylhs.value.stmt) = new Stmt(AssignStmt::make(valueNames, value));
  }

    break;

  case 45:

    {
    string valueName = convertAndFree((yystack_[0].value.string));
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(valueName);
  }

    break;

  case 46:

    {
    string valueName = convertAndFree((yystack_[0].value.string));
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(valueName);
  }

    break;

  case 47:

    {
    string setName = convertAndFree((yystack_[5].value.string));
    string fieldName = convertAndFree((yystack_[3].value.string));
    if ((yystack_[1].value.expr) == nullptr) { (yylhs.value.stmt) = nullptr; break; } // TODO: remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (!ctx->hasSymbol(setName)) {
      REPORT_ERROR(setName + " is not defined in scope", yystack_[5].location);
    }

    const RWExprPair &setExprPair = ctx->getSymbol(setName);
    if (!setExprPair.isWritable()) {
      REPORT_ERROR(setName + " is not writable", yystack_[5].location);
    }

    auto setExpr = setExprPair.getWriteExpr();
    (yylhs.value.stmt) = new Stmt(FieldWrite::make(setExpr, fieldName, value));
  }

    break;

  case 48:

    {
    std::string tensorName = convertAndFree((yystack_[6].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    if ((yystack_[1].value.expr) == nullptr) { (yylhs.value.stmt) = nullptr; break; } // TODO: remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if(!ctx->hasSymbol(tensorName)) { (yylhs.value.stmt)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(tensorName)) {
      REPORT_ERROR(tensorName + " is not defined in scope", yystack_[6].location);
    }

    const RWExprPair &tensorExprPair = ctx->getSymbol(tensorName);
    if (!tensorExprPair.isWritable()) {
      REPORT_ERROR(tensorName + " is not writable", yystack_[6].location);
    }

    auto tensorExpr = tensorExprPair.getWriteExpr();
    (yylhs.value.stmt) = new Stmt(TensorWrite::make(tensorExpr, *indices, value));
  }

    break;

  case 49:

    {
    // TODO
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 50:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 51:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 52:

    {
    (yylhs.value.stmt) = NULL;
    delete (yystack_[3].value.expr);
    delete (yystack_[2].value.stmt);
  }

    break;

  case 54:

    {
    delete (yystack_[0].value.stmt);
  }

    break;

  case 56:

    {
    delete (yystack_[1].value.expr);
    delete (yystack_[0].value.stmt);
  }

    break;

  case 57:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 58:

    {
    std::string name = convertAndFree((yystack_[5].value.string));
    auto type = convertAndDelete((yystack_[3].value.type));
    const TensorType *tensorType = type.toTensor();

    Expr literal = convertAndDelete((yystack_[1].value.expr));

    assert(literal.type().isTensor() &&
           "Only tensor literals are currently supported");
    auto literalType = literal.type();

    // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
    // cast $tensor_literal to a 1xn matrix.
    const TensorType *literalTensorType = literalType.toTensor();
    if (tensorType->order() == 2 && literalTensorType->order() == 1) {
      const_cast<Literal*>(toLiteral(literal))->cast(type);
    }

    // Typecheck: value and literal types must be equivalent.
    CHECK_TYPE_EQUALITY(type, literal.type(), yystack_[5].location);

    ctx->addSymbol(name, literal);

    (yylhs.value.stmt) = nullptr; // TODO: return an assignstmt
  }

    break;

  case 67:

    {
    string ident = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(ident)) { (yylhs.value.expr)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(ident)) {
      REPORT_ERROR(ident + " is not defined in scope", yystack_[0].location);
    }

    const RWExprPair &rwExprPair = ctx->getSymbol(ident);
    if (!rwExprPair.isReadable()) {
      REPORT_ERROR(ident + " is not readable", yystack_[0].location);
    }

    (yylhs.value.expr) = new Expr(rwExprPair.getReadExpr());
  }

    break;

  case 68:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 69:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(unaryElwiseExpr(UnaryOperator::Neg, expr));
  }

    break;

  case 70:

    {  // + - .* ./
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    BINARY_ELWISE_TYPE_CHECK(l.type(), r.type(), yystack_[1].location);
    (yylhs.value.expr) = new Expr(binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }

    break;

  case 71:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    // Scale
    if (ltype->order()==0 || rtype->order()==0) {
      (yylhs.value.expr) = new Expr(binaryElwiseExpr(l, BinaryOperator::Mul, r));
    }
    // Vector-Vector Multiplication (inner and outer product)
    else if (ltype->order() == 1 && rtype->order() == 1) {
      // Inner product
      if (!ltype->isColumnVector) {
        if (!rtype->isColumnVector) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (l.type() != r.type()) {
          REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
        }
        (yylhs.value.expr) = new Expr(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (rtype->isColumnVector) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (l.type() != r.type()) {
          REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
        }
        (yylhs.value.expr) = new Expr(outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (ltype->order() == 2 && rtype->order() == 1) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      if (ltype->dimensions[0] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->order() == 2 && rtype->order() == 2) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.expr) = NULL;
    }
  }

    break;

  case 72:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 73:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[1].value.expr));

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    const TensorType *type = expr.type().toTensor();
    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(unaryElwiseExpr(UnaryOperator::None, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(unaryElwiseExpr(UnaryOperator::None, expr));
        if (!type->isColumnVector) {
          transposeVector(*(yylhs.value.expr));
        }
        break;
      case 2:
        (yylhs.value.expr) = new Expr(transposeMatrix(expr));
        break;
      default:
        REPORT_ERROR("cannot transpose >2-order tensors using '", yystack_[1].location);
        (yylhs.value.expr) = NULL;
    }
  }

    break;

  case 74:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 75:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 76:

    { (yylhs.value.binop) = BinaryOperator::Add; }

    break;

  case 77:

    { (yylhs.value.binop) = BinaryOperator::Sub; }

    break;

  case 78:

    { (yylhs.value.binop) = BinaryOperator::Mul; }

    break;

  case 79:

    { (yylhs.value.binop) = BinaryOperator::Div; }

    break;

  case 80:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 81:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 82:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 83:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 84:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 85:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 86:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.string) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    if (!(*(yystack_[2].value.expr)).type().defined()) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr elemOrSet = convertAndDelete((yystack_[2].value.expr));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    Type type = elemOrSet.type();
    if (!(type.isElement() || type.isSet())) {
      std::stringstream errorStr;
      errorStr << "only elements and sets have fields";
      REPORT_ERROR(errorStr.str(), yystack_[2].location);
    }

    (yylhs.value.expr) = new Expr(FieldRead::make(elemOrSet, fieldName));
  }

    break;

  case 90:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    if (ctx->hasSymbol(name)) {
      const RWExprPair &exprPair = ctx->getSymbol(name);
      if (!exprPair.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      // The parenthesis read can read from a tensor or a tuple.
      auto expr = exprPair.getReadExpr();
      if (expr.type().isTensor()) {
        (yylhs.value.expr) = new Expr(TensorRead::make(expr, *indices));
      }
      else if (expr.type().isTuple()) {
        if (indices->size() != 1) {
          REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
        }

        (yylhs.value.expr) = new Expr(TupleRead::make(expr, (*indices)[0]));
      }
      else {
        REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
      }
    }
    else if (ctx->containsFunction(name)) {
      (yylhs.value.expr) = NULL;
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 91:

    {
    (yylhs.value.expr) = NULL;
  }

    break;

  case 92:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto actuals = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));
    (yylhs.value.expr) = new Expr(Call::make(name, *actuals, Call::Internal));
  }

    break;

  case 93:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 94:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 95:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 96:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 97:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expr) = NULL;
  }

    break;

  case 99:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 104:

    {
    (yylhs.value.type) = (yystack_[0].value.type);
  }

    break;

  case 105:

    {
    (yylhs.value.type) = (yystack_[0].value.type);
  }

    break;

  case 106:

    {
    (yylhs.value.type) = (yystack_[0].value.type);
  }

    break;

  case 107:

    {
    (yylhs.value.type) = (yystack_[0].value.type);
  }

    break;

  case 108:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 109:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType));
  }

    break;

  case 110:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto eps = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType));
  }

    break;

  case 111:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 112:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 113:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 114:

    {
    auto componentType = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 115:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 116:

    {
    auto blockTypePtr = convertAndDelete((yystack_[1].value.type));
    const TensorType *blockType = blockTypePtr.toTensor();

    auto componentType = blockType->componentType;

    auto outerDimensions = unique_ptr<vector<IndexSet>>((yystack_[4].value.indexSets));
    auto blockDimensions = blockType->dimensions;

    vector<IndexDomain> dimensions;
    if (blockType->order() == 0) {
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
      if (blockType->order() != outerDimensions->size()) {
        REPORT_ERROR("Blocktype order (" + to_string(blockType->order()) +
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

    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions));
  }

    break;

  case 117:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 118:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 119:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 120:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 121:

    {
    std::string setName = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsExtern(setName)) {
      REPORT_ERROR("the set has not been declared", yystack_[0].location);
    }

    Expr set = ctx->getExtern(setName);
    if (!set.type().isSet()) {
      REPORT_ERROR("an index set must be a set, a range or dynamic (*)", yystack_[0].location);
    }
    (yylhs.value.indexSet) = new IndexSet(set);
  }

    break;

  case 122:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 123:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 124:

    {
    (yylhs.value.type) = new Type(Int(32));
  }

    break;

  case 125:

    {
    (yylhs.value.type) = new Type(Float(64));
  }

    break;

  case 128:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 130:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Float(64), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 131:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Int(32), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 132:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 134:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 135:

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

  case 136:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 137:

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

  case 138:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 139:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 140:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 142:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 143:

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

  case 144:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 145:

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

  case 146:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 147:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 148:

    {
    auto scalarTensorType = TensorType::make(Int(32));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }

    break;

  case 149:

    {
    auto scalarTensorType = TensorType::make(Float(64));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }

    break;

  case 150:

    {
    std::string name = convertAndFree((yystack_[6].value.string));
    auto actuals =
        unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    auto expected = convertAndDelete((yystack_[1].value.expr));

    std::vector<Expr> literalArgs;
    literalArgs.reserve(actuals->size());
    for (auto &arg : *actuals) {
      if (dynamic_cast<const Literal*>(arg.expr()) == nullptr) {
        REPORT_ERROR("function calls in tests must have literal arguments", yystack_[7].location);
      }
      literalArgs.push_back(arg);
    }

    std::vector<Expr> expecteds;
    expecteds.push_back(expected);
    (yylhs.value.test) = new Test(name, literalArgs, expecteds);
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


  const short int  Parser ::yypact_ninf_ = -187;

  const signed char  Parser ::yytable_ninf_ = -122;

  const short int
   Parser ::yypact_[] =
  {
    -187,   138,  -187,  -187,  -187,    -8,    37,    41,    59,    68,
     184,    71,   122,   184,    12,  -187,   184,  -187,  -187,  -187,
    -187,    81,  -187,   131,  -187,  -187,   -14,  -187,  -187,  -187,
    -187,  -187,  -187,   255,    65,   126,  -187,  -187,   137,   130,
    -187,  -187,  -187,  -187,  -187,   144,  -187,  -187,   184,   175,
    -187,   151,   158,   156,   182,   177,     5,   178,   192,   179,
     235,  -187,  -187,   157,   189,   187,   185,   196,   194,   200,
     203,   208,   226,   241,   199,   242,   199,   245,   184,   184,
     184,  -187,  -187,  -187,   184,   184,  -187,  -187,   184,  -187,
     184,   184,   184,   184,   184,   184,   184,   246,  -187,   355,
     223,     2,   214,     1,   148,   100,  -187,   251,   184,   233,
     232,   199,  -187,   184,    82,   184,  -187,    77,    80,  -187,
     228,   260,   264,  -187,   237,   274,   290,  -187,   268,   265,
     271,  -187,   275,   128,   128,   226,   226,   226,   375,   395,
     395,   128,   128,   355,   282,    62,  -187,   276,   272,   184,
     184,   277,  -187,  -187,  -187,  -187,   113,    73,  -187,  -187,
     299,   328,  -187,  -187,  -187,  -187,   286,   319,   316,   327,
     141,  -187,   180,   229,  -187,   295,   199,   324,  -187,  -187,
     260,   196,  -187,   274,   208,  -187,  -187,    59,  -187,  -187,
    -187,  -187,   184,   355,   315,   148,   148,    82,    67,  -187,
     328,   312,   367,   356,  -187,   184,   199,   184,   348,   325,
      84,    98,  -187,   362,   357,  -187,  -187,   335,  -187,   -18,
     -12,   114,  -187,   353,   361,   394,  -187,    89,  -187,     5,
    -187,   355,  -187,    67,  -187,  -187,   384,    59,  -187,  -187,
    -187,   385,    82,  -187,   386,   383,  -187,  -187,  -187,  -187,
     376,   403,  -187,  -187,   148,  -187,   428,  -187,  -187,   429,
     -11,  -187,    91,   396,   102,  -187,  -187,  -187,   431,   100,
    -187,   429,  -187,  -187,  -187
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   148,   149,    67,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    51,     0,     3,     7,     4,
       5,     0,     6,     0,     8,    37,     0,    38,    39,    40,
      41,    42,    43,     0,    59,    60,    62,    63,    64,     0,
      65,    89,    66,    61,   126,   127,   129,     9,    93,     0,
      12,     0,     0,     0,     0,    67,    33,    64,     0,     0,
       0,   146,   138,     0,     0,   133,   132,   136,     0,   141,
     140,   144,    69,     0,    33,     0,    33,     0,     0,     0,
       0,    50,    76,    77,     0,     0,    78,    79,     0,    73,
       0,     0,     0,     0,     0,     0,    93,     0,   128,    95,
       0,     0,     0,     0,     0,     0,    10,     0,    93,    77,
      55,    34,    35,    93,     0,    93,    68,     0,     0,   130,
       0,     0,     0,   131,     0,     0,     0,    18,     0,     0,
       0,    46,     0,    82,    83,    71,    72,    74,    75,    80,
      81,    84,    85,    70,     0,     0,    86,    90,     0,     0,
       0,     0,    11,    13,   124,   125,     0,     0,   114,   108,
       0,     0,    27,   104,   105,   106,   107,    98,    94,     0,
       0,    36,   148,    67,   123,     0,    33,     0,   134,   142,
       0,   137,   139,     0,   145,   147,    16,    23,    20,    44,
      91,    49,     0,    96,     0,     0,     0,     0,     0,   117,
       0,     0,     0,   100,    52,     0,    33,     0,     0,     0,
       0,     0,    17,     0,    24,    25,    21,     0,    47,     0,
       0,     0,   118,     0,     0,     0,    99,     0,    97,    33,
      54,   122,    57,     0,   135,   143,    28,     0,    48,    14,
     115,     0,     0,    58,   109,     0,   102,   103,   101,    56,
       0,     0,    22,    26,     0,   119,     0,   113,   150,     0,
       0,   111,     0,     0,     0,    30,   116,   110,     0,     0,
      29,     0,   112,    32,    31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -173,  -187,  -187,
     183,   -69,  -187,     7,  -187,  -187,  -187,  -187,  -187,  -187,
    -187,  -187,  -187,  -187,   -10,  -187,  -187,  -187,  -187,  -187,
      15,  -187,  -187,  -187,   -19,   -33,  -187,  -187,  -187,  -187,
     181,  -134,  -187,  -187,  -187,  -103,  -187,  -177,  -187,  -187,
    -186,  -187,  -187,  -187,   -59,   332,  -187,  -187,   -61,   323,
    -187,  -187
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    17,    18,    19,   103,   153,    20,    21,   212,
      74,    22,    23,   216,    76,   213,   214,    53,   252,   264,
     265,   110,   111,   112,    25,    26,    27,    28,    29,    30,
     169,   170,    31,    32,    33,    34,    35,    36,    95,    37,
      57,    39,    40,    41,   100,   168,    42,   203,   228,   248,
     162,   163,   164,   262,   165,   166,   221,   176,   158,    43,
      44,    45,    64,    65,    66,    67,    68,    69,    70,    71,
      46,    47
  };

  const short int
   Parser ::yytable_[] =
  {
      56,   157,   118,    60,   117,   128,    72,   130,    24,   151,
       3,     4,   223,     5,   215,   101,    38,    61,    62,     7,
     222,   240,   266,     9,    48,   239,    77,   201,    10,   152,
      78,    11,   -45,    49,   199,   148,   -45,    13,    99,    14,
     199,   199,   149,    79,    80,    50,    63,   250,    15,    51,
      82,   109,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,   145,   253,   255,   224,    52,   132,   133,
     134,    38,     3,     4,   135,   136,    54,   144,   137,    58,
     138,   139,   140,   141,   142,   143,    99,   172,     4,    38,
     173,    38,   219,   220,   144,   191,   177,    73,    99,    72,
       9,    14,   149,    99,   175,    99,   -87,   208,   159,   154,
     155,   156,   178,   160,    13,   179,    14,   198,   171,   234,
     121,   210,   211,   125,   267,   199,    38,   121,    16,   174,
      59,   268,   161,   235,   246,   270,   247,   230,     2,   193,
     194,   125,   271,     3,     4,   196,     5,   197,    75,   241,
       6,   260,     7,     8,   242,   -19,     9,   154,   155,   156,
     249,    10,    61,    62,    11,   205,   206,   -88,    12,    96,
      13,    97,    14,    82,    83,    84,    85,    86,    87,    88,
      89,    15,   217,   102,    16,  -120,  -120,   175,  -120,     3,
       4,    38,    55,   104,  -120,   229,    98,   231,  -120,   106,
     105,   107,     9,  -120,     3,     4,  -120,     5,  -120,   108,
     113,   115,  -120,     7,  -120,  -120,    13,     9,    14,   114,
    -120,    38,    10,  -120,   119,    11,  -120,   120,   121,   123,
      16,    13,   175,    14,  -121,  -121,   122,  -121,     3,     4,
     124,    55,    15,  -121,    38,    16,   125,  -121,   126,   127,
     129,     9,  -121,   131,   146,  -121,   147,  -121,   150,   167,
     -53,   108,   180,  -121,  -121,    13,    62,    14,   116,  -121,
     182,   183,  -121,    79,    80,    86,    87,    88,    89,    61,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   185,   186,   187,    81,   188,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   190,   192,   -92,   189,   195,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   200,   159,   207,   199,   202,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   204,   149,   209,   218,   225,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   226,   232,   227,   238,   233,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   236,   243,   237,   244,   245,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    79,    80,   251,   257,   254,   256,   258,
      82,    83,    84,    85,    86,    87,    88,    89,  -122,    91,
      92,    93,    94,    79,    80,   259,   261,   263,   269,   272,
      82,    83,    84,    85,    86,    87,    88,    89,   184,     0,
     273,    93,    94,   181,   274
  };

  const short int
   Parser ::yycheck_[] =
  {
      10,   104,    63,    13,    63,    74,    16,    76,     1,     8,
       5,     6,   198,     8,   187,    48,     1,     5,     6,    14,
     197,    33,    33,    18,    32,    43,    40,   161,    23,    28,
      44,    26,    40,    41,    52,    33,    44,    32,    48,    34,
      52,    52,    40,    38,    39,     8,    34,   233,    43,     8,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    96,   237,   242,   200,     8,    78,    79,
      80,    56,     5,     6,    84,    85,     8,    96,    88,     8,
      90,    91,    92,    93,    94,    95,    96,     5,     6,    74,
       8,    76,   195,   196,   113,    33,   115,    16,   108,   109,
      18,    34,    40,   113,   114,   115,    41,   176,     8,     9,
      10,    11,    35,    13,    32,    35,    34,    44,   111,    35,
      43,   180,   183,    43,    33,    52,   111,    43,    46,    47,
       8,    40,    32,    35,    45,    33,    47,   206,     0,   149,
     150,    43,    40,     5,     6,    32,     8,    34,    17,    35,
      12,   254,    14,    15,    40,    17,    18,     9,    10,    11,
     229,    23,     5,     6,    26,    24,    25,    41,    30,    32,
      32,    41,    34,    45,    46,    47,    48,    49,    50,    51,
      52,    43,   192,     8,    46,     5,     6,   197,     8,     5,
       6,   176,     8,    42,    14,   205,    52,   207,    18,    43,
      42,    19,    18,    23,     5,     6,    26,     8,    28,    32,
      32,    32,    32,    14,    34,    35,    32,    18,    34,    27,
      40,   206,    23,    43,    35,    26,    46,    40,    43,    35,
      46,    32,   242,    34,     5,     6,    40,     8,     5,     6,
      40,     8,    43,    14,   229,    46,    43,    18,    40,     8,
       8,    18,    23,     8,     8,    26,    33,    28,    44,     8,
      28,    32,    34,    34,    35,    32,     6,    34,    33,    40,
       6,    34,    43,    38,    39,    49,    50,    51,    52,     5,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,     5,    28,    32,    43,    28,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    33,    44,    41,    43,    42,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    36,     8,    42,    52,    20,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    28,    40,    33,    43,    47,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,     8,    28,    21,    43,    54,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    33,    43,    40,    37,     5,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    31,    33,    32,    32,    43,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    38,    39,    32,     8,     8,    42,     8,
      45,    46,    47,    48,    49,    50,    51,    52,   125,    -1,
     269,    56,    57,   121,   271
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     5,     6,     8,    12,    14,    15,    18,
      23,    26,    30,    32,    34,    43,    46,    60,    61,    62,
      65,    66,    69,    70,    81,    82,    83,    84,    85,    86,
      87,    90,    91,    92,    93,    94,    95,    97,    98,    99,
     100,   101,   104,   117,   118,   119,   128,   129,    32,    41,
       8,     8,     8,    75,     8,     8,    92,    98,     8,     8,
      92,     5,     6,    34,   120,   121,   122,   123,   124,   125,
     126,   127,    92,    16,    68,    17,    72,    40,    44,    38,
      39,    43,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    96,    32,    41,    52,    92,
     102,   103,     8,    63,    42,    42,    43,    19,    32,    46,
      79,    80,    81,    32,    27,    32,    33,   122,   126,    35,
      40,    43,    40,    35,    40,    43,    40,     8,    79,     8,
      79,     8,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,   102,   103,     8,    33,    33,    40,
      44,     8,    28,    64,     9,    10,    11,   113,   116,     8,
      13,    32,   108,   109,   110,   112,   113,     8,   103,    88,
      89,    81,     5,     8,    47,    92,   115,   102,    35,    35,
      34,   123,     6,    34,   127,     5,    28,    32,    28,    43,
      33,    33,    44,    92,    92,    42,    32,    34,    44,    52,
      36,   109,    20,   105,    28,    24,    25,    42,    79,    33,
     122,   126,    67,    73,    74,    75,    71,    92,    43,   113,
     113,   114,   115,   118,   109,    47,     8,    21,   106,    92,
      79,    92,    28,    54,    35,    35,    33,    40,    43,    43,
      33,    35,    40,    43,    37,     5,    45,    47,   107,    79,
     118,    31,    76,    75,    32,   115,    32,    33,    43,    32,
     113,     8,   111,     8,    77,    78,    33,    33,    40,    42,
      33,    40,     8,   108,    78
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    63,    64,    66,    67,    65,    68,    70,
      71,    69,    72,    73,    73,    74,    74,    75,    76,    76,
      77,    77,    78,    79,    79,    80,    80,    81,    81,    81,
      81,    81,    81,    81,    82,    83,    83,    84,    85,    85,
      86,    86,    87,    88,    88,    89,    89,    90,    91,    92,
      92,    92,    92,    92,    92,    92,    92,    93,    94,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      97,    97,    97,    97,    97,    97,    98,    99,    99,    99,
     100,   100,   101,   102,   102,   103,   103,   104,   105,   105,
     106,   106,   107,   107,   108,   108,   108,   108,   109,   110,
     110,   111,   111,   112,   113,   113,   113,   113,   114,   114,
     115,   115,   115,   115,   116,   116,   117,   118,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   126,   126,   127,   127,   128,   128,
     129
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     3,     6,     7,     4,
       2,     1,     5,     0,     3,     0,     4,     6,     7,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       3,     3,     3,     2,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       4,     4,     4,     0,     1,     1,     3,     6,     0,     2,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     4,
       7,     1,     3,     5,     1,     4,     7,     2,     1,     3,
       1,     1,     3,     1,     1,     1,     1,     1,     2,     1,
       3,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       8
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
  "stmts", "stmt", "assign_stmt", "value_name_list", "field_write_stmt",
  "tensor_write_stmt", "expr_stmt", "if_stmt", "else_clauses",
  "elif_clauses", "for_stmt", "const_stmt", "expr", "ident_expr",
  "paren_expr", "linear_algebra_expr", "elwise_binary_op", "boolean_expr",
  "field_read_expr", "set_read_expr", "call_or_paren_read_expr",
  "call_expr", "expr_list_or_empty", "expr_list", "map_expr", "with",
  "reduce", "reduction_op", "type", "element_type", "set_type",
  "endpoints", "tuple_type", "tensor_type", "index_sets", "index_set",
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
       0,   264,   264,   266,   270,   271,   279,   287,   290,   294,
     301,   310,   323,   326,   334,   344,   344,   344,   352,   372,
     372,   372,   380,   406,   409,   415,   420,   428,   437,   440,
     446,   451,   459,   469,   472,   480,   486,   495,   496,   497,
     498,   499,   500,   501,   505,   519,   524,   532,   553,   573,
     582,   585,   591,   597,   599,   603,   605,   612,   618,   647,
     648,   649,   650,   651,   652,   653,   654,   660,   681,   690,
     698,   710,   775,   781,   809,   814,   823,   824,   825,   826,
     832,   838,   844,   850,   856,   862,   873,   892,   893,   894,
     900,   933,   939,   947,   950,   956,   962,   973,   981,   983,
     987,   989,   992,   993,   999,  1002,  1005,  1008,  1014,  1021,
    1025,  1035,  1039,  1046,  1058,  1062,  1065,  1107,  1117,  1122,
    1130,  1133,  1146,  1152,  1158,  1161,  1211,  1215,  1216,  1220,
    1224,  1231,  1242,  1249,  1253,  1257,  1271,  1275,  1290,  1294,
    1301,  1308,  1312,  1316,  1330,  1334,  1349,  1353,  1360,  1364,
    1373
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



