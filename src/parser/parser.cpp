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

    const_cast<ExprNodeBase*>(to<ExprNodeBase>(vec))->type = transposedVector;
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

      case 67: // extern


        { delete (yysym.value.var); }

        break;

      case 68: // element_type_decl


        { delete (yysym.value.type); }

        break;

      case 69: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 70: // field_decl


        { delete (yysym.value.field); }

        break;

      case 71: // procedure


        { delete (yysym.value.function); }

        break;

      case 74: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 75: // function


        { delete (yysym.value.function); }

        break;

      case 78: // function_header


        { delete (yysym.value.function); }

        break;

      case 79: // arguments


        { delete (yysym.value.vars); }

        break;

      case 80: // argument_list


        { delete (yysym.value.vars); }

        break;

      case 81: // argument_decl


        { delete (yysym.value.var); }

        break;

      case 82: // results


        { delete (yysym.value.vars); }

        break;

      case 83: // result_list


        { delete (yysym.value.vars); }

        break;

      case 84: // result_decl


        { delete (yysym.value.var); }

        break;

      case 85: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 90: // idents


        { delete (yysym.value.strings); }

        break;

      case 91: // with


        { delete (yysym.value.expr); }

        break;

      case 92: // reduce


        {}

        break;

      case 93: // reduce_op


        {}

        break;

      case 97: // if_body


        { delete (yysym.value.stmt); }

        break;

      case 98: // else_clauses


        { delete (yysym.value.stmt); }

        break;

      case 103: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 104: // expr


        { delete (yysym.value.expr); }

        break;

      case 105: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 106: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 107: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 108: // elwise_binary_op


        {}

        break;

      case 109: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 110: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 111: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 112: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 113: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 114: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 115: // type


        { delete (yysym.value.type); }

        break;

      case 116: // element_type


        { delete (yysym.value.type); }

        break;

      case 117: // set_type


        { delete (yysym.value.type); }

        break;

      case 118: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 119: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 120: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 121: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 122: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 123: // component_type


        { delete (yysym.value.scalarType); }

        break;

      case 124: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 125: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 126: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 127: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 128: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 129: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 130: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 131: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 132: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 133: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 134: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 135: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 136: // signed_int_literal


        {}

        break;

      case 137: // signed_float_literal


        {}

        break;

      case 139: // system_generator


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
   
    Expr cond = convertAndDelete((yystack_[3].value.expr));
    ctx->scope();
    Stmt trueStmt = convertAndDelete((yystack_[2].value.stmt));
    Stmt elseStmt = convertAndDelete((yystack_[1].value.stmt));
    ctx->addStatement(IfThenElse::make(cond, trueStmt, elseStmt));
  }

    break;

  case 58:

    {
    ctx->scope();
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 59:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 60:

    {
    ctx->unscope();
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 62:

    {
    string varName = convertAndFree((yystack_[2].value.string));
    Var var(varName, Int);
    ctx->scope();

    // If we need to write to loop variables, then that should be added as a
    // separate loop structure (that can't be vectorized easily)
    ctx->addSymbol(varName, var, Symbol::Read);
  }

    break;

  case 63:

    {
    ctx->unscope();
  }

    break;

  case 64:

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

  case 65:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 66:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 74:

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

  case 75:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 76:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 77:

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

  case 78:

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

    if (ltype->order()==0 || rtype->order()==0) {
      (yylhs.value.expr) = new Expr(builder->binaryElwiseExpr(l, IRBuilder::Div, r));
    }
    else {
      REPORT_ERROR("division not supported for these tensor types" , yystack_[2].location);
    }
  }

    break;

  case 80:

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

  case 81:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 82:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 83:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 84:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 85:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 86:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 87:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Eq::make(l, r));
  }

    break;

  case 88:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ne::make(l, r));
  }

    break;

  case 89:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Gt::make(l, r));
  }

    break;

  case 90:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Lt::make(l, r));
  }

    break;

  case 91:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ge::make(l, r));
  }

    break;

  case 92:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Le::make(l, r));
  }

    break;

  case 93:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(And::make(l, r));
  }

    break;

  case 94:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Or::make(l, r));
  }

    break;

  case 95:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Not::make(r));
  }

    break;

  case 96:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Xor::make(l, r));
  }

    break;

  case 97:

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

  case 101:

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

  case 102:

    {
    (yylhs.value.expr) = NULL;
  }

    break;

  case 103:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 104:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 105:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 106:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 111:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 112:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 113:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 114:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 115:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 116:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 117:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 118:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 119:

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

  case 120:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 121:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 122:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 123:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 124:

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

  case 125:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 126:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 127:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 128:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 131:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 133:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 134:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 135:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 137:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 138:

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

  case 139:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 140:

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

  case 141:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 142:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 143:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 145:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 146:

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

  case 147:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 148:

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

  case 149:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 150:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 151:

    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Int));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }

    break;

  case 152:

    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Float));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }

    break;

  case 153:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 154:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 155:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 156:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 157:

    {
    std::string name = convertAndFree((yystack_[6].value.string));
    auto actuals = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    auto expected = convertAndDelete((yystack_[1].value.expr));

    std::vector<Expr> literalArgs;
    literalArgs.reserve(actuals->size());
    for (auto &arg : *actuals) {
      if (!isa<Literal>(arg)) {
        REPORT_ERROR("function calls in tests must have literal arguments", yystack_[7].location);
      }
      literalArgs.push_back(arg);
    }

    std::vector<Expr> expecteds;
    expecteds.push_back(expected);
    ctx->addTest(new FunctionTest(name, literalArgs, expecteds));
  }

    break;

  case 158:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 160:

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


  const short int  Parser ::yypact_ninf_ = -212;

  const signed char  Parser ::yytable_ninf_ = -125;

  const short int
   Parser ::yypact_[] =
  {
    -212,   124,  -212,  -212,  -212,   125,    82,    97,   100,   229,
     126,   127,   229,    72,  -212,   245,   229,  -212,  -212,  -212,
    -212,   130,  -212,   155,  -212,  -212,  -212,    77,  -212,  -212,
    -212,  -212,   221,  -212,  -212,   285,   102,   146,  -212,  -212,
     161,   162,   164,  -212,  -212,   160,  -212,  -212,  -212,  -212,
     229,   195,  -212,   172,   175,   176,   188,   405,   148,   190,
     196,    -4,   261,    56,    18,   192,   191,   194,   198,   204,
     200,   199,   205,  -212,  -212,  -212,  -212,   474,   405,   239,
     221,   241,   221,   247,   225,   228,   221,  -212,   229,   229,
    -212,  -212,  -212,   229,   229,  -212,  -212,   229,  -212,   229,
     229,   229,   229,   229,   229,   229,   229,   229,   229,   251,
    -212,   405,   227,     9,   218,     6,   187,   168,  -212,   229,
    -212,   240,   229,    12,   229,    79,  -212,     0,    31,  -212,
    -212,  -212,   232,     7,     7,  -212,   234,    21,    21,  -212,
     242,   237,   244,  -212,   266,   309,  -212,  -212,  -212,   470,
     470,   474,   474,   474,   429,   453,   453,   470,   470,   405,
     405,   405,   405,   243,    68,  -212,  -212,   230,   229,   229,
     236,  -212,  -212,  -212,  -212,   167,    26,  -212,  -212,   246,
     273,  -212,  -212,  -212,  -212,   231,   248,   221,   256,    22,
     181,  -212,   333,  -212,   253,  -212,  -212,   250,  -212,  -212,
       7,   282,   198,  -212,    21,   286,   205,  -212,  -212,   100,
    -212,   274,  -212,  -212,  -212,   229,   405,   357,   187,   187,
      12,   115,  -212,   273,   249,  -212,  -212,   229,   267,   288,
      92,   105,  -212,   264,   258,  -212,  -212,   294,   381,  -212,
      -5,   -19,   128,  -212,   259,   289,   299,   405,   115,   263,
    -212,  -212,  -212,   296,   100,   305,  -212,  -212,  -212,   297,
      12,  -212,   313,   316,   307,   344,   321,  -212,  -212,   362,
     352,   187,  -212,   367,  -212,  -212,   332,   370,  -212,   348,
    -212,   -13,  -212,   111,   115,   355,   150,  -212,  -212,  -212,
    -212,  -212,   391,   368,   168,  -212,   370,  -212,   115,  -212,
    -212,  -212
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   153,   155,    74,     0,     0,     0,     0,
       0,     0,     0,     0,    66,     0,     0,     3,     7,     4,
       5,     0,     6,     0,     8,    37,    38,     0,    39,    40,
      41,    42,    33,    43,    44,     0,    67,    68,    70,    71,
      72,     0,    73,    69,   129,   130,   132,   151,   152,     9,
     103,     0,    12,     0,     0,     0,    74,     0,    71,    72,
       0,     0,     0,     0,     0,     0,   136,   135,   139,     0,
     144,   143,   147,   149,   141,   153,   155,    76,    95,     0,
      33,     0,    33,     0,     0,     0,    34,    35,     0,     0,
      65,    83,    84,     0,     0,    85,    86,     0,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   103,     0,
     131,   105,     0,     0,     0,     0,     0,     0,    10,   103,
      58,    59,   103,     0,   103,     0,    75,     0,     0,   154,
     156,   133,     0,     0,     0,   134,     0,     0,     0,    18,
       0,     0,     0,    48,     0,     0,    63,    61,    36,    90,
      89,    78,    79,    81,    82,    87,    88,    92,    91,    93,
      94,    96,    77,     0,     0,    97,   101,     0,     0,     0,
       0,    11,    13,   127,   128,     0,     0,   117,   111,     0,
       0,    27,   107,   108,   109,   110,   104,    33,     0,   153,
      74,   126,     0,    62,     0,   160,   159,     0,   137,   145,
       0,     0,   140,   142,     0,     0,   148,   150,    16,    23,
      20,     0,    45,   102,    56,     0,   106,     0,     0,     0,
       0,     0,   120,     0,     0,    60,    57,     0,     0,     0,
       0,     0,    17,     0,    24,    25,    21,     0,     0,    54,
       0,     0,     0,   121,     0,     0,     0,   125,     0,     0,
     158,   138,   146,    28,     0,    49,    55,    14,   118,     0,
       0,    64,   112,     0,     0,     0,     0,    22,    26,     0,
      51,     0,   122,     0,   116,   157,     0,     0,    50,     0,
      46,     0,   114,     0,     0,     0,     0,    30,    53,    52,
     119,   113,     0,     0,     0,    29,     0,   115,     0,    32,
      31,   161
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,
    -212,  -212,  -212,  -212,  -212,  -212,  -212,  -191,  -212,  -212,
     121,   -50,  -212,    14,  -212,  -212,  -212,  -212,  -212,  -212,
    -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,  -212,
      -9,  -212,  -212,  -212,  -212,   392,    23,  -212,  -212,     8,
     -39,   129,  -159,  -212,  -212,  -212,  -107,  -212,  -189,  -212,
    -212,  -211,  -212,  -212,  -212,   -58,   292,  -212,  -212,   -59,
     284,  -212,   -12,   -11,  -212,  -212,  -212
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    17,    18,    19,   115,   172,    20,    21,   232,
      80,    22,    23,   236,    82,   233,   234,    55,   267,   286,
     287,    85,    86,    87,    25,    26,    27,   270,   280,   289,
      28,    29,    30,   121,   188,    31,    32,   147,    33,    34,
      35,    36,    37,    38,   107,    39,    59,    41,    42,   112,
     186,   181,   182,   183,   283,   184,   185,   242,   193,   177,
      43,    44,    45,    65,    66,    67,    68,    69,    70,    71,
      72,    46,    47,    48,    49,   197,   250
  };

  const short int
   Parser ::yytable_[] =
  {
      57,    73,    74,    62,   128,   127,    77,    78,   120,   176,
     244,   113,     4,   170,   258,    24,   189,     4,   235,   190,
     290,   224,   129,   130,    40,     3,  -123,  -123,   124,  -123,
     140,   243,   142,   222,   171,   198,  -123,   264,   257,   222,
     125,   111,   167,   133,    12,  -123,    13,   222,  -123,   168,
    -123,    73,    74,   201,  -123,    40,  -123,  -123,    15,   191,
       3,     4,  -123,   268,   245,  -123,   199,   205,  -123,   164,
     221,   272,    16,   293,   137,   145,     3,     4,   222,   149,
     150,    40,  -123,   195,   151,   152,   196,   301,   153,    52,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   111,
     148,   214,    64,    40,    53,    40,    63,    54,   168,    40,
     111,   240,   241,   111,   192,   111,   163,    83,    64,     3,
       4,    84,    74,   203,     2,    73,   207,   251,     3,     4,
     163,     5,   194,    60,    61,   133,     6,   225,     7,     8,
     252,   -19,   230,   -98,   291,   231,    79,     9,   137,    13,
      10,   292,     3,     4,    11,     5,    12,    50,    13,   216,
     217,    64,     7,   259,   281,   -47,    51,    14,   260,   -47,
      15,     9,    81,   -33,    10,   178,   -33,   173,   174,   175,
      12,   179,    13,   295,    16,  -124,  -124,   -99,  -124,    74,
     296,    14,    73,   108,    15,  -124,   173,   174,   175,   219,
     180,   220,   114,   109,  -124,  -100,   238,  -124,    16,  -124,
      40,   192,   110,   119,   116,  -124,  -124,   117,   247,   118,
     119,  -124,   122,   123,  -124,     3,     4,   131,     5,     3,
       4,   132,    56,     3,     4,     7,    56,   133,   134,   135,
     136,  -124,   137,   144,     9,   138,   139,    10,   141,    75,
      76,   192,    56,    12,   143,    13,   146,    12,   165,    13,
     166,    12,   169,    13,    14,   187,   200,    15,   204,   209,
     208,    15,   210,   211,   215,    15,   213,    12,   218,    13,
     178,    16,   223,   222,   226,    16,   228,   130,   168,    16,
     129,    15,   229,   237,   126,   249,   246,   253,   254,    88,
      89,   255,   261,   263,   265,    16,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   248,   106,    88,    89,   269,   262,   266,    90,   271,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   273,   106,    88,    89,   274,
     275,   276,   212,   277,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   278,
     106,    88,    89,   279,   282,   227,   284,   285,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   288,   106,    88,    89,   294,   297,   298,
     239,    58,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   300,   106,    88,
      89,   206,     0,   299,   256,   202,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,     0,   106,    88,    89,     0,     0,     0,     0,     0,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,     0,   106,    88,    89,     0,
       0,     0,     0,     0,    91,    92,    93,    94,    95,    96,
      97,    98,  -125,   100,   101,   102,   103,   104,   105,     0,
     106,    88,    89,     0,     0,     0,     0,     0,    91,    92,
      93,    94,    95,    96,    97,    98,     0,     0,     0,   102,
     103,   104,   105,     0,   106,    91,    92,    93,    94,    95,
      96,    97,    98,    95,    96,    97,    98,     0,   104,   105,
       0,   106,   104,   105,     0,   106
  };

  const short int
   Parser ::yycheck_[] =
  {
       9,    13,    13,    12,    63,    63,    15,    16,    58,   116,
     221,    50,     5,     7,    33,     1,     4,     5,   209,     7,
      33,   180,     4,     5,     1,     4,     4,     5,    32,     7,
      80,   220,    82,    52,    28,    35,    14,   248,    43,    52,
      44,    50,    33,    43,    32,    23,    34,    52,    26,    40,
      28,    63,    63,    46,    32,    32,    34,    35,    46,    47,
       4,     5,    40,   254,   223,    43,    35,    46,    46,   108,
      44,   260,    60,   284,    43,    84,     4,     5,    52,    88,
      89,    58,    60,     4,    93,    94,     7,   298,    97,     7,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      86,    33,    46,    80,     7,    82,    34,     7,    40,    86,
     119,   218,   219,   122,   123,   124,   108,    40,    46,     4,
       5,    44,   133,   134,     0,   137,   138,    35,     4,     5,
     122,     7,   124,     7,     7,    43,    12,   187,    14,    15,
      35,    17,   200,    41,    33,   204,    16,    23,    43,    34,
      26,    40,     4,     5,    30,     7,    32,    32,    34,   168,
     169,    46,    14,    35,   271,    40,    41,    43,    40,    44,
      46,    23,    17,    25,    26,     7,    28,     9,    10,    11,
      32,    13,    34,    33,    60,     4,     5,    41,     7,   200,
      40,    43,   204,    32,    46,    14,     9,    10,    11,    32,
      32,    34,     7,    41,    23,    41,   215,    26,    60,    28,
     187,   220,    52,    32,    42,    34,    35,    42,   227,    43,
      32,    40,    32,    27,    43,     4,     5,    35,     7,     4,
       5,    40,     7,     4,     5,    14,     7,    43,    40,    35,
      40,    60,    43,    18,    23,    40,     7,    26,     7,     4,
       5,   260,     7,    32,     7,    34,    28,    32,     7,    34,
      33,    32,    44,    34,    43,    25,    34,    46,    34,    32,
      28,    46,    28,     7,    44,    46,    33,    32,    42,    34,
       7,    60,    36,    52,    28,    60,    33,     5,    40,    60,
       4,    46,    42,    19,    33,     7,    47,    33,    40,    38,
      39,     7,    43,     4,    41,    60,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    54,    61,    38,    39,    20,    37,    31,    43,    32,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    32,    61,    38,    39,    33,
      43,     7,    43,    32,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,     7,
      61,    38,    39,    21,     7,    42,    44,     7,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    45,    61,    38,    39,    42,     7,    31,
      43,     9,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,   296,    61,    38,
      39,   137,    -1,   294,    43,   133,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    -1,    61,    38,    39,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    -1,    61,    38,    39,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      61,    38,    39,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    48,    49,    50,    51,    52,    -1,    -1,    -1,    56,
      57,    58,    59,    -1,    61,    45,    46,    47,    48,    49,
      50,    51,    52,    49,    50,    51,    52,    -1,    58,    59,
      -1,    61,    58,    59,    -1,    61
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    65,     0,     4,     5,     7,    12,    14,    15,    23,
      26,    30,    32,    34,    43,    46,    60,    66,    67,    68,
      71,    72,    75,    76,    87,    88,    89,    90,    94,    95,
      96,    99,   100,   102,   103,   104,   105,   106,   107,   109,
     110,   111,   112,   124,   125,   126,   135,   136,   137,   138,
      32,    41,     7,     7,     7,    81,     7,   104,   109,   110,
       7,     7,   104,    34,    46,   127,   128,   129,   130,   131,
     132,   133,   134,   136,   137,     4,     5,   104,   104,    16,
      74,    17,    78,    40,    44,    85,    86,    87,    38,    39,
      43,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    61,   108,    32,    41,
      52,   104,   113,   114,     7,    69,    42,    42,    43,    32,
      85,    97,    32,    27,    32,    44,    33,   129,   133,     4,
       5,    35,    40,    43,    40,    35,    40,    43,    40,     7,
      85,     7,    85,     7,    18,   104,    28,   101,    87,   104,
     104,   104,   104,   104,   104,   104,   104,   104,   104,   104,
     104,   104,   104,   113,   114,     7,    33,    33,    40,    44,
       7,    28,    70,     9,    10,    11,   120,   123,     7,    13,
      32,   115,   116,   117,   119,   120,   114,    25,    98,     4,
       7,    47,   104,   122,   113,     4,     7,   139,    35,    35,
      34,    46,   130,   137,    34,    46,   134,   136,    28,    32,
      28,     7,    43,    33,    33,    44,   104,   104,    42,    32,
      34,    44,    52,    36,   116,    85,    28,    42,    33,    42,
     129,   133,    73,    79,    80,    81,    77,    19,   104,    43,
     120,   120,   121,   122,   125,   116,    47,   104,    54,     7,
     140,    35,    35,    33,    40,     7,    43,    43,    33,    35,
      40,    43,    37,     4,   125,    41,    31,    82,    81,    20,
      91,    32,   122,    32,    33,    43,     7,    32,     7,    21,
      92,   120,     7,   118,    44,     7,    83,    84,    45,    93,
      33,    33,    40,   125,    42,    33,    40,     7,    31,   115,
      84,   125
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    64,    65,    65,    66,    66,    66,    66,    66,    66,
      67,    68,    69,    69,    70,    72,    73,    71,    74,    76,
      77,    75,    78,    79,    79,    80,    80,    81,    82,    82,
      83,    83,    84,    85,    85,    86,    86,    87,    87,    87,
      87,    87,    87,    87,    87,    88,    89,    90,    90,    91,
      91,    92,    92,    93,    94,    95,    95,    96,    97,    98,
      98,    99,   100,   101,   102,   103,   103,   104,   104,   104,
     104,   104,   104,   104,   105,   106,   107,   107,   107,   107,
     107,   107,   107,   108,   108,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   110,   111,   111,
     111,   112,   112,   113,   113,   114,   114,   115,   115,   115,
     115,   116,   117,   117,   118,   118,   119,   120,   120,   120,
     120,   121,   121,   122,   122,   122,   122,   123,   123,   124,
     125,   125,   125,   126,   126,   127,   127,   128,   128,   129,
     129,   130,   130,   131,   131,   132,   132,   133,   133,   134,
     134,   135,   135,   136,   136,   137,   137,   138,   138,   139,
     139,   140
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     8,     1,     3,     0,
       2,     0,     2,     1,     6,     7,     4,     5,     1,     0,
       2,     3,     4,     1,     7,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     3,     3,     3,
       2,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     3,     1,     1,
       1,     4,     4,     0,     1,     1,     3,     1,     1,     1,
       1,     1,     4,     7,     1,     3,     5,     1,     4,     7,
       2,     1,     3,     1,     1,     3,     1,     1,     1,     1,
       1,     2,     1,     3,     3,     1,     1,     3,     5,     1,
       3,     1,     3,     1,     1,     3,     5,     1,     3,     1,
       3,     1,     1,     1,     2,     1,     2,     8,     6,     1,
       1,     7
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
  "\"!=\"", "\"<=\"", "\">=\"", "\"and\"", "\"or\"", "\"not\"", "\"xor\"",
  "\"true\"", "\"false\"", "$accept", "program", "program_element",
  "extern", "element_type_decl", "field_decl_list", "field_decl",
  "procedure", "$@1", "$@2", "procedure_header", "function", "$@3", "$@4",
  "function_header", "arguments", "argument_list", "argument_decl",
  "results", "result_list", "result_decl", "stmt_block", "stmts", "stmt",
  "assign_stmt", "map_stmt", "idents", "with", "reduce", "reduce_op",
  "field_write_stmt", "tensor_write_stmt", "if_stmt", "if_body",
  "else_clauses", "for_stmt", "for_stmt_header", "for_stmt_footer",
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
       0,   276,   276,   278,   281,   282,   290,   298,   299,   300,
     305,   315,   328,   331,   339,   349,   349,   349,   357,   374,
     374,   374,   382,   403,   406,   412,   417,   425,   434,   437,
     443,   448,   456,   466,   469,   476,   477,   480,   481,   482,
     483,   484,   485,   486,   487,   492,   533,   585,   589,   596,
     599,   612,   615,   621,   626,   646,   666,   672,   683,   690,
     693,   706,   709,   720,   725,   753,   756,   764,   765,   766,
     767,   768,   769,   770,   776,   796,   805,   813,   832,   900,
     920,   948,   953,   962,   963,   964,   965,   971,   977,   983,
     989,   995,  1001,  1007,  1013,  1019,  1024,  1035,  1069,  1070,
    1071,  1077,  1110,  1132,  1135,  1141,  1147,  1158,  1159,  1160,
    1161,  1165,  1177,  1181,  1191,  1200,  1212,  1224,  1228,  1231,
    1273,  1283,  1288,  1296,  1299,  1313,  1319,  1325,  1328,  1378,
    1382,  1383,  1387,  1391,  1398,  1409,  1416,  1420,  1424,  1438,
    1442,  1457,  1461,  1468,  1475,  1479,  1483,  1497,  1501,  1516,
    1520,  1527,  1531,  1538,  1541,  1547,  1550,  1557,  1576,  1600,
    1601,  1609
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
      55,    56,    57,    58,    59,    60,    61,    62,    63
    };
    const unsigned int user_token_number_max_ = 318;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



