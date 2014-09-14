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

      case 80: // elwise_binary_op


        {}

        break;

      case 81: // call_expr


        { delete (yysym.value.call); }

        break;

      case 82: // actual_list


        { delete (yysym.value.expressions); }

        break;

      case 83: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 88: // lhs_expr_list


        { delete (yysym.value.VarAccesses); }

        break;

      case 89: // lhs_expr


        { delete (yysym.value.VarAccess); }

        break;

      case 90: // type


        { delete (yysym.value.type); }

        break;

      case 92: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 93: // tensor_type


        { delete (yysym.value.tensorType); }

        break;

      case 94: // nested_dimensions


        { delete (yysym.value.IndexSetProducts); }

        break;

      case 95: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 96: // dimension


        { delete (yysym.value.indexSet); }

        break;

      case 97: // component_type


        {}

        break;

      case 98: // literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 99: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 100: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

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


        { delete (yysym.value.TensorLiteral); }

        break;

      case 110: // test


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

  case 45:

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

  case 47:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 48:

    {
    if ((yystack_[0].value.expression) == NULL) {  // TODO: Remove check
      (yylhs.value.expression) = NULL;
      break;
    }

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expression) = new shared_ptr<Expression>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 49:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) {  // TODO: Remove this check
      (yylhs.value.expression) = NULL;
      break;
    }
    std::shared_ptr<Expression> l = convertAndDelete((yystack_[2].value.expression));
    std::shared_ptr<Expression> r = convertAndDelete((yystack_[0].value.expression));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    BINARY_ELWISE_TYPE_CHECK(l->getType(), r->getType(), yystack_[1].location);
    (yylhs.value.expression) = new shared_ptr<Expression>(binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }

    break;

  case 50:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) {  // TODO: Remove this check
      (yylhs.value.expression) = NULL;
      break;
    }

    std::shared_ptr<Expression> l = convertAndDelete((yystack_[2].value.expression));
    std::shared_ptr<Expression> r = convertAndDelete((yystack_[0].value.expression));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    TensorType *ltype = tensorTypePtr(l->getType());
    TensorType *rtype = tensorTypePtr(r->getType());

    // Scale
    if (ltype->getOrder()==0 || rtype->getOrder()==0) {
      (yylhs.value.expression) = new shared_ptr<Expression>(binaryElwiseExpr(l, IndexExpr::MUL, r));
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
        (yylhs.value.expression) = new shared_ptr<Expression>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (ctx->isColumnVector(*r->getType())) {
          REPORT_ERROR("cannot multiply two column vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(*l->getType(), *r->getType(), yystack_[1].location);
        }
        (yylhs.value.expression) = new shared_ptr<Expression>(outerProduct(l, r));
      }
    }
    // Matrix-Vector
    else if (ltype->getOrder() == 2 && rtype->getOrder() == 1) {
      if (ltype->getDimensions()[1] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*l->getType(), *rtype, yystack_[1].location);
      }
      (yylhs.value.expression) = new shared_ptr<Expression>(gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->getOrder() == 1 && rtype->getOrder() == 2) {
      if (ltype->getDimensions()[0] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*ltype, *rtype, yystack_[1].location);
      }
      (yylhs.value.expression) = new shared_ptr<Expression>(gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->getOrder() == 2 && rtype->getOrder() == 2) {
      if (ltype->getDimensions()[1] != rtype->getDimensions()[0]){
        REPORT_TYPE_MISSMATCH(*ltype, *rtype, yystack_[1].location);
      }
      (yylhs.value.expression) = new shared_ptr<Expression>(gemm(l,r));
    }
    else {
      REPORT_ERROR("cannot multiply >2-order tensors using *", yystack_[1].location);
      (yylhs.value.expression) = NULL;
    }
  }

    break;

  case 51:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 52:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 53:

    {
    if ((yystack_[1].value.expression) == NULL) {  // TODO: Remove this check
      (yylhs.value.expression) = NULL;
      break;
    }

    auto expr = shared_ptr<Expression>(*(yystack_[1].value.expression));
    delete (yystack_[1].value.expression);

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    TensorType *type = tensorTypePtr(expr->getType());

    switch (type->getOrder()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expression) = new shared_ptr<Expression>(unaryElwiseExpr(IndexExpr::NONE, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expression) = new shared_ptr<Expression>(unaryElwiseExpr(IndexExpr::NONE, expr));
        if (!ctx->isColumnVector(*expr->getType())) {
          ctx->toggleColumnVector(*(*(yylhs.value.expression))->getType());
        }
        break;
      case 2:
        (yylhs.value.expression) = new shared_ptr<Expression>(transposeMatrix(expr));
        break;
      default:
        REPORT_ERROR("cannot transpose >2-order tensors using '", yystack_[1].location);
        (yylhs.value.expression) = NULL;
    }
  }

    break;

  case 54:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 55:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 56:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 57:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 58:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 59:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 60:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 61:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[1].value.expression);
  }

    break;

  case 62:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 63:

    {
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 64:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 65:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 66:

    {
    (yylhs.value.binop) = IndexExpr::ADD;
  }

    break;

  case 67:

    {
    (yylhs.value.binop) = IndexExpr::SUB;
  }

    break;

  case 68:

    {
    (yylhs.value.binop) = IndexExpr::MUL;
  }

    break;

  case 69:

    {
    (yylhs.value.binop) = IndexExpr::DIV;
  }

    break;

  case 70:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.expressions));
    delete (yystack_[1].value.expressions);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 71:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 72:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 73:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 74:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 75:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
  }

    break;

  case 77:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 82:

    {
    (yylhs.value.VarAccesses) = new VariableAccessVector();
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 83:

    {
    (yylhs.value.VarAccesses) = (yystack_[2].value.VarAccesses);
    if ((yystack_[0].value.VarAccess) == NULL) break;  // TODO: Remove check
    (yylhs.value.VarAccesses)->push_back((yystack_[0].value.VarAccess));
  }

    break;

  case 84:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    auto variableStore = new VariableAccess;
    variableStore->name = name;
    (yylhs.value.VarAccess) = variableStore;
  }

    break;

  case 85:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    std::string field = convertAndFree((yystack_[0].value.string));
    (yylhs.value.VarAccess) = NULL;
  }

    break;

  case 86:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.VarAccess) = NULL;
    delete (yystack_[1].value.expressions);
  }

    break;

  case 87:

    {
    (yylhs.value.type) = NULL;
  }

    break;

  case 88:

    {
//    $$ = $element_type;
  }

    break;

  case 89:

    {
    (yylhs.value.type) = (yystack_[0].value.tensorType);
  }

    break;

  case 92:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.elementType) = ctx->getElementType(name);
  }

    break;

  case 93:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[0].value.componentType));
  }

    break;

  case 94:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[1].value.componentType), *(yystack_[3].value.IndexSetProducts));
    delete (yystack_[3].value.IndexSetProducts);
  }

    break;

  case 95:

    {
    (yylhs.value.tensorType) = new TensorType((yystack_[2].value.componentType), *(yystack_[4].value.IndexSetProducts));
    ctx->toggleColumnVector(*(yylhs.value.tensorType));
    delete (yystack_[4].value.IndexSetProducts);
  }

    break;

  case 96:

    {
    (yylhs.value.IndexSetProducts) = new std::vector<IndexSetProduct>();
  }

    break;

  case 97:

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

  case 98:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 99:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 100:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 101:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 102:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 103:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 104:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 107:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    ctx->toggleColumnVector(*(*(yylhs.value.TensorLiteral))->getType());
  }

    break;

  case 109:

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

  case 110:

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

  case 111:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 113:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 114:

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

  case 115:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 116:

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

  case 117:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 118:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 119:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 121:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 122:

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

  case 123:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 124:

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

  case 125:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 126:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 127:

    {
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 128:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 129:

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

  const signed char  Parser ::yytable_ninf_ = -85;

  const short int
   Parser ::yypact_[] =
  {
    -103,     9,  -103,  -103,  -103,  -103,     5,    31,    43,    54,
      57,    72,    84,   132,   -27,   115,   132,     6,  -103,   132,
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,   197,  -103,  -103,    40,  -103,  -103,  -103,    79,
    -103,  -103,   132,   124,  -103,   116,    45,  -103,    67,   118,
     135,   217,  -103,   135,   114,   177,  -103,  -103,    99,   136,
     111,   129,   141,   140,   146,   133,   147,   282,    77,   132,
     132,   132,   132,  -103,  -103,  -103,   132,   132,  -103,  -103,
     132,  -103,   132,   132,   132,   132,   132,   132,   179,   132,
    -103,   217,   159,    85,  -103,    25,    91,  -103,  -103,  -103,
    -103,   153,  -103,    62,  -103,  -103,   104,    17,   166,   187,
     132,   145,    13,  -103,    28,    46,  -103,   167,   191,   193,
    -103,   170,   199,   201,  -103,  -103,   270,   270,   217,   217,
     282,   282,   282,   237,   257,   257,   270,   270,   217,     3,
    -103,     8,  -103,  -103,   132,   163,  -103,  -103,   168,   125,
    -103,   200,   176,  -103,   172,  -103,    88,  -103,   189,  -103,
     215,   220,   213,   119,   214,  -103,  -103,   191,   141,  -103,
     199,   147,  -103,   132,  -103,   217,    91,    13,   149,    22,
    -103,    87,  -103,    45,  -103,   250,   250,   266,   258,  -103,
     132,  -103,  -103,    47,    58,    85,   254,   267,   247,  -103,
    -103,  -103,   109,  -103,   264,   290,  -103,  -103,    97,  -103,
     148,  -103,   217,   174,  -103,  -103,  -103,  -103,   273,  -103,
      22,   289,  -103,  -103,  -103,  -103,  -103,   174,  -103,  -103,
    -103
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   127,   128,    47,    45,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,     0,
       3,     7,     4,     5,     6,    28,     8,    30,    31,    32,
      33,    34,     0,    64,    65,     0,    82,    46,   105,   106,
     108,     9,    71,     0,    15,     0,     0,    28,     0,     0,
      45,    28,    36,     0,     0,     0,   125,   117,     0,     0,
     112,   111,   115,     0,   120,   119,   123,    48,     0,     0,
       0,     0,     0,    38,    66,    67,     0,     0,    68,    69,
       0,    53,     0,     0,     0,     0,     0,     0,     0,     0,
     107,    73,     0,     0,    85,     0,     0,    92,   103,   104,
      96,     0,    87,    88,    89,    93,     0,     0,    23,     0,
      71,    43,     0,    61,     0,     0,   109,     0,     0,     0,
     110,     0,     0,     0,    19,    29,    57,    58,    63,    62,
      50,    51,    54,    52,    55,    56,    59,    60,    49,    84,
      83,     0,    70,    86,     0,     0,    14,    16,     0,     0,
      10,     0,     0,    18,     0,    21,     0,    25,     0,    20,
      76,    72,     0,     0,     0,   113,   121,     0,   116,   118,
       0,   124,   126,     0,    37,    74,     0,     0,     0,     0,
      12,     0,    90,     0,    22,     0,     0,     0,    78,    40,
       0,    28,   129,     0,     0,     0,     0,     0,     0,   100,
     101,   102,     0,    98,     0,     0,    27,    26,     0,    77,
       0,    75,    28,    42,   114,   122,    17,    35,    94,    97,
       0,     0,    13,    24,    80,    81,    79,    44,    95,    99,
      91
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,  -103,
    -103,  -103,  -103,   138,   142,   -46,   324,  -103,  -103,  -103,
    -103,  -103,  -103,  -103,   -13,  -103,   311,  -103,   -82,  -103,
    -103,  -103,  -103,  -103,   244,   150,  -103,  -103,   -88,  -103,
    -103,   117,   156,  -103,  -102,  -103,  -103,  -103,   -54,   218,
    -103,  -103,   -56,   216,  -103,  -103
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    20,    21,   181,    22,    95,   147,    23,    24,
      25,   108,   159,   156,   157,    68,   125,    27,    28,    29,
      30,    31,   162,   163,    32,    87,    33,    92,    93,    34,
     188,   211,   226,    35,    36,   101,   102,   103,   104,   149,
     202,   203,   105,    37,    38,    39,    59,    60,    61,    62,
      63,    64,    65,    66,    40,    41
  };

  const short int
   Parser ::yytable_[] =
  {
      51,   106,   115,    55,   114,   111,    67,   141,   148,     2,
     164,    56,    57,    52,     3,     4,     5,     6,     3,     4,
       7,     8,     9,    10,    11,   154,    12,   199,   161,    91,
     200,    13,   173,   145,    42,    14,    15,    58,    16,    44,
      17,    43,   -84,    43,    17,   144,   -84,   155,   174,    18,
     146,    45,    19,    97,    98,    99,   126,   127,   128,   129,
     165,   100,    46,   130,   131,    47,   201,   132,   118,   133,
     134,   135,   136,   137,   138,   197,    91,    88,   166,   214,
      48,    89,     3,     4,     5,     6,   122,   118,   196,     8,
     215,   195,    49,   151,    12,   152,   107,    91,   122,    13,
      98,    99,   124,    14,    56,    57,    16,   100,    17,     3,
       4,     5,     6,   193,   194,   143,     8,    18,   184,   204,
      19,    12,   144,    53,   205,   185,    13,   223,    90,   153,
      14,   175,    94,    16,   185,    17,   109,     3,     4,     5,
      50,   219,   190,   191,    18,   213,   220,    19,   117,    12,
       3,     4,     5,     6,   178,    96,   179,     8,    98,    99,
      91,    16,    12,    17,   110,   112,   227,    13,   116,   118,
     -41,    14,   120,   122,    16,    19,    17,   212,   119,     3,
       4,     5,     6,   121,   123,    18,     8,   139,    19,   142,
     224,    12,   225,   150,   158,   160,    13,    57,   167,   169,
      14,   170,   176,    16,    56,    17,   172,   113,   180,   177,
     182,   183,    69,    70,    18,    71,    72,    19,   186,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    69,    70,   187,    71,    72,    73,   189,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    69,    70,   192,    71,    72,   144,   154,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    69,    70,   209,    71,    72,   218,   210,    74,
      75,    76,    77,    78,    79,    80,    81,   -85,    83,    84,
      85,    86,    69,    70,   216,    71,    72,   221,   222,    74,
      75,    76,    77,    78,    79,    80,    81,   217,    71,    72,
      85,    86,    74,    75,    76,    77,    78,    79,    80,    81,
      71,    72,   228,   230,   208,    26,    54,   207,    78,    79,
      80,    81,   140,   206,   198,     0,   168,   229,   171
  };

  const short int
   Parser ::yycheck_[] =
  {
      13,    47,    58,    16,    58,    51,    19,    89,    96,     0,
     112,     5,     6,    40,     5,     6,     7,     8,     5,     6,
      11,    12,    13,    14,    15,     8,    17,     5,   110,    42,
       8,    22,    29,     8,    29,    26,    27,    31,    29,     8,
      31,    38,    37,    38,    31,    37,    41,    30,    40,    40,
      25,     8,    43,     8,     9,    10,    69,    70,    71,    72,
      32,    16,     8,    76,    77,     8,    44,    80,    40,    82,
      83,    84,    85,    86,    87,   177,    89,    37,    32,    32,
       8,    41,     5,     6,     7,     8,    40,    40,   176,    12,
      32,   173,     8,    31,    17,    33,    29,   110,    40,    22,
       9,    10,    25,    26,     5,     6,    29,    16,    31,     5,
       6,     7,     8,   167,   170,    30,    12,    40,    30,    32,
      43,    17,    37,     8,    37,    37,    22,    30,    49,    25,
      26,   144,     8,    29,    37,    31,    18,     5,     6,     7,
       8,    32,    23,    24,    40,   191,    37,    43,    37,    17,
       5,     6,     7,     8,    29,    39,    31,    12,     9,    10,
     173,    29,    17,    31,    29,    51,   212,    22,    32,    40,
      25,    26,    32,    40,    29,    43,    31,   190,    37,     5,
       6,     7,     8,    37,    37,    40,    12,     8,    43,    30,
      42,    17,    44,    40,    28,     8,    22,     6,    31,     6,
      26,    31,    39,    29,     5,    31,     5,    30,     8,    41,
      34,    39,    35,    36,    40,    38,    39,    43,    29,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    35,    36,    19,    38,    39,    40,    25,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    35,    36,    40,    38,    39,    37,     8,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    35,    36,     8,    38,    39,    30,    20,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    35,    36,    40,    38,    39,    33,     8,    42,
      43,    44,    45,    46,    47,    48,    49,    40,    38,    39,
      53,    54,    42,    43,    44,    45,    46,    47,    48,    49,
      38,    39,    49,    34,   186,     1,    15,   185,    46,    47,
      48,    49,    88,   183,   178,    -1,   118,   220,   122
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    56,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    31,    40,    43,
      57,    58,    60,    63,    64,    65,    71,    72,    73,    74,
      75,    76,    79,    81,    84,    88,    89,    98,    99,   100,
     109,   110,    29,    38,     8,     8,     8,     8,     8,     8,
       8,    79,    40,     8,    81,    79,     5,     6,    31,   101,
     102,   103,   104,   105,   106,   107,   108,    79,    70,    35,
      36,    38,    39,    40,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    80,    37,    41,
      49,    79,    82,    83,     8,    61,    39,     8,     9,    10,
      16,    90,    91,    92,    93,    97,    70,    29,    66,    18,
      29,    70,    51,    30,   103,   107,    32,    37,    40,    37,
      32,    37,    40,    37,    25,    71,    79,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,     8,
      89,    83,    30,    30,    37,     8,    25,    62,    93,    94,
      40,    31,    33,    25,     8,    30,    68,    69,    28,    67,
       8,    83,    77,    78,    99,    32,    32,    31,   104,     6,
      31,   108,     5,    29,    40,    79,    39,    41,    29,    31,
       8,    59,    34,    39,    30,    37,    29,    19,    85,    25,
      23,    24,    40,   103,   107,    83,    93,    99,    97,     5,
       8,    44,    95,    96,    32,    37,    90,    69,    68,     8,
      20,    86,    79,    70,    32,    32,    40,    40,    30,    32,
      37,    33,     8,    30,    42,    44,    87,    70,    49,    96,
      34
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
      79,    79,    79,    79,    79,    79,    80,    80,    80,    80,
      81,    82,    82,    83,    83,    84,    85,    85,    86,    86,
      87,    87,    88,    88,    89,    89,    89,    90,    90,    90,
      91,    91,    92,    93,    93,    93,    94,    94,    95,    95,
      96,    96,    96,    97,    97,    98,    99,    99,    99,   100,
     100,   101,   101,   102,   102,   103,   103,   104,   104,   105,
     105,   106,   106,   107,   107,   108,   108,   109,   109,   110
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       4,     4,     1,     3,     4,     0,     2,     4,     4,     3,
       4,     2,     3,     0,     4,     1,     3,     3,     0,     2,
       1,     1,     1,     1,     1,     7,     2,     4,     2,     1,
       5,     0,     3,     0,     4,     1,     1,     1,     2,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       4,     0,     1,     1,     3,     6,     0,     2,     0,     2,
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
  "expr", "elwise_binary_op", "call_expr", "actual_list", "expr_list",
  "map_expr", "with", "reduce", "reduction_op", "lhs_expr_list",
  "lhs_expr", "type", "set_type", "element_type", "tensor_type",
  "nested_dimensions", "dimensions", "dimension", "component_type",
  "literal", "tensor_literal", "dense_tensor_literal",
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
       0,   236,   236,   238,   242,   252,   255,   264,   267,   271,
     278,   282,   288,   291,   305,   313,   316,   323,   335,   342,
     351,   389,   392,   403,   407,   418,   422,   428,   436,   439,
     448,   449,   450,   451,   452,   456,   486,   492,   533,   536,
     542,   548,   550,   554,   556,   570,   580,   581,   584,   596,
     610,   678,   683,   688,   721,   726,   731,   736,   741,   746,
     751,   756,   760,   765,   770,   773,   784,   787,   790,   793,
     802,   811,   814,   820,   826,   835,   842,   844,   848,   850,
     853,   854,   876,   881,   889,   895,   900,   930,   933,   936,
     941,   942,   945,   951,   954,   958,   965,   968,  1006,  1011,
    1018,  1021,  1025,  1030,  1033,  1102,  1105,  1106,  1110,  1113,
    1122,  1133,  1140,  1143,  1147,  1160,  1164,  1178,  1182,  1188,
    1195,  1198,  1202,  1215,  1219,  1233,  1237,  1243,  1248,  1258
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



