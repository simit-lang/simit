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

  #include "scanner.h"
  #include "irutils.h"
  #include "util.h"
  using namespace std;
  using namespace simit;  // TODO: Remove
  using namespace simit::internal;

  #define REPORT_ERROR(msg, loc)  \
    do {                          \
      error((loc), (msg));        \
      YYERROR;                    \
    } while (0)

  void Parser::error(const Parser::location_type &loc, const std::string &msg) {
    ctx->addError(Error(loc.begin.line, loc.begin.column,
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

  bool compare(const TensorType *l ,const TensorType *r, ParserContext *ctx) {
    if (ctx->isColumnVector(l) != ctx->isColumnVector(r)) {
      return false;
    }
    if (*l != *r) {
      return false;
    }
    return true;
  }

  std::string tensorTypeString(const TensorType *tensorType, ParserContext *ctx){
    std::stringstream ss;
    ss << *tensorType;
    std::string str = ss.str();
    if (ctx->isColumnVector(tensorType)) {
      str += "'";
    }
    return str;
  }

  #define CHECK_TYPE_EQUALITY(t1, t2, loc)               \
    do {                                                 \
      if (!compare(t1, t2, ctx)) {                       \
        std::stringstream errorStr;                      \
        errorStr << "type missmatch ("                   \
                 << tensorTypeString(t1, ctx) << " != "  \
                 << tensorTypeString(t2, ctx) << ")";    \
          REPORT_ERROR(errorStr.str(), loc);             \
        } \
    } while (0)


  #define BINARY_ELWISE_TYPE_CHECK(t1, t2, loc)        \
    do {                                               \
      if (t1->getOrder() > 0 && t2->getOrder() > 0) {  \
        CHECK_TYPE_EQUALITY(t1, t2, loc);              \
      }                                                \
    }                                                  \
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
   Parser :: Parser  (Scanner *scanner_yyarg, ParserContext *ctx_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      scanner (scanner_yyarg),
      ctx (ctx_yyarg)
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

      case 64: // procedure


        { delete (yysym.value.Function); }

        break;

      case 65: // function


        { delete (yysym.value.Function); }

        break;

      case 66: // function_header


        { delete (yysym.value.Function); }

        break;

      case 67: // arguments


        { delete (yysym.value.Arguments); }

        break;

      case 68: // results


        { delete (yysym.value.Results); }

        break;

      case 69: // formal_decl


        { delete (yysym.value.Formal); }

        break;

      case 70: // formal_list


        { delete (yysym.value.Formals); }

        break;

      case 71: // stmt_block


        { delete (yysym.value.IRNodes); }

        break;

      case 72: // stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 73: // const_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 74: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 77: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 78: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 79: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 80: // expr


        { delete (yysym.value.Tensor); }

        break;

      case 81: // call_expr


        { delete (yysym.value.Call); }

        break;

      case 82: // actual_list


        { delete (yysym.value.TensorList); }

        break;

      case 83: // expr_list


        { delete (yysym.value.TensorList); }

        break;

      case 91: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 92: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 94: // type


        { delete (yysym.value.TensorType); }

        break;

      case 95: // element_type


        { delete (yysym.value.TensorType); }

        break;

      case 96: // tensor_type


        { delete (yysym.value.TensorType); }

        break;

      case 97: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 98: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 99: // dimension


        { delete (yysym.value.IndexSet); }

        break;

      case 100: // component_type


        {}

        break;

      case 101: // literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 103: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 104: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 105: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 106: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 107: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 108: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 109: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 110: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 111: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 112: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 113: // scalar_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 114: // test


        { delete (yysym.value.Test); }

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
  case 4:

    {
    (yylhs.value.IRNode) = NULL;
  }

    break;

  case 5:

    {
    (yylhs.value.IRNode) = NULL;
  }

    break;

  case 6:

    {
    std::unique_ptr<Function> function((yystack_[0].value.Function));

    auto name = function->getName();
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
    ctx->addTest((yystack_[0].value.Test));
  }

    break;

  case 10:

    {
    std::string ident = convertAndFree((yystack_[4].value.string));
    delete (yystack_[3].value.TensorType);
  }

    break;

  case 11:

    {
    std::string ident = convertAndFree((yystack_[7].value.string));
    delete (yystack_[6].value.TensorType);
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
    std::string ident = convertAndFree((yystack_[1].value.string));
  }

    break;

  case 20:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.IRNodes);
  }

    break;

  case 21:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[1].value.IRNodes));
    (yylhs.value.Function) = (yystack_[2].value.Function);
    (yylhs.value.Function)->addStatements(*statements);
    ctx->unscope();
  }

    break;

  case 22:

    {
    std::string ident = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<shared_ptr<Argument>>>((yystack_[2].value.Arguments));
    auto results = unique_ptr<vector<shared_ptr<Result>>>((yystack_[0].value.Results));

    (yylhs.value.Function) = new Function(ident, *arguments, *results);

    ctx->scope();
    for (auto argument : *arguments) {
      ctx->addTensorSymbol(argument->getName(), argument);
    }

    for (auto result : *results) {
      ctx->addTensorSymbol(result->getName(), result);
    }
  }

    break;

  case 23:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
  }

    break;

  case 24:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
    for (auto formal : *(yystack_[0].value.Formals)) {
      auto result = new Argument(formal->name, formal->type);
      (yylhs.value.Arguments)->push_back(shared_ptr<Argument>(result));
    }
    delete (yystack_[0].value.Formals);
 }

    break;

  case 25:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    (yylhs.value.Results)->push_back(shared_ptr<Result>(new Result("asd", NULL)));
  }

    break;

  case 26:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    for (auto formal : *(yystack_[1].value.Formals)) {
      auto result = new Result(formal->name, formal->type);
      (yylhs.value.Results)->push_back(shared_ptr<Result>(result));
    }
    delete (yystack_[1].value.Formals);
  }

    break;

  case 27:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    (yylhs.value.Formal) = new FormalData(ident, (yystack_[0].value.TensorType));
  }

    break;

  case 28:

    {
    (yylhs.value.Formals) = new simit::util::OwnershipVector<FormalData *>();
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 29:

    {
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 30:

    {
    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
  }

    break;

  case 31:

    {
    (yylhs.value.IRNodes) = (yystack_[1].value.IRNodes);
    if ((yystack_[0].value.IRNodes) == NULL) break;  // TODO: Remove check
    (yylhs.value.IRNodes)->insert((yylhs.value.IRNodes)->end(), (yystack_[0].value.IRNodes)->begin(), (yystack_[0].value.IRNodes)->end());
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 37:

    {
    std::string ident = convertAndFree((yystack_[5].value.string));
    auto tensorType = unique_ptr<TensorType>((yystack_[3].value.TensorType));

    auto tensorLiteral = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[1].value.TensorLiteral);

    tensorLiteral->setName(ident);

    // If $type is a 1xn matrix and $tensor_literal is a vector then we cast
    // $tensor_literal to a 1xn matrix.
    if (tensorType->getOrder() == 2 && tensorLiteral->getOrder() == 1) {
      tensorLiteral->cast(tensorType.release());
    }

    // Typecheck: value and literal types must be equivalent.
    //            Note that the use of $tensor_type is deliberate as tensorType
    //            can have been released.
    CHECK_TYPE_EQUALITY((yystack_[3].value.TensorType), tensorLiteral->getType(), yystack_[5].location);

    ctx->addTensorSymbol(tensorLiteral->getName(), tensorLiteral);

    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
    (yylhs.value.IRNodes)->push_back(tensorLiteral);
  }

    break;

  case 38:

    {
    std::string ident = convertAndFree((yystack_[5].value.string));
    delete (yystack_[3].value.TensorType);
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 39:

    {
    (yylhs.value.IRNodes) = NULL;
    delete (yystack_[3].value.Tensor);
    delete (yystack_[2].value.IRNodes);
  }

    break;

  case 41:

    {
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 43:

    {
    delete (yystack_[1].value.Tensor);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 44:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 45:

    {
    auto lhsList = unique_ptr<VariableAccessVector>((yystack_[3].value.VarAccesses));
    auto rhsList = unique_ptr<vector<shared_ptr<TensorNode>>>((yystack_[1].value.TensorList));

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
      std::shared_ptr<TensorNode> &rhs = *rhsIter;

      // TODO: Remove these checks
      if (rhs == NULL) continue;
      if (!ctx->hasSymbol(lhs->name)) continue;

      assert(ctx->hasSymbol(lhs->name));
      shared_ptr<IRNode> lhsTensor = ctx->getSymbol(lhs->name);
      assert(dynamic_pointer_cast<TensorNode>(lhsTensor) != NULL);
      if (auto result = dynamic_pointer_cast<Result>(lhsTensor)) {
        CHECK_TYPE_EQUALITY(result->getType(), rhs->getType(), yystack_[2].location);

        rhs->setName(result->getName() + "_val");  // TODO: Create unique name
        result->setValue(rhs);
        (yylhs.value.IRNodes)->push_back(rhs);
      }
      else {
        NOT_SUPPORTED_YET;
      }
    }
  }

    break;

  case 46:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 47:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 48:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    if (!ctx->hasSymbol(ident)) {
      // TODO: reintroduce error
      // REPORT_ERROR(ident + " is not defined in scope", @1);
      (yylhs.value.Tensor) = NULL;
      break;
    }

    const std::shared_ptr<IRNode> &node = ctx->getSymbol(ident);

    shared_ptr<TensorNode> tensor = dynamic_pointer_cast<TensorNode>(node);
    if (tensor == NULL) {
      REPORT_ERROR(ident + " is not a tensor", yystack_[0].location);
    }

    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(tensor);
  }

    break;

  case 50:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 51:

    {
    if ((yystack_[0].value.Tensor) == NULL) {  // TODO: Remove check
      (yylhs.value.Tensor) = NULL;
      break;
    }
    std::shared_ptr<TensorNode> expr = convertAndDelete((yystack_[0].value.Tensor));
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 52:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }
    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::ADD, r));
  }

    break;

  case 53:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }

    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::SUB, r));
  }

    break;

  case 54:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }
    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::MUL, r));
  }

    break;

  case 55:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }
    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::DIV, r));
  }

    break;

  case 56:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }

    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    bool scaleOp = (l->getType()->getOrder()==0 || r->getType()->getOrder()==0);

    if (!scaleOp && (l->getType()->getOrder()>2 || r->getType()->getOrder()>2)){
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.Tensor) = NULL;
    }

    // Scale
    if (scaleOp) {
      (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::MUL, r));
    }
    // Vector-Vector Multiplication (inner and outer product)
    else if (l->getType()->getOrder() == 1 || r->getType()->getOrder() == 1) {
      // Inner product
      if (!ctx->isColumnVector(l->getType())) {
        if (!ctx->isColumnVector(r->getType())) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (ctx->isColumnVector(r->getType())) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(outerProduct(l, r));
      }
    }
    // Matrix-Vector, Vector-Matrix and Matrix-Matrix Multiplcation
    else {
      (yylhs.value.Tensor) = NULL;
    }
  }

    break;

  case 57:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 58:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 59:

    {
    if ((yystack_[1].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }

    auto expr = shared_ptr<TensorNode>(*(yystack_[1].value.Tensor));
    delete (yystack_[1].value.Tensor);

    switch (expr->getType()->getOrder()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(unaryElwiseExpr(IndexExpr::NONE, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(unaryElwiseExpr(IndexExpr::NONE, expr));
        if (!ctx->isColumnVector(expr->getType())) {
          ctx->toggleColumnVector((*(yylhs.value.Tensor))->getType());
        }
        break;
      case 2:
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(transposeMatrix(expr));
        break;
      default:
        REPORT_ERROR("cannot transpose >2-order tensors using '", yystack_[1].location);
        (yylhs.value.Tensor) = NULL;
    }
  }

    break;

  case 60:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 61:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 62:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 63:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 64:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 65:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 66:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 67:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[1].value.Tensor);
  }

    break;

  case 68:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 69:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 70:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 71:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 72:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 73:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.TensorList));
    delete (yystack_[1].value.TensorList);
    (yylhs.value.Call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 74:

    {
    (yylhs.value.TensorList) = new vector<shared_ptr<TensorNode>>();
  }

    break;

  case 75:

    {
    (yylhs.value.TensorList) = (yystack_[0].value.TensorList);
  }

    break;

  case 76:

    {
    (yylhs.value.TensorList) = new std::vector<std::shared_ptr<TensorNode>>();
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 77:

    {
    (yylhs.value.TensorList) = (yystack_[2].value.TensorList);
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 78:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
  }

    break;

  case 80:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 86:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 87:

    {
    std::string ident = convertAndFree((yystack_[1].value.string));
  }

    break;

  case 88:

    {
    free((void*)(yystack_[3].value.string));
    free((void*)(yystack_[1].value.string));
  }

    break;

  case 91:

    {
    (yylhs.value.VarAccesses) = new VariableAccessVector();
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 92:

    {
    (yylhs.value.VarAccesses) = (yystack_[2].value.VarAccesses);
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 93:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto variableStore = new VariableAccess;
    variableStore->name = name;
    (yylhs.value.VarAccess) = variableStore;
  }

    break;

  case 94:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.VarAccess) = NULL;
    delete (yystack_[1].value.TensorList);
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
    delete (yystack_[0].value.TensorType);
    std::string ident = convertAndFree((yystack_[2].value.string));
  }

    break;

  case 97:

    {
    (yylhs.value.TensorType) = (yystack_[0].value.TensorType);
  }

    break;

  case 98:

    {
    (yylhs.value.TensorType) = (yystack_[0].value.TensorType);
  }

    break;

  case 99:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));

    (yylhs.value.TensorType) = new TensorType(Type::ELEMENT);
  }

    break;

  case 100:

    {
    (yylhs.value.TensorType) = new TensorType((yystack_[0].value.Type));
  }

    break;

  case 101:

    {
    (yylhs.value.TensorType) = new TensorType((yystack_[1].value.Type), *(yystack_[3].value.IndexSetProducts));
    delete (yystack_[3].value.IndexSetProducts);
  }

    break;

  case 102:

    {
    (yylhs.value.TensorType) = new TensorType((yystack_[2].value.Type), *(yystack_[4].value.IndexSetProducts));
    ctx->toggleColumnVector((yylhs.value.TensorType));
    delete (yystack_[4].value.IndexSetProducts);
  }

    break;

  case 103:

    {
    (yylhs.value.IndexSetProducts) = new std::vector<IndexSetProduct>();
  }

    break;

  case 104:

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

  case 105:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.IndexSet));
    delete (yystack_[0].value.IndexSet);
  }

    break;

  case 106:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.IndexSet));
    delete (yystack_[0].value.IndexSet);
  }

    break;

  case 107:

    {
    (yylhs.value.IndexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 108:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.IndexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 109:

    {
    (yylhs.value.IndexSet) = new IndexSet();
  }

    break;

  case 110:

    {
    (yylhs.value.Type) = Type::INT;
  }

    break;

  case 111:

    {
    (yylhs.value.Type) = Type::FLOAT;
  }

    break;

  case 116:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    ctx->toggleColumnVector((*(yylhs.value.TensorLiteral))->getType());
  }

    break;

  case 118:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::FLOAT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 119:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::INT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 120:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 122:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 123:

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

  case 124:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 125:

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

  case 126:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 127:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 128:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 130:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 131:

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

  case 132:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 133:

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

  case 134:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 135:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 136:

    {
    auto scalarType = new TensorType(Type::INT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 137:

    {
    auto scalarType = new TensorType(Type::FLOAT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 138:

    {
    auto call = shared_ptr<Call>(*(yystack_[3].value.Call));
    auto expected = shared_ptr<Literal>(*(yystack_[1].value.TensorLiteral));
    delete (yystack_[3].value.Call);
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
    (yylhs.value.Test) = new Test(call->getName(), literalArgs, expecteds);
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


  const short int  Parser ::yypact_ninf_ = -153;

  const signed char  Parser ::yytable_ninf_ = -94;

  const short int
   Parser ::yypact_[] =
  {
    -153,    14,  -153,  -153,  -153,  -153,    15,     5,    16,    26,
      63,    94,   116,   214,   124,   158,   214,    11,  -153,   214,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,   236,  -153,  -153,  -153,    72,   152,  -153,  -153,
    -153,  -153,   120,  -153,  -153,    89,   179,    25,   151,   240,
    -153,   163,   177,   172,   256,  -153,   172,   153,   216,  -153,
    -153,    41,   170,   173,   171,   175,   183,   187,   176,   196,
      44,   145,   214,   214,   214,   214,  -153,   214,   214,   214,
     214,   214,   214,   214,  -153,   214,   214,   214,   214,   214,
     113,  -153,  -153,   256,  -153,     2,   217,   214,  -153,   256,
     210,   -19,  -153,   203,  -153,   222,   245,   233,   240,  -153,
    -153,  -153,  -153,    54,  -153,  -153,  -153,   174,   269,   285,
      89,   105,    32,  -153,   101,   103,  -153,   265,   291,   307,
    -153,   286,   311,   328,  -153,  -153,   309,   309,   256,   256,
     190,   190,    44,    44,   256,   256,    44,   276,   296,   296,
     309,   309,   329,  -153,   338,    10,  -153,    18,  -153,  -153,
     214,   240,  -153,  -153,  -153,   295,   318,    57,   352,   327,
    -153,   323,   333,  -153,   330,   345,   331,   340,    85,   326,
    -153,  -153,   291,   175,  -153,   311,   196,  -153,   335,   336,
     214,  -153,   256,  -153,  -153,    32,   188,     7,  -153,    67,
     332,   240,   343,   269,   365,   354,  -153,   214,  -153,  -153,
     128,   133,  -153,  -153,   -19,   337,   339,   346,  -153,  -153,
    -153,   146,  -153,   342,   370,  -153,  -153,   351,  -153,  -153,
    -153,    79,  -153,   256,   201,  -153,  -153,  -153,  -153,   334,
    -153,     7,   347,  -153,   269,  -153,   132,  -153,  -153,   344,
      68,  -153,  -153
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    84,     1,   136,   137,    50,    48,     0,     0,     0,
       0,     0,     0,    84,     0,     0,    84,     0,    46,    84,
       3,     7,     4,     5,     6,    30,     8,    32,    33,    34,
      35,    36,     0,    70,    72,    71,    84,     0,    91,    49,
     112,   113,   115,   117,     9,    84,     0,    15,     0,     0,
      30,     0,     0,    48,    30,    44,     0,     0,     0,   134,
     126,     0,     0,   121,   120,   124,     0,   129,   128,   132,
      51,    84,    84,    84,    84,    84,    47,    84,    84,    84,
      84,    84,    84,    84,    59,    84,    84,    84,    84,    84,
      48,    89,    90,    83,    85,     0,     0,    84,   116,    76,
       0,     0,    95,     0,    14,     0,    17,     0,     0,    99,
     110,   111,   103,     0,    97,    98,   100,    84,    23,     0,
      84,    84,     0,    67,     0,     0,   118,     0,     0,     0,
     119,     0,     0,     0,    21,    31,    63,    64,    69,    68,
      52,    53,    56,    57,    54,    55,    60,    58,    61,    62,
      65,    66,     0,    86,     0,    93,    92,     0,    73,    94,
      84,     0,    16,    18,    19,     0,     0,     0,     0,     0,
      20,     0,     0,    28,    24,    79,    75,     0,     0,     0,
     122,   130,     0,   125,   127,     0,   133,   135,     0,     0,
      84,    45,    77,    96,   114,     0,     0,     0,    12,     0,
       0,     0,    25,     0,     0,    81,    39,    84,    30,   138,
       0,     0,    88,    87,     0,     0,     0,     0,   107,   108,
     109,     0,   105,     0,     0,    10,    27,     0,    22,    29,
      80,     0,    78,    30,    84,   123,   131,    38,    37,   101,
     104,     0,     0,    13,     0,    82,    84,   102,   106,     0,
       0,    11,    26
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -153,  -153,  -153,  -153,  -153,  -153,  -153,   279,  -153,  -153,
    -153,  -153,  -153,  -153,   184,   138,   -49,   385,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,   -13,   373,  -153,   -90,  -153,
    -153,  -153,  -153,  -153,  -153,   159,  -153,   293,  -153,  -152,
     283,   284,  -153,  -153,   154,   197,  -153,   200,  -114,  -153,
    -153,  -153,   -57,   268,  -153,  -153,   -59,   266,  -153,  -153
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    20,    21,   199,    22,   104,   105,   106,    23,
      24,    25,   172,   228,   173,   174,    71,   135,    27,    28,
     177,   178,    29,    30,    31,    32,    33,   100,   101,    34,
     205,   232,    35,    36,    94,    95,    37,    38,   107,   113,
     114,   115,   167,   221,   222,   116,    39,    40,    41,    42,
      62,    63,    64,    65,    66,    67,    68,    69,    43,    44
  };

  const short int
   Parser ::yytable_[] =
  {
      54,   117,   125,    58,   124,   121,    70,   157,   179,   193,
     153,   159,   218,    47,     2,   219,    59,    60,   160,     3,
       4,     5,     6,    93,    48,     7,     8,     9,    10,    11,
     176,    12,    99,   103,    49,   154,    13,     3,     4,   190,
      14,    15,    61,    16,    45,    17,    59,    60,    46,   226,
     -17,   220,   -93,    46,    18,   160,   -93,    19,   191,   136,
     137,   138,   139,    17,   140,   141,   142,   143,   144,   145,
     146,    50,   147,   148,   149,   150,   151,     3,     4,     5,
      90,   216,    74,    75,    99,   168,   196,   169,   197,    12,
      81,    82,    83,    84,     3,     4,     5,    53,   252,   223,
     214,    16,    51,    17,   224,   203,    12,    99,   207,   208,
       3,     4,     5,     6,    91,    19,    92,     8,    16,   -74,
      17,    91,    12,    92,    52,   210,   211,    13,   -42,   -42,
     -40,    14,    19,   180,    16,   181,    17,     3,     4,     5,
       6,   128,   120,   132,     8,    18,   152,   192,    19,    12,
       3,     4,     5,     6,    13,   -43,   -43,     8,    14,   234,
     235,    16,    12,    17,    55,   236,    56,    13,   128,    98,
     134,    14,    18,   132,    16,    19,    17,    99,   240,     3,
       4,     5,     6,   241,   246,    18,     8,   102,    19,    96,
     108,    12,   118,    97,   233,   119,    13,   110,   111,   170,
      14,   120,   126,    16,   122,    17,     3,     4,     5,     6,
     127,   128,   129,     8,    18,   130,   132,    19,    12,     3,
       4,     5,    53,    13,   131,   155,   -41,    14,    74,    75,
      16,    12,    17,   133,    79,    80,    81,    82,    83,    84,
     158,    18,   161,    16,    19,    17,   123,   162,   109,   110,
     111,    72,    73,   103,    74,    75,   112,    19,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    72,    73,   164,    74,    75,    76,   171,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    72,    73,   175,    74,    75,   182,    60,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    72,    73,   184,    74,    75,    59,   185,    77,    78,
      79,    80,    81,    82,    83,    84,   -94,    86,    87,    88,
      89,    72,    73,   187,    74,    75,   194,   188,    77,    78,
      79,    80,    81,    82,    83,    84,   189,    74,    75,    88,
      89,    77,    78,    79,    80,    81,    82,    83,    84,   195,
     198,   200,   201,   202,   204,   206,   209,   203,   160,   212,
     213,   227,   225,   230,   231,   242,   239,   237,   243,   238,
     244,   249,   250,   247,   251,   163,    26,   229,    57,   156,
     245,   165,   166,   217,   215,   248,   183,     0,   186
  };

  const short int
   Parser ::yycheck_[] =
  {
      13,    50,    61,    16,    61,    54,    19,    97,   122,   161,
       8,    30,     5,     8,     0,     8,     5,     6,    37,     5,
       6,     7,     8,    36,     8,    11,    12,    13,    14,    15,
     120,    17,    45,     8,     8,    33,    22,     5,     6,    29,
      26,    27,    31,    29,    29,    31,     5,     6,    38,   201,
      25,    44,    37,    38,    40,    37,    41,    43,    40,    72,
      73,    74,    75,    31,    77,    78,    79,    80,    81,    82,
      83,     8,    85,    86,    87,    88,    89,     5,     6,     7,
       8,   195,    38,    39,    97,    31,    29,    33,    31,    17,
      46,    47,    48,    49,     5,     6,     7,     8,    30,    32,
     190,    29,     8,    31,    37,    37,    17,   120,    23,    24,
       5,     6,     7,     8,    42,    43,    44,    12,    29,    30,
      31,    42,    17,    44,     8,   182,   185,    22,    23,    24,
      25,    26,    43,    32,    29,    32,    31,     5,     6,     7,
       8,    40,    29,    40,    12,    40,    33,   160,    43,    17,
       5,     6,     7,     8,    22,    23,    24,    12,    26,   208,
      32,    29,    17,    31,    40,    32,     8,    22,    40,    49,
      25,    26,    40,    40,    29,    43,    31,   190,    32,     5,
       6,     7,     8,    37,   233,    40,    12,     8,    43,    37,
      39,    17,    29,    41,   207,    18,    22,     9,    10,    25,
      26,    29,    32,    29,    51,    31,     5,     6,     7,     8,
      37,    40,    37,    12,    40,    32,    40,    43,    17,     5,
       6,     7,     8,    22,    37,     8,    25,    26,    38,    39,
      29,    17,    31,    37,    44,    45,    46,    47,    48,    49,
      30,    40,    39,    29,    43,    31,    30,    25,     8,     9,
      10,    35,    36,     8,    38,    39,    16,    43,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    35,    36,    40,    38,    39,    40,     8,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    35,    36,     8,    38,    39,    31,     6,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    35,    36,     6,    38,    39,     5,    31,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    35,    36,     5,    38,    39,    41,     8,    42,    43,
      44,    45,    46,    47,    48,    49,     8,    38,    39,    53,
      54,    42,    43,    44,    45,    46,    47,    48,    49,    41,
       8,    34,    39,    30,    19,    25,    40,    37,    37,    34,
      34,    28,    40,     8,    20,    33,    30,    40,     8,    40,
      29,    34,   244,    49,    40,   106,     1,   203,    15,    96,
     231,   108,   108,   196,   194,   241,   128,    -1,   132
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    56,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    31,    40,    43,
      57,    58,    60,    64,    65,    66,    72,    73,    74,    77,
      78,    79,    80,    81,    84,    87,    88,    91,    92,   101,
     102,   103,   104,   113,   114,    29,    38,     8,     8,     8,
       8,     8,     8,     8,    80,    40,     8,    81,    80,     5,
       6,    31,   105,   106,   107,   108,   109,   110,   111,   112,
      80,    71,    35,    36,    38,    39,    40,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
       8,    42,    44,    80,    89,    90,    37,    41,    49,    80,
      82,    83,     8,     8,    61,    62,    63,    93,    39,     8,
       9,    10,    16,    94,    95,    96,   100,    71,    29,    18,
      29,    71,    51,    30,   107,   111,    32,    37,    40,    37,
      32,    37,    40,    37,    25,    72,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    33,     8,    33,     8,    92,    83,    30,    30,
      37,    39,    25,    62,    40,    95,    96,    97,    31,    33,
      25,     8,    67,    69,    70,     8,    83,    75,    76,   103,
      32,    32,    31,   108,     6,    31,   112,     5,     8,     8,
      29,    40,    80,    94,    41,    41,    29,    31,     8,    59,
      34,    39,    30,    37,    19,    85,    25,    23,    24,    40,
     107,   111,    34,    34,    83,   102,   103,   100,     5,     8,
      44,    98,    99,    32,    37,    40,    94,    28,    68,    69,
       8,    20,    86,    80,    71,    32,    32,    40,    40,    30,
      32,    37,    33,     8,    29,    90,    71,    49,    99,    34,
      70,    40,    30
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    55,    56,    56,    57,    57,    57,    57,    57,    57,
      58,    58,    59,    59,    60,    61,    61,    62,    62,    63,
      64,    65,    66,    67,    67,    68,    68,    69,    70,    70,
      71,    71,    72,    72,    72,    72,    72,    73,    73,    74,
      75,    75,    76,    76,    77,    78,    79,    79,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    81,    82,    82,    83,    83,    84,    85,
      85,    86,    86,    87,    88,    88,    89,    89,    89,    90,
      90,    91,    91,    92,    92,    92,    93,    94,    94,    95,
      96,    96,    96,    97,    97,    98,    98,    99,    99,    99,
     100,   100,   101,   101,   102,   103,   103,   103,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   109,
     110,   110,   111,   111,   112,   112,   113,   113,   114
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       6,     9,     1,     3,     3,     0,     2,     0,     2,     2,
       4,     3,     6,     0,     1,     0,     4,     3,     1,     3,
       0,     2,     1,     1,     1,     1,     1,     7,     7,     5,
       0,     3,     0,     4,     2,     4,     1,     2,     1,     1,
       1,     2,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     1,     4,     0,     1,     1,     3,     6,     0,
       2,     0,     2,     2,     0,     2,     2,     4,     4,     1,
       1,     1,     3,     1,     4,     3,     3,     1,     1,     1,
       1,     5,     6,     0,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     2,     1,     3,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       3,     5,     1,     3,     1,     3,     1,     1,     5
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
  "$accept", "program", "program_element", "extern", "endpoints", "struct",
  "struct_decl_block", "struct_decl_list", "field_decl", "procedure",
  "function", "function_header", "arguments", "results", "formal_decl",
  "formal_list", "stmt_block", "stmt", "const_stmt", "if_stmt",
  "else_clauses", "elif_clauses", "return_stmt", "assign_stmt",
  "expr_stmt", "expr", "call_expr", "actual_list", "expr_list", "map_expr",
  "with", "reduce", "index_expr", "reduction_indices", "reduction_index",
  "reduction_op", "lhs_expr_list", "lhs_expr", "var_decl", "type",
  "element_type", "tensor_type", "nested_dimensions", "dimensions",
  "dimension", "component_type", "literal", "element_literal",
  "tensor_literal", "dense_tensor_literal", "float_dense_tensor_literal",
  "float_dense_ndtensor_literal", "float_dense_matrix_literal",
  "float_dense_vector_literal", "int_dense_tensor_literal",
  "int_dense_ndtensor_literal", "int_dense_matrix_literal",
  "int_dense_vector_literal", "scalar_literal", "test", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   281,   281,   283,   287,   290,   293,   302,   305,   309,
     315,   319,   325,   328,   335,   339,   341,   343,   345,   348,
     358,   365,   374,   411,   414,   425,   429,   440,   447,   451,
     458,   461,   470,   471,   472,   473,   474,   478,   503,   511,
     517,   519,   523,   525,   532,   538,   581,   584,   597,   615,
     616,   619,   627,   638,   650,   661,   673,   715,   720,   725,
     754,   759,   764,   769,   774,   779,   784,   789,   793,   798,
     803,   806,   809,   818,   827,   830,   836,   842,   851,   858,
     860,   864,   866,   871,   873,   875,   878,   881,   884,   890,
     891,   913,   918,   926,   932,   937,   946,   973,   976,   981,
     988,   991,   995,  1002,  1005,  1043,  1048,  1055,  1058,  1062,
    1067,  1070,  1139,  1140,  1142,  1146,  1147,  1151,  1154,  1162,
    1172,  1179,  1182,  1186,  1199,  1203,  1217,  1221,  1227,  1234,
    1237,  1241,  1254,  1258,  1272,  1276,  1282,  1287,  1297
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



