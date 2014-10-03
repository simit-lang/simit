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

      case 58: // program_element


        { delete (yysym.value.IRNode); }

        break;

      case 61: // element_type_decl


        { delete (yysym.value.elementType); }

        break;

      case 62: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 63: // field_decl


        { delete (yysym.value.field); }

        break;

      case 64: // procedure


        { delete (yysym.value.function); }

        break;

      case 65: // function


        { delete (yysym.value.function); }

        break;

      case 66: // function_header


        { delete (yysym.value.function); }

        break;

      case 67: // arguments


        { delete (yysym.value.Arguments); }

        break;

      case 68: // results


        { delete (yysym.value.Results); }

        break;

      case 69: // formal_list


        { delete (yysym.value.Formals); }

        break;

      case 70: // formal_decl


        { delete (yysym.value.Formal); }

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

      case 74: // return_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 75: // assign_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 76: // expr_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 77: // if_stmt


        { delete (yysym.value.IRNodes); }

        break;

      case 80: // expr


        { delete (yysym.value.expression); }

        break;

      case 81: // ident_expr


        { delete (yysym.value.expression); }

        break;

      case 82: // paren_expr


        { delete (yysym.value.expression); }

        break;

      case 83: // linear_algebra_expr


        { delete (yysym.value.indexExpr); }

        break;

      case 84: // elwise_binary_op


        {}

        break;

      case 85: // boolean_expr


        { delete (yysym.value.expression); }

        break;

      case 86: // field_read_expr


        { delete (yysym.value.fieldRead); }

        break;

      case 87: // set_read_expr


        { delete (yysym.value.expression); }

        break;

      case 88: // call_or_tensor_read_expr


        { delete (yysym.value.expression); }

        break;

      case 89: // call_expr


        { delete (yysym.value.call); }

        break;

      case 90: // actual_list


        { delete (yysym.value.expressions); }

        break;

      case 91: // expr_list


        { delete (yysym.value.expressions); }

        break;

      case 92: // range_expr


        { delete (yysym.value.expression); }

        break;

      case 93: // map_expr


        { delete (yysym.value.expression); }

        break;

      case 97: // write_expr_list


        { delete (yysym.value.writeinfos); }

        break;

      case 98: // write_expr


        { delete (yysym.value.writeinfo); }

        break;

      case 99: // field_write_expr


        { delete (yysym.value.fieldWrite); }

        break;

      case 100: // tensor_write_expr


        { delete (yysym.value.tensorWrite); }

        break;

      case 101: // type


        { delete (yysym.value.type); }

        break;

      case 102: // element_type


        { delete (yysym.value.elementType); }

        break;

      case 103: // set_type


        { delete (yysym.value.setType); }

        break;

      case 107: // tensor_type


        { delete (yysym.value.tensorType); }

        break;

      case 108: // nested_dimensions


        { delete (yysym.value.IndexDomains); }

        break;

      case 109: // dimensions


        { delete (yysym.value.IndexSets); }

        break;

      case 110: // dimension


        { delete (yysym.value.indexSet); }

        break;

      case 111: // component_type


        {}

        break;

      case 113: // tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 114: // dense_tensor_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 115: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 116: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 117: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 118: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 119: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 120: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 121: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 122: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 123: // scalar_literal


        { delete (yysym.value.TensorLiteral); }

        break;

      case 124: // test


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
    std::string ident = convertAndFree((yystack_[3].value.string));
    delete (yystack_[1].value.type);
  }

    break;

  case 11:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 12:

    {
    std::string ident = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 13:

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

  case 14:

    {
    (yylhs.value.fields) = new ElementType::FieldsMapType();
  }

    break;

  case 15:

    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yylhs.value.fields)->insert(*(yystack_[0].value.field));
    delete (yystack_[0].value.field);
  }

    break;

  case 16:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    std::shared_ptr<TensorType> tensorType = convertAndDelete((yystack_[1].value.tensorType));
    (yylhs.value.field) = new pair<string,shared_ptr<ir::TensorType>>(name, tensorType);
  }

    break;

  case 17:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    delete (yystack_[1].value.IRNodes);
  }

    break;

  case 18:

    {
    auto statements = unique_ptr<vector<shared_ptr<IRNode>>>((yystack_[1].value.IRNodes));
    (yylhs.value.function) = (yystack_[2].value.function);
    (yylhs.value.function)->addStatements(*statements);
    ctx->unscope();
  }

    break;

  case 19:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    auto arguments = unique_ptr<vector<shared_ptr<Argument>>>((yystack_[1].value.Arguments));
    auto results = unique_ptr<vector<shared_ptr<Result>>>((yystack_[0].value.Results));

    (yylhs.value.function) = new Function(ident, *arguments, *results);

    ctx->scope();

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

  case 20:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
  }

    break;

  case 21:

    {
    (yylhs.value.Arguments) = new vector<shared_ptr<Argument>>();
    for (auto formal : *(yystack_[1].value.Formals)) {
      auto result = new Argument(formal->name, formal->type);
      (yylhs.value.Arguments)->push_back(shared_ptr<Argument>(result));
    }
    delete (yystack_[1].value.Formals);
 }

    break;

  case 22:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    (yylhs.value.Results)->push_back(shared_ptr<Result>(new Result("asd", NULL)));
  }

    break;

  case 23:

    {
    (yylhs.value.Results) = new vector<shared_ptr<Result>>();
    for (auto formal : *(yystack_[1].value.Formals)) {
      auto result = new Result(formal->name, formal->type);
      (yylhs.value.Results)->push_back(shared_ptr<Result>(result));
    }
    delete (yystack_[1].value.Formals);
  }

    break;

  case 24:

    {
    (yylhs.value.Formals) = new simit::util::OwnershipVector<FormalData *>();
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 25:

    {
    (yylhs.value.Formals)->push_back((yystack_[0].value.Formal));
  }

    break;

  case 26:

    {
    std::string ident = convertAndFree((yystack_[2].value.string));
    std::shared_ptr<Type> type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.Formal) = new FormalData(ident, type);
  }

    break;

  case 27:

    {
    (yylhs.value.IRNodes) = new vector<shared_ptr<IRNode>>();
  }

    break;

  case 28:

    {
    (yylhs.value.IRNodes) = (yystack_[1].value.IRNodes);
    if ((yystack_[0].value.IRNodes) == NULL) break;  // TODO: Remove check
    (yylhs.value.IRNodes)->insert((yylhs.value.IRNodes)->end(), (yystack_[0].value.IRNodes)->begin(), (yystack_[0].value.IRNodes)->end());
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 34:

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

  case 35:

    {
    (yylhs.value.IRNodes) = NULL;
  }

    break;

  case 36:

    {
    auto lhsList = unique_ptr<vector<WriteInfo*>>((yystack_[3].value.writeinfos));
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
      unique_ptr<WriteInfo> lhs(*lhsIter);
      shared_ptr<Expression> &rhs = *rhsIter;

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
            result->setValue(rhs);
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

          if (auto result = dynamic_pointer_cast<Result>(fieldWrite->getSet())){
            result->setValue(fieldWrite);
            (yylhs.value.IRNodes)->push_back(fieldWrite);
          }

          break;
        }
        case WriteInfo::Kind::TENSOR: {
          break;
        }
      }
    }
  }

    break;

  case 37:

    {
    (yylhs.value.IRNodes) = NULL;
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
    delete (yystack_[3].value.expression);
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
    delete (yystack_[1].value.expression);
    delete (yystack_[0].value.IRNodes);
  }

    break;

  case 53:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    if (!ctx->hasSymbol(ident)) {
      // TODO: reintroduce error
      // REPORT_ERROR(ident + " is not defined in scope", @1);
      (yylhs.value.expression) = NULL;
      break;
    }

    const RWExprPair &rwExpr = ctx->getSymbol(ident);
    if (!rwExpr.isReadable()) {
      REPORT_ERROR(ident + " is not readable", yystack_[0].location);
    }

    (yylhs.value.expression) = new shared_ptr<Expression>(rwExpr.getReadExpr());
  }

    break;

  case 54:

    {
    if ((yystack_[1].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = (yystack_[1].value.expression);
  }

    break;

  case 55:

    {
    if ((yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check

    std::shared_ptr<Expression> expr = convertAndDelete((yystack_[0].value.expression));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(unaryElwiseExpr(IndexExpr::NEG, expr));
  }

    break;

  case 56:

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

  case 57:

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
      if (!ctx->isColumnVector(*l->getType())) {
        if (!ctx->isColumnVector(*r->getType())) {
          REPORT_ERROR("cannot multiply two row vectors", yystack_[1].location);
        }
        if (*l->getType() != *r->getType()) {
          REPORT_TYPE_MISSMATCH(*l->getType(), *r->getType(), yystack_[1].location);
        }
        (yylhs.value.indexExpr) = new shared_ptr<IndexExpr>(innerProduct(l, r));
      }
      // Outer product (l is a column vector)
      else {
        if (ctx->isColumnVector(*r->getType())) {
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

  case 58:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 59:

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
        if (!ctx->isColumnVector(*expr->getType())) {
          ctx->toggleColumnVector(*(*(yylhs.value.indexExpr))->getType());
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

  case 60:

    {
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 61:

    {  // Solve
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.indexExpr) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexExpr) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 62:

    { (yylhs.value.binop) = IndexExpr::ADD; }

    break;

  case 63:

    { (yylhs.value.binop) = IndexExpr::SUB; }

    break;

  case 64:

    { (yylhs.value.binop) = IndexExpr::MUL; }

    break;

  case 65:

    { (yylhs.value.binop) = IndexExpr::DIV; }

    break;

  case 66:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 67:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 68:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 69:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 70:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

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

  case 76:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 77:

    {
    (yylhs.value.expression) = NULL;
  }

    break;

  case 78:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto call = new Call(name, *(yystack_[1].value.expressions));
    delete (yystack_[1].value.expressions);
    (yylhs.value.call) = new std::shared_ptr<Call>(call);
  }

    break;

  case 79:

    {
    (yylhs.value.expressions) = new vector<shared_ptr<Expression>>();
  }

    break;

  case 80:

    {
    (yylhs.value.expressions) = (yystack_[0].value.expressions);
  }

    break;

  case 81:

    {
    (yylhs.value.expressions) = new std::vector<std::shared_ptr<Expression>>();
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 82:

    {
    (yylhs.value.expressions) = (yystack_[2].value.expressions);
    if ((yystack_[0].value.expression) == NULL) break;  // TODO: Remove check
    (yylhs.value.expressions)->push_back(*(yystack_[0].value.expression));
    delete (yystack_[0].value.expression);
  }

    break;

  case 83:

    {
    if ((yystack_[2].value.expression) == NULL || (yystack_[0].value.expression) == NULL) { (yylhs.value.expression) = NULL; break; } // TODO: Remove check
    (yylhs.value.expression) = NULL;
    delete (yystack_[2].value.expression);
    delete (yystack_[0].value.expression);
  }

    break;

  case 84:

    {
    string function((yystack_[4].value.string));
    string target((yystack_[2].value.string));
    free((void*)(yystack_[4].value.string));
    free((void*)(yystack_[2].value.string));
    (yylhs.value.expression) = NULL;
  }

    break;

  case 86:

    {
    std::string neighbor = convertAndFree((yystack_[0].value.string));
  }

    break;

  case 91:

    {
    (yylhs.value.writeinfos) = new vector<WriteInfo*>();
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back((yystack_[0].value.writeinfo));
  }

    break;

  case 92:

    {
    (yylhs.value.writeinfos) = (yystack_[2].value.writeinfos);
    if ((yystack_[0].value.writeinfo) == NULL) break;  // TODO: Remove check
    (yylhs.value.writeinfos)->push_back((yystack_[0].value.writeinfo));
  }

    break;

  case 93:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.writeinfo) = new WriteInfo(name);
  }

    break;

  case 94:

    {
    (yylhs.value.writeinfo) = new WriteInfo((yystack_[0].value.fieldWrite));
  }

    break;

  case 95:

    {
    (yylhs.value.writeinfo) = NULL;
  }

    break;

  case 96:

    {
    if ((yystack_[2].value.string) == NULL) { (yylhs.value.fieldWrite) = NULL; break; } // TODO: Remove check

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

    auto setExpr = shared_ptr<Expression>(setExprPair.getWriteExpr());
    (yylhs.value.fieldWrite) = new shared_ptr<FieldWrite>(new FieldWrite(setExpr, fieldName));
  }

    break;

  case 97:

    {
    (yylhs.value.fieldWrite) = NULL;
  }

    break;

  case 98:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    (yylhs.value.tensorWrite) = NULL;
  }

    break;

  case 99:

    {
    (yylhs.value.tensorWrite) = NULL;
  }

    break;

  case 100:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.elementType));
  }

    break;

  case 101:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.setType));
  }

    break;

  case 102:

    {
    (yylhs.value.type) = NULL;
  }

    break;

  case 103:

    {
    (yylhs.value.type) = reinterpret_cast<std::shared_ptr<ir::Type>*>((yystack_[0].value.tensorType));
  }

    break;

  case 104:

    {
    std::string name = convertAndFree((yystack_[0].value.string));
    (yylhs.value.elementType) = new std::shared_ptr<ElementType>(ctx->getElementType(name));
  }

    break;

  case 105:

    {
    std::shared_ptr<ElementType> elementType = convertAndDelete((yystack_[1].value.elementType));
    (yylhs.value.setType) = new std::shared_ptr<SetType>(new SetType(elementType));
  }

    break;

  case 106:

    {
    (yylhs.value.setType) = NULL;
  }

    break;

  case 107:

    {
  }

    break;

  case 108:

    {
  }

    break;

  case 109:

    {
  }

    break;

  case 110:

    {
  }

    break;

  case 111:

    {
  }

    break;

  case 112:

    {
    (yylhs.value.tensorType) = new std::shared_ptr<TensorType>(new TensorType((yystack_[0].value.componentType)));
  }

    break;

  case 113:

    {
    (yylhs.value.tensorType) = new std::shared_ptr<TensorType>(new TensorType((yystack_[1].value.componentType),
                                                        *(yystack_[3].value.IndexDomains)));
    delete (yystack_[3].value.IndexDomains);
  }

    break;

  case 114:

    {
    (yylhs.value.tensorType) = new std::shared_ptr<TensorType>(new TensorType((yystack_[2].value.componentType),
                                                        *(yystack_[4].value.IndexDomains)));
    ctx->toggleColumnVector(**(yylhs.value.tensorType));
    delete (yystack_[4].value.IndexDomains);
  }

    break;

  case 115:

    {
    (yylhs.value.IndexDomains) = new std::vector<IndexDomain>();
  }

    break;

  case 116:

    {
    (yylhs.value.IndexDomains) = (yystack_[3].value.IndexDomains);

    auto parentDims = (yylhs.value.IndexDomains);
    auto childDims = unique_ptr<std::vector<IndexSet>>((yystack_[1].value.IndexSets));

    // If there are no previous dimensions then create IndexDomains
    if (parentDims->size() == 0) {
      for (auto &dim : *childDims) {
        UNUSED(dim);
        parentDims->push_back(IndexDomain());
      }
    }

    // Handle case where there are more child than parent dimensions
    if (childDims->size() > parentDims->size()) {
      for (size_t i=0; i < childDims->size() - parentDims->size(); ++i) {
        size_t numNestings = (*parentDims)[0].getFactors().size();
        std::vector<IndexSet> indexSets(numNestings, IndexSet(1));
        parentDims->push_back(IndexDomain(indexSets));
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
    for (size_t i=0; i<(yylhs.value.IndexDomains)->size(); ++i) {
      (*parentDims)[i] = (*parentDims)[i] * IndexDomain((*childDims)[i]);
    }
  }

    break;

  case 117:

    {
    (yylhs.value.IndexSets) = new std::vector<IndexSet>();
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 118:

    {
    (yylhs.value.IndexSets) = (yystack_[2].value.IndexSets);
    (yylhs.value.IndexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 119:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 120:

    {
    string ident = convertAndFree((yystack_[0].value.string));
    (yylhs.value.indexSet) = new IndexSet(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 121:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 122:

    {
    (yylhs.value.componentType) = ComponentType::INT;
  }

    break;

  case 123:

    {
    (yylhs.value.componentType) = ComponentType::FLOAT;
  }

    break;

  case 126:

    {
    (yylhs.value.TensorLiteral) = (yystack_[1].value.TensorLiteral);
    ctx->toggleColumnVector(*(*(yylhs.value.TensorLiteral))->getType());
  }

    break;

  case 128:

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

  case 129:

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
    auto scalarType = new TensorType(ComponentType::INT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.num));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 147:

    {
    auto scalarType = new TensorType(ComponentType::FLOAT);
    auto literal = new Literal(std::shared_ptr<TensorType>(scalarType), &(yystack_[0].value.fnum));
    (yylhs.value.TensorLiteral) = new shared_ptr<Literal>(literal);
  }

    break;

  case 148:

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


  const short int  Parser ::yypact_ninf_ = -158;

  const signed char  Parser ::yytable_ninf_ = -94;

  const short int
   Parser ::yypact_[] =
  {
    -158,    10,  -158,  -158,  -158,    59,    11,    21,    64,   108,
     110,   118,    26,    42,   122,    26,     7,  -158,    26,  -158,
    -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,   233,   104,   114,  -158,  -158,   116,   121,  -158,  -158,
    -158,  -158,    44,  -158,   125,  -158,  -158,  -158,   124,  -158,
    -158,    26,   132,  -158,   140,   146,  -158,   135,   164,   158,
     253,   151,  -158,   161,   141,   213,  -158,  -158,   162,   159,
     157,   155,   168,   166,   170,   169,   176,    87,   115,    26,
      26,    26,  -158,  -158,  -158,    26,    26,  -158,  -158,    26,
    -158,    26,    26,    26,    26,    26,    26,   192,    26,   194,
      41,    26,    26,  -158,   253,   178,    28,  -158,    49,   153,
      37,   143,    33,   187,   210,    26,   171,    26,    30,  -158,
      32,    35,  -158,   188,   215,   217,  -158,   193,   214,   219,
    -158,  -158,   306,   306,   253,    87,    87,    87,   273,   293,
     293,   306,   306,   253,  -158,   196,   190,  -158,    -9,   114,
    -158,    90,    71,   191,  -158,    26,   195,  -158,  -158,  -158,
    -158,  -158,   197,  -158,  -158,   198,   226,   200,  -158,  -158,
    -158,  -158,  -158,   202,  -158,    73,  -158,   206,  -158,   218,
     220,   147,   216,   204,  -158,  -158,   215,   168,  -158,   214,
     176,  -158,  -158,    26,  -158,  -158,   253,   153,    31,    30,
     226,   203,    74,  -158,  -158,    37,  -158,   244,   244,   246,
     250,  -158,    26,  -158,  -158,  -158,    66,    67,    28,   231,
     172,    15,   234,   256,   287,  -158,   226,  -158,  -158,    75,
    -158,   107,  -158,   253,   199,  -158,  -158,  -158,   263,  -158,
    -158,  -158,    84,  -158,  -158,   265,  -158,  -158,  -158,  -158,
    -158,  -158,   199,   261,  -158,    15,   304,  -158,  -158,  -158,
     101,  -158,   307,  -158
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   146,   147,    53,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    38,     0,     3,
       7,     4,     5,     6,    27,     8,    29,    30,    31,    32,
      33,     0,    44,    45,    47,    48,    49,     0,    50,    75,
      51,    52,     0,    91,    94,    95,    46,   124,   125,   127,
       9,    79,     0,    14,     0,     0,    27,     0,     0,    53,
      27,    45,    35,     0,     0,     0,   144,   136,     0,     0,
     131,   130,   134,     0,   139,   138,   142,    55,     0,     0,
       0,     0,    37,    62,    63,     0,     0,    64,    65,     0,
      59,     0,     0,     0,     0,     0,     0,     0,    79,     0,
       0,     0,     0,   126,    81,     0,     0,    96,     0,     0,
       0,     0,     0,    22,     0,    79,    42,    79,     0,    54,
       0,     0,   128,     0,     0,     0,   129,     0,     0,     0,
      18,    28,    68,    69,    83,    57,    58,    60,    61,    66,
      67,    70,    71,    56,    97,     0,    80,    72,    93,     0,
      92,     0,     0,    76,    98,     0,     0,    13,    15,   122,
     123,   115,     0,   112,   104,     0,     0,     0,   100,   101,
     102,   103,    17,     0,    20,     0,    24,     0,    19,    85,
       0,     0,     0,     0,   132,   140,     0,   135,   137,     0,
     143,   145,    77,     0,    36,    99,    82,     0,     0,     0,
       0,   110,     0,   108,    10,     0,    21,     0,     0,     0,
      87,    39,     0,    27,    78,   148,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,    26,    25,     0,
      86,     0,    84,    27,    41,   133,   141,    16,     0,   119,
     120,   121,     0,   117,    34,   105,   111,   109,    23,    89,
      90,    88,    43,   113,   116,     0,     0,   114,   118,    11,
       0,   106,     0,    12
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,  -158,  -158,   106,   127,   -55,   330,  -158,  -158,  -158,
    -158,  -158,  -158,  -158,    -1,  -158,     3,  -158,  -158,  -158,
    -158,  -158,  -158,   318,   -43,   -49,  -158,  -158,  -158,  -158,
    -158,  -158,   235,  -158,  -158,   139,  -157,  -158,  -158,  -158,
     119,  -101,  -158,  -158,   102,   138,  -158,  -112,  -158,  -158,
    -158,   -61,   236,  -158,  -158,   -65,   237,  -158,  -158
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    19,    20,   260,    21,   108,   158,    22,    23,
      24,   113,   178,   175,   176,    78,   131,    26,    27,    28,
      29,    30,   180,   181,   104,    32,    61,    34,    96,    35,
      36,    37,    38,    39,   105,   146,    40,    41,   210,   232,
     251,    42,    43,    44,    45,   167,   168,   169,   170,   202,
     203,   171,   198,   242,   243,   163,    46,    47,    48,    69,
      70,    71,    72,    73,    74,    75,    76,    49,    50
  };

  const short int
   Parser ::yytable_[] =
  {
      31,   111,   106,   121,    33,   116,   183,   120,   162,   201,
       2,    60,    66,    67,    65,     3,     4,    77,     5,    53,
     239,   193,     6,   240,     7,     8,     9,    10,    11,    54,
      52,     3,     4,    12,    59,     3,     4,    13,    14,    68,
      15,   173,    16,   223,    11,   164,   159,   160,   161,   148,
     165,    17,   151,   152,    18,   145,    15,   156,    16,   154,
     241,   220,    16,   221,   174,   184,   155,   166,   185,   201,
      18,    15,    55,   124,   182,   157,   128,    31,   132,   133,
     134,    33,   100,    62,   135,   136,   101,   222,   137,    51,
     138,   139,   140,   141,   142,   143,   219,   -93,    52,   235,
     236,   -93,   195,   149,   206,   225,   248,   124,   128,   155,
      31,   207,   226,   207,    33,    31,    56,   254,    57,    33,
       3,     4,   255,     5,   217,   216,    58,    81,   155,     7,
      63,   194,   261,    11,    87,    88,    89,    90,    12,   262,
     107,   130,    13,   -73,   218,    15,    98,    16,     3,     4,
     249,     5,   250,    97,   196,   102,    17,     7,   234,    18,
      99,    11,   159,   160,   161,   112,    12,    66,    67,   172,
      13,   212,   213,    15,   103,    16,     3,     4,   252,     5,
     109,   159,   160,   114,    17,     7,   110,    18,   115,    11,
     -74,   117,   122,   118,    12,   123,   124,   -40,    13,   126,
     144,    15,   147,    16,     3,     4,   125,     5,   127,   153,
     128,   233,    17,     7,   129,    18,   177,    11,   179,    66,
     186,    67,    12,   188,   191,   189,    13,   192,   155,    15,
     -78,    16,   200,    31,   164,   197,   208,    33,   209,   199,
      17,   204,   205,    18,   119,   215,   211,   214,   224,    79,
      80,    31,   173,    81,   230,    33,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    79,
      80,   231,   237,    81,    82,   244,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    79,
      80,   245,   246,    81,   253,   256,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    79,
      80,   257,   259,    81,   229,   263,    83,    84,    85,    86,
      87,    88,    89,    90,   -94,    92,    93,    94,    95,    79,
      80,    25,    64,    81,   228,   150,    83,    84,    85,    86,
      87,    88,    89,    90,   227,   247,    81,    94,    95,    83,
      84,    85,    86,    87,    88,    89,    90,   258,   238,     0,
     187,     0,     0,     0,     0,   190
  };

  const short int
   Parser ::yycheck_[] =
  {
       1,    56,    51,    68,     1,    60,   118,    68,   109,   166,
       0,    12,     5,     6,    15,     5,     6,    18,     8,     8,
       5,    30,    12,     8,    14,    15,    16,    17,    18,     8,
      39,     5,     6,    23,     8,     5,     6,    27,    28,    32,
      30,     8,    32,   200,    18,     8,     9,    10,    11,     8,
      13,    41,   101,   102,    44,    98,    30,     8,    32,    31,
      45,    30,    32,    32,    31,    33,    38,    30,    33,   226,
      44,    30,     8,    41,   117,    26,    41,    78,    79,    80,
      81,    78,    38,    41,    85,    86,    42,   199,    89,    30,
      91,    92,    93,    94,    95,    96,   197,    38,    39,    33,
      33,    42,    31,   100,    31,    31,    31,    41,    41,    38,
     111,    38,    38,    38,   111,   116,     8,    33,     8,   116,
       5,     6,    38,     8,   189,   186,     8,    40,    38,    14,
       8,    41,    31,    18,    47,    48,    49,    50,    23,    38,
       8,    26,    27,    39,   193,    30,    30,    32,     5,     6,
      43,     8,    45,    39,   155,    30,    41,    14,   213,    44,
      39,    18,     9,    10,    11,    30,    23,     5,     6,    26,
      27,    24,    25,    30,    50,    32,     5,     6,   233,     8,
      40,     9,    10,    19,    41,    14,    40,    44,    30,    18,
      39,    30,    33,    52,    23,    38,    41,    26,    27,    33,
       8,    30,     8,    32,     5,     6,    38,     8,    38,    31,
      41,   212,    41,    14,    38,    44,    29,    18,     8,     5,
      32,     6,    23,     6,     5,    32,    27,    31,    38,    30,
      39,    32,    34,   234,     8,    40,    30,   234,    20,    42,
      41,    41,    40,    44,    31,    41,    26,    31,    45,    36,
      37,   252,     8,    40,     8,   252,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    36,
      37,    21,    41,    40,    41,    41,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    36,
      37,    35,     5,    40,    31,    30,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    36,
      37,    50,     8,    40,   208,     8,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    36,
      37,     1,    14,    40,   207,   100,    43,    44,    45,    46,
      47,    48,    49,    50,   205,   226,    40,    54,    55,    43,
      44,    45,    46,    47,    48,    49,    50,   255,   220,    -1,
     124,    -1,    -1,    -1,    -1,   128
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    57,     0,     5,     6,     8,    12,    14,    15,    16,
      17,    18,    23,    27,    28,    30,    32,    41,    44,    58,
      59,    61,    64,    65,    66,    72,    73,    74,    75,    76,
      77,    80,    81,    82,    83,    85,    86,    87,    88,    89,
      92,    93,    97,    98,    99,   100,   112,   113,   114,   123,
     124,    30,    39,     8,     8,     8,     8,     8,     8,     8,
      80,    82,    41,     8,    89,    80,     5,     6,    32,   115,
     116,   117,   118,   119,   120,   121,   122,    80,    71,    36,
      37,    40,    41,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    84,    39,    30,    39,
      38,    42,    30,    50,    80,    90,    91,     8,    62,    40,
      40,    71,    30,    67,    19,    30,    71,    30,    52,    31,
     117,   121,    33,    38,    41,    38,    33,    38,    41,    38,
      26,    72,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,     8,    90,    91,     8,     8,    82,
      98,    91,    91,    31,    31,    38,     8,    26,    63,     9,
      10,    11,   107,   111,     8,    13,    30,   101,   102,   103,
     104,   107,    26,     8,    31,    69,    70,    29,    68,     8,
      78,    79,    90,   113,    33,    33,    32,   118,     6,    32,
     122,     5,    31,    30,    41,    31,    80,    40,   108,    42,
      34,   102,   105,   106,    41,    40,    31,    38,    30,    20,
      94,    26,    24,    25,    31,    41,   117,   121,    91,   107,
      30,    32,   113,   102,    45,    31,    38,   101,    70,    69,
       8,    21,    95,    80,    71,    33,    33,    41,   111,     5,
       8,    45,   109,   110,    41,    35,     5,   106,    31,    43,
      45,    96,    71,    31,    33,    38,    30,    50,   110,     8,
      60,    31,    38,     8
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    56,    57,    57,    58,    58,    58,    58,    58,    58,
      59,    60,    60,    61,    62,    62,    63,    64,    65,    66,
      67,    67,    68,    68,    69,    69,    70,    71,    71,    72,
      72,    72,    72,    72,    73,    74,    75,    76,    76,    77,
      78,    78,    79,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    81,    82,    83,    83,    83,    83,    83,
      83,    83,    84,    84,    84,    84,    85,    85,    85,    85,
      85,    85,    86,    87,    87,    87,    88,    88,    89,    90,
      90,    91,    91,    92,    93,    94,    94,    95,    95,    96,
      96,    97,    97,    98,    98,    98,    99,    99,   100,   100,
     101,   101,   101,   101,   102,   103,   103,   104,   105,   105,
     106,   106,   107,   107,   107,   108,   108,   109,   109,   110,
     110,   110,   111,   111,   112,   113,   113,   113,   114,   114,
     115,   115,   116,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   124
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       5,     1,     3,     4,     0,     2,     4,     4,     3,     4,
       2,     3,     0,     4,     1,     3,     3,     0,     2,     1,
       1,     1,     1,     1,     7,     2,     4,     2,     1,     5,
       0,     3,     0,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     3,     2,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     4,     4,     4,     0,
       1,     1,     3,     3,     6,     0,     2,     0,     2,     1,
       1,     1,     3,     1,     1,     1,     3,     3,     4,     4,
       1,     1,     1,     1,     1,     4,     7,     3,     1,     3,
       1,     3,     1,     5,     6,     0,     4,     1,     3,     1,
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
  "\"identifier\"", "\"int\"", "\"float\"", "\"tensor\"", "\"element\"",
  "\"set\"", "\"const\"", "\"extern\"", "\"proc\"", "\"func\"", "\"map\"",
  "\"to\"", "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"",
  "\"else\"", "\"end\"", "\"return\"", "\"%!\"", "\"->\"", "\"(\"",
  "\")\"", "\"[\"", "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\",\"",
  "\".\"", "\":\"", "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"",
  "\".*\"", "\"./\"", "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"", "\"!=\"",
  "\"<=\"", "\">=\"", "$accept", "program", "program_element", "extern",
  "endpoints", "element_type_decl", "field_decl_list", "field_decl",
  "procedure", "function", "function_header", "arguments", "results",
  "formal_list", "formal_decl", "stmt_block", "stmt", "const_stmt",
  "return_stmt", "assign_stmt", "expr_stmt", "if_stmt", "else_clauses",
  "elif_clauses", "expr", "ident_expr", "paren_expr",
  "linear_algebra_expr", "elwise_binary_op", "boolean_expr",
  "field_read_expr", "set_read_expr", "call_or_tensor_read_expr",
  "call_expr", "actual_list", "expr_list", "range_expr", "map_expr",
  "with", "reduce", "reduction_op", "write_expr_list", "write_expr",
  "field_write_expr", "tensor_write_expr", "type", "element_type",
  "set_type", "tuple_type", "tuple_element_type_list",
  "tuple_element_type", "tensor_type", "nested_dimensions", "dimensions",
  "dimension", "component_type", "literal_expr", "tensor_literal",
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
       0,   237,   237,   239,   243,   244,   247,   256,   259,   263,
     270,   276,   279,   292,   306,   309,   317,   331,   338,   347,
     393,   396,   407,   411,   422,   426,   432,   441,   444,   453,
     454,   455,   456,   457,   461,   491,   497,   564,   567,   573,
     579,   581,   585,   587,   601,   602,   603,   604,   605,   606,
     607,   608,   609,   615,   636,   652,   660,   672,   737,   743,
     773,   778,   787,   788,   789,   790,   796,   802,   808,   814,
     820,   826,   841,   861,   862,   863,   874,   878,   884,   893,
     896,   902,   908,   919,   930,   938,   940,   944,   946,   949,
     950,   998,  1003,  1011,  1015,  1018,  1024,  1044,  1050,  1054,
    1080,  1083,  1086,  1089,  1095,  1102,  1106,  1111,  1115,  1117,
    1121,  1123,  1128,  1131,  1136,  1144,  1147,  1185,  1190,  1197,
    1200,  1204,  1209,  1212,  1281,  1284,  1285,  1289,  1292,  1301,
    1312,  1319,  1322,  1326,  1339,  1343,  1357,  1361,  1367,  1374,
    1377,  1381,  1394,  1398,  1412,  1416,  1422,  1427,  1437
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



