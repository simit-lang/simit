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

      case 68: // extern


        { delete (yysym.value.var); }

        break;

      case 69: // element_type_decl


        { delete (yysym.value.type); }

        break;

      case 70: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 71: // field_decl


        { delete (yysym.value.field); }

        break;

      case 72: // procedure


        { delete (yysym.value.function); }

        break;

      case 75: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 76: // function


        { delete (yysym.value.function); }

        break;

      case 79: // function_header


        { delete (yysym.value.function); }

        break;

      case 80: // arguments


        { delete (yysym.value.vars); }

        break;

      case 81: // argument_list


        { delete (yysym.value.vars); }

        break;

      case 82: // argument_decl


        { delete (yysym.value.var); }

        break;

      case 83: // results


        { delete (yysym.value.vars); }

        break;

      case 84: // result_list


        { delete (yysym.value.vars); }

        break;

      case 85: // result_decl


        { delete (yysym.value.var); }

        break;

      case 86: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 91: // idents


        { delete (yysym.value.strings); }

        break;

      case 92: // with


        { delete (yysym.value.expr); }

        break;

      case 93: // reduce


        {}

        break;

      case 94: // reduce_op


        {}

        break;

      case 98: // while_stmt_header


        { delete (yysym.value.expr); }

        break;

      case 99: // while_body


        { delete (yysym.value.stmt); }

        break;

      case 101: // if_stmt


        { delete (yysym.value.stmt); }

        break;

      case 102: // if_body


        { delete (yysym.value.stmt); }

        break;

      case 103: // else_clauses


        { delete (yysym.value.stmt); }

        break;

      case 105: // for_stmt_header


        { delete (yysym.value.var); }

        break;

      case 108: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 110: // expr


        { delete (yysym.value.expr); }

        break;

      case 111: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 112: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 113: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 114: // elwise_binary_op


        {}

        break;

      case 115: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 116: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 117: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 118: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 119: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 120: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 121: // type


        { delete (yysym.value.type); }

        break;

      case 122: // element_type


        { delete (yysym.value.type); }

        break;

      case 123: // set_type


        { delete (yysym.value.type); }

        break;

      case 124: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 125: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 126: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 127: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 128: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 129: // component_type


        { delete (yysym.value.scalarType); }

        break;

      case 130: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 131: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 132: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 133: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 134: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 135: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 136: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 137: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 138: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 139: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 140: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 141: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 142: // signed_int_literal


        {}

        break;

      case 143: // signed_float_literal


        {}

        break;

      case 145: // system_generator


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

  case 47:

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

  case 48:

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

  case 49:

    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 50:

    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 51:

    {
    (yylhs.value.expr) = new Expr();
  }

    break;

  case 52:

    {
    std::string neighborsName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(neighborsName)) {
      REPORT_ERROR("undefined set '" + neighborsName + "'", yystack_[0].location);
    }
    Expr neighbors = ctx->getSymbol(neighborsName).getExpr();

    (yylhs.value.expr) = new Expr(neighbors);
  }

    break;

  case 53:

    {
    (yylhs.value.reductionop) =  ReductionOperator::Undefined;
  }

    break;

  case 54:

    {
    (yylhs.value.reductionop) =  (yystack_[0].value.reductionop);
  }

    break;

  case 55:

    {
    (yylhs.value.reductionop) = ReductionOperator::Sum;
  }

    break;

  case 56:

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

  case 57:

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

  case 58:

    {
    //TODO: This rule really should be:
    // field_read_expr "(" expr_list ")" "=" expr ";"
    // but this requires reworking a lot of the parser.
 
    string setName = convertAndFree((yystack_[8].value.string));
    string fieldName = convertAndFree((yystack_[6].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    if ((yystack_[1].value.expr) == nullptr) break;  // TODO: remove check
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (!ctx->hasSymbol(setName)) {
      REPORT_ERROR(setName + " is not defined in scope", yystack_[8].location);
    }

    const Symbol &setSymbol = ctx->getSymbol(setName);
    if (!setSymbol.isWritable()) {
      REPORT_ERROR(setName + " is not writable", yystack_[8].location);
    }

    Expr setExpr = setSymbol.getExpr();

    Expr tensorExpr = FieldRead::make(setExpr, fieldName);
    ctx->addStatement(TensorWrite::make(tensorExpr, *indices, value));
  }

    break;

  case 59:

    {
    Expr cond = convertAndDelete((yystack_[2].value.expr));
    Stmt body = convertAndDelete((yystack_[1].value.stmt));
    
    ctx->addStatement(While::make(cond, body));

  }

    break;

  case 60:

    {
    ctx->scope();
    (yylhs.value.expr) = new Expr(convertAndDelete((yystack_[0].value.expr)));
  }

    break;

  case 61:

    {

    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 62:

    {
    ctx->unscope();
  }

    break;

  case 63:

    {
    Expr cond = convertAndDelete((yystack_[3].value.expr));
    ctx->scope();
    Stmt trueStmt = convertAndDelete((yystack_[2].value.stmt));
    ctx->unscope();
    Stmt elseStmt = convertAndDelete((yystack_[1].value.stmt));
    Stmt *result = new Stmt(IfThenElse::make(cond, trueStmt, elseStmt));
    ctx->addStatement(*result);
    (yylhs.value.stmt) = result;
  }

    break;

  case 64:

    {
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 65:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 66:

    {
    ctx->scope();
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
    ctx->unscope();
  }

    break;

  case 67:

    {
    Expr cond = convertAndDelete((yystack_[2].value.expr));
    ctx->scope();
    Stmt trueStmt = convertAndDelete((yystack_[1].value.stmt));
    ctx->unscope();
    Stmt elseStmt = convertAndDelete((yystack_[0].value.stmt));
    Stmt *result = new Stmt(IfThenElse::make(cond, trueStmt, elseStmt));
    ctx->addStatement(*result);
    (yylhs.value.stmt) = result;
  }

    break;

  case 68:

    {    
    if((yystack_[2].value.indexSet)->getKind()==IndexSet::Set){
      ctx->addStatement(For::make(*(yystack_[3].value.var),ForDomain(*(yystack_[2].value.indexSet)), *(yystack_[1].value.stmt)));
    }
    delete (yystack_[3].value.var);
    delete (yystack_[2].value.indexSet);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 69:

    {    
    ctx->addStatement(ForRange::make(*(yystack_[5].value.var), *(yystack_[4].value.expr), *(yystack_[2].value.expr), *(yystack_[1].value.stmt)));
    delete (yystack_[5].value.var);
    delete (yystack_[4].value.expr);
    delete (yystack_[2].value.expr);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 70:

    {
 string varName = convertAndFree((yystack_[1].value.string));
 Var * var = new Var(varName, Int);     
 ctx->scope();
 // If we need to write to loop variables, then that should be added as a
 // separate loop structure (that can't be vectorized easily)
 ctx->addSymbol(varName, *var, Symbol::Read);

 (yylhs.value.var)=var;
 }

    break;

  case 71:

    {
    ctx->unscope();
  }

    break;

  case 72:

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

  case 73:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 74:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 75:

    {
     Expr expr = Expr(convertAndDelete((yystack_[1].value.expr)));
     ctx->addStatement(Stmt(Print::make(expr)));
  }

    break;

  case 83:

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

  case 84:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 85:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 86:

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

  case 87:

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

  case 88:

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

  case 89:

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

  case 90:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 91:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 92:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 93:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 94:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 95:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 96:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 97:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Eq::make(l, r));
  }

    break;

  case 98:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ne::make(l, r));
  }

    break;

  case 99:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Gt::make(l, r));
  }

    break;

  case 100:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Lt::make(l, r));
  }

    break;

  case 101:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ge::make(l, r));
  }

    break;

  case 102:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Le::make(l, r));
  }

    break;

  case 103:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(And::make(l, r));
  }

    break;

  case 104:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Or::make(l, r));
  }

    break;

  case 105:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Not::make(r));
  }

    break;

  case 106:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Xor::make(l, r));
  }

    break;

  case 107:

    {
    bool val = true;
    (yylhs.value.expr) = new Expr(Literal::make(TensorType::make(ScalarType::Boolean), &val));
  }

    break;

  case 108:

    {
    bool val = false;
    (yylhs.value.expr) = new Expr(Literal::make(TensorType::make(ScalarType::Boolean), &val));
  }

    break;

  case 109:

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

  case 113:

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

  case 114:

    {
    (yylhs.value.expr) = NULL;
  }

    break;

  case 115:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 116:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 117:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 118:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 123:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 124:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 125:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 126:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 127:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 128:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 129:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 130:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 131:

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

  case 132:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 133:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 134:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 135:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 136:

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

  case 137:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 138:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 139:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 142:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 144:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 145:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 146:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 148:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 149:

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

  case 150:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 151:

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

  case 152:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 153:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 154:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 156:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 157:

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

  case 158:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 159:

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

  case 160:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 161:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 162:

    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Int));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }

    break;

  case 163:

    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Float));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }

    break;

  case 164:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 165:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 166:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 167:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 168:

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

  case 169:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 171:

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


  const short int  Parser ::yypact_ninf_ = -222;

  const short int  Parser ::yytable_ninf_ = -137;

  const short int
   Parser ::yypact_[] =
  {
    -222,   184,  -222,  -222,  -222,    88,    27,    40,    65,   378,
     378,    76,   110,   378,   378,    33,  -222,   439,   378,  -222,
    -222,  -222,  -222,  -222,  -222,     4,  -222,   127,  -222,  -222,
    -222,    56,  -222,  -222,  -222,   365,  -222,  -222,   369,  -222,
    -222,  -222,   463,   132,   138,  -222,  -222,   113,   153,   155,
    -222,  -222,   106,  -222,  -222,  -222,  -222,   378,   188,  -222,
     162,   165,   172,   176,   691,   288,    28,   191,   -11,   485,
     520,   189,    26,   181,   190,   183,   192,   186,   199,   198,
     203,   201,  -222,  -222,  -222,  -222,   204,   691,   220,   365,
     243,   365,   246,   403,  -222,   365,  -222,   224,   331,   129,
    -222,   542,   365,   378,   378,   378,   378,  -222,  -222,  -222,
     378,   378,  -222,  -222,   378,  -222,   378,   378,   378,   378,
     378,   378,   378,   378,   254,  -222,   691,   229,    14,    41,
      16,   202,   109,  -222,   378,  -222,   197,  -222,   378,   123,
    -222,  -222,  -222,     5,    60,  -222,  -222,  -222,   226,    13,
      13,  -222,   230,    15,    15,  -222,   244,   259,   267,  -222,
     293,   577,  -222,  -222,  -222,   378,   271,   748,   748,   119,
     119,   204,   204,   204,   713,   770,   770,   119,   119,   691,
     691,   266,   260,  -222,  -222,   258,   378,   378,   378,   261,
    -222,  -222,  -222,  -222,    91,    92,  -222,  -222,   268,   301,
    -222,  -222,  -222,  -222,   255,   378,   365,   279,   275,  -222,
    -222,   269,  -222,  -222,    13,   310,   186,  -222,    15,   313,
     201,  -222,  -222,    65,  -222,   298,  -222,   227,  -222,  -222,
    -222,   378,   691,    50,   599,   202,   202,    21,    36,  -222,
     301,   270,    28,  -222,  -222,   265,   317,   104,   117,  -222,
     290,   284,  -222,  -222,   321,   235,   271,   634,   282,  -222,
      32,   -10,  -222,  -222,   122,  -222,   285,   292,   326,   197,
      36,   289,  -222,  -222,  -222,   305,    65,   318,  -222,  -222,
     378,  -222,  -222,   306,    21,  -222,   307,   308,  -222,   297,
     338,   311,  -222,  -222,   341,   327,   656,   202,  -222,   346,
    -222,  -222,   315,   347,  -222,   309,  -222,  -222,    45,  -222,
     115,    36,   320,   151,  -222,  -222,  -222,  -222,  -222,   351,
     329,   109,  -222,   347,  -222,    36,  -222,  -222,  -222
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   164,   166,    83,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    74,     0,     0,   107,
     108,     3,     7,     4,     5,     0,     6,     0,     8,    37,
      38,     0,    39,    40,    42,    33,    41,    43,     0,    44,
      45,    46,     0,    76,    77,    79,    80,    81,     0,    82,
      78,   140,   141,   143,   162,   163,     9,   115,     0,    12,
       0,     0,     0,    83,     0,    80,    80,     0,     0,     0,
       0,    80,     0,     0,     0,   147,   146,   150,     0,   155,
     154,   158,   160,   152,   164,   166,    85,   105,     0,    33,
       0,    33,     0,     0,    61,    34,    35,     0,   164,    83,
     137,     0,    33,     0,     0,     0,     0,    73,    92,    93,
       0,     0,    94,    95,     0,    89,     0,     0,     0,     0,
       0,     0,     0,   115,     0,   142,   117,     0,     0,     0,
       0,     0,     0,    10,   115,    64,    65,    70,   115,     0,
      75,    84,    96,     0,     0,   165,   167,   144,     0,     0,
       0,   145,     0,     0,     0,    18,     0,     0,     0,    50,
       0,     0,    36,    62,    59,     0,     0,   103,   104,   100,
      99,    87,    88,    90,    91,    97,    98,   102,   101,   106,
      86,     0,   116,   109,   113,     0,     0,     0,     0,     0,
      11,    13,   138,   139,     0,     0,   129,   123,     0,     0,
      27,   119,   120,   121,   122,     0,    33,     0,     0,   171,
     170,     0,   148,   156,     0,     0,   151,   153,     0,     0,
     159,   161,    16,    23,    20,     0,    47,    33,    71,    68,
     114,     0,   118,     0,     0,     0,     0,     0,     0,   132,
       0,     0,    80,    66,    63,     0,     0,     0,     0,    17,
       0,    24,    25,    21,     0,    93,     0,     0,     0,    56,
       0,     0,   135,   136,     0,   133,     0,     0,     0,    65,
       0,     0,   169,   149,   157,    28,     0,    51,    69,    57,
       0,    14,   130,     0,     0,    72,   124,     0,    67,     0,
       0,     0,    22,    26,     0,    53,     0,     0,   134,     0,
     128,   168,     0,     0,    52,     0,    48,    58,     0,   126,
       0,     0,     0,     0,    30,    55,    54,   131,   125,     0,
       0,     0,    29,     0,   127,     0,    32,    31,   172
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,  -222,  -222,  -222,  -222,  -222,  -222,  -207,  -222,  -222,
      37,   -23,  -222,    20,  -222,  -222,  -222,  -222,  -222,  -222,
    -222,  -222,  -222,  -222,  -222,  -222,  -222,   125,   102,  -222,
    -222,   128,  -222,  -222,  -222,    -9,  -222,  -222,  -222,  -222,
       1,  -222,  -222,  -222,   -96,   -44,    54,  -160,  -222,  -222,
    -222,  -117,  -222,  -217,  -222,  -222,  -221,  -222,  -222,  -222,
     -65,   237,  -222,  -222,   -66,   225,  -222,   -13,   -12,  -222,
    -222,  -222
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    21,    22,    23,   130,   191,    24,    25,   249,
      89,    26,    27,   253,    91,   250,   251,    62,   292,   313,
     314,   135,    95,    96,    29,    30,    31,   295,   306,   316,
      32,    33,    34,    35,    97,   164,    36,   136,   207,    37,
      38,   229,    39,    40,    41,    42,    43,    44,    45,   122,
      46,    47,    48,    49,   127,   182,   200,   201,   202,   310,
     203,   204,   264,   102,   196,    50,    51,    52,    74,    75,
      76,    77,    78,    79,    80,    81,    53,    54,    55,    56,
     211,   272
  };

  const short int
   Parser ::yytable_[] =
  {
      64,    64,    82,    83,    69,    70,   144,   143,    86,    87,
      65,    66,    94,   128,   195,    71,   252,   266,     4,     3,
     265,    28,    88,   189,   138,   262,   282,   181,   263,   101,
       3,     4,     3,     4,    59,     5,   139,     3,     4,   241,
       3,     4,   208,   212,     7,   239,   190,    60,   126,   289,
     185,   149,     9,    10,   -33,   -33,    11,   186,   -33,    82,
      83,    13,   215,    14,   219,    15,   156,   298,   158,   293,
      72,   100,    61,    15,    16,    73,   187,    17,   281,   166,
     267,   317,    73,    67,   161,    73,   258,   239,   188,    18,
     320,    19,    20,   186,   167,   168,   169,   170,   213,    92,
     239,   171,   172,    93,   328,   173,   153,   174,   175,   176,
     177,   178,   179,   180,   126,   162,   197,    68,   260,   261,
     192,   193,   194,    57,   198,   126,   236,   209,   237,   126,
     210,   -49,    58,  -136,  -136,   -49,  -136,    83,   217,   238,
      82,   221,   273,   233,   199,  -136,    90,   239,   123,   247,
     149,   318,   248,  -136,  -136,   274,   227,  -136,   319,  -136,
     283,   125,  -136,   153,   134,   284,  -136,   108,   109,   110,
     111,   112,   113,   114,   115,  -136,  -110,   232,   126,   234,
     308,   121,  -111,   243,     2,   145,   146,   322,     3,     4,
    -136,     5,  -136,  -136,   323,   129,    64,   124,     6,  -112,
       7,     8,    83,   -19,   256,    82,   242,   131,     9,    10,
     132,   134,    11,   192,   193,   194,    12,    13,   133,    14,
     137,    15,   257,   205,   206,   142,   148,   155,   147,   150,
      16,     3,     4,    17,     5,   103,   104,   151,   149,    84,
      85,   152,    63,     7,   154,    18,    86,    19,    20,   153,
     157,     9,    10,   159,   163,    11,   112,   113,   114,   115,
      13,   183,    14,   214,    15,   184,   121,   218,   105,   106,
      14,   296,    15,    16,   222,   108,   255,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    18,   121,
      19,    20,   -60,   -60,   223,   -60,    18,   224,    19,    20,
     225,   228,   230,   186,   -60,   231,   235,   240,   197,   244,
     239,   245,   -60,   -60,   246,   146,   -60,   145,   -60,   254,
     268,   -60,   270,   -60,   271,   -60,   275,   276,   277,   280,
     287,   285,   286,   290,   -60,  -135,  -135,   -60,  -135,   291,
     294,   297,   299,   301,   300,   302,   303,  -135,   304,   -60,
     305,   -60,   -60,   309,   312,  -135,  -135,   315,   324,  -135,
     327,  -135,   311,   325,  -135,   321,  -135,   269,  -135,     3,
       4,   288,     5,    98,     4,   326,    99,  -135,   220,     0,
    -135,     7,     3,     4,   278,    63,   216,     0,     0,     9,
      10,     0,  -135,    11,  -135,  -135,     0,     0,    13,     0,
      14,     0,    15,     0,    14,     0,    15,     3,     4,     0,
      63,    16,     0,    14,    17,    15,     0,     0,    17,   100,
       0,     0,     0,   160,     0,     0,    18,    17,    19,    20,
      18,     0,    19,    20,     0,     0,     0,     0,    14,    18,
      15,    19,    20,    84,    85,     0,    63,     0,     0,     0,
       0,     0,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,    19,    20,     0,     0,
       0,   103,   104,     0,    14,     0,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       0,     0,     0,   103,   104,     0,     0,     0,     0,     0,
      18,     0,    19,    20,   105,   106,     0,     0,     0,   107,
       0,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,     0,   121,   105,   106,   103,   104,
       0,   140,     0,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,     0,   121,     0,     0,
     103,   104,     0,     0,     0,     0,   141,     0,     0,     0,
       0,   105,   106,     0,     0,     0,     0,     0,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,     0,   121,   105,   106,   103,   104,   165,     0,     0,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,     0,   121,     0,     0,   103,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   106,
       0,     0,     0,   226,     0,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,     0,   121,
     105,   106,   103,   104,     0,   259,     0,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       0,   121,     0,     0,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   106,     0,     0,     0,
     279,     0,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,     0,   121,   105,   106,   103,
     104,     0,   307,     0,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,     0,   121,     0,
       0,   103,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,   106,     0,     0,     0,     0,     0,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,     0,   121,   105,   106,   103,   104,     0,     0,
       0,   108,   109,   110,   111,   112,   113,   114,   115,  -137,
     117,   118,   119,   120,     0,   121,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
     106,     0,     0,     0,     0,     0,   108,   109,   110,   111,
     112,   113,   114,   115,     0,   117,   118,   119,   120,     0,
     121,   105,   106,     0,     0,     0,     0,     0,   108,   109,
     110,   111,   112,   113,   114,   115,     0,     0,     0,   119,
     120,     0,   121
  };

  const short int
   Parser ::yycheck_[] =
  {
       9,    10,    15,    15,    13,    14,    72,    72,    17,    18,
       9,    10,    35,    57,   131,    14,   223,   238,     5,     4,
     237,     1,    18,     7,    35,     4,    36,   123,     7,    38,
       4,     5,     4,     5,     7,     7,    47,     4,     5,   199,
       4,     5,   138,    38,    16,    55,    30,     7,    57,   270,
      36,    46,    24,    25,    26,    27,    28,    43,    30,    72,
      72,    33,    49,    35,    49,    37,    89,   284,    91,   276,
      37,    50,     7,    37,    46,    49,    35,    49,    46,   102,
     240,    36,    49,     7,    93,    49,    36,    55,    47,    61,
     311,    63,    64,    43,   103,   104,   105,   106,    38,    43,
      55,   110,   111,    47,   325,   114,    46,   116,   117,   118,
     119,   120,   121,   122,   123,    95,     7,     7,   235,   236,
      11,    12,    13,    35,    15,   134,    35,     4,    37,   138,
       7,    43,    44,     4,     5,    47,     7,   149,   150,    47,
     153,   154,    38,   187,    35,    16,    19,    55,    35,   214,
      46,    36,   218,    24,    25,    38,   165,    28,    43,    30,
      38,    55,    33,    46,    35,    43,    37,    48,    49,    50,
      51,    52,    53,    54,    55,    46,    44,   186,   187,   188,
     297,    62,    44,   206,     0,     4,     5,    36,     4,     5,
      61,     7,    63,    64,    43,     7,   205,    44,    14,    44,
      16,    17,   214,    19,   227,   218,   205,    45,    24,    25,
      45,    35,    28,    11,    12,    13,    32,    33,    46,    35,
      29,    37,   231,    26,    27,    36,    43,     7,    38,    43,
      46,     4,     5,    49,     7,     8,     9,    38,    46,     4,
       5,    43,     7,    16,    43,    61,   255,    63,    64,    46,
       7,    24,    25,     7,    30,    28,    52,    53,    54,    55,
      33,     7,    35,    37,    37,    36,    62,    37,    41,    42,
      35,   280,    37,    46,    30,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,     4,     5,    35,     7,    61,    30,    63,    64,
       7,    30,    36,    43,    16,    47,    45,    39,     7,    30,
      55,    36,    24,    25,    45,     5,    28,     4,    30,    21,
      50,    33,    57,    35,     7,    37,    36,    43,     7,    47,
       4,    46,    40,    44,    46,     4,     5,    49,     7,    34,
      22,    35,    35,    46,    36,     7,    35,    16,     7,    61,
      23,    63,    64,     7,     7,    24,    25,    48,     7,    28,
     323,    30,    47,    34,    33,    45,    35,   242,    37,     4,
       5,   269,     7,     4,     5,   321,     7,    46,   153,    -1,
      49,    16,     4,     5,   256,     7,   149,    -1,    -1,    24,
      25,    -1,    61,    28,    63,    64,    -1,    -1,    33,    -1,
      35,    -1,    37,    -1,    35,    -1,    37,     4,     5,    -1,
       7,    46,    -1,    35,    49,    37,    -1,    -1,    49,    50,
      -1,    -1,    -1,    20,    -1,    -1,    61,    49,    63,    64,
      61,    -1,    63,    64,    -1,    -1,    -1,    -1,    35,    61,
      37,    63,    64,     4,     5,    -1,     7,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    -1,    63,    64,    -1,    -1,
      -1,     8,     9,    -1,    35,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      61,    -1,    63,    64,    41,    42,    -1,    -1,    -1,    46,
      -1,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    41,    42,     8,     9,
      -1,    46,    -1,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    -1,    -1,
       8,     9,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    41,    42,     8,     9,    45,    -1,    -1,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    -1,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      -1,    -1,    -1,    46,    -1,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      41,    42,     8,     9,    -1,    46,    -1,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    62,    -1,    -1,     8,     9,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    -1,    -1,    -1,
      46,    -1,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    41,    42,     8,
       9,    -1,    46,    -1,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    -1,
      -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    41,    42,     8,     9,    -1,    -1,
      -1,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    -1,    -1,    -1,    -1,    -1,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    57,    58,    59,    60,    -1,
      62,    41,    42,    -1,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    -1,    59,
      60,    -1,    62
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    66,     0,     4,     5,     7,    14,    16,    17,    24,
      25,    28,    32,    33,    35,    37,    46,    49,    61,    63,
      64,    67,    68,    69,    72,    73,    76,    77,    88,    89,
      90,    91,    95,    96,    97,    98,   101,   104,   105,   107,
     108,   109,   110,   111,   112,   113,   115,   116,   117,   118,
     130,   131,   132,   141,   142,   143,   144,    35,    44,     7,
       7,     7,    82,     7,   110,   115,   115,     7,     7,   110,
     110,   115,    37,    49,   133,   134,   135,   136,   137,   138,
     139,   140,   142,   143,     4,     5,   110,   110,    18,    75,
      19,    79,    43,    47,    86,    87,    88,    99,     4,     7,
      50,   110,   128,     8,     9,    41,    42,    46,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    62,   114,    35,    44,    55,   110,   119,   120,     7,
      70,    45,    45,    46,    35,    86,   102,    29,    35,    47,
      46,    36,    36,   135,   139,     4,     5,    38,    43,    46,
      43,    38,    43,    46,    43,     7,    86,     7,    86,     7,
      20,   110,    88,    30,   100,    45,    86,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   119,   120,     7,    36,    36,    43,    35,    47,     7,
      30,    71,    11,    12,    13,   126,   129,     7,    15,    35,
     121,   122,   123,   125,   126,    26,    27,   103,   119,     4,
       7,   145,    38,    38,    37,    49,   136,   143,    37,    49,
     140,   142,    30,    35,    30,     7,    46,   110,    30,   106,
      36,    47,   110,   120,   110,    45,    35,    37,    47,    55,
      39,   122,   115,    86,    30,    36,    45,   135,   139,    74,
      80,    81,    82,    78,    21,    49,    86,   110,    36,    46,
     126,   126,     4,     7,   127,   128,   131,   122,    50,   102,
      57,     7,   146,    38,    38,    36,    43,     7,   106,    46,
      47,    46,    36,    38,    43,    46,    40,     4,   103,   131,
      44,    34,    83,    82,    22,    92,   110,    35,   128,    35,
      36,    46,     7,    35,     7,    23,    93,    46,   126,     7,
     124,    47,     7,    84,    85,    48,    94,    36,    36,    43,
     131,    45,    36,    43,     7,    34,   121,    85,   131
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    65,    66,    66,    67,    67,    67,    67,    67,    67,
      68,    69,    70,    70,    71,    73,    74,    72,    75,    77,
      78,    76,    79,    80,    80,    81,    81,    82,    83,    83,
      84,    84,    85,    86,    86,    87,    87,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    89,    90,    91,
      91,    92,    92,    93,    93,    94,    95,    96,    96,    97,
      98,    99,   100,   101,   102,   103,   103,   103,   104,   104,
     105,   106,   107,   108,   108,   109,   110,   110,   110,   110,
     110,   110,   110,   111,   112,   113,   113,   113,   113,   113,
     113,   113,   114,   114,   114,   114,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   116,
     117,   117,   117,   118,   118,   119,   119,   120,   120,   121,
     121,   121,   121,   122,   123,   123,   124,   124,   125,   126,
     126,   126,   126,   127,   127,   128,   128,   128,   129,   129,
     130,   131,   131,   131,   132,   132,   133,   133,   134,   134,
     135,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   146
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     5,     2,     0,
       0,     5,     6,     0,     1,     1,     3,     3,     0,     4,
       1,     3,     3,     0,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     8,     1,
       3,     0,     2,     0,     2,     1,     6,     7,     9,     3,
       2,     1,     1,     5,     1,     0,     2,     4,     4,     6,
       3,     1,     7,     2,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     3,     2,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     1,     1,     3,
       1,     1,     1,     4,     4,     0,     1,     1,     3,     1,
       1,     1,     1,     1,     4,     7,     1,     3,     5,     1,
       4,     7,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     3,     3,     1,     1,     3,     5,
       1,     3,     1,     3,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     1,     2,     1,     2,     8,     6,
       1,     1,     7
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "\"int literal\"",
  "\"float literal\"", "\"string literal\"", "\"identifier\"", "\"and\"",
  "\"or\"", "NEG", "\"int\"", "\"float\"", "\"tensor\"", "\"element\"",
  "\"set\"", "\"const\"", "\"extern\"", "\"proc\"", "\"func\"", "\"map\"",
  "\"to\"", "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"",
  "\"else\"", "\"for\"", "\"in\"", "\"end\"", "\"return\"", "\"%!\"",
  "\"print\"", "\"->\"", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"{\"",
  "\"}\"", "\"<\"", "\">\"", "\",\"", "\".\"", "\":\"", "\";\"", "\"=\"",
  "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\".*\"", "\"./\"", "\"^\"", "\"'\"",
  "\"\\\\\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"", "\"not\"", "\"xor\"",
  "\"true\"", "\"false\"", "$accept", "program", "program_element",
  "extern", "element_type_decl", "field_decl_list", "field_decl",
  "procedure", "$@1", "$@2", "procedure_header", "function", "$@3", "$@4",
  "function_header", "arguments", "argument_list", "argument_decl",
  "results", "result_list", "result_decl", "stmt_block", "stmts", "stmt",
  "assign_stmt", "map_stmt", "idents", "with", "reduce", "reduce_op",
  "field_write_stmt", "tensor_write_stmt", "while_stmt",
  "while_stmt_header", "while_body", "while_end", "if_stmt", "if_body",
  "else_clauses", "for_stmt", "for_stmt_header", "for_stmt_footer",
  "const_stmt", "expr_stmt", "print_stmt", "expr", "ident_expr",
  "paren_expr", "linear_algebra_expr", "elwise_binary_op", "boolean_expr",
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
       0,   277,   277,   279,   282,   283,   291,   299,   300,   301,
     306,   316,   329,   332,   340,   350,   350,   350,   358,   375,
     375,   375,   383,   404,   407,   413,   418,   426,   435,   438,
     444,   449,   457,   467,   470,   477,   478,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   495,   536,   588,
     592,   599,   602,   615,   618,   624,   629,   649,   669,   696,
     706,   713,   719,   725,   738,   744,   747,   752,   764,   772,
     782,   794,   799,   827,   830,   835,   843,   844,   845,   846,
     847,   848,   849,   855,   875,   884,   892,   911,   979,   999,
    1027,  1032,  1041,  1042,  1043,  1044,  1050,  1054,  1060,  1066,
    1072,  1078,  1084,  1090,  1096,  1102,  1107,  1113,  1117,  1125,
    1159,  1160,  1161,  1167,  1200,  1222,  1225,  1231,  1237,  1248,
    1249,  1250,  1251,  1255,  1267,  1271,  1281,  1290,  1302,  1314,
    1318,  1321,  1363,  1373,  1378,  1386,  1389,  1403,  1409,  1412,
    1462,  1466,  1467,  1471,  1475,  1482,  1493,  1500,  1504,  1508,
    1522,  1526,  1541,  1545,  1552,  1559,  1563,  1567,  1581,  1585,
    1600,  1604,  1611,  1615,  1622,  1625,  1631,  1634,  1641,  1660,
    1684,  1685,  1693
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
    };
    const unsigned int user_token_number_max_ = 319;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



