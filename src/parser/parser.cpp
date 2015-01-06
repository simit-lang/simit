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

      case 70: // extern


        { delete (yysym.value.var); }

        break;

      case 71: // element_type_decl


        { delete (yysym.value.type); }

        break;

      case 72: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 73: // field_decl


        { delete (yysym.value.field); }

        break;

      case 74: // procedure


        { delete (yysym.value.function); }

        break;

      case 77: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 78: // function


        { delete (yysym.value.function); }

        break;

      case 81: // function_header


        { delete (yysym.value.function); }

        break;

      case 82: // arguments


        { delete (yysym.value.vars); }

        break;

      case 83: // argument_list


        { delete (yysym.value.vars); }

        break;

      case 84: // argument_decl


        { delete (yysym.value.var); }

        break;

      case 85: // results


        { delete (yysym.value.vars); }

        break;

      case 86: // result_list


        { delete (yysym.value.vars); }

        break;

      case 87: // result_decl


        { delete (yysym.value.var); }

        break;

      case 88: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 92: // var_decl


        { delete (yysym.value.var); }

        break;

      case 95: // partial_expr_list


        { delete (yysym.value.exprs); }

        break;

      case 96: // idents


        { delete (yysym.value.strings); }

        break;

      case 97: // with


        { delete (yysym.value.expr); }

        break;

      case 98: // reduce


        {}

        break;

      case 99: // reduce_op


        {}

        break;

      case 103: // while_stmt_header


        { delete (yysym.value.expr); }

        break;

      case 104: // while_body


        { delete (yysym.value.stmt); }

        break;

      case 106: // if_stmt


        { delete (yysym.value.stmt); }

        break;

      case 107: // if_body


        { delete (yysym.value.stmt); }

        break;

      case 108: // else_clauses


        { delete (yysym.value.stmt); }

        break;

      case 110: // for_stmt_header


        { delete (yysym.value.var); }

        break;

      case 113: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 115: // expr


        { delete (yysym.value.expr); }

        break;

      case 116: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 117: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 118: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 119: // elwise_binary_op


        {}

        break;

      case 120: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 121: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 122: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 123: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 124: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 125: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 126: // type


        { delete (yysym.value.type); }

        break;

      case 127: // element_type


        { delete (yysym.value.type); }

        break;

      case 128: // set_type


        { delete (yysym.value.type); }

        break;

      case 129: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 130: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 131: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 132: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 133: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 134: // component_type


        { delete (yysym.value.scalarType); }

        break;

      case 135: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 136: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 137: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 138: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 139: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 140: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 141: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 142: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 143: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 144: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 145: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 146: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 147: // signed_int_literal


        {}

        break;

      case 148: // signed_float_literal


        {}

        break;

      case 150: // system_generator


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
      arguments.push_back(ext);
    }

    (yylhs.value.function) = new Func(name, arguments, results, Stmt());
  }

    break;

  case 19:

    {
    Func func = convertAndDelete((yystack_[0].value.function));
    auto arguments = func.getArguments();

    for (auto &extPair : ctx->getExterns()) {
      Var ext = ctx->getExtern(extPair.first);
      arguments.push_back(ext);
    }

    (yylhs.value.function) = new Func(func.getName(), arguments, func.getResults(), func.getBody());
  }

    break;

  case 20:

    {ctx->scope();}

    break;

  case 21:

    {ctx->unscope();}

    break;

  case 22:

    {
    Func func = convertAndDelete((yystack_[3].value.function));
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = new Func(func.getName(), func.getArguments(), func.getResults(), body);
  }

    break;

  case 23:

    {
    std::string name = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<Var>>((yystack_[2].value.vars));
    auto results = unique_ptr<vector<Var>>((yystack_[0].value.vars));
    auto newArguments = unique_ptr<vector<Var>>(new vector<Var>());
    
    std::set<std::string> argNames;
    for (Var &arg : *arguments) {
      auto found = arg.getName().find("___inout___");
      if (found != string::npos) {
        // this is an inout param
        string newName = arg.getName().substr(0,found);
        auto newArg = Var(newName, arg.getType());
        ctx->addSymbol(newName, newArg, Symbol::ReadWrite);
        argNames.insert(newArg.getName());
        newArguments->push_back(newArg);
      } else {
        ctx->addSymbol(arg.getName(), arg, Symbol::Read);
        argNames.insert(arg.getName());
        newArguments->push_back(arg);
      }
    }

    (yylhs.value.function) = new Func(name, *newArguments, *results, Stmt());

    for (Var &res : *results) {
      Symbol::Access access = (argNames.find(res.getName()) != argNames.end())
                              ? Symbol::ReadWrite : Symbol::ReadWrite;
      ctx->addSymbol(res.getName(), res, access);
    }
  }

    break;

  case 24:

    {
    (yylhs.value.vars) = new vector<Var>;
  }

    break;

  case 25:

    {
    (yylhs.value.vars) = (yystack_[0].value.vars);
 }

    break;

  case 26:

    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(argument);
  }

    break;

  case 27:

    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(argument);
  }

    break;

  case 28:

    {
    // this is by no means the best way to do this, but it works.
    std::string name = convertAndFree((yystack_[2].value.string)).append("___inout___");

    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }

    break;

  case 30:

    {
    (yylhs.value.vars) = new vector<Var>;
  }

    break;

  case 31:

    {
    (yylhs.value.vars) = (yystack_[1].value.vars);
  }

    break;

  case 32:

    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(result);
  }

    break;

  case 33:

    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(result);
  }

    break;

  case 35:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 36:

    {
    vector<Stmt> stmts = *ctx->getStatements();
    if (stmts.size() == 0) {(yylhs.value.stmt) = new Stmt(Pass::make()); break;} // TODO: remove
    (yylhs.value.stmt) = new Stmt((stmts.size() == 1) ? stmts[0] : Block::make(stmts));
  }

    break;

  case 50:

    {
    Var var = convertAndDelete((yystack_[1].value.var));

    if (ctx->hasSymbol(var.getName())) {
      REPORT_ERROR("variable redeclaration", yystack_[1].location);
    }

    ctx->addSymbol(var.getName(), var, Symbol::ReadWrite);
    ctx->addStatement(VarDecl::make(var));
  }

    break;

  case 51:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "initialization expression is null" ;

    Var var = convertAndDelete((yystack_[3].value.var));
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (isScalar(value.type())) {
      if (!var.getType().isTensor()) {
        REPORT_ERROR("attempting to assign a scalar to a non-tensor", yystack_[1].location);
      }
    }
    else {
      CHECK_TYPE_EQUALITY(var.getType(), value.type(), yystack_[1].location);
    }

    ctx->addSymbol(var.getName(), var, Symbol::ReadWrite);
    ctx->addStatement(VarDecl::make(var));
    ctx->addStatement(AssignStmt::make(var, value));
  }

    break;

  case 52:

    {
    std::string name = convertAndFree((yystack_[2].value.string));
    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }

    break;

  case 53:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression is null" ;

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

  case 54:

    {
    auto varNames = unique_ptr<vector<string>>((yystack_[8].value.strings));

    auto partialExprList = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    
    string funcName = convertAndFree((yystack_[5].value.string));
    string targetsName = convertAndFree((yystack_[2].value.string));

    Expr neighbor = convertAndDelete((yystack_[1].value.expr));
    ReductionOperator reduction((yystack_[0].value.reductionop));

    if (!ctx->containsFunction(funcName)) {
      REPORT_ERROR("undefined function '" + funcName + "'", yystack_[5].location);
    }
    Func func = ctx->getFunction(funcName);

    if (varNames->size() != func.getResults().size()) {
      REPORT_ERROR("the number of variables (" + to_string(varNames->size()) +
                   ") does not match the number of results returned by " +
                   func.getName() + " (" + to_string(func.getResults().size()) +
                   ")", yystack_[8].location);
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
          REPORT_ERROR(varName + " is not writable", yystack_[8].location);
        }

        var = symbol.getVar();
      }
      else {
        var = Var(varName, results[i].getType());
        ctx->addSymbol(varName, var, Symbol::ReadWrite);
      }
      vars.push_back(var);
    }

    ctx->addStatement(Map::make(vars, func, *partialExprList, targets, neighbor,
      reduction));
  }

    break;

  case 55:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 56:

    {
    (yylhs.value.exprs) = (yystack_[1].value.exprs);
  }

    break;

  case 57:

    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 58:

    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 59:

    {
    (yylhs.value.expr) = new Expr();
  }

    break;

  case 60:

    {
    std::string neighborsName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(neighborsName)) {
      REPORT_ERROR("undefined set '" + neighborsName + "'", yystack_[0].location);
    }
    Expr neighbors = ctx->getSymbol(neighborsName).getExpr();

    (yylhs.value.expr) = new Expr(neighbors);
  }

    break;

  case 61:

    {
    (yylhs.value.reductionop) =  ReductionOperator::Undefined;
  }

    break;

  case 62:

    {
    (yylhs.value.reductionop) =  (yystack_[0].value.reductionop);
  }

    break;

  case 63:

    {
    (yylhs.value.reductionop) = ReductionOperator::Sum;
  }

    break;

  case 64:

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

  case 65:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression" << "is null" ;

    std::string tensorName = convertAndFree((yystack_[6].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
    Expr value = convertAndDelete((yystack_[1].value.expr));

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

  case 66:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression" << "is null" ;
 
    string setName = convertAndFree((yystack_[8].value.string));
    string fieldName = convertAndFree((yystack_[6].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[4].value.exprs));
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

  case 67:

    {
    Expr cond = convertAndDelete((yystack_[2].value.expr));
    Stmt body = convertAndDelete((yystack_[1].value.stmt));
    
    ctx->addStatement(While::make(cond, body));

  }

    break;

  case 68:

    {
    ctx->scope();
    (yylhs.value.expr) = new Expr(convertAndDelete((yystack_[0].value.expr)));
  }

    break;

  case 69:

    {

    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 70:

    {
    ctx->unscope();
  }

    break;

  case 71:

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

  case 72:

    {
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 73:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 74:

    {
    ctx->scope();
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
    ctx->unscope();
  }

    break;

  case 75:

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

  case 76:

    {    
    if((yystack_[2].value.indexSet)->getKind()==IndexSet::Set){
      ctx->addStatement(For::make(*(yystack_[3].value.var),ForDomain(*(yystack_[2].value.indexSet)), *(yystack_[1].value.stmt)));
    }
    delete (yystack_[3].value.var);
    delete (yystack_[2].value.indexSet);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 77:

    {    
    ctx->addStatement(ForRange::make(*(yystack_[5].value.var), *(yystack_[4].value.expr), *(yystack_[2].value.expr), *(yystack_[1].value.stmt)));
    delete (yystack_[5].value.var);
    delete (yystack_[4].value.expr);
    delete (yystack_[2].value.expr);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 78:

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

  case 79:

    {
    ctx->unscope();
  }

    break;

  case 80:

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

  case 81:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 82:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 83:

    {
     Expr expr = Expr(convertAndDelete((yystack_[1].value.expr)));
     ctx->addStatement(Stmt(Print::make(expr)));
  }

    break;

  case 91:

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

  case 92:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "expression in parenthesis is null" ;
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 93:

    {
    iassert((yystack_[0].value.expr) != nullptr) << "negated expression is null" ;

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 94:

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

  case 95:

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

  case 96:

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

  case 97:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "transposed expression is null";
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

  case 98:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 99:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 100:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 101:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 102:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 103:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 104:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 105:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Eq::make(l, r));
  }

    break;

  case 106:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ne::make(l, r));
  }

    break;

  case 107:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Gt::make(l, r));
  }

    break;

  case 108:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Lt::make(l, r));
  }

    break;

  case 109:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ge::make(l, r));
  }

    break;

  case 110:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Le::make(l, r));
  }

    break;

  case 111:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(And::make(l, r));
  }

    break;

  case 112:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Or::make(l, r));
  }

    break;

  case 113:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Not::make(r));
  }

    break;

  case 114:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Xor::make(l, r));
  }

    break;

  case 115:

    {
    bool val = true;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 116:

    {
    bool val = false;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 117:

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

  case 121:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto indices = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    if (ctx->hasSymbol(name)) {
      const Symbol &symbol = ctx->getSymbol(name);
      if (!symbol.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      // The parenthesis read can be a read from a tensor or a tuple.
      auto readExpr = symbol.getExpr();
      if (readExpr.type().isTensor()) {
        (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, *indices));
      }
      else if (readExpr.type().isTuple()) {
        if (indices->size() != 1) {
          REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
        }
        (yylhs.value.expr) = new Expr(TupleRead::make(readExpr, (*indices)[0]));
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

  case 122:

    {
    Expr readExpr = convertAndDelete((yystack_[3].value.expr));
    auto indices = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    // The parenthesis read can be a read from a tensor or a tuple.
    if (readExpr.type().isTensor()) {
      (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, *indices));
    }
    else if (readExpr.type().isTuple()) {
      if (indices->size() != 1) {
        REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(TupleRead::make(readExpr, (*indices)[0]));
    }
    else {
      REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
    }
  }

    break;

  case 123:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 124:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 125:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 126:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 131:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 132:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 133:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 134:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 135:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 136:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 137:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 138:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 139:

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

  case 140:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 141:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 142:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 143:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 144:

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

  case 145:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 146:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 147:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 150:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 152:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values));
  }

    break;

  case 153:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 154:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 156:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 157:

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

  case 158:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 159:

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

  case 160:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 161:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 162:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 164:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 165:

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

  case 166:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 167:

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

  case 168:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 169:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 170:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.num)));
  }

    break;

  case 171:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.fnum)));
  }

    break;

  case 172:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 173:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 174:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 175:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 176:

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

  case 177:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 179:

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


  const short int  Parser ::yypact_ninf_ = -227;

  const short int  Parser ::yytable_ninf_ = -145;

  const short int
   Parser ::yypact_[] =
  {
    -227,   200,  -227,  -227,  -227,    87,    83,   142,   156,    20,
    -227,  -227,   449,   449,   159,   164,   449,   449,    32,  -227,
     454,   449,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,    57,  -227,  -227,  -227,   370,  -227,  -227,
     123,  -227,  -227,  -227,   481,   -18,   -15,  -227,  -227,  -227,
      85,   139,  -227,  -227,    69,  -227,  -227,  -227,  -227,   449,
     174,  -227,   140,   124,   143,   184,   145,  -227,   187,   189,
     160,   745,   828,    28,   167,     1,   513,   543,   161,    35,
     173,   162,   158,   153,   165,   169,   166,   175,   170,  -227,
    -227,  -227,  -227,   314,   745,   205,   418,  -227,   370,  -227,
     190,   328,   412,  -227,   574,   370,   449,   449,   449,   449,
     449,  -227,  -227,  -227,   449,   449,  -227,  -227,   449,  -227,
     449,   449,   449,   449,   449,   449,   449,   218,  -227,   745,
     193,    63,    51,    15,   128,  -227,   449,   135,   181,  -227,
     195,   370,  -227,   195,   370,   449,  -227,   151,  -227,   449,
     113,  -227,  -227,  -227,    13,    44,  -227,  -227,  -227,   194,
      29,    29,  -227,   197,    19,    19,  -227,   231,   600,  -227,
    -227,  -227,   449,   208,   802,   802,   203,   202,   875,   875,
     314,   314,   314,   771,   854,   854,   875,   875,   745,   745,
    -227,  -227,   196,   449,   449,   449,   206,  -227,  -227,  -227,
    -227,  -227,    27,   201,   242,  -227,  -227,  -227,  -227,   198,
    -227,   631,    80,   128,    20,   220,   227,   449,   370,   228,
     212,  -227,  -227,   214,  -227,  -227,    29,   257,   165,  -227,
      19,   260,   170,  -227,   230,  -227,   264,  -227,  -227,  -227,
     449,   745,   106,   657,   135,   135,    17,   242,   222,  -227,
    -227,    38,  -227,   232,   234,  -227,  -227,  -227,    28,  -227,
    -227,   216,   270,   102,   105,   449,   259,   449,   239,   208,
     688,   235,  -227,    24,    -8,  -227,  -227,     6,  -227,   241,
     281,   240,   251,    20,  -227,  -227,   151,    38,   243,  -227,
    -227,  -227,   254,   286,   543,  -227,  -227,   449,  -227,  -227,
     258,    17,   261,   262,  -227,   269,  -227,  -227,  -227,   248,
     290,  -227,   285,   714,   135,  -227,   303,  -227,   142,  -227,
     287,   306,   309,  -227,    21,  -227,   119,   131,  -227,  -227,
      38,  -227,   288,  -227,  -227,  -227,   330,  -227,   142,   304,
    -227,  -227,  -227,  -227,    38,  -227
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   172,   174,    91,     0,     0,     0,     0,
      15,    20,     0,     0,     0,     0,     0,     0,     0,    82,
       0,     0,   115,   116,     3,     7,     4,     5,     6,     8,
      39,    40,    41,     0,    42,    43,    45,    35,    44,    46,
       0,    47,    48,    49,     0,    84,    85,    87,    88,    89,
       0,    90,    86,   148,   149,   151,   170,   171,     9,   123,
       0,    12,     0,     0,     0,     0,     0,    29,     0,     0,
      91,     0,    68,    88,     0,     0,     0,     0,    88,     0,
       0,     0,   155,   154,   158,     0,   163,   162,   166,   168,
     160,   172,   174,    93,   113,     0,     0,    69,    36,    37,
       0,   172,    91,   145,     0,    35,     0,     0,   123,     0,
       0,    81,   100,   101,     0,     0,   102,   103,     0,    97,
       0,     0,     0,     0,     0,     0,     0,     0,   150,   125,
       0,     0,     0,     0,     0,    50,     0,     0,     0,    10,
      18,    35,    19,     0,    35,   123,    72,    73,    78,   123,
       0,    83,    92,   104,     0,     0,   173,   175,   152,     0,
       0,     0,   153,     0,     0,     0,    58,     0,     0,    38,
      70,    67,     0,     0,   111,   112,     0,   124,   108,   107,
      95,    96,    98,    99,   105,   106,   110,   109,   114,    94,
     117,   121,     0,     0,     0,     0,     0,    11,    13,   131,
     146,   147,     0,     0,     0,    52,   127,   128,   129,   130,
     137,     0,     0,     0,    24,     0,     0,     0,    35,     0,
       0,   179,   178,     0,   156,   164,     0,     0,   159,   161,
       0,     0,   167,   169,    55,    53,    35,    79,    76,   122,
       0,   126,     0,     0,     0,     0,     0,     0,     0,   140,
      51,     0,    28,     0,    25,    26,    16,    21,    88,    74,
      71,     0,     0,     0,     0,   123,     0,   123,   101,     0,
       0,     0,    64,     0,     0,   143,   144,     0,   141,     0,
       0,     0,    30,     0,    17,    22,    73,     0,     0,   177,
     157,   165,     0,     0,   125,    77,    65,     0,    14,   138,
       0,     0,   132,     0,    80,     0,    23,    27,    75,     0,
       0,    56,    59,     0,     0,   142,     0,   136,     0,   176,
       0,     0,    61,    66,     0,   134,     0,     0,    32,    34,
       0,    60,     0,    54,   139,   133,     0,    31,     0,     0,
      63,    62,   135,    33,     0,   180
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,   272,  -227,  -227,  -196,  -227,  -227,
       4,   -23,  -227,    18,  -227,    -7,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
      81,    60,  -227,  -227,    74,  -227,  -227,  -227,   -11,  -227,
    -227,  -227,  -227,    -9,  -227,  -227,  -227,   -97,   -42,   134,
    -179,  -227,  -227,  -227,  -122,  -227,  -226,  -227,  -227,  -225,
    -227,  -227,  -227,   -72,   188,  -227,  -227,   -63,   185,  -227,
      -6,    -5,  -227,  -227,  -227
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    24,    25,    26,   133,   198,    27,    68,   284,
     141,    28,    69,   285,   142,   253,   254,    66,   306,   327,
     328,   146,    98,    99,    30,    67,    31,    32,   266,    33,
     322,   333,   341,    34,    35,    36,    37,   100,   171,    38,
     147,   219,    39,    40,   238,    41,    42,    43,    44,    45,
      46,    47,   126,    48,    49,    50,    51,   130,   177,   205,
     206,   207,   326,   208,   209,   277,   105,   210,    52,    53,
      54,    81,    82,    83,    84,    85,    86,    87,    88,    55,
      56,    57,    58,   223,   289
  };

  const short int
   Parser ::yytable_[] =
  {
      63,    71,    71,    72,    73,    76,    77,   154,    78,    93,
      94,   176,    89,    90,    97,   212,   155,   131,   255,    29,
     278,   275,   196,     3,   276,   248,   281,    62,  -118,   104,
     299,  -119,     3,     4,     4,     5,     3,     4,   149,     3,
       4,    65,     3,     4,     7,     8,   300,   197,   129,   249,
     150,   301,   220,   224,    12,    13,   -35,   -35,    14,   334,
     -35,   160,   309,    16,   245,    17,   246,    18,   279,   103,
     231,    79,   298,    89,    90,   315,    19,    18,   249,    20,
     227,   249,   173,    80,   225,   168,    80,   307,   194,    80,
      61,    21,   164,    22,    23,   174,   175,   129,   178,   179,
     195,   192,    95,   180,   181,   339,    96,   182,   193,   183,
     184,   185,   186,   187,   188,   189,   169,   221,   215,   345,
     222,   216,   273,   274,    59,   211,   128,   101,     4,   251,
     102,   127,   -57,    60,   129,   199,   -57,   249,   129,   200,
     201,   202,   290,   203,   271,   291,   200,   201,   202,    62,
     160,   193,   242,   164,   263,    90,   229,   335,    89,   233,
      17,   236,    18,    64,   336,   204,    74,   264,   292,   337,
     176,    75,   135,   136,    20,   103,   338,   156,   157,   217,
     218,   132,   241,   129,   243,  -120,    21,   134,    22,    23,
     137,   138,   324,   139,   140,   259,   143,   145,   148,   153,
       2,   160,   158,   159,     3,     4,    71,     5,   258,   162,
     161,   163,   166,   269,     6,   165,     7,     8,     9,    10,
      11,    90,   170,   164,    89,   190,    12,    13,   213,   270,
      14,   191,   214,   226,    15,    16,   230,    17,   234,    18,
     237,   239,   247,    91,    92,   240,    70,   193,    19,   199,
     261,    20,   256,   244,   129,   249,   294,    93,    78,   257,
     260,   262,   157,    21,   156,    22,    23,   265,     3,     4,
     282,     5,   106,   107,   280,   287,    17,   288,    18,   283,
       7,     8,   293,   302,   297,   303,   313,   305,   304,   310,
      12,    13,   311,   312,    14,   314,   319,   320,   316,    16,
     317,   267,    21,    18,    22,    23,   318,   109,   110,   321,
     325,   329,    19,   331,   112,   268,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    21,   125,    22,
      23,   329,  -143,  -143,   332,  -143,   330,   342,   340,   286,
     344,   144,   343,   295,  -143,  -143,   308,   252,   228,   232,
       0,   108,     0,     0,  -143,  -143,     0,     0,  -143,     0,
    -143,     0,     0,  -143,     0,  -143,     0,  -143,   116,   117,
     118,   119,     0,     0,     3,     4,  -143,     5,   125,  -143,
       0,     0,     0,     0,     0,     0,     7,     8,     0,     0,
       0,  -143,     0,  -143,  -143,     0,    12,    13,     0,     0,
      14,     0,     0,     0,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,     0,     0,  -144,  -144,    19,  -144,
       0,    20,     3,     4,     0,    70,     0,     0,  -144,  -144,
       0,     0,     0,    21,     0,    22,    23,     0,  -144,  -144,
     167,     0,  -144,     0,  -144,     0,     0,  -144,     0,   145,
       0,  -144,     0,     3,     4,    17,    70,    18,    91,    92,
    -144,    70,     0,     0,     0,     0,     0,     0,     0,    20,
       0,     0,     0,     0,     0,  -144,     0,  -144,  -144,     0,
       0,    21,     0,    22,    23,     0,    17,     0,    18,   106,
     107,    17,     0,    18,     0,     0,     0,     0,     0,     0,
      20,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,    21,     0,    22,    23,     0,    21,   108,    22,
      23,   106,   107,     0,   109,   110,     0,     0,     0,   111,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,     0,   125,     0,     0,     0,     0,
     108,   106,   107,     0,     0,     0,   109,   110,     0,     0,
       0,   151,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,     0,     0,
     108,   152,   106,   107,     0,     0,   109,   110,     0,     0,
       0,     0,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,   106,   107,
       0,   108,     0,     0,     0,     0,     0,   109,   110,     0,
       0,   172,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   108,   125,   106,
     107,     0,     0,   109,   110,     0,     0,     0,   235,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,     0,   125,   106,   107,     0,   108,     0,
       0,     0,     0,     0,   109,   110,     0,     0,     0,   250,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   108,   125,   106,   107,     0,     0,
     109,   110,     0,     0,     0,   272,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
       0,   125,   106,   107,     0,   108,     0,     0,     0,     0,
       0,   109,   110,     0,     0,     0,   296,     0,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   108,   125,   106,   107,     0,     0,   109,   110,     0,
       0,     0,   323,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,   125,   106,
     107,     0,   108,     0,     0,     0,     0,     0,   109,   110,
       0,     0,     0,     0,     0,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   108,   125,
     106,   107,     0,     0,   109,   110,     0,     0,     0,     0,
       0,   112,   113,   114,   115,   116,   117,   118,   119,  -145,
     121,   122,   123,   124,     0,   125,   -88,   -88,     0,   108,
       0,     0,     0,     0,     0,   109,   110,     0,     0,     0,
       0,     0,   112,   113,   114,   115,   116,   117,   118,   119,
       0,   121,   122,   123,   124,     0,   125,     0,     0,     0,
       0,   -88,   -88,     0,     0,     0,     0,     0,   -88,     0,
     -88,   -88,   -88,   -88,   -88,   -88,   -88,   -88,   -88,   -88,
     -88,   108,   -88,     0,     0,     0,     0,   109,   110,     0,
       0,     0,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   108,     0,     0,   123,   124,     0,   125,     0,
       0,     0,     0,     0,     0,   112,   113,   114,   115,   116,
     117,   118,   119,     0,     0,     0,     0,     0,     0,   125
  };

  const short int
   Parser ::yycheck_[] =
  {
       7,    12,    13,    12,    13,    16,    17,    79,    17,    20,
      21,   108,    18,    18,    37,   137,    79,    59,   214,     1,
     246,     4,     7,     4,     7,   204,   251,     7,    46,    40,
      38,    46,     4,     5,     5,     7,     4,     5,    37,     4,
       5,    21,     4,     5,    16,    17,    40,    32,    59,    57,
      49,    45,   149,    40,    26,    27,    28,    29,    30,    38,
      32,    48,   287,    35,    37,    37,    39,    39,   247,    52,
      51,    39,    48,    79,    79,   301,    48,    39,    57,    51,
      51,    57,   105,    51,    40,    96,    51,   283,    37,    51,
       7,    63,    48,    65,    66,   106,   107,   108,   109,   110,
      49,    38,    45,   114,   115,   330,    49,   118,    45,   120,
     121,   122,   123,   124,   125,   126,    98,     4,   141,   344,
       7,   144,   244,   245,    37,   136,    57,     4,     5,    49,
       7,    46,    45,    46,   145,     7,    49,    57,   149,    11,
      12,    13,    40,    15,    38,    40,    11,    12,    13,     7,
      48,    45,   194,    48,   226,   160,   161,    38,   164,   165,
      37,   172,    39,     7,    45,    37,     7,   230,   265,    38,
     267,     7,    48,    49,    51,    52,    45,     4,     5,    28,
      29,     7,   193,   194,   195,    46,    63,    47,    65,    66,
      47,     7,   314,    48,     7,   218,     7,    37,    31,    38,
       0,    48,    40,    45,     4,     5,   217,     7,   217,    40,
      45,    45,     7,   236,    14,    45,    16,    17,    18,    19,
      20,   226,    32,    48,   230,     7,    26,    27,    47,   240,
      30,    38,    37,    39,    34,    35,    39,    37,     7,    39,
      32,    38,    41,     4,     5,    49,     7,    45,    48,     7,
      38,    51,    32,    47,   265,    57,   267,   268,   267,    32,
      32,    47,     5,    63,     4,    65,    66,    37,     4,     5,
      38,     7,     8,     9,    52,    59,    37,     7,    39,    45,
      16,    17,    23,    42,    49,     4,   297,    36,    48,    46,
      26,    27,    38,     7,    30,    37,    48,     7,    37,    35,
      38,    37,    63,    39,    65,    66,    37,    43,    44,    24,
       7,   318,    48,     7,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,   338,     4,     5,    25,     7,    49,     7,    50,   258,
      36,    69,   338,   269,    16,    17,   286,   213,   160,   164,
      -1,    37,    -1,    -1,    26,    27,    -1,    -1,    30,    -1,
      32,    -1,    -1,    35,    -1,    37,    -1,    39,    54,    55,
      56,    57,    -1,    -1,     4,     5,    48,     7,    64,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    16,    17,    -1,    -1,
      -1,    63,    -1,    65,    66,    -1,    26,    27,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    -1,    37,    -1,    39,
      -1,    -1,    -1,    -1,    -1,    -1,     4,     5,    48,     7,
      -1,    51,     4,     5,    -1,     7,    -1,    -1,    16,    17,
      -1,    -1,    -1,    63,    -1,    65,    66,    -1,    26,    27,
      22,    -1,    30,    -1,    32,    -1,    -1,    35,    -1,    37,
      -1,    39,    -1,     4,     5,    37,     7,    39,     4,     5,
      48,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    63,    -1,    65,    66,    -1,
      -1,    63,    -1,    65,    66,    -1,    37,    -1,    39,     8,
       9,    37,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    65,    66,    -1,    63,    37,    65,
      66,     8,     9,    -1,    43,    44,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    -1,    -1,
      37,     8,     9,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,    -1,    -1,
      37,    38,     8,     9,    -1,    -1,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,     8,     9,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    47,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    37,    64,     8,
       9,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,     8,     9,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    37,    64,     8,     9,    -1,    -1,
      43,    44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,     8,     9,    -1,    37,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    37,    64,     8,     9,    -1,    -1,    43,    44,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,     8,
       9,    -1,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    37,    64,
       8,     9,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,     8,     9,    -1,    37,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      -1,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      -1,    59,    60,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    37,    64,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    37,    -1,    -1,    61,    62,    -1,    64,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    64
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    68,     0,     4,     5,     7,    14,    16,    17,    18,
      19,    20,    26,    27,    30,    34,    35,    37,    39,    48,
      51,    63,    65,    66,    69,    70,    71,    74,    78,    90,
      91,    93,    94,    96,   100,   101,   102,   103,   106,   109,
     110,   112,   113,   114,   115,   116,   117,   118,   120,   121,
     122,   123,   135,   136,   137,   146,   147,   148,   149,    37,
      46,     7,     7,    92,     7,    21,    84,    92,    75,    79,
       7,   115,   120,   120,     7,     7,   115,   115,   120,    39,
      51,   138,   139,   140,   141,   142,   143,   144,   145,   147,
     148,     4,     5,   115,   115,    45,    49,    88,    89,    90,
     104,     4,     7,    52,   115,   133,     8,     9,    37,    43,
      44,    48,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    64,   119,    46,    57,   115,
     124,   125,     7,    72,    47,    48,    49,    47,     7,    48,
       7,    77,    81,     7,    81,    37,    88,   107,    31,    37,
      49,    48,    38,    38,   140,   144,     4,     5,    40,    45,
      48,    45,    40,    45,    48,    45,     7,    22,   115,    90,
      32,   105,    47,    88,   115,   115,   124,   125,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
       7,    38,    38,    45,    37,    49,     7,    32,    73,     7,
      11,    12,    13,    15,    37,   126,   127,   128,   130,   131,
     134,   115,   131,    47,    37,    88,    88,    28,    29,   108,
     124,     4,     7,   150,    40,    40,    39,    51,   141,   148,
      39,    51,   145,   147,     7,    48,   115,    32,   111,    38,
      49,   115,   125,   115,    47,    37,    39,    41,   127,    57,
      48,    49,   126,    82,    83,    84,    32,    32,   120,    88,
      32,    38,    47,   140,   144,    37,    95,    37,    51,    88,
     115,    38,    48,   131,   131,     4,     7,   132,   133,   127,
      52,   136,    38,    45,    76,    80,   107,    59,     7,   151,
      40,    40,   124,    23,   115,   111,    48,    49,    48,    38,
      40,    45,    42,     4,    48,    36,    85,    84,   108,   136,
      46,    38,     7,   115,    37,   133,    37,    38,    37,    48,
       7,    24,    97,    48,   131,     7,   129,    86,    87,    92,
      49,     7,    25,    98,    38,    38,    45,    38,    45,   136,
      50,    99,     7,    87,    36,   136
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    67,    68,    68,    69,    69,    69,    69,    69,    69,
      70,    71,    72,    72,    73,    75,    76,    74,    77,    77,
      79,    80,    78,    81,    82,    82,    83,    83,    84,    84,
      85,    85,    86,    86,    87,    88,    88,    89,    89,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      91,    91,    92,    93,    94,    95,    95,    96,    96,    97,
      97,    98,    98,    99,   100,   101,   101,   102,   103,   104,
     105,   106,   107,   108,   108,   108,   109,   109,   110,   111,
     112,   113,   113,   114,   115,   115,   115,   115,   115,   115,
     115,   116,   117,   118,   118,   118,   118,   118,   118,   118,
     119,   119,   119,   119,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   121,   122,   122,
     122,   123,   123,   124,   124,   125,   125,   126,   126,   126,
     126,   127,   128,   128,   129,   129,   130,   131,   131,   131,
     131,   132,   132,   133,   133,   133,   134,   134,   135,   136,
     136,   136,   137,   137,   138,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     146,   146,   147,   147,   148,   148,   149,   149,   150,   150,
     151
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     6,     1,     1,
       0,     0,     6,     5,     0,     1,     1,     3,     4,     1,
       0,     4,     1,     3,     1,     0,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     5,     3,     4,     9,     0,     3,     1,     3,     0,
       2,     0,     2,     1,     6,     7,     9,     3,     2,     1,
       1,     5,     1,     0,     2,     4,     4,     6,     3,     1,
       7,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     3,     3,     3,     2,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     1,     1,     3,     1,     1,
       1,     4,     4,     0,     1,     1,     3,     1,     1,     1,
       1,     1,     4,     7,     1,     3,     5,     1,     4,     7,
       2,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     3,     3,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     1,     2,     1,     2,     8,     6,     1,     1,
       7
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "\"int literal\"",
  "\"float literal\"", "\"string literal\"", "\"identifier\"", "\"and\"",
  "\"or\"", "NEG", "\"int\"", "\"float\"", "\"tensor\"", "\"element\"",
  "\"set\"", "\"var\"", "\"const\"", "\"extern\"", "\"proc\"", "\"func\"",
  "\"inout\"", "\"map\"", "\"to\"", "\"with\"", "\"reduce\"", "\"while\"",
  "\"if\"", "\"elif\"", "\"else\"", "\"for\"", "\"in\"", "\"end\"",
  "\"return\"", "\"%!\"", "\"print\"", "\"->\"", "\"(\"", "\")\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\",\"", "\".\"", "\":\"",
  "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\".*\"", "\"./\"",
  "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"",
  "\"not\"", "\"xor\"", "\"true\"", "\"false\"", "$accept", "program",
  "program_element", "extern", "element_type_decl", "field_decl_list",
  "field_decl", "procedure", "$@1", "$@2", "procedure_header", "function",
  "$@3", "$@4", "function_header", "arguments", "argument_list",
  "argument_decl", "results", "result_list", "result_decl", "stmt_block",
  "stmts", "stmt", "var_decl_stmt", "var_decl", "assign_stmt", "map_stmt",
  "partial_expr_list", "idents", "with", "reduce", "reduce_op",
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
       0,   280,   280,   282,   285,   286,   294,   302,   303,   304,
     309,   319,   332,   335,   343,   353,   353,   353,   361,   373,
     387,   387,   387,   395,   429,   432,   438,   443,   451,   458,
     463,   466,   472,   477,   485,   491,   494,   501,   502,   505,
     506,   507,   508,   509,   510,   511,   512,   513,   514,   515,
     520,   530,   552,   563,   604,   659,   662,   669,   673,   680,
     683,   696,   699,   705,   710,   730,   752,   776,   786,   793,
     799,   805,   818,   824,   827,   832,   844,   852,   862,   873,
     878,   906,   909,   914,   922,   923,   924,   925,   926,   927,
     928,   934,   954,   963,   971,   990,  1058,  1078,  1106,  1111,
    1120,  1121,  1122,  1123,  1129,  1133,  1139,  1145,  1151,  1157,
    1163,  1169,  1175,  1181,  1186,  1192,  1196,  1204,  1238,  1239,
    1240,  1247,  1280,  1317,  1320,  1326,  1332,  1343,  1344,  1345,
    1346,  1350,  1362,  1366,  1376,  1385,  1397,  1409,  1413,  1416,
    1458,  1468,  1473,  1481,  1484,  1498,  1504,  1507,  1557,  1561,
    1562,  1566,  1570,  1577,  1588,  1595,  1599,  1603,  1617,  1621,
    1636,  1640,  1647,  1654,  1658,  1662,  1676,  1680,  1695,  1699,
    1706,  1709,  1715,  1718,  1724,  1727,  1734,  1753,  1777,  1778,
    1786
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66
    };
    const unsigned int user_token_number_max_ = 321;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



