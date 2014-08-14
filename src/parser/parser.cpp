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
      error(loc, msg);            \
      YYERROR;                    \
    } while (0)

  void Parser::error(const Parser::location_type &loc, const std::string &msg) {
    ctx->errors.push_back(Error(loc.begin.line, loc.begin.column,
                                loc.end.line, loc.end.column,
                                msg));
  }

  #undef yylex
  #define yylex scanner->lex

  inline std::string convertAndFree(const char *str) {
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

      case 80: // expr_list


        { delete (yysym.value.TensorList); }

        break;

      case 88: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 89: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 91: // type


        { delete (yysym.value.TensorType); }

        break;

      case 92: // element_type


        { delete (yysym.value.TensorType); }

        break;

      case 93: // tensor_type


        { delete (yysym.value.TensorType); }

        break;

      case 94: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 95: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 96: // dimension


        { delete (yysym.value.IndexSet); }

        break;

      case 97: // component_type


        {}

        break;

      case 99: // tensor_literal


        { delete (yysym.value.Literal); }

        break;

      case 100: // dense_tensor_literal


        { delete (yysym.value.Literal); }

        break;

      case 101: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 102: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 103: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 104: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 105: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 106: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 107: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 108: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 109: // scalar_literal


        { delete (yysym.value.Literal); }

        break;

      case 110: // test


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
    ctx->functions.push_back((yystack_[0].value.Function));
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
      ctx->symtable[argument->getName()] = argument;
    }

    for (auto result : *results) {
      ctx->symtable[result->getName()] = result;
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

    ctx->symtable[tensorLiteral->getName()] = tensorLiteral;

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

      if (rhs == NULL) continue;  // TODO: Remove this

      auto lhsTensor = ctx->symtable[lhs->name];
      if (lhsTensor == NULL) {
//        NOT_SUPPORTED_YET;
      }
      else {
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
    std::shared_ptr<IRNode> &node = ctx->symtable[ident];
    if (node == NULL) {
      // TODO: reintroduce error
      // REPORT_ERROR(ident + " is not defined in scope", @1);
      (yylhs.value.Tensor) = NULL;
      break;
    }

    shared_ptr<TensorNode> tensor = dynamic_pointer_cast<TensorNode>(node);
    if (tensor == NULL) {
      REPORT_ERROR(ident + " is not a tensor", yystack_[0].location);
    }

    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(tensor);
  }

    break;

  case 49:

    {
    (yylhs.value.Tensor) = NULL;
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
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 57:

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

  case 58:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[1].value.Tensor);
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
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 66:

    {
    (yylhs.value.Tensor) = NULL;
    delete (yystack_[1].value.Tensor);
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
    delete (yystack_[2].value.Tensor);
    delete (yystack_[0].value.Tensor);
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
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 72:

    {
    delete (yystack_[2].value.Tensor);
    std::vector<std::shared_ptr<TensorNode>> args;
    auto call = new Call(NULL, args);  // TODO: Fix
    (yylhs.value.Call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 73:

    {
    delete (yystack_[3].value.Tensor);
    delete (yystack_[1].value.TensorList);
    std::vector<std::shared_ptr<TensorNode>> args;
    auto call = new Call(NULL, args);  // TODO: Fix
    (yylhs.value.Call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 74:

    {
    (yylhs.value.TensorList) = new vector<shared_ptr<TensorNode>>();
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

  case 112:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::FLOAT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 113:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(Type::INT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 114:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 116:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 117:

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

  case 118:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 119:

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

  case 120:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 121:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 122:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 124:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 125:

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

  case 126:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 127:

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

  case 128:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 129:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 130:

    {
    auto scalarType = new TensorType(Type::INT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.num));
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 131:

    {
    auto scalarType = new TensorType(Type::FLOAT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.fnum));
    (yylhs.value.Literal) = new shared_ptr<Literal>(literal);
  }

    break;

  case 132:

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


  const short int  Parser ::yypact_ninf_ = -175;

  const signed char  Parser ::yytable_ninf_ = -92;

  const short int
   Parser ::yypact_[] =
  {
    -175,    67,  -175,  -175,  -175,  -175,    -5,    -6,    14,    21,
      33,    58,    84,   249,    48,   249,   249,  -175,   249,  -175,
    -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,  -175,
    -175,   251,  -175,  -175,  -175,   141,    49,  -175,  -175,   249,
     117,     5,    98,    60,  -175,   113,   136,  -175,   299,  -175,
     299,   103,   275,    77,   174,     9,   249,   249,   249,   249,
    -175,   249,   249,   249,   249,   249,  -175,   249,   249,   249,
     249,   249,   127,  -175,  -175,   299,  -175,     4,   156,   249,
     299,    71,  -175,   126,  -175,   142,   158,   128,    60,  -175,
    -175,  -175,  -175,    80,  -175,  -175,  -175,   199,   161,   163,
      92,    13,  -175,  -175,  -175,  -175,    75,   230,   230,   299,
     299,   358,   358,    77,    77,    77,   323,   347,   347,   230,
     230,   164,  -175,   166,    -4,  -175,    93,  -175,   249,    60,
    -175,  -175,  -175,   137,   146,   100,   180,   155,  -175,   153,
     160,  -175,   157,   176,   168,    79,  -175,  -175,    15,   162,
    -175,  -175,  -175,   167,   175,  -175,   299,  -175,  -175,    13,
     134,    -2,  -175,    -9,   170,    60,   169,   161,   190,   188,
    -175,   249,  -175,  -175,  -175,   151,   181,   178,   172,   182,
     186,   183,   187,   185,  -175,  -175,  -175,   193,   194,   196,
    -175,  -175,  -175,    90,  -175,   202,   215,  -175,  -175,   208,
    -175,  -175,  -175,   109,  -175,   299,   224,     3,    45,  -175,
     207,   234,   237,  -175,   213,   240,   242,  -175,  -175,  -175,
    -175,    -2,   214,  -175,   161,  -175,   133,  -175,  -175,   234,
     182,  -175,   240,   185,  -175,  -175,   211,    89,    51,    55,
    -175,  -175,  -175,  -175
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    82,     1,    50,    49,    51,    48,     0,     0,     0,
       0,     0,     0,    82,     0,    82,    82,    46,    82,     3,
       7,     4,     5,     6,    30,     8,    32,    33,    34,    35,
      36,     0,    69,    71,    70,     0,     0,    89,     9,    82,
       0,    15,     0,     0,    30,     0,     0,    48,    30,    44,
       0,    69,     0,    57,    82,    82,    82,    82,    82,    82,
      47,    82,    82,    82,    82,    82,    58,    82,    82,    82,
      82,    82,    48,    87,    88,    81,    83,     0,     0,    82,
      74,     0,    93,     0,    14,     0,    17,     0,     0,    97,
     107,   108,   100,     0,    95,    96,    98,    82,    23,     0,
      42,     0,    66,    21,    31,    72,     0,    62,    63,    68,
      67,    53,    54,    55,    56,    59,    52,    60,    61,    64,
      65,     0,    84,     0,    91,    90,     0,    92,    82,     0,
      16,    18,    19,     0,     0,     0,     0,     0,    20,     0,
       0,    28,    24,    77,     0,     0,   130,   131,     0,     0,
     110,   111,    73,     0,     0,    45,    75,    94,   109,     0,
       0,     0,    12,     0,     0,     0,    25,     0,     0,    79,
      39,    82,    30,   128,   120,     0,     0,   115,   114,   118,
       0,   123,   122,   126,   132,    86,    85,     0,     0,     0,
     104,   105,   106,     0,   102,     0,     0,    10,    27,     0,
      22,    29,    78,     0,    76,    30,    82,     0,     0,   112,
       0,     0,     0,   113,     0,     0,     0,    38,    37,    99,
     101,     0,     0,    13,     0,    80,    43,   116,   124,     0,
     119,   121,     0,   127,   129,   103,     0,     0,     0,     0,
      11,    26,   117,   125
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -175,  -175,  -175,  -175,  -175,  -175,  -175,   177,  -175,  -175,
    -175,  -175,  -175,  -175,    85,    34,   -44,   259,  -175,  -175,
    -175,  -175,  -175,  -175,  -175,    -8,   246,   -15,  -175,  -175,
    -175,  -175,  -175,  -175,    59,  -175,   192,  -175,  -120,   191,
     195,  -175,  -175,    44,   111,  -175,   122,  -175,  -175,  -175,
    -164,    73,  -175,  -175,  -174,    70,  -175,  -175
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    19,    20,   163,    21,    84,    85,    86,    22,
      23,    24,   140,   200,   141,   142,    54,   104,    26,    27,
     144,   145,    28,    29,    30,    31,    32,    81,    33,   169,
     204,    34,    35,    76,    77,    36,    37,    87,    93,    94,
      95,   135,   193,   194,    96,   187,   149,   150,   176,   177,
     178,   179,   180,   181,   182,   183,   151,    38
  };

  const short int
   Parser ::yytable_[] =
  {
      97,   208,    41,   190,   100,    48,   191,    50,    52,   157,
      53,   207,   122,    83,     3,     4,     5,    47,   146,   147,
     173,   174,    42,   195,    39,    39,    12,    75,   196,    43,
     -17,    80,   -91,    40,    40,   227,   -91,   123,    16,   105,
     106,    44,   192,   211,   148,   198,   175,    80,   107,   108,
     109,   110,    18,   111,   112,   113,   114,   115,   239,   116,
     117,   118,   119,   120,   126,   238,    45,     2,    89,    90,
      91,    80,     3,     4,     5,     6,    92,   228,     7,     8,
       9,    10,    11,   242,    12,   215,    78,   243,    49,    13,
      79,   211,    46,    14,    15,   215,    16,     3,     4,     5,
       6,   127,   171,   172,     8,   152,    55,    17,   128,    12,
      18,   136,   128,   137,    13,    58,    59,   -40,    14,   241,
     156,    16,   220,    65,    66,    82,   167,   221,   206,   160,
     128,   161,    17,   155,   -82,    18,   -82,    88,     3,     4,
       5,     6,    98,    90,    91,     8,     3,     4,     5,    72,
      12,    73,   101,    74,    99,    13,   173,   174,    12,    14,
     121,   226,    16,   205,   124,   129,    83,   130,   132,   139,
      16,   143,   153,    17,   154,   -82,    18,   -82,   158,     3,
       4,     5,     6,    73,    18,    74,     8,   159,   162,   164,
     166,    12,   165,   170,   167,   168,    13,   199,   202,   103,
      14,   185,   184,    16,     3,     4,     5,     6,   203,   186,
     197,     8,   211,   209,    17,   210,    12,    18,   213,   212,
     214,    13,   216,   223,   138,    14,   219,   215,    16,     3,
       4,     5,     6,   217,   218,   222,     8,   224,   229,    17,
     174,    12,    18,   231,   232,   173,    13,   234,   236,   -41,
      14,   240,   201,    16,     3,     4,     5,    47,   237,    55,
      25,    51,   225,   131,    17,   235,    12,    18,    58,    59,
     125,   189,    61,    62,    63,    64,    65,    66,    16,   133,
      55,   188,     0,   134,   230,   233,    56,    57,     0,    58,
      59,    60,    18,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    55,   102,     0,     0,     0,     0,
      56,    57,     0,    58,    59,     0,     0,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    55,     0,
       0,     0,     0,     0,    56,    57,     0,    58,    59,     0,
       0,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    55,     0,     0,     0,     0,     0,    56,    57,
       0,    58,    59,     0,     0,    61,    62,    63,    64,    65,
      66,   -92,    68,    69,    70,    71,    55,     0,     0,     0,
       0,     0,    56,    57,     0,    58,    59,    55,     0,    61,
      62,    63,    64,    65,    66,     0,    58,    59,    70,    71,
       0,     0,    63,    64,    65,    66
  };

  const short int
   Parser ::yycheck_[] =
  {
      44,   175,     8,     5,    48,    13,     8,    15,    16,   129,
      18,   175,     8,     8,     5,     6,     7,     8,     5,     6,
       5,     6,     8,    32,    29,    29,    17,    35,    37,     8,
      25,    39,    37,    38,    38,    32,    41,    33,    29,    30,
      55,     8,    44,    40,    31,   165,    31,    55,    56,    57,
      58,    59,    43,    61,    62,    63,    64,    65,   232,    67,
      68,    69,    70,    71,    79,   229,     8,     0,     8,     9,
      10,    79,     5,     6,     7,     8,    16,    32,    11,    12,
      13,    14,    15,    32,    17,    40,    37,    32,    40,    22,
      41,    40,     8,    26,    27,    40,    29,     5,     6,     7,
       8,    30,    23,    24,    12,    30,    29,    40,    37,    17,
      43,    31,    37,    33,    22,    38,    39,    25,    26,    30,
     128,    29,    32,    46,    47,     8,    37,    37,   172,    29,
      37,    31,    40,    40,    42,    43,    44,    39,     5,     6,
       7,     8,    29,     9,    10,    12,     5,     6,     7,     8,
      17,    42,    49,    44,    18,    22,     5,     6,    17,    26,
      33,   205,    29,   171,     8,    39,     8,    25,    40,     8,
      29,     8,     8,    40,     8,    42,    43,    44,    41,     5,
       6,     7,     8,    42,    43,    44,    12,    41,     8,    34,
      30,    17,    39,    25,    37,    19,    22,    28,     8,    25,
      26,    34,    40,    29,     5,     6,     7,     8,    20,    34,
      40,    12,    40,    32,    40,    37,    17,    43,    32,    37,
      37,    22,    37,     8,    25,    26,    30,    40,    29,     5,
       6,     7,     8,    40,    40,    33,    12,    29,    31,    40,
       6,    17,    43,     6,    31,     5,    22,     5,    34,    25,
      26,    40,   167,    29,     5,     6,     7,     8,   224,    29,
       1,    15,   203,    86,    40,   221,    17,    43,    38,    39,
      78,   160,    42,    43,    44,    45,    46,    47,    29,    88,
      29,   159,    -1,    88,   211,   215,    35,    36,    -1,    38,
      39,    40,    43,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    -1,    38,    39,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    29,    -1,
      -1,    -1,    -1,    -1,    35,    36,    -1,    38,    39,    -1,
      -1,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    29,    -1,    -1,    -1,    -1,    -1,    35,    36,
      -1,    38,    39,    -1,    -1,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    29,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    38,    39,    29,    -1,    42,
      43,    44,    45,    46,    47,    -1,    38,    39,    51,    52,
      -1,    -1,    44,    45,    46,    47
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    54,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    40,    43,    55,
      56,    58,    62,    63,    64,    70,    71,    72,    75,    76,
      77,    78,    79,    81,    84,    85,    88,    89,   110,    29,
      38,     8,     8,     8,     8,     8,     8,     8,    78,    40,
      78,    79,    78,    78,    69,    29,    35,    36,    38,    39,
      40,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,     8,    42,    44,    78,    86,    87,    37,    41,
      78,    80,     8,     8,    59,    60,    61,    90,    39,     8,
       9,    10,    16,    91,    92,    93,    97,    69,    29,    18,
      69,    49,    30,    25,    70,    30,    80,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    78,
      78,    33,     8,    33,     8,    89,    80,    30,    37,    39,
      25,    60,    40,    92,    93,    94,    31,    33,    25,     8,
      65,    67,    68,     8,    73,    74,     5,     6,    31,    99,
     100,   109,    30,     8,     8,    40,    78,    91,    41,    41,
      29,    31,     8,    57,    34,    39,    30,    37,    19,    82,
      25,    23,    24,     5,     6,    31,   101,   102,   103,   104,
     105,   106,   107,   108,    40,    34,    34,    98,    99,    97,
       5,     8,    44,    95,    96,    32,    37,    40,    91,    28,
      66,    67,     8,    20,    83,    78,    69,   103,   107,    32,
      37,    40,    37,    32,    37,    40,    37,    40,    40,    30,
      32,    37,    33,     8,    29,    87,    69,    32,    32,    31,
     104,     6,    31,   108,     5,    96,    34,    68,   103,   107,
      40,    30,    32,    32
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
      78,    78,    79,    79,    80,    80,    81,    82,    82,    83,
      83,    84,    85,    85,    86,    86,    86,    87,    87,    88,
      88,    89,    89,    89,    90,    91,    91,    92,    93,    93,
      94,    94,    95,    95,    96,    96,    96,    97,    97,    98,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     104,   104,   105,   105,   106,   106,   107,   107,   108,   108,
     109,   109,   110
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       6,     9,     1,     3,     3,     0,     2,     0,     2,     2,
       4,     3,     6,     0,     1,     0,     4,     3,     1,     3,
       0,     2,     1,     1,     1,     1,     1,     7,     7,     5,
       0,     3,     0,     4,     2,     4,     1,     2,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       1,     1,     3,     4,     1,     3,     6,     0,     2,     0,
       2,     2,     0,     2,     2,     4,     4,     1,     1,     1,
       3,     1,     4,     3,     3,     1,     1,     1,     1,     5,
       0,     4,     1,     3,     1,     1,     1,     1,     1,     0,
       1,     1,     3,     3,     1,     1,     3,     5,     1,     3,
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
  "call_expr", "expr_list", "map_expr", "with", "reduce", "index_expr",
  "reduction_indices", "reduction_index", "reduction_op", "lhs_expr_list",
  "lhs_expr", "var_decl", "type", "element_type", "tensor_type",
  "nested_dimensions", "dimensions", "dimension", "component_type",
  "element_literal", "tensor_literal", "dense_tensor_literal",
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
       0,   171,   171,   173,   177,   180,   183,   186,   189,   193,
     199,   203,   209,   212,   219,   223,   225,   227,   229,   232,
     242,   249,   258,   296,   299,   310,   314,   325,   333,   337,
     344,   347,   356,   357,   358,   359,   360,   364,   394,   402,
     408,   410,   414,   416,   423,   429,   473,   476,   490,   508,
     511,   514,   517,   522,   527,   532,   537,   542,   566,   570,
     575,   580,   585,   590,   595,   600,   605,   609,   614,   619,
     622,   625,   634,   640,   650,   656,   665,   672,   674,   679,
     681,   686,   688,   690,   693,   696,   699,   705,   706,   728,
     733,   741,   747,   752,   761,   789,   792,   797,   803,   806,
     812,   815,   853,   858,   865,   868,   872,   877,   880,   952,
     956,   957,   960,   968,   978,   985,   988,   992,  1005,  1009,
    1023,  1027,  1033,  1040,  1043,  1047,  1060,  1064,  1078,  1082,
    1088,  1093,  1103
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



