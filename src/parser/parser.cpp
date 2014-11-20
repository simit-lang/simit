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

  void Parser::error(const Parser::location_type &loc,
                           const std::string &msg) {
    errors->push_back(ParseError(loc.begin.line, loc.begin.column,
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
    iassert(vec.type().isTensor());
    const TensorType *ttype = vec.type().toTensor();
    iassert(ttype->order() == 1);

    Type transposedVector = TensorType::make(ttype->componentType,
                                             ttype->dimensions,
                                             !ttype->isColumnVector);

    const_cast<ExprNodeBase*>(vec.expr())->type = transposedVector;
  }

  bool compare(const Type &l, const Type &r, ProgramContext *ctx) {
    if (l.kind() != r.kind()) {
      return false;
    }

//    if (l.isTensor()) {
//      if (l.toTensor()->isColumnVector != r.toTensor()->isColumnVector) {
//        return false;
//      }
//    }

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
      iassert(lt.isTensor() && rt.isTensor());    \
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
   Parser :: Parser  (Scanner *scanner_yyarg, ProgramContext *ctx_yyarg, std::vector<ParseError> *errors_yyarg)
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

      case 84: // idents


        { delete (yysym.value.strings); }

        break;

      case 85: // with


        { delete (yysym.value.expr); }

        break;

      case 86: // reduce


        {}

        break;

      case 87: // reduce_op


        {}

        break;

      case 97: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 98: // expr


        { delete (yysym.value.expr); }

        break;

      case 99: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 100: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 101: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 102: // elwise_binary_op


        {}

        break;

      case 103: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 104: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 105: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 106: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 107: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 108: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 109: // type


        { delete (yysym.value.type); }

        break;

      case 110: // element_type


        { delete (yysym.value.type); }

        break;

      case 111: // set_type


        { delete (yysym.value.type); }

        break;

      case 112: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 113: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 114: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 115: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 116: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 117: // component_type


        { delete (yysym.value.scalarType); }

        break;

      case 118: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 119: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 120: // dense_tensor_literal


        { delete (yysym.value.expr); }

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


        { delete (yysym.value.expr); }

        break;

      case 130: // signed_int_literal


        {}

        break;

      case 131: // signed_float_literal


        {}

        break;

      case 133: // system_generator


        { delete (yysym.value.system); }

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
    unique_ptr<vector<Field>> fields((yystack_[1].value.fields));

    if (ctx->containsElementType(name)) {
      REPORT_ERROR("struct redefinition (" + name + ")", yylhs.location);
    }

    ctx->addElementType(ElementType::make(name, *fields));
  }

    break;

  case 12:

    {
    (yylhs.value.fields) = new vector<Field>;
  }

    break;

  case 13:

    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yylhs.value.fields)->push_back(*(yystack_[0].value.field));
    delete (yystack_[0].value.field);
  }

    break;

  case 14:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto tensorType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.field) = new Field(name, tensorType);
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

      // TODO: Make extResult a mutable parameter
      arguments.push_back(ext);
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
      ctx->addSymbol(arg.getName(), arg, Symbol::Read);
      argNames.insert(arg.getName());
    }

    for (Var &res : *results) {
      Symbol::Access access = (argNames.find(res.getName()) != argNames.end())
                              ? Symbol::ReadWrite : Symbol::ReadWrite;
      ctx->addSymbol(res.getName(), res, access);
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

  case 45:

    {
    if ((yystack_[1].value.expr) == nullptr) {break;} // TODO: Remove check

    auto varNames = convertAndDelete((yystack_[3].value.strings));
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (varNames.size() > 1) {
      REPORT_ERROR("can only assign to one value in a non-map statement",
                   yystack_[3].location);
    }
    string varName = varNames[0];

    Var var;
    if (ctx->hasSymbol(varName)) {
      Symbol symbol = ctx->getSymbol(varName);

      if (!symbol.isWritable()) {
        REPORT_ERROR(varName + " is not writable", yystack_[3].location);
      }

      CHECK_TYPE_EQUALITY(symbol.getVar().getType(), value.type(), yystack_[1].location);

      var = symbol.getVar();
    }
    else {
      var = Var(varName, value.type());
      ctx->addSymbol(varName, var, Symbol::ReadWrite);
    }

    // TODO: This should be dealt with inside the ident_expr rule
    if (isa<VarExpr>(value) && value.type().isTensor()) {
      // The statement assign a tensor to a tensor, so we change it to an
      // assignment index expr
      value = ctx->getBuilder()->unaryElwiseExpr(IRBuilder::None, value);
    }

    ctx->addStatement(AssignStmt::make(var, value));
  }

    break;

  case 46:

    {
    auto varNames = unique_ptr<vector<string>>((yystack_[7].value.strings));

    string funcName = convertAndFree((yystack_[4].value.string));
    string targetsName = convertAndFree((yystack_[2].value.string));

    Expr neighbor = convertAndDelete((yystack_[1].value.expr));
    ReductionOperator reduction((yystack_[0].value.reductionop));

    if (!ctx->containsFunction(funcName)) {
      REPORT_ERROR("undefined function '" + funcName + "'", yystack_[4].location);
    }
    Func func = ctx->getFunction(funcName);

    if (varNames->size() != func.getResults().size()) {
      REPORT_ERROR("the number of variables (" + to_string(varNames->size()) +
                   ") does not match the number of results returned by " +
                   func.getName() + " (" + to_string(func.getResults().size()) +
                   ")", yystack_[7].location);
    }

    if (!ctx->hasSymbol(targetsName)) {
      REPORT_ERROR("undefined set '" + targetsName + "'", yystack_[2].location);
    }
    Expr targets = ctx->getSymbol(targetsName).getExpr();

    auto &results = func.getResults();
    vector<Var> vars;
    for (size_t i=0; i < results.size(); ++i) {
      string varName = (*varNames)[i];
      Var var;
      if (ctx->hasSymbol(varName)) {
        Symbol symbol = ctx->getSymbol(varName);

        if (!symbol.isWritable()) {
          REPORT_ERROR(varName + " is not writable", yystack_[7].location);
        }

        var = symbol.getVar();
      }
      else {
        var = Var(varName, results[i].getType());
        ctx->addSymbol(varName, var, Symbol::ReadWrite);
      }
      vars.push_back(var);
    }

    ctx->addStatement(Map::make(vars, func, targets, neighbor, reduction));
  }

    break;

  case 47:

    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 48:

    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 49:

    {
    (yylhs.value.expr) = new Expr();
  }

    break;

  case 50:

    {
    std::string neighborsName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(neighborsName)) {
      REPORT_ERROR("undefined set '" + neighborsName + "'", yystack_[0].location);
    }
    Expr neighbors = ctx->getSymbol(neighborsName).getExpr();

    (yylhs.value.expr) = new Expr(neighbors);
  }

    break;

  case 51:

    {
    (yylhs.value.reductionop) =  ReductionOperator::Undefined;
  }

    break;

  case 52:

    {
    (yylhs.value.reductionop) =  (yystack_[0].value.reductionop);
  }

    break;

  case 53:

    {
    (yylhs.value.reductionop) = ReductionOperator::Sum;
  }

    break;

  case 54:

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

  case 55:

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

  case 56:

    {
    // TODO
  }

    break;

  case 57:

    {
    delete (yystack_[3].value.expr);
    delete (yystack_[2].value.stmt);
  }

    break;

  case 59:

    {
    delete (yystack_[0].value.stmt);
  }

    break;

  case 61:

    {
    delete (yystack_[1].value.expr);
    delete (yystack_[0].value.stmt);
  }

    break;

  case 63:

    {
    string varName = convertAndFree((yystack_[2].value.string));
    Var var(varName, Int);
    ctx->scope();

    // If we need to write to loop variables, then that should be added as a
    // separate loop structure (that can't be vectorized easily)
    ctx->addSymbol(varName, var, Symbol::Read);
  }

    break;

  case 64:

    {
    ctx->unscope();
  }

    break;

  case 65:

    {
    std::string name = convertAndFree((yystack_[5].value.string));
    auto type = convertAndDelete((yystack_[3].value.type));
    const TensorType *tensorType = type.toTensor();

    Expr literalExpr = convertAndDelete((yystack_[1].value.expr));

    iassert(literalExpr.type().isTensor())
        << "Only tensor literals are currently supported";
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

  case 66:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 67:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 75:

    {
    string ident = convertAndFree((yystack_[0].value.string));

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

  case 76:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 77:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 78:

    {  // + - .* ./
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    if (ltype->order()>0&&rtype->order()>0 && !compare(l.type(),r.type(),ctx)) {
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
    }

    BINARY_ELWISE_TYPE_CHECK(l.type(), r.type(), yystack_[1].location);
    (yylhs.value.expr) = new Expr(ctx->getBuilder()->binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }

    break;

  case 79:

    {
    iassert((yystack_[2].value.expr) && (yystack_[0].value.expr));
    IRBuilder *builder = ctx->getBuilder();

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    // Scale
    if (ltype->order()==0 || rtype->order()==0) {
      (yylhs.value.expr) = new Expr(builder->binaryElwiseExpr(l, IRBuilder::Mul, r));
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
      // TODO: Figure out how column vectors should be handled here
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      // TODO: Figure out how column vectors should be handled here
      if (ltype->dimensions[0] != rtype->dimensions[0]) {
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

  case 80:

    {
    iassert((yystack_[2].value.expr) && (yystack_[0].value.expr));
    IRBuilder *builder = ctx->getBuilder();

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    if (ltype->order()==0 || rtype->order()==0) {
      (yylhs.value.expr) = new Expr(builder->binaryElwiseExpr(l, IRBuilder::Div, r));
    }
    else {
      REPORT_ERROR("division not supported for these tensor types" , yystack_[2].location);
    }
  }

    break;

  case 81:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    IRBuilder *builder = ctx->getBuilder();
    Expr expr = convertAndDelete((yystack_[1].value.expr));

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    const TensorType *type = expr.type().toTensor();
    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(IRBuilder::None, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(IRBuilder::None, expr));
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

  case 82:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 83:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 84:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 85:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 86:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 87:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 88:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 89:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 90:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 91:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 92:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 93:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 94:

    {
    iassert((yystack_[2].value.expr));
    iassert((yystack_[2].value.expr)->type().defined());

    Expr elemOrSet = convertAndDelete((yystack_[2].value.expr));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    Type type = elemOrSet.type();
    if (!(type.isElement() || type.isSet())) {
      std::stringstream errorStr;
      errorStr << "only elements and sets have fields";
      REPORT_ERROR(errorStr.str(), yystack_[2].location);
    }

    const ElementType *elemType = nullptr;
    if (elemOrSet.type().isElement()) {
      elemType = elemOrSet.type().toElement();
    }
    else if (elemOrSet.type().isSet()) {
      const SetType *setType = elemOrSet.type().toSet();
      elemType = setType->elementType.toElement();
    }
    iassert(elemType);

    if (!elemType->hasField(fieldName)) {
      REPORT_ERROR("undefined field '" + toString(elemOrSet)+"."+fieldName+ "'",
                   yystack_[0].location);
    }

    (yylhs.value.expr) = new Expr(FieldRead::make(elemOrSet, fieldName));
  }

    break;

  case 98:

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
      Func func = ctx->getFunction(name);
      (yylhs.value.expr) = new Expr(Call::make(func, *indices));
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 99:

    {
    (yylhs.value.expr) = NULL;
  }

    break;

  case 100:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 101:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 102:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 103:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 108:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 109:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 110:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 111:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 112:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
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
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
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
//      if (blockType->order() != outerDimensions->size()) {
//        REPORT_ERROR("Blocktype order (" + to_string(blockType->order()) +
//                     ") differ from number of dimensions", @index_sets);
//      }

//      iassert(blockDimensions.size() == outerDimensions->size());
      for (size_t i=0; i < outerDimensions->size(); ++i) {
        vector<IndexSet> dimension;
        dimension.push_back((*outerDimensions)[i]);
        dimension.insert(dimension.end(),
                         blockDimensions[i].getIndexSets().begin(),
                         blockDimensions[i].getIndexSets().end());

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
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 125:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
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
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 131:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
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
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Int));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }

    break;

  case 149:

    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Float));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }

    break;

  case 150:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 151:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 152:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 153:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 154:

    {
    std::string name = convertAndFree((yystack_[6].value.string));
    auto actuals = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
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
    ctx->addTest(new FunctionTest(name, literalArgs, expecteds));
  }

    break;

  case 155:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 157:

    {
    System *sys = new System;
    //sys->elements = new simit::Set<>;
    //createElements(sys->elements, $numElems);
    (yylhs.value.system) = sys;
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


  const short int  Parser ::yypact_ninf_ = -197;

  const signed char  Parser ::yytable_ninf_ = -122;

  const short int
   Parser ::yypact_[] =
  {
    -197,   130,  -197,  -197,  -197,   125,    18,    28,    30,   167,
      72,    99,   167,   109,  -197,   243,  -197,  -197,  -197,  -197,
      -1,  -197,   101,  -197,  -197,  -197,    56,  -197,  -197,  -197,
    -197,   220,  -197,  -197,   283,   145,   147,  -197,  -197,   171,
     163,   165,  -197,  -197,   159,  -197,  -197,  -197,  -197,   167,
     208,  -197,   174,   180,   192,   191,    13,   196,   214,   -18,
     263,    29,   127,   207,   209,   210,   211,   224,   221,   219,
     228,  -197,  -197,  -197,  -197,   187,   262,   220,   264,   220,
     266,   226,   242,   220,  -197,   167,   167,  -197,  -197,  -197,
     167,   167,  -197,  -197,   167,  -197,   167,   167,   167,   167,
     167,   167,   167,   269,  -197,   383,   245,    62,   235,     3,
     246,   185,  -197,   167,   173,   252,   167,    94,   167,   175,
    -197,     6,    84,  -197,  -197,  -197,   247,     8,     8,  -197,
     248,     4,     4,  -197,   256,   253,   259,  -197,   284,   303,
    -197,  -197,  -197,   436,   436,   187,   187,   187,   403,   423,
     423,   436,   436,   383,   257,    96,  -197,  -197,   249,   167,
     167,   255,  -197,  -197,  -197,  -197,   136,    81,  -197,  -197,
     268,   291,  -197,  -197,  -197,  -197,   254,   265,   271,   124,
     186,   260,  -197,   323,  -197,   274,  -197,  -197,   281,  -197,
    -197,     8,   319,   211,  -197,     4,   321,   228,  -197,  -197,
      30,  -197,   308,  -197,  -197,  -197,   167,   383,   343,   246,
     246,    94,   112,  -197,   291,   296,  -197,   167,   220,   167,
     290,   338,   140,   146,  -197,   314,   324,  -197,  -197,   356,
     363,  -197,    60,   -14,    80,  -197,   340,   329,   380,    13,
    -197,   383,   112,   326,  -197,  -197,  -197,   354,    30,   367,
    -197,  -197,  -197,   371,    94,  -197,   372,   374,  -197,   362,
     416,   392,  -197,  -197,   418,   405,   246,  -197,   420,  -197,
    -197,   399,   437,  -197,   400,  -197,    -4,  -197,   119,   112,
     404,   121,  -197,  -197,  -197,  -197,  -197,   440,   432,   185,
    -197,   437,  -197,   112,  -197,  -197,  -197
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   150,   152,    75,     0,     0,     0,     0,
       0,     0,     0,     0,    67,     0,     3,     7,     4,     5,
       0,     6,     0,     8,    37,    38,     0,    39,    40,    41,
      42,    33,    43,    44,     0,    68,    69,    71,    72,    73,
       0,    74,    70,   126,   127,   129,   148,   149,     9,   100,
       0,    12,     0,     0,     0,    75,    33,    73,     0,     0,
       0,     0,     0,     0,   133,   132,   136,     0,   141,   140,
     144,   146,   138,   150,   152,    77,     0,    33,     0,    33,
       0,     0,     0,    34,    35,     0,     0,    66,    84,    85,
       0,     0,    86,    87,     0,    81,     0,     0,     0,     0,
       0,     0,   100,     0,   128,   102,     0,     0,     0,     0,
       0,     0,    10,   100,    85,    60,   100,     0,   100,     0,
      76,     0,     0,   151,   153,   130,     0,     0,     0,   131,
       0,     0,     0,    18,     0,     0,     0,    48,     0,     0,
      64,    62,    36,    90,    91,    79,    80,    82,    83,    88,
      89,    92,    93,    78,     0,     0,    94,    98,     0,     0,
       0,     0,    11,    13,   124,   125,     0,     0,   114,   108,
       0,     0,    27,   104,   105,   106,   107,   101,     0,     0,
     150,    75,   123,     0,    63,     0,   157,   156,     0,   134,
     142,     0,     0,   137,   139,     0,     0,   145,   147,    16,
      23,    20,     0,    45,    99,    56,     0,   103,     0,     0,
       0,     0,     0,   117,     0,     0,    57,     0,    33,     0,
       0,     0,     0,     0,    17,     0,    24,    25,    21,     0,
       0,    54,     0,     0,     0,   118,     0,     0,     0,    33,
      59,   122,     0,     0,   155,   135,   143,    28,     0,    49,
      55,    14,   115,     0,     0,    65,   109,     0,    61,     0,
       0,     0,    22,    26,     0,    51,     0,   119,     0,   113,
     154,     0,     0,    50,     0,    46,     0,   111,     0,     0,
       0,     0,    30,    53,    52,   116,   110,     0,     0,     0,
      29,     0,   112,     0,    32,    31,   158
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -177,  -197,  -197,
     176,   -55,  -197,    11,  -197,  -197,  -197,  -197,  -197,  -197,
    -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,  -197,
      -9,  -197,  -197,  -197,  -197,  -197,     1,  -197,  -197,   -74,
     -28,   177,  -141,  -197,  -197,  -197,   -99,  -197,  -168,  -197,
    -197,  -196,  -197,  -197,  -197,   -52,   337,  -197,  -197,   -57,
     334,  -197,    -8,    -6,  -197,  -197,  -197
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    16,    17,    18,   109,   163,    19,    20,   224,
      77,    21,    22,   228,    79,   225,   226,    54,   262,   281,
     282,    82,    83,    84,    24,    25,    26,   265,   275,   284,
      27,    28,    29,   178,   179,    30,    31,   141,    32,    33,
      34,    35,    36,    37,   101,    38,    57,    40,    41,   106,
     177,   172,   173,   174,   278,   175,   176,   234,   184,   168,
      42,    43,    44,    63,    64,    65,    66,    67,    68,    69,
      70,    45,    46,    47,    48,   188,   244
  };

  const short int
   Parser ::yytable_[] =
  {
      56,   115,    39,    60,   122,    71,    75,    72,     3,   121,
     161,   167,    23,     4,   118,    76,   236,     3,     4,   252,
       5,   107,   134,   227,   136,    51,   119,     7,   154,   285,
     215,   162,    39,     3,     4,    52,     9,    53,   213,    10,
     105,   189,   154,   235,   185,    12,   259,    13,   213,   127,
     196,    85,    86,    71,   192,    72,    14,    39,    88,   114,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   263,   139,   237,   155,    62,   143,   144,    39,    58,
      39,   145,   146,   288,    39,   147,   267,   148,   149,   150,
     151,   152,   153,   105,   142,   158,    80,   296,   180,     4,
      81,   181,   159,   251,   105,    75,    59,   105,   183,   105,
     232,   233,   213,     3,     4,   253,     3,     4,    78,   190,
     254,    72,   194,    71,   198,   212,    12,   131,    13,   205,
       2,   123,   124,   213,     3,     4,   159,     5,   223,   222,
      15,   182,     6,    61,     7,     8,    13,   -19,   217,   218,
     207,   208,   286,     9,   290,    62,    10,    49,    62,   287,
      11,   291,    12,   240,    13,   -47,    50,   276,   210,   -47,
     211,     3,     4,    14,    55,   245,    15,    73,    74,   186,
      55,   246,   187,   127,   258,    72,   -95,    71,   -96,   131,
    -120,  -120,   169,  -120,   164,   165,   166,   230,   170,    12,
    -120,    13,   183,   102,   103,    12,   -97,    13,   239,  -120,
     241,   104,  -120,    15,  -120,   108,   110,   171,  -120,    39,
    -120,  -120,   111,   113,     3,     4,  -120,     5,   116,  -120,
       3,     4,  -120,    55,     7,   112,    92,    93,    94,    95,
      39,   117,   125,     9,   138,   183,    10,    73,    74,   126,
      55,   128,    12,   127,    13,   164,   165,   166,    12,   129,
      13,   130,   131,    14,  -121,  -121,    15,  -121,   132,   133,
     140,   135,    15,   137,  -121,    12,   156,    13,   157,   160,
     -58,   191,   195,  -121,   199,   200,  -121,   201,  -121,    15,
     204,   202,   113,   206,  -121,  -121,   120,   209,   169,   216,
    -121,    85,    86,  -121,   214,   159,   213,   220,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   221,   124,   123,    87,   229,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   238,   242,   243,   203,   247,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   249,   248,   219,   256,   260,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   255,   257,   261,   231,   264,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   266,   268,   270,   250,   269,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   271,   272,   273,   274,   277,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    85,    86,   279,   280,   283,   289,   292,    88,    89,
      90,    91,    92,    93,    94,    95,  -122,    97,    98,    99,
     100,    85,    86,   293,   193,   197,   294,   295,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,     0,    99,
     100,    88,    89,    90,    91,    92,    93,    94,    95
  };

  const short int
   Parser ::yycheck_[] =
  {
       9,    56,     1,    12,    61,    13,    15,    13,     4,    61,
       7,   110,     1,     5,    32,    16,   212,     4,     5,    33,
       7,    49,    77,   200,    79,     7,    44,    14,   102,    33,
     171,    28,    31,     4,     5,     7,    23,     7,    52,    26,
      49,    35,   116,   211,   118,    32,   242,    34,    52,    43,
      46,    38,    39,    61,    46,    61,    43,    56,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,   248,    81,   214,   102,    46,    85,    86,    77,     7,
      79,    90,    91,   279,    83,    94,   254,    96,    97,    98,
      99,   100,   101,   102,    83,    33,    40,   293,     4,     5,
      44,     7,    40,    43,   113,   114,     7,   116,   117,   118,
     209,   210,    52,     4,     5,    35,     4,     5,    17,    35,
      40,   127,   128,   131,   132,    44,    32,    43,    34,    33,
       0,     4,     5,    52,     4,     5,    40,     7,   195,   191,
      46,    47,    12,    34,    14,    15,    34,    17,    24,    25,
     159,   160,    33,    23,    33,    46,    26,    32,    46,    40,
      30,    40,    32,   218,    34,    40,    41,   266,    32,    44,
      34,     4,     5,    43,     7,    35,    46,     4,     5,     4,
       7,    35,     7,    43,   239,   191,    41,   195,    41,    43,
       4,     5,     7,     7,     9,    10,    11,   206,    13,    32,
      14,    34,   211,    32,    41,    32,    41,    34,   217,    23,
     219,    52,    26,    46,    28,     7,    42,    32,    32,   218,
      34,    35,    42,    32,     4,     5,    40,     7,    32,    43,
       4,     5,    46,     7,    14,    43,    49,    50,    51,    52,
     239,    27,    35,    23,    18,   254,    26,     4,     5,    40,
       7,    40,    32,    43,    34,     9,    10,    11,    32,    35,
      34,    40,    43,    43,     4,     5,    46,     7,    40,     7,
      28,     7,    46,     7,    14,    32,     7,    34,    33,    44,
      28,    34,    34,    23,    28,    32,    26,    28,    28,    46,
      33,     7,    32,    44,    34,    35,    33,    42,     7,    28,
      40,    38,    39,    43,    36,    40,    52,    33,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    42,     5,     4,    43,    19,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    47,    54,     7,    43,    33,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,     7,    40,    42,    37,    41,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    43,     4,    31,    43,    20,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    32,    32,    43,    43,    33,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,     7,    32,     7,    21,     7,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    44,     7,    45,    42,     7,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    38,    39,    31,   127,   131,   289,   291,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    56,
      57,    45,    46,    47,    48,    49,    50,    51,    52
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     4,     5,     7,    12,    14,    15,    23,
      26,    30,    32,    34,    43,    46,    60,    61,    62,    65,
      66,    69,    70,    81,    82,    83,    84,    88,    89,    90,
      93,    94,    96,    97,    98,    99,   100,   101,   103,   104,
     105,   106,   118,   119,   120,   129,   130,   131,   132,    32,
      41,     7,     7,     7,    75,     7,    98,   104,     7,     7,
      98,    34,    46,   121,   122,   123,   124,   125,   126,   127,
     128,   130,   131,     4,     5,    98,    16,    68,    17,    72,
      40,    44,    79,    80,    81,    38,    39,    43,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,   102,    32,    41,    52,    98,   107,   108,     7,    63,
      42,    42,    43,    32,    46,    79,    32,    27,    32,    44,
      33,   123,   127,     4,     5,    35,    40,    43,    40,    35,
      40,    43,    40,     7,    79,     7,    79,     7,    18,    98,
      28,    95,    81,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,   107,   108,     7,    33,    33,    40,
      44,     7,    28,    64,     9,    10,    11,   114,   117,     7,
      13,    32,   109,   110,   111,   113,   114,   108,    91,    92,
       4,     7,    47,    98,   116,   107,     4,     7,   133,    35,
      35,    34,    46,   124,   131,    34,    46,   128,   130,    28,
      32,    28,     7,    43,    33,    33,    44,    98,    98,    42,
      32,    34,    44,    52,    36,   110,    28,    24,    25,    42,
      33,    42,   123,   127,    67,    73,    74,    75,    71,    19,
      98,    43,   114,   114,   115,   116,   119,   110,    47,    98,
      79,    98,    54,     7,   134,    35,    35,    33,    40,     7,
      43,    43,    33,    35,    40,    43,    37,     4,    79,   119,
      41,    31,    76,    75,    20,    85,    32,   116,    32,    33,
      43,     7,    32,     7,    21,    86,   114,     7,   112,    44,
       7,    77,    78,    45,    87,    33,    33,    40,   119,    42,
      33,    40,     7,    31,   109,    78,   119
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    58,    59,    59,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    63,    64,    66,    67,    65,    68,    70,
      71,    69,    72,    73,    73,    74,    74,    75,    76,    76,
      77,    77,    78,    79,    79,    80,    80,    81,    81,    81,
      81,    81,    81,    81,    81,    82,    83,    84,    84,    85,
      85,    86,    86,    87,    88,    89,    89,    90,    91,    91,
      92,    92,    93,    94,    95,    96,    97,    97,    98,    98,
      98,    98,    98,    98,    98,    99,   100,   101,   101,   101,
     101,   101,   101,   101,   102,   102,   102,   102,   103,   103,
     103,   103,   103,   103,   104,   105,   105,   105,   106,   106,
     107,   107,   108,   108,   109,   109,   109,   109,   110,   111,
     111,   112,   112,   113,   114,   114,   114,   114,   115,   115,
     116,   116,   116,   116,   117,   117,   118,   119,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   125,   126,   126,   127,   127,   128,   128,   129,   129,
     130,   130,   131,   131,   132,   132,   133,   133,   134
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     8,     1,     3,     0,
       2,     0,     2,     1,     6,     7,     4,     5,     0,     3,
       0,     4,     3,     4,     1,     7,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     3,     3,
       3,     2,     3,     3,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     4,     4,
       0,     1,     1,     3,     1,     1,     1,     1,     1,     4,
       7,     1,     3,     5,     1,     4,     7,     2,     1,     3,
       1,     1,     3,     1,     1,     1,     1,     1,     2,     1,
       3,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       1,     2,     1,     2,     8,     6,     1,     1,     7
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
  "assign_stmt", "map_stmt", "idents", "with", "reduce", "reduce_op",
  "field_write_stmt", "tensor_write_stmt", "if_stmt", "else_clauses",
  "elif_clauses", "for_stmt", "for_stmt_header", "for_stmt_footer",
  "const_stmt", "expr_stmt", "expr", "ident_expr", "paren_expr",
  "linear_algebra_expr", "elwise_binary_op", "boolean_expr",
  "field_read_expr", "set_read_expr", "call_or_paren_read_expr",
  "expr_list_or_empty", "expr_list", "type", "element_type", "set_type",
  "endpoints", "tuple_type", "tensor_type", "index_sets", "index_set",
  "component_type", "literal_expr", "tensor_literal",
  "dense_tensor_literal", "float_dense_tensor_literal",
  "float_dense_ndtensor_literal", "float_dense_matrix_literal",
  "float_dense_vector_literal", "int_dense_tensor_literal",
  "int_dense_ndtensor_literal", "int_dense_matrix_literal",
  "int_dense_vector_literal", "scalar_literal", "signed_int_literal",
  "signed_float_literal", "test", "system_generator", "extern_assert", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   271,   271,   273,   276,   277,   285,   293,   294,   295,
     300,   310,   323,   326,   334,   344,   344,   344,   352,   369,
     369,   369,   377,   398,   401,   407,   412,   420,   429,   432,
     438,   443,   451,   461,   464,   471,   472,   475,   476,   477,
     478,   479,   480,   481,   482,   487,   528,   580,   584,   591,
     594,   607,   610,   616,   621,   641,   661,   667,   672,   674,
     678,   680,   686,   689,   700,   705,   733,   736,   744,   745,
     746,   747,   748,   749,   750,   756,   776,   785,   793,   812,
     880,   900,   928,   933,   942,   943,   944,   945,   951,   957,
     963,   969,   975,   981,   992,  1026,  1027,  1028,  1034,  1067,
    1089,  1092,  1098,  1104,  1115,  1116,  1117,  1118,  1122,  1134,
    1138,  1148,  1157,  1169,  1181,  1185,  1188,  1230,  1240,  1245,
    1253,  1256,  1270,  1276,  1282,  1285,  1335,  1339,  1340,  1344,
    1348,  1355,  1366,  1373,  1377,  1381,  1395,  1399,  1414,  1418,
    1425,  1432,  1436,  1440,  1454,  1458,  1473,  1477,  1484,  1488,
    1495,  1498,  1504,  1507,  1514,  1533,  1557,  1558,  1566
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



