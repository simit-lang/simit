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

  void transposeVector(const std::shared_ptr<Expression> &vec) {
    assert(vec->getType().isTensor());
    const TensorType *ttype = vec->getType().toTensor();
    assert(ttype->order() == 1);

    Type transposedVector = TensorType::make(ttype->componentType,
                                             ttype->dimensions,
                                             !ttype->isColumnVector);
    vec->setType(transposedVector);
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
      if (!expr->getType().isTensor()) {                \
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


        { delete (yysym.value.expression); }

        break;

      case 61: // extern


        { delete (yysym.value.expression); }

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


        { delete (yysym.value.expressions); }

        break;

      case 80: // stmt


        { delete (yysym.value.expression); }

        break;

      case 81: // const_stmt


        { delete (yysym.value.expression); }

        break;

      case 82: // assign_stmt


        { delete (yysym.value.expression); }

        break;

      case 83: // expr_stmt


        { delete (yysym.value.expression); }

        break;

      case 84: // if_stmt


        { delete (yysym.value.expression); }

        break;

      case 87: // for_stmt


        { delete (yysym.value.expression); }

        break;

      case 88: // expr


        { delete (yysym.value.expression); }

        break;

      case 89: // ident_expr


        { delete (yysym.value.expression); }

        break;

      case 90: // paren_expr


        { delete (yysym.value.expression); }

        break;

      case 91: // linear_algebra_expr


        { delete (yysym.value.indexExpr); }

        break;

      case 92: // elwise_binary_op


        {}

        break;

      case 93: // boolean_expr


        { delete (yysym.value.expression); }

        break;

      case 94: // field_read_expr


        { delete (yysym.value.fieldRead); }

        break;

      case 95: // set_read_expr


        { delete (yysym.value.expression); }

        break;

      case 96: // call_or_paren_read_expr


        { delete (yysym.value.expression); }

        break;

      case 97: // call_expr


        { delete (yysym.value.call); }

        break;

      case 98: // expr_list_or_empty


        { delete (yysym.value.expressions); }

        break;

      case 99: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 100: // map_expr


        { delete (yysym.value.expression); }

        break;

      case 104: // write_expr_list


        { delete (yysym.value.writeinfos); }

        break;

      case 105: // write_expr


        { delete (yysym.value.writeinfo); }

        break;

      case 106: // field_write_expr


        { delete (yysym.value.fieldWrite); }

        break;

      case 107: // tensor_write_expr


        { delete (yysym.value.tensorWrite); }

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


        { delete (yysym.value.expressions); }

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


        { delete (yysym.value.TensorLiteral); }

        break;

      case 119: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

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


        { delete (yysym.value.TensorLiteral); }

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
    (yylhs.value.expression) = NULL;
  }

    break;

  case 8:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[0].value.expression);
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
    auto statements = unique_ptr<vector<shared_ptr<Expression>>>((yystack_[2].value.expressions));
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
    auto statements = unique_ptr<vector<shared_ptr<Expression>>>((yystack_[2].value.expressions));
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
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 34:

    {
    (yylhs.value.expressions) = (yystack_[1].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 40:

    {
    std::string ident = convertAndFree((yystack_[5].value.string));
    auto type = convertAndDelete((yystack_[3].value.type));
    const TensorType *tensorType = type.toTensor();

    auto literal = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[1].value.TensorLiteral);

    assert(literal->getType().isTensor() &&
           "Only tensor literals are currently supported");
    auto literalType = literal->getType();

    literal->setName(ident);

    // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
    // cast $tensor_literal to a 1xn matrix.
    const TensorType *literalTensorType = literalType.toTensor();
    if (tensorType->order() == 2 && literalTensorType->order() == 1) {
      literal->cast(type);
    }

    // Typecheck: value and literal types must be equivalent.
    CHECK_TYPE_EQUALITY(type, literal->getType(), yystack_[5].location);

    ctx->addSymbol(literal->getName(), literal);
    (yylhs.value.expression) = new shared_ptr<Expression>(literal);
  }

    break;

  case 41:

    {
    (yylhs.value.expression) = NULL;  // TODO: Remove this

    auto lhsList = unique_ptr<vector<unique_ptr<WriteInfo>>>((yystack_[3].value.writeinfos));
    auto rhsList = unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));

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

      if (rhs == NULL) continue;  // TODO: Remove check

      assert(lhs->kind == WriteInfo::VARIABLE ||
             lhs->kind == WriteInfo::FIELD || lhs->kind == WriteInfo::TENSOR);
      switch (lhs->kind) {
        case WriteInfo::VARIABLE: {
          std::string variableName = *lhs->variableName;

          // TODO: Remove check
          if (!ctx->hasSymbol(variableName)) continue;

          assert(ctx->hasSymbol(variableName));

          const RWExprPair &varExprPair = ctx->getSymbol(variableName);
          assert(varExprPair.isWritable());

          shared_ptr<Expression> lhsTensor = varExprPair.getWriteExpr();
          auto result = dynamic_pointer_cast<Result>(lhsTensor);
          if (result) {
            CHECK_TYPE_EQUALITY(result->getType(), rhs->getType(), yystack_[2].location);
            rhs->setName(result->getName());
            result->addValue(rhs);
            (yylhs.value.expression) = new std::shared_ptr<Expression>(rhs);
          }
          else {
            NOT_SUPPORTED_YET;
          }
          break;
        }
        case WriteInfo::FIELD: {
          std::shared_ptr<FieldWrite> fieldWrite(*lhs->fieldWrite);
          fieldWrite->setValue(rhs);

          auto result = dynamic_pointer_cast<Result>(fieldWrite->getTarget());
          if (result) {
            result->addValue(fieldWrite);
            (yylhs.value.expression) = new std::shared_ptr<Expression>(fieldWrite);
          }
          else {
            NOT_SUPPORTED_YET;
          }

          break;
        }
        case WriteInfo::TENSOR: {
          std::shared_ptr<TensorWrite> tensorWrite(*lhs->tensorWrite);
          tensorWrite->setValue(rhs);

          auto result = dynamic_pointer_cast<Result>(tensorWrite->getTensor());
          if (result){
            result->addValue(tensorWrite);
            (yylhs.value.expression) = new std::shared_ptr<Expression>(tensorWrite);
          }
          else {
            NOT_SUPPORTED_YET;
          }

          break;
        }
      }
    }
  }

    break;

  case 42:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 43:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 44:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[3].value.expression);
    delete (yystack_[2].value.expressions);
  }

    break;

  case 46:

    {
    delete (yystack_[0].value.expressions);
  }

    break;

  case 48:

    {
    delete (yystack_[1].value.expression);
    delete (yystack_[0].value.expressions);
  }

    break;

  case 49:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 58:

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

  case 59:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = (yystack_[1].value.expression);
  }

    break;

  case 60:

    {
    if ((yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 61:

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

  case 62:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> l = convertAndDelete((yystack_[2].value.expression));
    std::shared_ptr<Expression> r = convertAndDelete((yystack_[0].value.expression));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l->getType().toTensor();
    const TensorType *rtype = r->getType().toTensor();

    // Scale
    if (ltype->order()==0 || rtype->order()==0) {
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(binaryElwiseExpr(l, IndexExpr::MUL, r));
    }
    // Vector-Vector Multiplication (inner and outer product)
    else if (ltype->order() == 1 && rtype->order() == 1) {
      // Inner product
      if (!ltype->isColumnVector) {
        if (!rtype->isColumnVector) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (l->getType() != r->getType()) {
          REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (rtype->isColumnVector) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (l->getType() != r->getType()) {
          REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (ltype->order() == 2 && rtype->order() == 1) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      if (ltype->dimensions[0] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->order() == 2 && rtype->order() == 2) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.indexExpr) = NULL;
    }
  }

    break;

  case 63:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 64:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    auto expr = shared_ptr<Expression>(*(yystack_[1].value.expression));
    delete (yystack_[1].value.expression);

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    const TensorType *type = expr->getType().toTensor();

    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NONE, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NONE, expr));
        if (!type->isColumnVector) {
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

  case 65:

    {
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 66:

    {  // Solve
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 67:

    { (yylhs.value.binop) = IndexExpr::ADD; }

    break;

  case 68:

    { (yylhs.value.binop) = IndexExpr::SUB; }

    break;

  case 69:

    { (yylhs.value.binop) = IndexExpr::MUL; }

    break;

  case 70:

    { (yylhs.value.binop) = IndexExpr::DIV; }

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
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.string) == NULL) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check
    if (!(*(yystack_[2].value.expression))->getType().defined()) { (yylhs.value.fieldRead) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> elemOrSet = convertAndDelete((yystack_[2].value.expression));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    Type type = elemOrSet->getType();
    if (!(type.isElement() || type.isSet())) {
      std::stringstream errorStr;
      errorStr << "only elements and sets have fields";
      REPORT_ERROR(errorStr.str(), yystack_[2].location);
    }

    (yylhs.value.fieldRead) = new shared_ptr<FieldRead>(new FieldRead(elemOrSet, fieldName));
  }

    break;

  case 81:

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
      if (expr->getType().isTensor()) {
        (yylhs.value.expression) = new shared_ptr<Expression>(new TensorRead(expr, *indices));
      }
      else if (expr->getType().isTuple()) {
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

  case 82:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 83:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto actuals =
        unique_ptr<vector<shared_ptr<Expression>>>((yystack_[1].value.expressions));
    auto call = new Call(name, *actuals);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 84:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 85:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 86:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 87:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 88:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 90:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 95:

    {
    (yylhs.value.writeinfos) = new vector<unique_ptr<WriteInfo>>;
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back(unique_ptr<WriteInfo>((yystack_[0].value.writeinfo)));
  }

    break;

  case 96:

    {
    (yylhs.value.writeinfos) = (yystack_[2].value.writeinfos);
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back(unique_ptr<WriteInfo>((yystack_[0].value.writeinfo)));
  }

    break;

  case 97:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.writeinfo) = new WriteInfo(name);
  }

    break;

  case 98:

    {
    (yylhs.value.writeinfo) = new WriteInfo((yystack_[0].value.fieldWrite));
  }

    break;

  case 99:

    {
    (yylhs.value.writeinfo) = new WriteInfo((yystack_[0].value.tensorWrite));
  }

    break;

  case 100:

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

  case 101:

    {
    (yylhs.value.fieldWrite) = NULL;
  }

    break;

  case 102:

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

  case 103:

    {
    // TODO
    (yylhs.value.tensorWrite) = NULL;
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
    auto eps = convertAndDelete((yystack_[1].value.expressions));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType));
  }

    break;

  case 111:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>;
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 112:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
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
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(ident);
  }

    break;

  case 122:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
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
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    transposeVector(*(yylhs.value.TensorLiteral));
  }

    break;

  case 130:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Float(64), idoms);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 131:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Int(32), idoms);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
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
    auto literal = new Literal(scalarTensorType, &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 149:

    {
    auto scalarTensorType = TensorType::make(Float(64));
    auto literal = new Literal(scalarTensorType, &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 150:

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

  const signed char  Parser ::yytable_ninf_ = -122;

  const short int
   Parser ::yypact_[] =
  {
    -186,    13,  -186,  -186,  -186,    67,    12,    36,    70,    72,
     219,    82,   107,   219,    17,  -186,   219,  -186,  -186,  -186,
    -186,    10,  -186,    51,  -186,  -186,  -186,  -186,  -186,  -186,
     277,   100,   105,  -186,  -186,   101,   125,  -186,  -186,  -186,
      79,  -186,   124,  -186,  -186,  -186,   116,  -186,  -186,   219,
     159,  -186,   127,   134,   135,   158,   150,   317,   142,   160,
     154,   141,   239,  -186,  -186,   115,   153,   156,   161,   157,
     171,   168,   164,   173,   167,   206,  -186,   213,  -186,   219,
     219,  -186,  -186,  -186,   219,   219,  -186,  -186,   219,  -186,
     219,   219,   219,   219,   219,   219,   214,   219,   218,     6,
     219,   219,  -186,   317,   195,    84,  -186,     9,   149,   289,
    -186,   222,   219,   147,    55,   219,    28,  -186,    29,    31,
    -186,   199,   229,   230,  -186,   204,   234,   240,  -186,   197,
     215,   197,   258,   258,   167,   167,   167,   337,   357,   357,
     258,   258,   317,  -186,   223,   208,  -186,     0,   105,  -186,
      95,    92,   205,  -186,   219,   210,  -186,  -186,  -186,  -186,
      18,    25,  -186,  -186,   221,   247,  -186,  -186,  -186,  -186,
     209,   238,  -186,   232,   139,   108,   236,  -186,   297,  -186,
     233,   220,  -186,  -186,   229,   157,  -186,   234,   173,  -186,
     241,    70,   245,  -186,   219,  -186,  -186,   317,   149,   149,
      55,    28,  -186,   247,   227,   259,   254,  -186,   219,  -186,
     219,   166,  -186,  -186,    61,    75,  -186,   248,   242,  -186,
    -186,    84,     3,   -17,    60,  -186,   237,   246,   296,  -186,
      20,  -186,   317,   197,   317,  -186,  -186,  -186,   280,    70,
    -186,  -186,   281,    55,  -186,   282,   279,  -186,  -186,  -186,
     197,   285,  -186,  -186,   149,  -186,   310,  -186,   311,    -4,
    -186,   104,   295,   117,  -186,  -186,  -186,   330,   289,  -186,
     311,  -186,  -186,  -186
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   148,   149,    58,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    43,     0,     3,     7,     4,
       5,     0,     6,     0,     8,    35,    36,    37,    38,    39,
       0,    50,    51,    53,    54,    55,     0,    56,    80,    57,
       0,    95,    98,    99,    52,   126,   127,   129,     9,    84,
       0,    12,     0,     0,     0,     0,    58,    33,    51,     0,
       0,     0,     0,   146,   138,     0,     0,   133,   132,   136,
       0,   141,   140,   144,    60,     0,    33,     0,    33,     0,
       0,    42,    67,    68,     0,     0,    69,    70,     0,    64,
       0,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,   128,    86,     0,     0,   100,     0,     0,     0,
      10,     0,    84,    47,     0,    84,     0,    59,     0,     0,
     130,     0,     0,     0,   131,     0,     0,     0,    18,    16,
       0,    20,    73,    74,    62,    63,    65,    66,    71,    72,
      75,    76,    61,   101,     0,    85,    77,    97,     0,    96,
       0,     0,    81,   102,     0,     0,    11,    13,   124,   125,
       0,     0,   114,   108,     0,     0,    27,   104,   105,   106,
     107,    89,    34,     0,     0,   148,    58,   123,     0,    33,
       0,     0,   134,   142,     0,   137,   139,     0,   145,   147,
       0,    23,     0,    82,     0,    41,   103,    87,     0,     0,
       0,     0,   117,     0,     0,     0,    91,    44,     0,    33,
       0,     0,    83,   150,     0,     0,    17,     0,    24,    25,
      21,     0,     0,     0,     0,   118,     0,     0,     0,    90,
       0,    88,    33,    46,   122,    49,   135,   143,    28,     0,
      14,   115,     0,     0,    40,   109,     0,    93,    94,    92,
      48,     0,    22,    26,     0,   119,     0,   113,     0,     0,
     111,     0,     0,     0,    30,   116,   110,     0,     0,    29,
       0,   112,    32,    31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
    -186,  -186,  -186,  -186,  -186,  -186,  -186,  -182,  -186,  -186,
      71,   -70,   339,  -186,  -186,  -186,  -186,  -186,  -186,  -186,
      -9,  -186,    -1,  -186,  -186,  -186,  -186,  -186,  -186,   345,
     -73,   -47,  -186,  -186,  -186,  -186,  -186,   260,  -186,  -186,
      90,  -154,  -186,  -186,  -186,  -105,  -186,  -185,  -186,  -186,
    -104,  -186,  -186,  -186,   -55,   255,  -186,  -186,   -60,   235,
    -186,  -186
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    17,    18,    19,   107,   157,    20,    21,   190,
      76,    22,    23,   192,    78,   217,   218,    54,   252,   263,
     264,   113,   172,    25,    26,    27,    28,   173,   174,    29,
      30,    31,    58,    33,    95,    34,    35,    36,    37,    38,
     104,   145,    39,   206,   231,   249,    40,    41,    42,    43,
     166,   167,   168,   261,   169,   170,   224,   179,   162,    44,
      45,    46,    66,    67,    68,    69,    70,    71,    72,    73,
      47,    48
  };

  const short int
   Parser ::yytable_[] =
  {
      32,    57,   105,   161,    62,   119,   129,    74,   131,   219,
     118,   204,   181,     2,   147,   225,   241,   155,     3,     4,
      51,     5,    63,    64,   144,     6,    75,     7,     8,   265,
     -19,     9,   194,     3,     4,   202,    10,   156,    13,    11,
     103,    50,   180,    12,    52,    13,   240,    14,   202,   227,
     199,    65,   200,   150,   151,   202,    15,   253,   255,    16,
     175,     4,    14,   176,   182,   247,   183,   248,    77,   201,
     132,   133,   122,     9,   126,   134,   135,   202,    53,   136,
      55,   137,   138,   139,   140,   141,   142,    13,   103,    14,
      59,   103,   103,   222,   223,   242,   236,   226,   148,    49,
     243,    16,   177,   103,   122,   178,   103,   -97,    50,   211,
     237,   -97,    32,  -120,  -120,    60,  -120,   153,   126,    99,
      63,    64,  -120,   100,   154,   196,  -120,   215,    32,   214,
      32,  -120,   154,    97,  -120,   154,  -120,   266,   195,   233,
    -120,   -78,  -120,  -120,   267,   197,    96,   221,  -120,   259,
     269,  -120,     3,     4,  -120,     5,   101,   270,   158,   159,
     160,     7,   250,   208,   209,     9,    98,   106,   102,   108,
      10,     3,     4,    11,     5,   -45,   109,   111,   110,    13,
       7,    14,   112,   -79,     9,   103,   115,   114,   120,    10,
      15,   178,    11,    16,   235,   116,   121,   123,    13,   232,
      14,   234,     3,     4,   122,     5,   124,   126,   125,    15,
      32,     7,    16,   127,   128,     9,    86,    87,    88,    89,
      10,   130,   143,    11,     3,     4,   146,    56,   152,    13,
     171,    14,    32,   184,   178,    64,   186,     9,   187,    63,
      15,  -121,  -121,    16,  -121,   189,   -83,   191,   154,    32,
    -121,    13,   198,    14,  -121,   163,   193,   203,   205,  -121,
     207,   202,  -121,   213,  -121,    16,   212,   229,   112,   216,
    -121,  -121,   117,   220,   228,   230,  -121,    79,    80,  -121,
     244,   238,   239,   245,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   163,   158,   159,
     160,   246,   164,    82,    83,    84,    85,    86,    87,    88,
      89,   251,   257,   254,   256,    79,    80,   258,   260,   262,
      81,   165,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    79,    80,   268,   271,   210,
      24,   273,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    79,    80,    61,   272,   149,
       0,   188,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    79,    80,   185,     0,     0,
       0,     0,    82,    83,    84,    85,    86,    87,    88,    89,
    -122,    91,    92,    93,    94,    79,    80,     0,     0,     0,
       0,     0,    82,    83,    84,    85,    86,    87,    88,    89,
       0,     0,     0,    93,    94
  };

  const short int
   Parser ::yycheck_[] =
  {
       1,    10,    49,   108,    13,    65,    76,    16,    78,   191,
      65,   165,   116,     0,     8,   200,    33,     8,     5,     6,
       8,     8,     5,     6,    97,    12,    16,    14,    15,    33,
      17,    18,    32,     5,     6,    52,    23,    28,    32,    26,
      49,    41,   115,    30,     8,    32,    43,    34,    52,   203,
      32,    34,    34,   100,   101,    52,    43,   239,   243,    46,
       5,     6,    34,     8,    35,    45,    35,    47,    17,    44,
      79,    80,    43,    18,    43,    84,    85,    52,     8,    88,
       8,    90,    91,    92,    93,    94,    95,    32,    97,    34,
       8,   100,   101,   198,   199,    35,    35,   201,    99,    32,
      40,    46,    47,   112,    43,   114,   115,    40,    41,   179,
      35,    44,   113,     5,     6,     8,     8,    33,    43,    40,
       5,     6,    14,    44,    40,    33,    18,   187,   129,   184,
     131,    23,    40,    32,    26,    40,    28,    33,    43,   209,
      32,    41,    34,    35,    40,   154,    41,   194,    40,   254,
      33,    43,     5,     6,    46,     8,    32,    40,     9,    10,
      11,    14,   232,    24,    25,    18,    41,     8,    52,    42,
      23,     5,     6,    26,     8,    28,    42,    19,    43,    32,
      14,    34,    32,    41,    18,   194,    32,    27,    35,    23,
      43,   200,    26,    46,    28,    54,    40,    40,    32,   208,
      34,   210,     5,     6,    43,     8,    35,    43,    40,    43,
     211,    14,    46,    40,     8,    18,    49,    50,    51,    52,
      23,     8,     8,    26,     5,     6,     8,     8,    33,    32,
       8,    34,   233,    34,   243,     6,     6,    18,    34,     5,
      43,     5,     6,    46,     8,     5,    41,    32,    40,   250,
      14,    32,    42,    34,    18,     8,    33,    36,    20,    23,
      28,    52,    26,    43,    28,    46,    33,     8,    32,    28,
      34,    35,    33,    28,    47,    21,    40,    38,    39,    43,
      43,    33,    40,    37,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     8,     9,    10,
      11,     5,    13,    45,    46,    47,    48,    49,    50,    51,
      52,    31,    33,    32,    32,    38,    39,    32,     8,     8,
      43,    32,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    42,     8,    42,
       1,   270,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    12,   268,    99,
      -1,   126,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,   122,    -1,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    56,    57
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     5,     6,     8,    12,    14,    15,    18,
      23,    26,    30,    32,    34,    43,    46,    60,    61,    62,
      65,    66,    69,    70,    80,    81,    82,    83,    84,    87,
      88,    89,    90,    91,    93,    94,    95,    96,    97,   100,
     104,   105,   106,   107,   117,   118,   119,   128,   129,    32,
      41,     8,     8,     8,    75,     8,     8,    88,    90,     8,
       8,    97,    88,     5,     6,    34,   120,   121,   122,   123,
     124,   125,   126,   127,    88,    16,    68,    17,    72,    38,
      39,    43,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    92,    41,    32,    41,    40,
      44,    32,    52,    88,    98,    99,     8,    63,    42,    42,
      43,    19,    32,    79,    27,    32,    54,    33,   122,   126,
      35,    40,    43,    40,    35,    40,    43,    40,     8,    79,
       8,    79,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,     8,    98,    99,     8,     8,    90,   105,
      99,    99,    33,    33,    40,     8,    28,    64,     9,    10,
      11,   113,   116,     8,    13,    32,   108,   109,   110,   112,
     113,     8,    80,    85,    86,     5,     8,    47,    88,   115,
      98,   118,    35,    35,    34,   123,     6,    34,   127,     5,
      67,    32,    71,    33,    32,    43,    33,    88,    42,    32,
      34,    44,    52,    36,   109,    20,   101,    28,    24,    25,
      42,    79,    33,    43,   122,   126,    28,    73,    74,    75,
      28,    99,   113,   113,   114,   115,   118,   109,    47,     8,
      21,   102,    88,    79,    88,    28,    35,    35,    33,    40,
      43,    33,    35,    40,    43,    37,     5,    45,    47,   103,
      79,    31,    76,    75,    32,   115,    32,    33,    32,   113,
       8,   111,     8,    77,    78,    33,    33,    40,    42,    33,
      40,     8,   108,    78
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    63,    64,    66,    67,    65,    68,    70,
      71,    69,    72,    73,    73,    74,    74,    75,    76,    76,
      77,    77,    78,    79,    79,    80,    80,    80,    80,    80,
      81,    82,    83,    83,    84,    85,    85,    86,    86,    87,
      88,    88,    88,    88,    88,    88,    88,    88,    89,    90,
      91,    91,    91,    91,    91,    91,    91,    92,    92,    92,
      92,    93,    93,    93,    93,    93,    93,    94,    95,    95,
      95,    96,    96,    97,    98,    98,    99,    99,   100,   101,
     101,   102,   102,   103,   103,   104,   104,   105,   105,   105,
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
       1,     3,     3,     0,     2,     1,     1,     1,     1,     1,
       7,     4,     2,     1,     5,     0,     3,     0,     4,     6,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     3,     3,     3,     2,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     4,     4,     4,     0,     1,     1,     3,     6,     0,
       2,     0,     2,     1,     1,     1,     3,     1,     1,     1,
       3,     3,     4,     4,     1,     1,     1,     1,     1,     4,
       7,     1,     3,     5,     1,     4,     7,     2,     1,     3,
       1,     1,     3,     1,     1,     1,     1,     1,     2,     1,
       3,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       5
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
  "stmt", "const_stmt", "assign_stmt", "expr_stmt", "if_stmt",
  "else_clauses", "elif_clauses", "for_stmt", "expr", "ident_expr",
  "paren_expr", "linear_algebra_expr", "elwise_binary_op", "boolean_expr",
  "field_read_expr", "set_read_expr", "call_or_paren_read_expr",
  "call_expr", "expr_list_or_empty", "expr_list", "map_expr", "with",
  "reduce", "reduction_op", "write_expr_list", "write_expr",
  "field_write_expr", "tensor_write_expr", "type", "element_type",
  "set_type", "endpoints", "tuple_type", "tensor_type", "index_sets",
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
       0,   249,   249,   251,   255,   256,   264,   272,   275,   279,
     286,   300,   313,   316,   324,   344,   344,   344,   352,   374,
     374,   374,   382,   406,   409,   415,   420,   428,   437,   440,
     446,   451,   459,   470,   473,   482,   483,   484,   485,   486,
     490,   520,   606,   609,   615,   621,   623,   627,   629,   636,
     643,   644,   645,   646,   647,   648,   649,   650,   656,   677,
     693,   701,   713,   778,   784,   814,   819,   828,   829,   830,
     831,   837,   843,   849,   855,   861,   867,   882,   901,   902,
     903,   914,   948,   954,   964,   967,   973,   979,   990,   998,
    1000,  1004,  1006,  1009,  1010,  1058,  1063,  1071,  1075,  1078,
    1084,  1102,  1108,  1126,  1142,  1145,  1148,  1151,  1157,  1164,
    1168,  1178,  1182,  1189,  1201,  1205,  1208,  1250,  1260,  1265,
    1273,  1276,  1280,  1286,  1292,  1295,  1364,  1367,  1368,  1372,
    1375,  1383,  1393,  1400,  1403,  1407,  1420,  1424,  1438,  1442,
    1448,  1455,  1458,  1462,  1475,  1479,  1493,  1497,  1503,  1508,
    1518
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



