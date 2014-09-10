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
  #include "irutils.h"
  using namespace std;
  using namespace simit::internal;

  std::string tensorTypeString(const TensorType *tensorType, ProgramContext *ctx){
    std::stringstream ss;
    ss << *tensorType;
    std::string str = ss.str();
    if (ctx->isColumnVector(tensorType)) {
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
               << tensorTypeString(t1, ctx) << " and " \
               << tensorTypeString(t2, ctx) << ")";    \
      REPORT_ERROR(errorStr.str(), loc);               \
    } while (0)

  #define REPORT_INDEX_VARIABLE_MISSMATCH(numIndexVars, order, loc) \
    do {                                                            \
      REPORT_ERROR("wrong number of index variables (" +            \
                    to_string(numIndexVars) +                       \
                    " index variables, but tensor order is " +      \
                    to_string(order), loc);                          \
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

  bool compare(const TensorType *l ,const TensorType *r, ProgramContext *ctx) {
    if (ctx->isColumnVector(l) != ctx->isColumnVector(r)) {
      return false;
    }
    if (*l != *r) {
      return false;
    }
    return true;
  }

  #define CHECK_TYPE_EQUALITY(t1, t2, loc)             \
    do {                                               \
      if (!compare(t1, t2, ctx)) {                     \
        REPORT_TYPE_MISSMATCH(t1, t2, loc);            \
      }                                                \
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


        { delete (yysym.value.Tensor); }

        break;

      case 80: // call_expr


        { delete (yysym.value.call); }

        break;

      case 81: // actual_list


        { delete (yysym.value.TensorList); }

        break;

      case 82: // expr_list


        { delete (yysym.value.TensorList); }

        break;

      case 87: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 88: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 89: // type


        { delete (yysym.value.tensorType); }

        break;

      case 91: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 92: // tensor_type


        { delete (yysym.value.tensorType); }

        break;

      case 93: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 94: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 95: // dimension


        { delete (yysym.value.indexSet); }

        break;

      case 96: // component_type


        {}

        break;

      case 97: // literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 98: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 99: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 100: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 101: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 102: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 103: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 104: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 105: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 106: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 107: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 108: // scalar_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 109: // test


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
  case 4:

    {
    (yylhs.value.IRNode) = NULL;
    std::unique_ptr<ElementType> elementType((yystack_[0].value.elementType));

    std::string name = elementType->getName();
    if (ctx->containsElementType(name)) {
      REPORT_ERROR("struct redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addElementType(elementType.release());
  }

    break;

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
    delete (yystack_[1].value.tensorType);
  }

    break;

  case 11:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.tensorType);
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
    std::unique_ptr<std::vector<ElementField*>> fields((yystack_[1].value.fields));
    (yylhs.value.elementType) = new ElementType(name, *fields);
  }

    break;

  case 15:

    {
    (yylhs.value.fields) = new std::vector<ElementField*>();
  }

    break;

  case 16:

    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yylhs.value.fields)->push_back((yystack_[0].value.field));
  }

    break;

  case 17:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.field) = new ElementField(name, (yystack_[1].value.tensorType));
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
      ctx->addTensorSymbol(argument->getName(), argument);
    }

    for (auto result : *results) {
      ctx->addTensorSymbol(result->getName(), result);
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
    (yylhs.value.Formal) = new FormalData(ident, (yystack_[0].value.tensorType));
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
    auto tensorType = unique_ptr<TensorType>((yystack_[3].value.tensorType));

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
    CHECK_TYPE_EQUALITY((yystack_[3].value.tensorType), tensorLiteral->getType(), yystack_[5].location);

    ctx->addTensorSymbol(tensorLiteral->getName(), tensorLiteral);

    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
    (yylhs.value.IRNodes)->push_back(tensorLiteral);
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
    delete (yystack_[3].value.Tensor);
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
    delete (yystack_[1].value.Tensor);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 45:

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

  case 47:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 48:

    {
    if ((yystack_[0].value.Tensor) == NULL) {  // TODO: Remove check
      (yylhs.value.Tensor) = NULL;
      break;
    }
    std::shared_ptr<TensorNode> expr = convertAndDelete((yystack_[0].value.Tensor));
    (yylhs.value.Tensor) = new shared_ptr<TensorNode>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 49:

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

  case 50:

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

  case 51:

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

  case 52:

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

  case 53:

    {
    if ((yystack_[2].value.Tensor) == NULL || (yystack_[0].value.Tensor) == NULL) {  // TODO: Remove this check
      (yylhs.value.Tensor) = NULL;
      break;
    }

    std::shared_ptr<TensorNode> l = convertAndDelete((yystack_[2].value.Tensor));
    std::shared_ptr<TensorNode> r = convertAndDelete((yystack_[0].value.Tensor));

    // Scale
    if (l->getType()->getOrder()==0 || r->getType()->getOrder()==0) {
      (yylhs.value.Tensor) = new shared_ptr<TensorNode>(binaryElwiseExpr(l, IndexExpr::MUL, r));
    }
    // Vector-Vector Multiplication (inner and outer product)
    else if (l->getType()->getOrder() == 1 && r->getType()->getOrder() == 1) {
      // Inner product
      if (!ctx->isColumnVector(l->getType())) {
        if (!ctx->isColumnVector(r->getType())) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
        }
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (ctx->isColumnVector(r->getType())) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
        }
        (yylhs.value.Tensor) = new shared_ptr<TensorNode>(outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (l->getType()->getOrder() == 2 && r->getType()->getOrder() == 1) {
      if (l->getType()->getDimensions()[1] != r->getType()->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.Tensor) = new shared_ptr<TensorNode>(gemv(l, r));
    }
    // Vector-Matrix
    else if (l->getType()->getOrder() == 1 && r->getType()->getOrder() == 2) {
      if (l->getType()->getDimensions()[0] != r->getType()->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.Tensor) = new shared_ptr<TensorNode>(gevm(l,r));
    }
    // Matrix-Matrix
    else if (l->getType()->getOrder() == 2 && r->getType()->getOrder() == 2) {
      if (l->getType()->getDimensions()[1] != r->getType()->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(l->getType(), r->getType(), yystack_[1].location);
      }
      (yylhs.value.Tensor) = new shared_ptr<TensorNode>(gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.Tensor) = NULL;
    }
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
    delete (yystack_[1].value.Tensor);
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
  }

    break;

  case 68:

    {
    (yylhs.value.Tensor) = NULL;
  }

    break;

  case 69:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.TensorList));
    delete (yystack_[1].value.TensorList);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 70:

    {
    (yylhs.value.TensorList) = new vector<shared_ptr<TensorNode>>();
  }

    break;

  case 71:

    {
    (yylhs.value.TensorList) = (yystack_[0].value.TensorList);
  }

    break;

  case 72:

    {
    (yylhs.value.TensorList) = new std::vector<std::shared_ptr<TensorNode>>();
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 73:

    {
    (yylhs.value.TensorList) = (yystack_[2].value.TensorList);
    if ((yystack_[0].value.Tensor) == NULL) break;  // TODO: Remove check
    (yylhs.value.TensorList)->push_back(*(yystack_[0].value.Tensor));
    delete (yystack_[0].value.Tensor);
  }

    break;

  case 74:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
  }

    break;

  case 76:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 81:

    {
    (yylhs.value.VarAccesses) = new VariableAccessVector();
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 82:

    {
    (yylhs.value.VarAccesses) = (yystack_[2].value.VarAccesses);
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 83:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto variableStore = new VariableAccess;
    variableStore->name = name;
    (yylhs.value.VarAccess) = variableStore;
  }

    break;

  case 84:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    std::string field = convertAndFree((yystack_[0].value.string));
    (yylhs.value.VarAccess) = NULL;
  }

    break;

  case 85:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.VarAccess) = NULL;
    delete (yystack_[1].value.TensorList);
  }

    break;

  case 86:

    {
    (yylhs.value.tensorType) = NULL;
  }

    break;

  case 87:

    {
//    $$ = $element_type;
  }

    break;

  case 88:

    {
    (yylhs.value.tensorType) = (yystack_[0].value.tensorType);
  }

    break;

  case 91:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.elementType) = ctx->getElementType(name);
  }

    break;

  case 92:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[0].value.componentType));
  }

    break;

  case 93:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[1].value.componentType), *(yystack_[3].value.IndexSetProducts));
    delete (yystack_[3].value.IndexSetProducts);
  }

    break;

  case 94:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[2].value.componentType), *(yystack_[4].value.IndexSetProducts));
    ctx->toggleColumnVector((yylhs.value.tensorType));
    delete (yystack_[4].value.IndexSetProducts);
  }

    break;

  case 95:

    {
    (yylhs.value.IndexSetProducts) = new std::vector<IndexSetProduct>();
  }

    break;

  case 96:

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

  case 97:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 98:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 99:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 100:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 101:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 102:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 103:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 106:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    ctx->toggleColumnVector((*(yylhs.value.TensorLiteral))->getType());
  }

    break;

  case 108:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::FLOAT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 109:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto isps = std::vector<IndexSetProduct>(values->dimSizes.rbegin(),
                                             values->dimSizes.rend());
    auto type = new TensorType(ComponentType::INT, isps);
    auto literal = new Literal(type, values->values.data());
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 110:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 112:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 113:

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

  case 114:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 115:

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

  case 116:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 117:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 118:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 120:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 121:

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

  case 122:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 123:

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

  case 124:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 125:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 126:

    {
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 127:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(scalarType, &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 128:

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


  const signed char  Parser ::yypact_ninf_ = -103;

  const signed char  Parser ::yytable_ninf_ = -84;

  const short int
   Parser ::yypact_[] =
  {
    -103,    10,  -103,  -103,  -103,  -103,    11,    23,    43,    46,
      52,    68,    80,   186,    65,    94,   186,    14,  -103,   186,
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,   208,  -103,  -103,    41,  -103,  -103,  -103,    59,
    -103,  -103,   186,   106,  -103,    84,    71,  -103,   144,   156,
     148,   228,  -103,   148,   135,   188,  -103,  -103,   150,   151,
     152,   147,   158,   164,   161,   157,   175,   104,    78,   186,
     186,   186,   186,  -103,   186,   186,   186,   186,   186,   186,
     186,  -103,   186,   186,   186,   186,   186,   205,   186,  -103,
     228,   184,    62,  -103,     5,   110,  -103,  -103,  -103,  -103,
     176,  -103,   -19,  -103,  -103,   105,     3,   191,   212,   186,
     132,    37,  -103,    -6,    57,  -103,   190,   216,   219,  -103,
     197,   240,   244,  -103,  -103,   162,   162,   228,   228,   280,
     280,   104,   104,   228,   228,   104,   248,   268,   268,   162,
     162,    17,  -103,     7,  -103,  -103,   186,   226,  -103,  -103,
     227,   149,  -103,   261,   251,  -103,   249,  -103,    95,  -103,
     260,  -103,   286,   271,   284,   136,   283,  -103,  -103,   216,
     158,  -103,   240,   175,  -103,   186,  -103,   228,   110,    37,
     160,    30,  -103,    92,  -103,    71,  -103,   312,   312,   322,
     311,  -103,   186,  -103,  -103,    61,    66,    62,   292,   293,
     304,  -103,  -103,  -103,   109,  -103,   302,   328,  -103,  -103,
      98,  -103,   140,  -103,   228,   159,  -103,  -103,  -103,  -103,
     288,  -103,    30,   305,  -103,  -103,  -103,  -103,  -103,   159,
    -103,  -103,  -103
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   126,   127,    47,    45,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,     0,
       3,     7,     4,     5,     6,    28,     8,    30,    31,    32,
      33,    34,     0,    67,    68,     0,    81,    46,   104,   105,
     107,     9,    70,     0,    15,     0,     0,    28,     0,     0,
      45,    28,    36,     0,     0,     0,   124,   116,     0,     0,
     111,   110,   114,     0,   119,   118,   122,    48,     0,     0,
       0,     0,     0,    38,     0,     0,     0,     0,     0,     0,
       0,    56,     0,     0,     0,     0,     0,     0,     0,   106,
      72,     0,     0,    84,     0,     0,    91,   102,   103,    95,
       0,    86,    87,    88,    92,     0,     0,    23,     0,    70,
      43,     0,    64,     0,     0,   108,     0,     0,     0,   109,
       0,     0,     0,    19,    29,    60,    61,    66,    65,    49,
      50,    53,    54,    51,    52,    57,    55,    58,    59,    62,
      63,    83,    82,     0,    69,    85,     0,     0,    14,    16,
       0,     0,    10,     0,     0,    18,     0,    21,     0,    25,
       0,    20,    75,    71,     0,     0,     0,   112,   120,     0,
     115,   117,     0,   123,   125,     0,    37,    73,     0,     0,
       0,     0,    12,     0,    89,     0,    22,     0,     0,     0,
      77,    40,     0,    28,   128,     0,     0,     0,     0,     0,
       0,    99,   100,   101,     0,    97,     0,     0,    27,    26,
       0,    76,     0,    74,    28,    42,   113,   121,    17,    35,
      93,    96,     0,     0,    13,    24,    79,    80,    78,    44,
      94,    98,    90
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,  -103,   153,   155,   -46,   337,  -103,  -103,  -103,
    -103,  -103,  -103,  -103,   -13,   325,  -103,   -81,  -103,  -103,
    -103,  -103,  -103,   256,   163,  -103,  -103,   -87,  -103,  -103,
     122,   165,  -103,  -102,  -103,  -103,  -103,   -54,   229,  -103,
    -103,   -56,   230,  -103,  -103
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    20,    21,   183,    22,    94,   149,    23,    24,
      25,   107,   161,   158,   159,    68,   124,    27,    28,    29,
      30,    31,   164,   165,    32,    33,    91,    92,    34,   190,
     213,   228,    35,    36,   100,   101,   102,   103,   151,   204,
     205,   104,    37,    38,    39,    59,    60,    61,    62,    63,
      64,    65,    66,    40,    41
  };

  const short int
   Parser ::yytable_[] =
  {
      51,   105,   114,    55,   113,   110,    67,   143,   150,   166,
       2,   156,   153,   147,   154,     3,     4,     5,     6,    56,
      57,     7,     8,     9,    10,    11,   167,    12,   163,    90,
     148,    44,    13,   157,   117,   201,    14,    15,   202,    16,
      42,    17,     3,     4,   146,    58,   175,   176,   -83,    43,
      18,    45,   -83,    19,    46,    43,   125,   126,   127,   128,
      47,   129,   130,   131,   132,   133,   134,   135,    17,   136,
     137,   138,   139,   140,   203,    90,    48,   199,    87,    96,
      97,    98,    88,     3,     4,     5,     6,    99,    49,   168,
       8,   198,   145,   216,   197,    12,    90,   121,   217,   146,
      13,   117,    53,   123,    14,    52,   121,    16,    89,    17,
       3,     4,     5,     6,    93,   195,   196,     8,    18,    97,
      98,    19,    12,    95,   206,   186,    99,    13,   225,   207,
     155,    14,   187,   177,    16,   187,    17,     3,     4,     5,
       6,   221,    71,    72,     8,    18,   222,   215,    19,    12,
      78,    79,    80,    81,    13,    56,    57,   -41,    14,   192,
     193,    16,    90,    17,     3,     4,     5,     6,   229,    97,
      98,     8,    18,   106,   108,    19,    12,   109,   180,   214,
     181,    13,   226,   115,   227,    14,   111,   117,    16,   116,
      17,     3,     4,     5,    50,   118,   119,   121,   120,    18,
      71,    72,    19,    12,    74,    75,    76,    77,    78,    79,
      80,    81,   122,   141,   144,    16,   152,    17,   112,   160,
     162,   169,    57,    69,    70,   171,    71,    72,   172,    19,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    69,    70,    56,    71,    72,    73,   174,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    69,    70,   178,    71,    72,   179,   182,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    69,    70,   184,    71,    72,   185,   188,
      74,    75,    76,    77,    78,    79,    80,    81,   -84,    83,
      84,    85,    86,    69,    70,   189,    71,    72,   146,   191,
      74,    75,    76,    77,    78,    79,    80,    81,    71,    72,
     156,    85,    86,   194,    76,    77,    78,    79,    80,    81,
     211,   212,   218,   219,   220,   223,   224,   230,    26,   232,
      54,   210,   209,   142,   231,   200,   170,     0,   208,     0,
       0,   173
  };

  const short int
   Parser ::yycheck_[] =
  {
      13,    47,    58,    16,    58,    51,    19,    88,    95,   111,
       0,     8,    31,     8,    33,     5,     6,     7,     8,     5,
       6,    11,    12,    13,    14,    15,    32,    17,   109,    42,
      25,     8,    22,    30,    40,     5,    26,    27,     8,    29,
      29,    31,     5,     6,    37,    31,    29,    40,    37,    38,
      40,     8,    41,    43,     8,    38,    69,    70,    71,    72,
       8,    74,    75,    76,    77,    78,    79,    80,    31,    82,
      83,    84,    85,    86,    44,    88,     8,   179,    37,     8,
       9,    10,    41,     5,     6,     7,     8,    16,     8,    32,
      12,   178,    30,    32,   175,    17,   109,    40,    32,    37,
      22,    40,     8,    25,    26,    40,    40,    29,    49,    31,
       5,     6,     7,     8,     8,   169,   172,    12,    40,     9,
      10,    43,    17,    39,    32,    30,    16,    22,    30,    37,
      25,    26,    37,   146,    29,    37,    31,     5,     6,     7,
       8,    32,    38,    39,    12,    40,    37,   193,    43,    17,
      46,    47,    48,    49,    22,     5,     6,    25,    26,    23,
      24,    29,   175,    31,     5,     6,     7,     8,   214,     9,
      10,    12,    40,    29,    18,    43,    17,    29,    29,   192,
      31,    22,    42,    32,    44,    26,    51,    40,    29,    37,
      31,     5,     6,     7,     8,    37,    32,    40,    37,    40,
      38,    39,    43,    17,    42,    43,    44,    45,    46,    47,
      48,    49,    37,     8,    30,    29,    40,    31,    30,    28,
       8,    31,     6,    35,    36,     6,    38,    39,    31,    43,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    35,    36,     5,    38,    39,    40,     5,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    35,    36,    39,    38,    39,    41,     8,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    35,    36,    34,    38,    39,    39,    29,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    35,    36,    19,    38,    39,    37,    25,
      42,    43,    44,    45,    46,    47,    48,    49,    38,    39,
       8,    53,    54,    40,    44,    45,    46,    47,    48,    49,
       8,    20,    40,    40,    30,    33,     8,    49,     1,    34,
      15,   188,   187,    87,   222,   180,   117,    -1,   185,    -1,
      -1,   121
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    56,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    31,    40,    43,
      57,    58,    60,    63,    64,    65,    71,    72,    73,    74,
      75,    76,    79,    80,    83,    87,    88,    97,    98,    99,
     108,   109,    29,    38,     8,     8,     8,     8,     8,     8,
       8,    79,    40,     8,    80,    79,     5,     6,    31,   100,
     101,   102,   103,   104,   105,   106,   107,    79,    70,    35,
      36,    38,    39,    40,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    37,    41,    49,
      79,    81,    82,     8,    61,    39,     8,     9,    10,    16,
      89,    90,    91,    92,    96,    70,    29,    66,    18,    29,
      70,    51,    30,   102,   106,    32,    37,    40,    37,    32,
      37,    40,    37,    25,    71,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,     8,    88,    82,    30,    30,    37,     8,    25,    62,
      92,    93,    40,    31,    33,    25,     8,    30,    68,    69,
      28,    67,     8,    82,    77,    78,    98,    32,    32,    31,
     103,     6,    31,   107,     5,    29,    40,    79,    39,    41,
      29,    31,     8,    59,    34,    39,    30,    37,    29,    19,
      84,    25,    23,    24,    40,   102,   106,    82,    92,    98,
      96,     5,     8,    44,    94,    95,    32,    37,    89,    69,
      68,     8,    20,    85,    79,    70,    32,    32,    40,    40,
      30,    32,    37,    33,     8,    30,    42,    44,    86,    70,
      49,    95,    34
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    55,    56,    56,    57,    57,    57,    57,    57,    57,
      58,    58,    59,    59,    60,    61,    61,    62,    63,    64,
      65,    66,    66,    67,    67,    68,    68,    69,    70,    70,
      71,    71,    71,    71,    71,    72,    73,    74,    75,    75,
      76,    77,    77,    78,    78,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    80,
      81,    81,    82,    82,    83,    84,    84,    85,    85,    86,
      86,    87,    87,    88,    88,    88,    89,    89,    89,    90,
      90,    91,    92,    92,    92,    93,    93,    94,    94,    95,
      95,    95,    96,    96,    97,    98,    98,    98,    99,    99,
     100,   100,   101,   101,   102,   102,   103,   103,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       4,     4,     1,     3,     4,     0,     2,     4,     4,     3,
       4,     2,     3,     0,     4,     1,     3,     3,     0,     2,
       1,     1,     1,     1,     1,     7,     2,     4,     2,     1,
       5,     0,     3,     0,     4,     1,     1,     1,     2,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     4,
       0,     1,     1,     3,     6,     0,     2,     0,     2,     1,
       1,     1,     3,     1,     3,     4,     1,     1,     1,     3,
       6,     1,     1,     5,     6,     0,     4,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     3,     3,
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
  "$accept", "program", "program_element", "extern", "endpoints",
  "element_type_decl", "field_decl_list", "field_decl", "procedure",
  "function", "function_header", "arguments", "results", "formal_list",
  "formal_decl", "stmt_block", "stmt", "const_stmt", "return_stmt",
  "assign_stmt", "expr_stmt", "if_stmt", "else_clauses", "elif_clauses",
  "expr", "call_expr", "actual_list", "expr_list", "map_expr", "with",
  "reduce", "reduction_op", "lhs_expr_list", "lhs_expr", "type",
  "set_type", "element_type", "tensor_type", "nested_dimensions",
  "dimensions", "dimension", "component_type", "literal", "tensor_literal",
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
       0,   217,   217,   219,   223,   233,   236,   245,   248,   252,
     259,   263,   269,   272,   286,   294,   297,   304,   316,   323,
     332,   369,   372,   383,   387,   398,   402,   408,   416,   419,
     428,   429,   430,   431,   432,   436,   464,   470,   512,   515,
     521,   527,   529,   533,   535,   549,   566,   567,   570,   578,
     589,   601,   612,   624,   686,   691,   696,   725,   730,   735,
     740,   745,   750,   755,   760,   764,   769,   774,   777,   786,
     795,   798,   804,   810,   819,   826,   828,   832,   834,   837,
     838,   860,   865,   873,   879,   884,   915,   918,   921,   926,
     927,   930,   936,   939,   943,   950,   953,   991,   996,  1003,
    1006,  1010,  1015,  1018,  1087,  1090,  1091,  1095,  1098,  1106,
    1116,  1123,  1126,  1130,  1143,  1147,  1161,  1165,  1171,  1178,
    1181,  1185,  1198,  1202,  1216,  1220,  1226,  1231,  1241
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



