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
  #include "ir_mutator.h"

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
    if (l.kind() != r.kind()) {
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
            case 4: // "int literal"


        {}

        break;

      case 5: // "float literal"


        {}

        break;

      case 6: // "string literal"


        { free((void*)((yysym.value.string))); }

        break;

      case 7: // "identifier"


        { free((void*)((yysym.value.string))); }

        break;

      case 61: // extern


        { delete (yysym.value.var); }

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


        { delete (yysym.value.vars); }

        break;

      case 74: // argument_list


        { delete (yysym.value.vars); }

        break;

      case 75: // argument_decl


        { delete (yysym.value.var); }

        break;

      case 76: // results


        { delete (yysym.value.vars); }

        break;

      case 77: // result_list


        { delete (yysym.value.vars); }

        break;

      case 78: // result_decl


        { delete (yysym.value.var); }

        break;

      case 79: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 90: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 91: // expr


        { delete (yysym.value.expr); }

        break;

      case 92: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 93: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 94: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 95: // elwise_binary_op


        {}

        break;

      case 96: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 97: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 98: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 99: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 100: // call_expr


        { delete (yysym.value.expr); }

        break;

      case 101: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 102: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 103: // map_expr


        { delete (yysym.value.expr); }

        break;

      case 107: // type


        { delete (yysym.value.type); }

        break;

      case 108: // element_type


        { delete (yysym.value.type); }

        break;

      case 109: // set_type


        { delete (yysym.value.type); }

        break;

      case 110: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 111: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 112: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 113: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 114: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 115: // component_type


        { delete (yysym.value.type); }

        break;

      case 116: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 117: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 118: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 119: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 120: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 121: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 122: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 123: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 124: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 125: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 126: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 127: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 128: // signed_int_literal


        {}

        break;

      case 129: // signed_float_literal


        {}

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
    Func func = convertAndDelete((yystack_[0].value.function));
    std::string name = func.getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("procedure redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(func);
  }

    break;

  case 6:

    {
    Func func = convertAndDelete((yystack_[0].value.function));
    std::string name = func.getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("function redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(func);
  }

    break;

  case 10:

    {
    Var externVar = convertAndDelete((yystack_[1].value.var));
    ctx->addExtern(externVar);
    ctx->addSymbol(externVar);
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
    Func func = convertAndDelete((yystack_[3].value.function));
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = new Func(func.getName(), func.getArguments(), func.getResults(), body);
  }

    break;

  case 18:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto arguments = vector<Var>();
    auto results = vector<Var>();

    for (auto &extPair : ctx->getExterns()) {
      Var ext = ctx->getExtern(extPair.first);

      // TODO: Replace extResult with mutable parameters
      results.push_back(ext);
      arguments.push_back(ext);

      ctx->addSymbol(ext);
    }

    (yylhs.value.function) = new Func(name, arguments, results, Stmt());
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
    Func func = convertAndDelete((yystack_[3].value.function));
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = new Func(func.getName(), func.getArguments(), func.getResults(), body);
  }

    break;

  case 22:

    {
    std::string name = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<Var>>((yystack_[2].value.vars));
    auto results = unique_ptr<vector<Var>>((yystack_[0].value.vars));
    (yylhs.value.function) = new Func(name, *arguments, *results, Stmt());

    std::set<std::string> argNames;
    for (Var &arg : *arguments) {
      ctx->addSymbol(arg.name, arg, Symbol::Read);
      argNames.insert(arg.name);
    }

    for (Var &res : *results) {
      Symbol::Access access = (argNames.find(res.name) != argNames.end())
                              ? Symbol::ReadWrite : Symbol::Write;

      ctx->addSymbol(res.name, res, access);
    }
  }

    break;

  case 23:

    {
    (yylhs.value.vars) = new vector<Var>;
  }

    break;

  case 24:

    {
    (yylhs.value.vars) = (yystack_[0].value.vars);
 }

    break;

  case 25:

    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(argument);
  }

    break;

  case 26:

    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(argument);
  }

    break;

  case 27:

    {
    std::string name = convertAndFree((yystack_[2].value.string));

    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }

    break;

  case 28:

    {
    (yylhs.value.vars) = new vector<Var>;
  }

    break;

  case 29:

    {
    (yylhs.value.vars) = (yystack_[1].value.vars);
  }

    break;

  case 30:

    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(result);
  }

    break;

  case 31:

    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(result);
  }

    break;

  case 32:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }

    break;

  case 33:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 34:

    {
    vector<Stmt> stmts = *ctx->getStatements();
    if (stmts.size() == 0) {(yylhs.value.stmt) = new Stmt(Pass::make()); break;} // TODO: remove
    (yylhs.value.stmt) = new Stmt((stmts.size() == 1) ? stmts[0] : Block::make(stmts));
  }

    break;

  case 44:

    {
    if ((yystack_[1].value.expr) == nullptr) { break; } // TODO: Remove check

    string name = convertAndFree((yystack_[3].value.string));
    Expr value = convertAndDelete((yystack_[1].value.expr));

    Var var;
    if (ctx->hasSymbol(name)) {
      Symbol symbol = ctx->getSymbol(name);

      if (!symbol.isWritable()) {
        REPORT_ERROR(name + " is not writable", yystack_[3].location);
      }

      var = symbol.getVar();
    }
    else {
      var = Var(name, value.type());
    }

    // TODO: Check valueNames.size() matches number of values produced by expr
    ctx->addStatement(AssignStmt::make(var, value));
  }

    break;

  case 45:

    {
    string setName = convertAndFree((yystack_[5].value.string));
    string fieldName = convertAndFree((yystack_[3].value.string));
    if ((yystack_[1].value.expr) == nullptr) break;  // TODO: remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (!ctx->hasSymbol(setName)) {
      REPORT_ERROR(setName + " is not defined in scope", yystack_[5].location);
    }

    const Symbol &setSymbol = ctx->getSymbol(setName);
    if (!setSymbol.isWritable()) {
      REPORT_ERROR(setName + " is not writable", yystack_[5].location);
    }

    Expr setExpr = setSymbol.getExpr();
    ctx->addStatement(FieldWrite::make(setExpr, fieldName, value));
  }

    break;

  case 46:

    {
    std::string tensorName = convertAndFree((yystack_[6].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    if ((yystack_[1].value.expr) == nullptr) break;  // TODO: remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if(!ctx->hasSymbol(tensorName)) break;  // TODO: Remove check

    if (!ctx->hasSymbol(tensorName)) {
      REPORT_ERROR(tensorName + " is not defined in scope", yystack_[6].location);
    }

    const Symbol &tensorSymbol = ctx->getSymbol(tensorName);
    if (!tensorSymbol.isWritable()) {
      REPORT_ERROR(tensorName + " is not writable", yystack_[6].location);
    }

    Expr tensorExpr = tensorSymbol.getExpr();
    ctx->addStatement(TensorWrite::make(tensorExpr, *indices, value));
  }

    break;

  case 47:

    {
    // TODO
  }

    break;

  case 48:

    {
    delete (yystack_[3].value.expr);
    delete (yystack_[2].value.stmt);
  }

    break;

  case 50:

    {
    delete (yystack_[0].value.stmt);
  }

    break;

  case 52:

    {
    delete (yystack_[1].value.expr);
    delete (yystack_[0].value.stmt);
  }

    break;

  case 54:

    {
    std::string name = convertAndFree((yystack_[5].value.string));
    auto type = convertAndDelete((yystack_[3].value.type));
    const TensorType *tensorType = type.toTensor();

    Expr literalExpr = convertAndDelete((yystack_[1].value.expr));

    assert(literalExpr.type().isTensor() &&
           "Only tensor literals are currently supported");
    auto litType = literalExpr.type();

    // If tensor_type is a 1xn matrix and $tensor_literal is a vector then we
    // cast $tensor_literal to a 1xn matrix.
    const TensorType *litTensorType = litType.toTensor();
    if (tensorType->order() == 2 && litTensorType->order() == 1) {
      const_cast<Literal*>(to<Literal>(literalExpr))->cast(type);
    }

    // Typecheck: value and literal types must be equivalent.
    CHECK_TYPE_EQUALITY(type, literalExpr.type(), yystack_[5].location);

    Var var(name, type);
    ctx->addConstant(var, literalExpr);
    ctx->addSymbol(var);
  }

    break;

  case 55:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 56:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 65:

    {
    string ident = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(ident)) { (yylhs.value.expr)=NULL; break; } // TODO: Remove check

    if (!ctx->hasSymbol(ident)) {
      REPORT_ERROR(ident + " is not defined in scope", yystack_[0].location);
    }

    const Symbol &symbol = ctx->getSymbol(ident);
    if (!symbol.isReadable()) {
      REPORT_ERROR(ident + " is not readable", yystack_[0].location);
    }

    Expr expr = symbol.getExpr();
    (yylhs.value.expr) = new Expr(expr);
  }

    break;

  case 66:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 67:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    IRBuilder::UnaryOperator op = IRBuilder::UnaryOperator::Neg;
    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(op, expr));
  }

    break;

  case 68:

    {  // + - .* ./
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    BINARY_ELWISE_TYPE_CHECK(l.type(), r.type(), yystack_[1].location);
    (yylhs.value.expr) = new Expr(ctx->getBuilder()->binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }

    break;

  case 69:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    IRBuilder *builder = ctx->getBuilder();

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    // Scale
    if (ltype->order()==0 || rtype->order()==0) {
      IRBuilder::BinaryOperator mulOp = IRBuilder::BinaryOperator::Mul;
      (yylhs.value.expr) = new Expr(builder->binaryElwiseExpr(l, mulOp, r));
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
        (yylhs.value.expr) = new Expr(builder->innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (rtype->isColumnVector) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (l.type() != r.type()) {
          REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
        }
        (yylhs.value.expr) = new Expr(builder->outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (ltype->order() == 2 && rtype->order() == 1) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      if (ltype->dimensions[0] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->order() == 2 && rtype->order() == 2) {
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.expr) = NULL;
    }
  }

    break;

  case 70:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 71:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    IRBuilder *builder = ctx->getBuilder();
    IRBuilder::UnaryOperator noneOp = IRBuilder::UnaryOperator::None;

    Expr expr = convertAndDelete((yystack_[1].value.expr));

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    const TensorType *type = expr.type().toTensor();
    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(noneOp, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(noneOp, expr));
        if (!type->isColumnVector) {
          transposeVector(*(yylhs.value.expr));
        }
        break;
      case 2:
        (yylhs.value.expr) = new Expr(builder->transposedMatrix(expr));
        break;
      default:
        REPORT_ERROR("cannot transpose >2-order tensors using '", yystack_[1].location);
        (yylhs.value.expr) = NULL;
    }
  }

    break;

  case 72:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 73:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 74:

    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Add; }

    break;

  case 75:

    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Sub; }

    break;

  case 76:

    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Mul; }

    break;

  case 77:

    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Div; }

    break;

  case 78:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 79:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

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

  case 88:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    if (ctx->hasSymbol(name)) {
      const Symbol &symbol = ctx->getSymbol(name);
      if (!symbol.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      // The parenthesis read can read from a tensor or a tuple.
      auto expr = symbol.getExpr();
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

  case 89:

    {
    (yylhs.value.expr) = NULL;
  }

    break;

  case 90:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto actuals = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));
    (yylhs.value.expr) = new Expr(Call::make(name, *actuals, Call::Internal));
  }

    break;

  case 91:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 92:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 93:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 94:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 95:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expr) = NULL;
  }

    break;

  case 97:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 106:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("Undefined element type '" + name + "' used" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 107:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType));
  }

    break;

  case 108:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto eps = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType));
  }

    break;

  case 109:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 110:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 111:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 112:

    {
    auto componentType = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 113:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 114:

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
      for (size_t i=0; i < outerDimensions->size(); ++i) {
        vector<IndexSet> dimension;
        dimension.push_back((*outerDimensions)[i]);
        dimension.insert(dimension.begin(),
                         blockDimensions[i].getIndexSets().begin(),
                         blockDimensions[i].getIndexSets().end());

        dimensions.push_back(IndexDomain(dimension));
      }
    }

    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions));
  }

    break;

  case 115:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 116:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 117:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 118:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 119:

    {
    std::string setName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(setName)) {
      REPORT_ERROR("the set has not been declared", yystack_[0].location);
    }

    Expr set = ctx->getSymbol(setName).getExpr();
    if (!set.type().isSet()) {
      REPORT_ERROR("an index set must be a set, a range or dynamic (*)", yystack_[0].location);
    }

    (yylhs.value.indexSet) = new IndexSet(set);
  }

    break;

  case 120:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 121:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 122:

    {
    (yylhs.value.type) = new Type(Int(32));
  }

    break;

  case 123:

    {
    (yylhs.value.type) = new Type(Float(64));
  }

    break;

  case 126:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 128:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Float(64), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 129:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(Int(32), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 130:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 132:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 133:

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

  case 134:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 135:

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

  case 136:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 137:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 138:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 140:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 141:

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

  case 142:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 143:

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

  case 144:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 145:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 146:

    {
    auto scalarTensorType = TensorType::make(Int(32));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }

    break;

  case 147:

    {
    auto scalarTensorType = TensorType::make(Float(64));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }

    break;

  case 148:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 149:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 150:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 151:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 152:

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
    ctx->addTest(new Test(name, literalArgs, expecteds));
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


  const short int  Parser ::yypact_ninf_ = -191;

  const signed char  Parser ::yytable_ninf_ = -120;

  const short int
   Parser ::yypact_[] =
  {
    -191,   235,  -191,  -191,  -191,    73,    26,    37,    47,    72,
      19,    88,   120,    19,    11,  -191,   121,  -191,  -191,  -191,
    -191,    85,  -191,   117,  -191,  -191,  -191,  -191,  -191,  -191,
    -191,  -191,   307,   109,   111,  -191,  -191,   110,   129,  -191,
    -191,  -191,  -191,  -191,   123,  -191,  -191,  -191,  -191,    19,
     172,    19,  -191,   147,   149,   152,   173,   165,   176,   166,
     169,   168,   287,    23,   164,   170,   167,   161,   171,   174,
     177,   163,   178,  -191,  -191,  -191,  -191,   135,   205,   250,
     209,   250,    19,    19,  -191,  -191,  -191,    19,    19,  -191,
    -191,    19,  -191,    19,    19,    19,    19,    19,    19,    19,
     213,  -191,   407,   201,    35,   193,   327,    15,   151,    83,
    -191,   234,    19,   285,   215,   250,  -191,    19,    14,    19,
    -191,    31,    69,  -191,  -191,  -191,   210,    24,    24,  -191,
     211,    21,    21,  -191,   218,   216,   223,   460,   460,   135,
     135,   135,   427,   447,   447,   460,   460,   407,   226,    66,
    -191,   219,   212,    19,    19,  -191,   220,  -191,  -191,  -191,
    -191,    79,    78,  -191,  -191,   227,   259,  -191,  -191,  -191,
    -191,   222,   251,   230,   244,   148,  -191,   131,   281,  -191,
     347,   250,   242,  -191,  -191,    24,   272,   171,  -191,    21,
     275,   178,  -191,  -191,    47,  -191,  -191,  -191,    19,   407,
     367,   151,   151,    14,    30,  -191,   259,   233,   276,   266,
    -191,    19,   250,    19,   263,   240,    89,    94,  -191,   264,
     258,  -191,  -191,   387,  -191,    48,    -3,     7,  -191,   257,
     265,   297,  -191,   101,  -191,   176,  -191,   407,  -191,    30,
    -191,  -191,   274,    47,  -191,  -191,  -191,   278,    14,  -191,
     279,   273,  -191,  -191,  -191,  -191,   269,   282,  -191,  -191,
     151,  -191,   301,  -191,  -191,   311,     3,  -191,   107,   280,
     118,  -191,  -191,  -191,   316,    83,  -191,   311,  -191,  -191,
    -191
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   148,   150,    65,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    56,     0,     3,     7,     4,
       5,     0,     6,     0,     8,    37,    38,    39,    40,    41,
      42,    43,     0,    57,    58,    60,    61,    62,     0,    63,
      87,    64,    59,   124,   125,   127,   146,   147,     9,    91,
       0,     0,    12,     0,     0,     0,     0,    65,    33,    62,
       0,     0,     0,     0,     0,     0,   131,   130,   134,     0,
     139,   138,   142,   144,   136,   148,   150,    67,     0,    33,
       0,    33,     0,     0,    55,    74,    75,     0,     0,    76,
      77,     0,    71,     0,     0,     0,     0,     0,     0,    91,
       0,   126,    93,     0,     0,     0,     0,     0,     0,     0,
      10,     0,    91,    75,    51,    34,    35,    91,     0,    91,
      66,     0,     0,   149,   151,   128,     0,     0,     0,   129,
       0,     0,     0,    18,     0,     0,     0,    80,    81,    69,
      70,    72,    73,    78,    79,    82,    83,    68,     0,     0,
      84,    88,     0,     0,     0,    44,     0,    11,    13,   122,
     123,     0,     0,   112,   106,     0,     0,    27,   102,   103,
     104,   105,    96,    92,     0,     0,    36,   148,    65,   121,
       0,    33,     0,   132,   140,     0,     0,   135,   137,     0,
       0,   143,   145,    16,    23,    20,    89,    47,     0,    94,
       0,     0,     0,     0,     0,   115,     0,     0,     0,    98,
      48,     0,    33,     0,     0,     0,     0,     0,    17,     0,
      24,    25,    21,     0,    45,     0,     0,     0,   116,     0,
       0,     0,    97,     0,    95,    33,    50,   120,    53,     0,
     133,   141,    28,     0,    46,    14,   113,     0,     0,    54,
     107,     0,   100,   101,    99,    52,     0,     0,    22,    26,
       0,   117,     0,   111,   152,     0,     0,   109,     0,     0,
       0,    30,   114,   108,     0,     0,    29,     0,   110,    32,
      31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -191,  -191,  -191,  -191,  -191,  -191,  -191,  -191,  -191,  -191,
    -191,  -191,  -191,  -191,  -191,  -191,  -191,  -180,  -191,  -191,
      50,   -71,  -191,     8,  -191,  -191,  -191,  -191,  -191,  -191,
    -191,  -191,  -191,   -10,  -191,  -191,  -191,  -191,  -191,     1,
    -191,  -191,  -191,   -79,   -37,  -191,  -191,  -191,  -191,    53,
    -135,  -191,  -191,  -191,  -104,  -191,  -190,  -191,  -191,  -187,
    -191,  -191,  -191,   -52,   202,  -191,  -191,   -58,   199,  -191,
     -13,    -7,  -191
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    17,    18,    19,   107,   158,    20,    21,   218,
      79,    22,    23,   222,    81,   219,   220,    55,   258,   270,
     271,   114,   115,   116,    25,    26,    27,    28,   174,   175,
      29,    30,    31,    32,    33,    34,    35,    98,    36,    59,
      38,    39,    40,   103,   173,    41,   209,   234,   254,   167,
     168,   169,   268,   170,   171,   227,   181,   163,    42,    43,
      44,    65,    66,    67,    68,    69,    70,    71,    72,    45,
      46,    47,    48
  };

  const short int
   Parser ::yytable_[] =
  {
      58,    73,    37,    62,   162,   122,    77,    74,   134,    24,
     136,   121,   104,   228,   221,     3,     4,   229,   177,     4,
     148,   178,   156,     3,     4,     3,    57,     3,     4,     4,
     246,   207,     9,    52,     3,     4,   272,     9,   148,   102,
     182,   106,   247,   157,    53,    63,    13,   248,    14,   205,
      73,    13,   256,    14,    54,   205,    74,    64,   261,    37,
      16,   179,   149,   259,    14,    16,   183,   190,   152,    64,
     186,   230,   137,   138,   127,   153,    64,   139,   140,    56,
      37,   141,    37,   142,   143,   144,   145,   146,   147,   102,
     164,   245,   159,   160,   161,    60,   165,   225,   226,   197,
     205,    78,   102,    77,   184,    49,   153,   102,   180,   102,
     214,   202,   131,   203,    50,   166,    37,    51,    73,   192,
      74,   188,   204,   176,   240,    75,    76,    61,    57,   241,
     205,   217,   127,   216,    80,  -118,  -118,   131,  -118,     9,
     273,   236,    99,   199,   200,  -118,   252,   274,   253,  -118,
     -85,   276,   -86,    13,  -118,    14,   266,  -118,   277,  -118,
     159,   160,   161,  -118,   255,  -118,  -118,    16,   123,   124,
     100,  -118,   211,   212,  -118,   101,    73,  -118,    74,   105,
       3,     4,    37,     5,    89,    90,    91,    92,   223,   108,
       7,   109,   111,   180,     9,   110,   118,   112,   117,    10,
     119,   235,    11,   237,   127,   125,   131,   126,    13,   129,
      14,   128,   133,    37,    82,    83,   135,   130,   132,    15,
     150,    85,   113,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,   151,     2,    37,   154,   180,     3,
       4,   172,     5,   -49,   185,   189,   193,     6,   194,     7,
       8,   195,   -19,     9,     3,     4,   198,     5,    10,   196,
     -90,    11,   201,   206,     7,    12,   164,    13,     9,    14,
     153,   208,   210,    10,   205,   215,    11,   124,    15,   123,
     231,    16,    13,   232,    14,  -119,  -119,   233,  -119,    75,
      76,   238,    57,    15,   239,  -119,    16,   242,   243,  -119,
     249,   251,   250,     9,  -119,   257,   263,  -119,   267,  -119,
     260,   262,   264,   112,   265,  -119,  -119,    13,   269,    14,
     120,  -119,   275,   278,  -119,    82,    83,   280,   279,   187,
     191,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
      84,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
     155,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,   213,
       0,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
     224,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
     244,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
       0,     0,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    82,    83,     0,     0,     0,
       0,     0,    85,    86,    87,    88,    89,    90,    91,    92,
    -120,    94,    95,    96,    97,    82,    83,     0,     0,     0,
       0,     0,    85,    86,    87,    88,    89,    90,    91,    92,
       0,     0,     0,    96,    97,    85,    86,    87,    88,    89,
      90,    91,    92
  };

  const short int
   Parser ::yycheck_[] =
  {
      10,    14,     1,    13,   108,    63,    16,    14,    79,     1,
      81,    63,    49,   203,   194,     4,     5,   204,     4,     5,
      99,     7,     7,     4,     5,     4,     7,     4,     5,     5,
      33,   166,    18,     7,     4,     5,    33,    18,   117,    49,
     119,    51,    35,    28,     7,    34,    32,    40,    34,    52,
      63,    32,   239,    34,     7,    52,    63,    46,   248,    58,
      46,    47,    99,   243,    34,    46,    35,    46,    33,    46,
      46,   206,    82,    83,    43,    40,    46,    87,    88,     7,
      79,    91,    81,    93,    94,    95,    96,    97,    98,    99,
       7,    43,     9,    10,    11,     7,    13,   201,   202,    33,
      52,    16,   112,   113,    35,    32,    40,   117,   118,   119,
     181,    32,    43,    34,    41,    32,   115,    44,   131,   132,
     127,   128,    44,   115,    35,     4,     5,     7,     7,    35,
      52,   189,    43,   185,    17,     4,     5,    43,     7,    18,
      33,   212,    32,   153,   154,    14,    45,    40,    47,    18,
      41,    33,    41,    32,    23,    34,   260,    26,    40,    28,
       9,    10,    11,    32,   235,    34,    35,    46,     4,     5,
      41,    40,    24,    25,    43,    52,   189,    46,   185,     7,
       4,     5,   181,     7,    49,    50,    51,    52,   198,    42,
      14,    42,    19,   203,    18,    43,    27,    32,    32,    23,
      32,   211,    26,   213,    43,    35,    43,    40,    32,    35,
      34,    40,     7,   212,    38,    39,     7,    40,    40,    43,
       7,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    33,     0,   235,    44,   248,     4,
       5,     7,     7,    28,    34,    34,    28,    12,    32,    14,
      15,    28,    17,    18,     4,     5,    44,     7,    23,    33,
      41,    26,    42,    36,    14,    30,     7,    32,    18,    34,
      40,    20,    28,    23,    52,    33,    26,     5,    43,     4,
      47,    46,    32,     7,    34,     4,     5,    21,     7,     4,
       5,    28,     7,    43,    54,    14,    46,    33,    40,    18,
      43,     4,    37,    18,    23,    31,    33,    26,     7,    28,
      32,    32,    43,    32,    32,    34,    35,    32,     7,    34,
      33,    40,    42,     7,    43,    38,    39,   277,   275,   127,
     131,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    42,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      43,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    38,    39,    -1,    -1,    -1,
      -1,    -1,    45,    46,    47,    48,    49,    50,    51,    52,
      -1,    -1,    -1,    56,    57,    45,    46,    47,    48,    49,
      50,    51,    52
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     4,     5,     7,    12,    14,    15,    18,
      23,    26,    30,    32,    34,    43,    46,    60,    61,    62,
      65,    66,    69,    70,    81,    82,    83,    84,    85,    88,
      89,    90,    91,    92,    93,    94,    96,    97,    98,    99,
     100,   103,   116,   117,   118,   127,   128,   129,   130,    32,
      41,    44,     7,     7,     7,    75,     7,     7,    91,    97,
       7,     7,    91,    34,    46,   119,   120,   121,   122,   123,
     124,   125,   126,   128,   129,     4,     5,    91,    16,    68,
      17,    72,    38,    39,    43,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    95,    32,
      41,    52,    91,   101,   102,     7,    91,    63,    42,    42,
      43,    19,    32,    46,    79,    80,    81,    32,    27,    32,
      33,   121,   125,     4,     5,    35,    40,    43,    40,    35,
      40,    43,    40,     7,    79,     7,    79,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,   101,   102,
       7,    33,    33,    40,    44,    43,     7,    28,    64,     9,
      10,    11,   112,   115,     7,    13,    32,   107,   108,   109,
     111,   112,     7,   102,    86,    87,    81,     4,     7,    47,
      91,   114,   101,    35,    35,    34,    46,   122,   129,    34,
      46,   126,   128,    28,    32,    28,    33,    33,    44,    91,
      91,    42,    32,    34,    44,    52,    36,   108,    20,   104,
      28,    24,    25,    42,    79,    33,   121,   125,    67,    73,
      74,    75,    71,    91,    43,   112,   112,   113,   114,   117,
     108,    47,     7,    21,   105,    91,    79,    91,    28,    54,
      35,    35,    33,    40,    43,    43,    33,    35,    40,    43,
      37,     4,    45,    47,   106,    79,   117,    31,    76,    75,
      32,   114,    32,    33,    43,    32,   112,     7,   110,     7,
      77,    78,    33,    33,    40,    42,    33,    40,     7,   107,
      78
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    63,    64,    66,    67,    65,    68,    70,
      71,    69,    72,    73,    73,    74,    74,    75,    76,    76,
      77,    77,    78,    79,    79,    80,    80,    81,    81,    81,
      81,    81,    81,    81,    82,    83,    84,    84,    85,    86,
      86,    87,    87,    88,    89,    90,    90,    91,    91,    91,
      91,    91,    91,    91,    91,    92,    93,    94,    94,    94,
      94,    94,    94,    94,    95,    95,    95,    95,    96,    96,
      96,    96,    96,    96,    97,    98,    98,    98,    99,    99,
     100,   101,   101,   102,   102,   103,   104,   104,   105,   105,
     106,   106,   107,   107,   107,   107,   108,   109,   109,   110,
     110,   111,   112,   112,   112,   112,   113,   113,   114,   114,
     114,   114,   115,   115,   116,   117,   117,   117,   118,   118,
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
       1,     3,     3,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     4,     6,     7,     4,     5,     0,
       3,     0,     4,     6,     7,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     3,     3,
       3,     2,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     4,     4,
       4,     0,     1,     1,     3,     6,     0,     2,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     4,     7,     1,
       3,     5,     1,     4,     7,     2,     1,     3,     1,     1,
       3,     1,     1,     1,     1,     1,     2,     1,     3,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       3,     5,     1,     3,     1,     3,     1,     1,     1,     2,
       1,     2,     8
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "\"int literal\"",
  "\"float literal\"", "\"string literal\"", "\"identifier\"", "NEG",
  "\"int\"", "\"float\"", "\"tensor\"", "\"element\"", "\"set\"",
  "\"const\"", "\"extern\"", "\"proc\"", "\"func\"", "\"map\"", "\"to\"",
  "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"", "\"else\"",
  "\"for\"", "\"in\"", "\"end\"", "\"return\"", "\"%!\"", "\"->\"",
  "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"",
  "\",\"", "\".\"", "\":\"", "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"",
  "\"/\"", "\".*\"", "\"./\"", "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"",
  "\"!=\"", "\"<=\"", "\">=\"", "$accept", "program", "program_element",
  "extern", "element_type_decl", "field_decl_list", "field_decl",
  "procedure", "$@1", "$@2", "procedure_header", "function", "$@3", "$@4",
  "function_header", "arguments", "argument_list", "argument_decl",
  "results", "result_list", "result_decl", "stmt_block", "stmts", "stmt",
  "assign_stmt", "field_write_stmt", "tensor_write_stmt", "if_stmt",
  "else_clauses", "elif_clauses", "for_stmt", "const_stmt", "expr_stmt",
  "expr", "ident_expr", "paren_expr", "linear_algebra_expr",
  "elwise_binary_op", "boolean_expr", "field_read_expr", "set_read_expr",
  "call_or_paren_read_expr", "call_expr", "expr_list_or_empty",
  "expr_list", "map_expr", "with", "reduce", "reduction_op", "type",
  "element_type", "set_type", "endpoints", "tuple_type", "tensor_type",
  "index_sets", "index_set", "component_type", "literal_expr",
  "tensor_literal", "dense_tensor_literal", "float_dense_tensor_literal",
  "float_dense_ndtensor_literal", "float_dense_matrix_literal",
  "float_dense_vector_literal", "int_dense_tensor_literal",
  "int_dense_ndtensor_literal", "int_dense_matrix_literal",
  "int_dense_vector_literal", "scalar_literal", "signed_int_literal",
  "signed_float_literal", "test", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   269,   269,   271,   274,   275,   283,   291,   292,   293,
     298,   308,   321,   324,   332,   342,   342,   342,   350,   370,
     370,   370,   378,   400,   403,   409,   414,   422,   431,   434,
     440,   445,   453,   463,   466,   473,   474,   477,   478,   479,
     480,   481,   482,   483,   486,   511,   531,   551,   557,   562,
     564,   568,   570,   576,   579,   607,   610,   618,   619,   620,
     621,   622,   623,   624,   625,   631,   653,   662,   671,   683,
     750,   756,   786,   791,   800,   801,   802,   803,   809,   815,
     821,   827,   833,   839,   850,   869,   870,   871,   877,   910,
     916,   924,   927,   933,   939,   950,   958,   960,   964,   966,
     969,   970,   976,   977,   978,   979,   983,   995,   999,  1009,
    1013,  1020,  1032,  1036,  1039,  1081,  1091,  1096,  1104,  1107,
    1121,  1127,  1133,  1136,  1186,  1190,  1191,  1195,  1199,  1206,
    1217,  1224,  1228,  1232,  1246,  1250,  1265,  1269,  1276,  1283,
    1287,  1291,  1305,  1309,  1324,  1328,  1335,  1339,  1346,  1349,
    1355,  1358,  1365
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



