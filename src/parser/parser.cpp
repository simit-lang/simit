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
  #include <algorithm>

  #include "scanner.h"
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
    ctx->errors.push_back(Error(loc.begin.line, loc.begin.column,
                                loc.end.line, loc.end.column,
                                msg));
  }

  #undef yylex
  #define yylex scanner->lex

  static inline std::string convertAndFree(const char *str) {
    std::string result = std::string(str);
    free((void*)str);
    return result;
  }




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
   Parser :: Parser  (Scanner *scanner_yyarg, ParserParams *ctx_yyarg)
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

      case 55: // program_element


        { delete (yysym.value.IRNode); }

        break;

      case 62: // procedure


        { delete (yysym.value.Function); }

        break;

      case 63: // function


        { delete (yysym.value.Function); }

        break;

      case 64: // function_header


        { delete (yysym.value.Function); }

        break;

      case 65: // arguments


        { delete (yysym.value.Arguments); }

        break;

      case 66: // results


        { delete (yysym.value.Results); }

        break;

      case 67: // formal_decl


        { delete (yysym.value.Formal); }

        break;

      case 68: // formal_list


        { delete (yysym.value.Formals); }

        break;

      case 69: // stmt_block


        { delete (yysym.value.IRNodes); }

        break;

      case 70: // stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 71: // const_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 72: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 75: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 76: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 77: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 78: // expr


        { delete (yysym.value.Tensor); }

        break;

      case 79: // call_expr


        { delete (yysym.value.Call); }

        break;

      case 80: // actual_list


        { delete (yysym.value.TensorList); }

        break;

      case 81: // expr_list


        { delete (yysym.value.TensorList); }

        break;

      case 89: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 90: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 92: // type


        { delete (yysym.value.TensorType); }

        break;

      case 93: // element_type


        { delete (yysym.value.TensorType); }

        break;

      case 94: // tensor_type


        { delete (yysym.value.TensorType); }

        break;

      case 95: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 96: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 97: // dimension


        { delete (yysym.value.IndexSet); }

        break;

      case 98: // component_type


        {}

        break;

      case 99: // literal


        { delete (yysym.value.Tensor); }

        break;

      case 101: // tensor_literal


        { delete (yysym.value.Literal); }

        break;

      case 102: // dense_tensor_literal


        { delete (yysym.value.Literal); }

        break;

      case 103: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 104: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 105: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 106: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 107: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 108: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 109: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 110: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 111: // scalar_literal


        { delete (yysym.value.Literal); }

        break;

      case 112: // test


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
    if (ctx->functions.find(name) != ctx->functions.end()) {
      REPORT_ERROR("function redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->functions[name] = function.release();
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
    ctx->tests.push_back((yystack_[0].value.Test));
  }

    break;

  case 10:

    {
    free((void*)(yystack_[4].value.string));
    delete (yystack_[3].value.TensorType);
  }

    break;

  case 11:

    {
    free((void*)(yystack_[7].value.string));
    delete (yystack_[6].value.TensorType);
  }

    break;

  case 12:

    {
    free((void*)(yystack_[0].value.string));
  }

    break;

  case 13:

    {
    free((void*)(yystack_[0].value.string));
  }

    break;

  case 14:

    {
    free((void*)(yystack_[1].value.string));
  }

    break;

  case 20:

    {
    free((void*)(yystack_[2].value.string));
    delete (yystack_[1].value.IRNodes);
  }

    break;

  case 21:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[1].value.IRNodes));
    (yylhs.value.Function) = (yystack_[2].value.Function);
    (yylhs.value.Function)->addStatements(*statements);
    ctx->symtable.unscope();
  }

    break;

  case 22:

    {
    string ident((yystack_[4].value.string));
    free((void*)(yystack_[4].value.string));
    auto arguments = unique_ptr<vector<shared_ptr<Argument>>>((yystack_[2].value.Arguments));
    auto results = unique_ptr<vector<shared_ptr<Result>>>((yystack_[0].value.Results));

    (yylhs.value.Function) = new Function(ident, *arguments, *results);

    ctx->symtable.scope();
    for (auto argument : *arguments) {
      ctx->symtable.insert(argument->getName(), argument);
    }

    for (auto result : *results) {
      ctx->symtable.insert(result->getName(), result);
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
    string ident((yystack_[2].value.string));
    free((void*)(yystack_[2].value.string));
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
    auto tensorType = unique_ptr<TensorType>((yystack_[3].value.TensorType));

    auto tensorLiteral = shared_ptr<Literal>(*(yystack_[1].value.Literal));
    delete (yystack_[1].value.Literal);

    tensorLiteral->setName((yystack_[5].value.string));
    free((void*)(yystack_[5].value.string));

    // If $type is a 1xn matrix and $tensor_literal is a vector then we cast
    // $tensor_literal to a 1xn matrix.
    if (tensorType->getOrder() == 2 && tensorLiteral->getOrder() == 1) {
      tensorLiteral->cast(tensorType.release());
    }

    // Typecheck: value and literal types must be equivalent.
    //            Note that the use of $tensor_type is deliberate as tensorType
    //            can have been released.
    if (*(yystack_[3].value.TensorType) != *(tensorLiteral->getType())) {
      stringstream ss;
      ss << "attempting to assign to a variable of type " << *(yystack_[3].value.TensorType)
         << " a literal of type " << *(tensorLiteral->getType());
      REPORT_ERROR(ss.str(), yystack_[2].location);
    }

    ctx->symtable.insert(tensorLiteral->getName(), tensorLiteral);

    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
    (yylhs.value.IRNodes)->push_back(tensorLiteral);
  }

    break;

  case 38:

    {
    free((void*)(yystack_[5].value.string));
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
      auto lhs = *lhsIter;
      auto rhs = *rhsIter;

      // TODO: Remove these checks
      if (rhs == NULL) continue;
      if (!ctx->symtable.contains(lhs->name)) continue;

      auto lhsTensor = ctx->symtable.get(lhs->name);
      if (auto result = dynamic_pointer_cast<Result>(lhsTensor)) {
        rhs->setName(result->getName() + "_val");
        auto store = new VariableStore(result, rhs);
        auto storePtr = std::shared_ptr<Store>(store);
        result->setValue(storePtr);
        (yylhs.value.IRNodes)->push_back(storePtr);
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
    string ident((yystack_[0].value.string));
    free((void*)(yystack_[0].value.string));
    if (!ctx->symtable.contains(ident)) {
      // TODO: reintroduce error
      // REPORT_ERROR(ident + " is not defined in scope", @1);
      (yylhs.value.Tensor) = NULL;
      break;
    }

    std::shared_ptr<IRNode> &node = ctx->symtable.get(ident);

    shared_ptr<TensorNode> tensor = dynamic_pointer_cast<TensorNode>(node);
    if (tensor == NULL) {
      REPORT_ERROR(ident + " is not a tensor", yystack_[0].location);
    }

    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(tensor);
  }

    break;

  case 49:

    {
    (yylhs.value.Tensor) = (yystack_[0].value.Tensor);
  }

    break;

  case 50:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 51:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 52:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 53:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 54:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 55:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 56:

    {
    if ((yystack_[0].value.Tensor) == NULL) {  // TODO: Remove check
      (yylhs.value.Tensor) = NULL;
      break;
    }

    auto expr = shared_ptr<TensorNode>(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);

    IndexVarFactory indexVarFactory;
    std::vector<IndexExpr::IndexVarPtr> indexVars;
    for (unsigned int i=0; i<expr->getOrder(); ++i) {
      IndexSetProduct indexSet = expr->getType()->getDimensions()[0];
      indexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
    }

    std::vector<IndexExpr::IndexedTensor> operands;
    operands.push_back(IndexExpr::IndexedTensor(expr, indexVars));

    auto indexExpr = new IndexExpr(indexVars, IndexExpr::NEG, operands);

    assert(indexExpr != NULL);
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(indexExpr);
  }

    break;

  case 57:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[1].value.Tensor);
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
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
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
    delete (yystack_[1].value.Tensor);
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
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 68:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 69:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 70:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 71:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.TensorList));
    delete (yystack_[1].value.TensorList);
    (yylhs.value.Call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 72:

    {
    (yylhs.value.TensorList) = new vector<shared_ptr<TensorNode>>();
  }

    break;

  case 73:

    {
    (yylhs.value.TensorList) = (yystack_[0].value.TensorList);
  }

    break;

  case 74:

    {
    (yylhs.value.TensorList) = new std::vector<std::shared_ptr<TensorNode>>();
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 75:

    {
    (yylhs.value.TensorList) = (yystack_[2].value.TensorList);
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 76:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
  }

    break;

  case 78:

    {
    string neighbor((yystack_[0].value.string));
    free((void*)(yystack_[0].value.string));
  }

    break;

  case 84:

    {
    free((void*)(yystack_[0].value.string));
  }

    break;

  case 85:

    {
    free((void*)(yystack_[1].value.string));
  }

    break;

  case 86:

    {
    free((void*)(yystack_[3].value.string));
    free((void*)(yystack_[1].value.string));
  }

    break;

  case 89:

    {
    (yylhs.value.VarAccesses) = new VariableAccessVector();
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 90:

    {
    (yylhs.value.VarAccesses) = (yystack_[2].value.VarAccesses);
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 91:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto variableStore = new VariableAccess;
    variableStore->name = name;
    (yylhs.value.VarAccess) = variableStore;
  }

    break;

  case 92:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.VarAccess) = NULL;
    delete (yystack_[1].value.TensorList);
  }

    break;

  case 93:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    std::string field = convertAndFree((yystack_[0].value.string));
    (yylhs.value.VarAccess) = NULL;
  }

    break;

  case 94:

    {
    delete (yystack_[0].value.TensorType);
    free((void*)(yystack_[2].value.string));
  }

    break;

  case 95:

    {
    (yylhs.value.TensorType) = (yystack_[0].value.TensorType);
  }

    break;

  case 96:

    {
    (yylhs.value.TensorType) = (yystack_[0].value.TensorType);
  }

    break;

  case 97:

    {
    free((void*)(yystack_[0].value.string));
    (yylhs.value.TensorType) = new TensorType(Type::ELEMENT);
  }

    break;

  case 98:

    {
    (yylhs.value.TensorType) = new TensorType((yystack_[0].value.Type));
  }

    break;

  case 99:

    {
    (yylhs.value.TensorType) = new TensorType((yystack_[1].value.Type), *(yystack_[3].value.IndexSetProducts));
    delete (yystack_[3].value.IndexSetProducts);
  }

    break;

  case 100:

    {
    (yylhs.value.IndexSetProducts) = new std::vector<IndexSetProduct>();
  }

    break;

  case 101:

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
        size_t numNestings = (*parentDims)[0].getIndexSets().size();
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

  case 102:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.IndexSet));
    delete (yystack_[0].value.IndexSet);
  }

    break;

  case 103:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.IndexSet));
    delete (yystack_[0].value.IndexSet);
  }

    break;

  case 104:

    {
    (yylhs.value.IndexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 105:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.IndexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 106:

    {
    (yylhs.value.IndexSet) = new IndexSet();
  }

    break;

  case 107:

    {
    (yylhs.value.Type) = Type::INT;
  }

    break;

  case 108:

    {
    (yylhs.value.Type) = Type::FLOAT;
  }

    break;

  case 114:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::FLOAT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 115:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::INT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 116:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 118:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 119:

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

  case 120:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 121:

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

  case 122:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 123:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 124:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 126:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 127:

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

  case 128:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 129:

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

  case 130:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 131:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 132:

    {
    auto scalarType = new TensorType(Type::INT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.num));
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 133:

    {
    auto scalarType = new TensorType(Type::FLOAT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.fnum));
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 134:

    {
    (yylhs.value.Test) = new Test(*(yystack_[3].value.Call), *(yystack_[1].value.Literal));
    delete (yystack_[3].value.Call);
    delete (yystack_[1].value.Literal);
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


  const short int  Parser ::yypact_ninf_ = -148;

  const signed char  Parser ::yytable_ninf_ = -92;

  const short int
   Parser ::yypact_[] =
  {
    -148,    13,  -148,  -148,  -148,  -148,    17,    40,    61,    73,
      88,    97,    99,   153,   128,   120,   153,    10,  -148,   153,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,   270,  -148,  -148,  -148,    70,   103,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,   250,   147,     9,   134,   115,
    -148,   149,   167,   158,   226,  -148,   158,   141,   252,  -148,
    -148,   200,   161,   160,   176,   162,   175,   180,   179,   183,
      91,   169,   153,   153,   153,   153,  -148,   153,   153,   153,
     153,   153,  -148,   153,   153,   153,   153,   153,   159,  -148,
    -148,   226,  -148,    14,   215,   153,   226,   194,    89,  -148,
     193,  -148,   201,   225,   197,   115,  -148,  -148,  -148,  -148,
     -21,  -148,  -148,  -148,   196,   230,   233,   250,   110,    32,
    -148,    11,    54,  -148,   203,   236,   237,  -148,   213,   241,
     242,  -148,  -148,    46,    46,   226,   226,    64,    64,    91,
      91,    91,   288,   306,   306,    46,    46,   243,  -148,   245,
       7,  -148,    43,  -148,  -148,   153,   115,  -148,  -148,  -148,
     209,   218,     2,   275,   251,  -148,   221,   254,  -148,   249,
     273,   274,   264,   187,   267,  -148,  -148,   236,   162,  -148,
     241,   183,  -148,   291,   294,   153,  -148,   226,  -148,  -148,
      32,   205,     6,  -148,    20,   289,   115,   315,   230,   338,
     327,  -148,   153,  -148,  -148,    66,   111,  -148,  -148,    89,
     314,   316,   325,  -148,  -148,  -148,    63,  -148,   326,   352,
    -148,  -148,   332,  -148,  -148,  -148,   123,  -148,   226,   223,
    -148,  -148,  -148,  -148,  -148,  -148,     6,   328,  -148,   230,
    -148,   140,  -148,   323,   119,  -148,  -148
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    82,     1,   132,   133,    50,    48,     0,     0,     0,
       0,     0,     0,    82,     0,     0,    82,     0,    46,    82,
       3,     7,     4,     5,     6,    30,     8,    32,    33,    34,
      35,    36,     0,    68,    70,    69,    82,     0,    89,    49,
     109,   110,   112,   113,     9,    82,     0,    15,     0,     0,
      30,     0,     0,    48,    30,    44,     0,     0,     0,   130,
     122,     0,     0,   117,   116,   120,     0,   125,   124,   128,
      56,    82,    82,    82,    82,    82,    47,    82,    82,    82,
      82,    82,    57,    82,    82,    82,    82,    82,    48,    87,
      88,    81,    83,     0,     0,    82,    74,     0,     0,    93,
       0,    14,     0,    17,     0,     0,    97,   107,   108,   100,
       0,    95,    96,    98,    82,    23,     0,    82,    82,     0,
      65,     0,     0,   114,     0,     0,     0,   115,     0,     0,
       0,    21,    31,    61,    62,    67,    66,    52,    53,    54,
      55,    58,    51,    59,    60,    63,    64,     0,    84,     0,
      91,    90,     0,    71,    92,    82,     0,    16,    18,    19,
       0,     0,     0,     0,     0,    20,     0,     0,    28,    24,
      77,    73,     0,     0,     0,   118,   126,     0,   121,   123,
       0,   129,   131,     0,     0,    82,    45,    75,    94,   111,
       0,     0,     0,    12,     0,     0,     0,    25,     0,     0,
      79,    39,    82,    30,   134,     0,     0,    86,    85,     0,
       0,     0,     0,   104,   105,   106,     0,   102,     0,     0,
      10,    27,     0,    22,    29,    78,     0,    76,    30,    82,
     119,   127,    38,    37,    99,   101,     0,     0,    13,     0,
      80,    82,   103,     0,     0,    11,    26
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -148,  -148,  -148,  -148,  -148,  -148,  -148,   261,  -148,  -148,
    -148,  -148,  -148,  -148,   168,   126,   -49,   366,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,   -13,   353,  -148,   -88,  -148,
    -148,  -148,  -148,  -148,  -148,   143,  -148,   276,  -148,  -147,
     266,   268,  -148,  -148,   136,   184,  -148,   185,  -111,  -148,
    -148,  -148,   -57,   253,  -148,  -148,   -59,   247,  -148,  -148
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    20,    21,   194,    22,   101,   102,   103,    23,
      24,    25,   167,   223,   168,   169,    71,   132,    27,    28,
     172,   173,    29,    30,    31,    32,    33,    97,    98,    34,
     200,   227,    35,    36,    92,    93,    37,    38,   104,   110,
     111,   112,   162,   216,   217,   113,    39,    40,    41,    42,
      62,    63,    64,    65,    66,    67,    68,    69,    43,    44
  };

  const short int
   Parser ::yytable_[] =
  {
      54,   114,   122,    58,   121,   118,    70,   152,   174,   188,
     163,   213,   164,     2,   214,    59,    60,   100,     3,     4,
       5,     6,   148,    91,     7,     8,     9,    10,    11,   171,
      12,   191,    96,   192,   -17,    13,   185,     3,     4,    14,
      15,    61,    16,   175,    17,    46,    45,   149,    47,   221,
     215,   125,   218,    18,   -91,    46,    19,   219,   -91,   133,
     134,   135,   136,    17,   137,   138,   139,   140,   141,    48,
     142,   143,   144,   145,   146,     3,     4,     5,    88,   211,
     155,    49,    96,   186,    74,    75,   176,    12,    77,    78,
      79,    80,    81,    82,   129,   235,    50,   209,   230,    16,
     236,    17,    74,    75,    96,    51,   125,    52,    79,    80,
      81,    82,    89,    19,    90,     3,     4,     5,     6,   154,
     205,   206,     8,   106,   107,   108,   155,    12,    56,    74,
      75,   109,    13,   -42,   -42,   -40,    14,    81,    82,    16,
      94,    17,   187,   231,    95,     3,     4,     5,     6,   246,
      18,   129,     8,    19,   229,    99,   198,    12,     3,     4,
       5,    53,    13,   -43,   -43,    89,    14,    90,    55,    16,
      12,    17,    96,   105,     3,     4,     5,     6,   115,   241,
      18,     8,    16,    19,    17,   116,    12,   117,   117,   228,
     119,    13,   147,   123,   131,    14,    19,   124,    16,   126,
      17,     3,     4,     5,     6,    59,    60,   127,     8,    18,
     202,   203,    19,    12,   107,   108,   125,   128,    13,   129,
     130,   165,    14,   150,   153,    16,   157,    17,     3,     4,
       5,     6,   156,   100,   177,     8,    18,   159,   166,    19,
      12,   170,    60,   179,   180,    13,    59,   182,   -41,    14,
     189,   183,    16,   184,    17,     3,     4,     5,    53,   190,
     196,    72,    73,    18,    74,    75,    19,    12,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    16,
     -72,    17,   120,   193,   197,   195,   198,    72,    73,   201,
      74,    75,   199,    19,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    72,    73,   204,    74,    75,
      76,   155,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    72,    73,   207,    74,    75,   208,   220,
      77,    78,    79,    80,    81,    82,   -92,    84,    85,    86,
      87,    72,    73,   222,    74,    75,   225,   226,    77,    78,
      79,    80,    81,    82,   232,   234,   233,    86,    87,   237,
     238,   239,   243,   245,   158,   244,   224,    26,    57,   240,
     151,   160,   242,   161,   210,   212,   181,     0,   178
  };

  const short int
   Parser ::yycheck_[] =
  {
      13,    50,    61,    16,    61,    54,    19,    95,   119,   156,
      31,     5,    33,     0,     8,     5,     6,     8,     5,     6,
       7,     8,     8,    36,    11,    12,    13,    14,    15,   117,
      17,    29,    45,    31,    25,    22,    29,     5,     6,    26,
      27,    31,    29,    32,    31,    38,    29,    33,     8,   196,
      44,    40,    32,    40,    37,    38,    43,    37,    41,    72,
      73,    74,    75,    31,    77,    78,    79,    80,    81,     8,
      83,    84,    85,    86,    87,     5,     6,     7,     8,   190,
      37,     8,    95,    40,    38,    39,    32,    17,    42,    43,
      44,    45,    46,    47,    40,    32,     8,   185,    32,    29,
      37,    31,    38,    39,   117,     8,    40,     8,    44,    45,
      46,    47,    42,    43,    44,     5,     6,     7,     8,    30,
     177,   180,    12,     8,     9,    10,    37,    17,     8,    38,
      39,    16,    22,    23,    24,    25,    26,    46,    47,    29,
      37,    31,   155,    32,    41,     5,     6,     7,     8,    30,
      40,    40,    12,    43,   203,     8,    37,    17,     5,     6,
       7,     8,    22,    23,    24,    42,    26,    44,    40,    29,
      17,    31,   185,    39,     5,     6,     7,     8,    29,   228,
      40,    12,    29,    43,    31,    18,    17,    29,    29,   202,
      49,    22,    33,    32,    25,    26,    43,    37,    29,    37,
      31,     5,     6,     7,     8,     5,     6,    32,    12,    40,
      23,    24,    43,    17,     9,    10,    40,    37,    22,    40,
      37,    25,    26,     8,    30,    29,    25,    31,     5,     6,
       7,     8,    39,     8,    31,    12,    40,    40,     8,    43,
      17,     8,     6,     6,    31,    22,     5,     5,    25,    26,
      41,     8,    29,     8,    31,     5,     6,     7,     8,    41,
      39,    35,    36,    40,    38,    39,    43,    17,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    29,
      30,    31,    30,     8,    30,    34,    37,    35,    36,    25,
      38,    39,    19,    43,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    35,    36,    40,    38,    39,
      40,    37,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    35,    36,    34,    38,    39,    34,    40,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    35,    36,    28,    38,    39,     8,    20,    42,    43,
      44,    45,    46,    47,    40,    30,    40,    51,    52,    33,
       8,    29,    34,    40,   103,   239,   198,     1,    15,   226,
      94,   105,   236,   105,   189,   191,   129,    -1,   125
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    54,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    31,    40,    43,
      55,    56,    58,    62,    63,    64,    70,    71,    72,    75,
      76,    77,    78,    79,    82,    85,    86,    89,    90,    99,
     100,   101,   102,   111,   112,    29,    38,     8,     8,     8,
       8,     8,     8,     8,    78,    40,     8,    79,    78,     5,
       6,    31,   103,   104,   105,   106,   107,   108,   109,   110,
      78,    69,    35,    36,    38,    39,    40,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,     8,    42,
      44,    78,    87,    88,    37,    41,    78,    80,    81,     8,
       8,    59,    60,    61,    91,    39,     8,     9,    10,    16,
      92,    93,    94,    98,    69,    29,    18,    29,    69,    49,
      30,   105,   109,    32,    37,    40,    37,    32,    37,    40,
      37,    25,    70,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    33,     8,    33,
       8,    90,    81,    30,    30,    37,    39,    25,    60,    40,
      93,    94,    95,    31,    33,    25,     8,    65,    67,    68,
       8,    81,    73,    74,   101,    32,    32,    31,   106,     6,
      31,   110,     5,     8,     8,    29,    40,    78,    92,    41,
      41,    29,    31,     8,    57,    34,    39,    30,    37,    19,
      83,    25,    23,    24,    40,   105,   109,    34,    34,    81,
     100,   101,    98,     5,     8,    44,    96,    97,    32,    37,
      40,    92,    28,    66,    67,     8,    20,    84,    78,    69,
      32,    32,    40,    40,    30,    32,    37,    33,     8,    29,
      88,    69,    97,    34,    68,    40,    30
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    53,    54,    54,    55,    55,    55,    55,    55,    55,
      56,    56,    57,    57,    58,    59,    59,    60,    60,    61,
      62,    63,    64,    65,    65,    66,    66,    67,    68,    68,
      69,    69,    70,    70,    70,    70,    70,    71,    71,    72,
      73,    73,    74,    74,    75,    76,    77,    77,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    79,    80,    80,    81,    81,    82,    83,    83,    84,
      84,    85,    86,    86,    87,    87,    87,    88,    88,    89,
      89,    90,    90,    90,    91,    92,    92,    93,    94,    94,
      95,    95,    96,    96,    97,    97,    97,    98,    98,    99,
      99,   100,   101,   101,   102,   102,   103,   103,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   109,
     110,   110,   111,   111,   112
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       6,     9,     1,     3,     3,     0,     2,     0,     2,     2,
       4,     3,     6,     0,     1,     0,     4,     3,     1,     3,
       0,     2,     1,     1,     1,     1,     1,     7,     7,     5,
       0,     3,     0,     4,     2,     4,     1,     2,     1,     1,
       1,     3,     3,     3,     3,     3,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     4,     0,     1,     1,     3,     6,     0,     2,     0,
       2,     2,     0,     2,     2,     4,     4,     1,     1,     1,
       3,     1,     4,     3,     3,     1,     1,     1,     1,     5,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     3,     3,     1,     1,     3,     5,
       1,     3,     1,     3,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     5
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
  "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"^\"", "\"'\"",
  "\"\\\\\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"", "$accept", "program",
  "program_element", "extern", "endpoints", "struct", "struct_decl_block",
  "struct_decl_list", "field_decl", "procedure", "function",
  "function_header", "arguments", "results", "formal_decl", "formal_list",
  "stmt_block", "stmt", "const_stmt", "if_stmt", "else_clauses",
  "elif_clauses", "return_stmt", "assign_stmt", "expr_stmt", "expr",
  "call_expr", "actual_list", "expr_list", "map_expr", "with", "reduce",
  "index_expr", "reduction_indices", "reduction_index", "reduction_op",
  "lhs_expr_list", "lhs_expr", "var_decl", "type", "element_type",
  "tensor_type", "nested_dimensions", "dimensions", "dimension",
  "component_type", "literal", "element_literal", "tensor_literal",
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
       0,   171,   171,   173,   177,   180,   183,   192,   195,   199,
     205,   209,   215,   218,   225,   229,   231,   233,   235,   238,
     248,   255,   264,   302,   305,   316,   320,   331,   339,   343,
     350,   353,   362,   363,   364,   365,   366,   370,   400,   408,
     414,   416,   420,   422,   429,   435,   476,   479,   493,   512,
     515,   518,   523,   528,   533,   538,   543,   567,   571,   576,
     581,   586,   591,   596,   601,   606,   610,   615,   620,   623,
     626,   635,   644,   647,   653,   659,   668,   675,   677,   682,
     684,   689,   691,   693,   696,   699,   702,   708,   709,   731,
     736,   744,   750,   755,   764,   792,   795,   800,   806,   809,
     815,   818,   856,   861,   868,   871,   875,   880,   883,   952,
     953,   955,   959,   960,   963,   971,   981,   988,   991,   995,
    1008,  1012,  1026,  1030,  1036,  1043,  1046,  1050,  1063,  1067,
    1081,  1085,  1091,  1096,  1106
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
      45,    46,    47,    48,    49,    50,    51,    52
    };
    const unsigned int user_token_number_max_ = 307;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



