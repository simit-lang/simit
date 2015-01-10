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

  std::string typeString(const Type &type); // GCC shut up
  void transposeVector(Expr vec);
  bool compare(const Type &l, const Type &r, ProgramContext *ctx);

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

  /// This function is only used to get and free the result of IDENT rules,
  /// since these are C char arrays. For other objects use convertAndDelete.
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
    // TODO: Lots of duplicate code in this rule, and the other assignment rule/
    //       the call_or_paren_read_expr. Not good, is going to be a source of
    //       bugs, and should be cleaned up.
    string name = convertAndFree((yystack_[3].value.string));
    vector<Expr> arguments = convertAndDelete((yystack_[1].value.exprs));
    vector<string> varNames = convertAndDelete((yystack_[5].value.strings));

    if (ctx->hasSymbol(name)) {
      if (varNames.size() > 1) {
        REPORT_ERROR("can only assign the results of calls and maps to "
                    "multiple variables", yystack_[5].location);
      }

      const Symbol &symbol = ctx->getSymbol(name);
      if (!symbol.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[5].location);
      }

      // The parenthesis read can be a read from a tensor or a tuple.
      Expr value;
      auto readExpr = symbol.getExpr();
      if (readExpr.type().isTensor()) {
        value = TensorRead::make(readExpr, arguments);
      }
      else if (readExpr.type().isTuple()) {
        if (arguments.size() != 1) {
          REPORT_ERROR("reading a tuple requires exactly one index", yystack_[3].location);
        }
        value = TupleRead::make(readExpr, arguments[0]);
      }
      else {
        REPORT_ERROR("can only access components in tensors and tuples", yystack_[5].location);
      }

      string varName = varNames[0];
      Var var;
      if (ctx->hasSymbol(varName)) {
        Symbol symbol = ctx->getSymbol(varName);
        if (!symbol.isWritable()) {
          REPORT_ERROR(varName + " is constant", yystack_[5].location);
        }
        CHECK_TYPE_EQUALITY(symbol.getVar().getType(), value.type(), yystack_[4].location);
        var = symbol.getVar();
      }
      else {
        var = Var(varName, value.type());
        ctx->addSymbol(varName, var, Symbol::ReadWrite);
      }
      iassert(var.defined());

      ctx->addStatement(AssignStmt::make(var, value));
    }
    else if (ctx->containsFunction(name)) {
      Func func = ctx->getFunction(name);
      std::vector<Var> results = func.getResults();

      if (varNames.size() != results.size()) {
        REPORT_ERROR("number of variables (" + to_string(varNames.size()) +
                    ") does not match number of function results (" +
                    to_string(func.getResults().size()) + ")", yystack_[5].location);
      }

      std::vector<Var> variables;
      for (size_t i=0; i < varNames.size(); ++i) {
        string varName = varNames[i];
        Var result = results[i];

        if (ctx->hasSymbol(varName)) {
          Symbol symbol = ctx->getSymbol(varName);
          if (!symbol.isWritable()) {
            REPORT_ERROR(varName + " is constant", yystack_[5].location);
          }
          variables.push_back(symbol.getVar());
        }
        else {
          // The variable does not already exist so we create it
          Var var(varName, result.getType());
          ctx->addSymbol(var.getName(), var, Symbol::ReadWrite);
          ctx->addStatement(VarDecl::make(var));
          variables.push_back(var);
        }
      }
      iassert(variables.size() == varNames.size());

      Stmt callStmt = CallStmt::make(variables, func, arguments);
      ctx->addStatement(callStmt);
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[5].location);
    }
  }

    break;

  case 54:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression is null" ;

    auto varNames = convertAndDelete((yystack_[3].value.strings));
    if (varNames.size() > 1) {
      REPORT_ERROR("can only assign the results of calls and maps to "
                   "multiple variables", yystack_[3].location);
    }

    string varName = varNames[0];
    Expr value = convertAndDelete((yystack_[1].value.expr));

    Var var;
    if (ctx->hasSymbol(varName)) {
      Symbol symbol = ctx->getSymbol(varName);

      if (!symbol.isWritable()) {
        REPORT_ERROR(varName + " is constant", yystack_[3].location);
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

  case 55:

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

  case 56:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 57:

    {
    (yylhs.value.exprs) = (yystack_[1].value.exprs);
  }

    break;

  case 58:

    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 59:

    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 60:

    {
    (yylhs.value.expr) = new Expr();
  }

    break;

  case 61:

    {
    std::string neighborsName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(neighborsName)) {
      REPORT_ERROR("undefined set '" + neighborsName + "'", yystack_[0].location);
    }
    Expr neighbors = ctx->getSymbol(neighborsName).getExpr();

    (yylhs.value.expr) = new Expr(neighbors);
  }

    break;

  case 62:

    {
    (yylhs.value.reductionop) =  ReductionOperator::Undefined;
  }

    break;

  case 63:

    {
    (yylhs.value.reductionop) =  (yystack_[0].value.reductionop);
  }

    break;

  case 64:

    {
    (yylhs.value.reductionop) = ReductionOperator::Sum;
  }

    break;

  case 65:

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

  case 66:

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

  case 67:

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

  case 68:

    {
    Expr cond = convertAndDelete((yystack_[2].value.expr));
    Stmt body = convertAndDelete((yystack_[1].value.stmt));
    
    ctx->addStatement(While::make(cond, body));
  }

    break;

  case 69:

    {
    ctx->scope();
    (yylhs.value.expr) = new Expr(convertAndDelete((yystack_[0].value.expr)));
  }

    break;

  case 70:

    {

    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 71:

    {
    ctx->unscope();
  }

    break;

  case 72:

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

  case 73:

    {
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
  }

    break;

  case 74:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 75:

    {
    ctx->scope();
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[0].value.stmt)));
    ctx->unscope();
  }

    break;

  case 76:

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

  case 77:

    {    
    if((yystack_[2].value.indexSet)->getKind()==IndexSet::Set){
      ctx->addStatement(For::make(*(yystack_[3].value.var),ForDomain(*(yystack_[2].value.indexSet)), *(yystack_[1].value.stmt)));
    }
    delete (yystack_[3].value.var);
    delete (yystack_[2].value.indexSet);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 78:

    {    
    ctx->addStatement(ForRange::make(*(yystack_[5].value.var), *(yystack_[4].value.expr), *(yystack_[2].value.expr), *(yystack_[1].value.stmt)));
    delete (yystack_[5].value.var);
    delete (yystack_[4].value.expr);
    delete (yystack_[2].value.expr);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 79:

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

  case 80:

    {
    ctx->unscope();
  }

    break;

  case 81:

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

  case 82:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 83:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 84:

    {
     Expr expr = Expr(convertAndDelete((yystack_[1].value.expr)));
     ctx->addStatement(Stmt(Print::make(expr)));
  }

    break;

  case 92:

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

  case 93:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "expression in parenthesis is null" ;
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 94:

    {
    iassert((yystack_[0].value.expr) != nullptr) << "negated expression is null" ;

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 95:

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

  case 97:

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

  case 98:

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

  case 99:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 100:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 101:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 102:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 103:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 104:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 105:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 106:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Eq::make(l, r));
  }

    break;

  case 107:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ne::make(l, r));
  }

    break;

  case 108:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Gt::make(l, r));
  }

    break;

  case 109:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Lt::make(l, r));
  }

    break;

  case 110:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ge::make(l, r));
  }

    break;

  case 111:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Le::make(l, r));
  }

    break;

  case 112:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(And::make(l, r));
  }

    break;

  case 113:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Or::make(l, r));
  }

    break;

  case 114:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Not::make(r));
  }

    break;

  case 115:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Xor::make(l, r));
  }

    break;

  case 116:

    {
    bool val = true;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 117:

    {
    bool val = false;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 118:

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

  case 122:

    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto arguments = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    if (ctx->hasSymbol(name)) {
      const Symbol &symbol = ctx->getSymbol(name);
      if (!symbol.isReadable()) {
        REPORT_ERROR(name + " is not readable", yystack_[3].location);
      }

      // The parenthesis read can be a read from a tensor or a tuple.
      auto readExpr = symbol.getExpr();
      if (readExpr.type().isTensor()) {
        (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, *arguments));
      }
      else if (readExpr.type().isTuple()) {
        if (arguments->size() != 1) {
          REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
        }
        (yylhs.value.expr) = new Expr(TupleRead::make(readExpr, (*arguments)[0]));
      }
      else {
        REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
      }
    }
    else if (ctx->containsFunction(name)) {
      Func func = ctx->getFunction(name);

      if (func.getResults().size() != 1) {
        REPORT_ERROR("called function must have one result", yystack_[3].location);
      }

      Var resultFormal = func.getResults()[0];
      Var resultActual = ctx->getBuilder()->temporary(resultFormal.getType());;

      Stmt callStmt = CallStmt::make({resultActual}, func, *arguments);

      ctx->addSymbol(resultActual.getName(), resultActual, Symbol::ReadWrite);
      ctx->addStatement(VarDecl::make(resultActual));
      ctx->addStatement(callStmt);

      (yylhs.value.expr) = new Expr(VarExpr::make(resultActual));
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 123:

    {
    Expr readExpr = convertAndDelete((yystack_[3].value.expr));
    auto arguments = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    // The parenthesis read can be a read from a tensor or a tuple.
    if (readExpr.type().isTensor()) {
      (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, *arguments));
    }
    else if (readExpr.type().isTuple()) {
      if (arguments->size() != 1) {
        REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(TupleRead::make(readExpr, (*arguments)[0]));
    }
    else {
      REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
    }
  }

    break;

  case 124:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 125:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 126:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 127:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 132:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 133:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 134:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 135:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 136:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 137:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 138:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 139:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 140:

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

  case 141:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 142:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 143:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 144:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 145:

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

  case 146:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 147:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 148:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 151:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 153:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values));
  }

    break;

  case 154:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 155:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 157:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 158:

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

  case 159:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 160:

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

  case 161:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 162:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 163:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 165:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 166:

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

  case 167:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 168:

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

  case 169:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 170:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 171:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.num)));
  }

    break;

  case 172:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.fnum)));
  }

    break;

  case 173:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 174:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 175:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 176:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 177:

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

  case 178:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 180:

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


  const short int  Parser ::yypact_ninf_ = -229;

  const short int  Parser ::yytable_ninf_ = -146;

  const short int
   Parser ::yypact_[] =
  {
    -229,   216,  -229,  -229,  -229,    86,    52,    79,    94,    40,
    -229,  -229,   384,   384,   133,   138,   384,   384,    32,  -229,
     405,   384,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
    -229,  -229,  -229,    39,  -229,  -229,  -229,   332,  -229,  -229,
     374,  -229,  -229,  -229,   450,   -20,    78,  -229,  -229,  -229,
     103,   111,  -229,  -229,   121,  -229,  -229,  -229,  -229,   384,
     144,  -229,    -1,   112,   135,   187,   149,  -229,   194,   199,
     162,   704,   818,   163,   177,     2,   476,   507,   172,    26,
     161,   173,   167,   169,   174,   178,   182,   189,   193,  -229,
    -229,  -229,  -229,   117,   704,   232,   380,  -229,   332,  -229,
     208,    28,   338,  -229,   533,   332,   384,   384,   384,   384,
     384,  -229,  -229,  -229,   384,   384,  -229,  -229,   384,  -229,
     384,   384,   384,   384,   384,   384,   384,   234,  -229,   704,
     206,     4,    43,    20,   115,  -229,   384,   192,   198,  -229,
     210,   332,  -229,   210,   332,   384,  -229,   158,  -229,   384,
      98,  -229,  -229,  -229,    60,    89,  -229,  -229,  -229,   209,
      19,    19,  -229,   213,    15,    15,  -229,   212,   247,   564,
    -229,  -229,  -229,   384,   224,   761,   761,   220,   217,   426,
     426,   117,   117,   117,   735,   844,   844,   426,   426,   704,
     704,  -229,  -229,   214,   384,   384,   384,   218,  -229,  -229,
    -229,  -229,  -229,   109,   225,   261,  -229,  -229,  -229,  -229,
     221,  -229,   590,    84,   115,    40,   237,   238,   384,   332,
     239,   236,  -229,  -229,   233,  -229,  -229,    19,   278,   174,
    -229,    15,   282,   193,  -229,   384,   250,  -229,   268,  -229,
    -229,  -229,   384,   704,     5,   621,   192,   192,    16,   261,
     240,  -229,  -229,    36,  -229,   251,   243,  -229,  -229,  -229,
     163,  -229,  -229,   241,   284,    96,    99,   255,   384,   273,
     384,   423,   224,   647,   248,  -229,    33,    -4,  -229,  -229,
      24,  -229,   257,   297,   254,   270,    40,  -229,  -229,   158,
      36,   258,  -229,  -229,  -229,   792,   271,   301,   507,  -229,
    -229,   384,  -229,  -229,   276,    16,   277,   272,  -229,   280,
    -229,  -229,  -229,   290,   333,  -229,   317,   678,   192,  -229,
     337,  -229,    79,  -229,   298,   339,   325,  -229,     0,  -229,
     105,   131,  -229,  -229,    36,  -229,   302,  -229,  -229,  -229,
     344,  -229,    79,   320,  -229,  -229,  -229,  -229,    36,  -229
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,   173,   175,    92,     0,     0,     0,     0,
      15,    20,     0,     0,     0,     0,     0,     0,     0,    83,
       0,     0,   116,   117,     3,     7,     4,     5,     6,     8,
      39,    40,    41,     0,    42,    43,    45,    35,    44,    46,
       0,    47,    48,    49,     0,    85,    86,    88,    89,    90,
       0,    91,    87,   149,   150,   152,   171,   172,     9,   124,
       0,    12,     0,     0,     0,     0,     0,    29,     0,     0,
      92,     0,    69,    89,     0,     0,     0,     0,    89,     0,
       0,     0,   156,   155,   159,     0,   164,   163,   167,   169,
     161,   173,   175,    94,   114,     0,     0,    70,    36,    37,
       0,   173,    92,   146,     0,    35,     0,     0,   124,     0,
       0,    82,   101,   102,     0,     0,   103,   104,     0,    98,
       0,     0,     0,     0,     0,     0,     0,     0,   151,   126,
       0,     0,     0,     0,     0,    50,     0,     0,     0,    10,
      18,    35,    19,     0,    35,   124,    73,    74,    79,   124,
       0,    84,    93,   105,     0,     0,   174,   176,   153,     0,
       0,     0,   154,     0,     0,     0,    59,    92,     0,     0,
      38,    71,    68,     0,     0,   112,   113,     0,   125,   109,
     108,    96,    97,    99,   100,   106,   107,   111,   110,   115,
      95,   118,   122,     0,     0,     0,     0,     0,    11,    13,
     132,   147,   148,     0,     0,     0,    52,   128,   129,   130,
     131,   138,     0,     0,     0,    24,     0,     0,     0,    35,
       0,     0,   180,   179,     0,   157,   165,     0,     0,   160,
     162,     0,     0,   168,   170,   124,    56,    54,    35,    80,
      77,   123,     0,   127,     0,     0,     0,     0,     0,     0,
       0,   141,    51,     0,    28,     0,    25,    26,    16,    21,
      89,    75,    72,     0,     0,     0,     0,     0,   124,     0,
     124,   102,     0,     0,     0,    65,     0,     0,   144,   145,
       0,   142,     0,     0,     0,    30,     0,    17,    22,    74,
       0,     0,   178,   158,   166,    53,     0,     0,   126,    78,
      66,     0,    14,   139,     0,     0,   133,     0,    81,     0,
      23,    27,    76,     0,     0,    57,    60,     0,     0,   143,
       0,   137,     0,   177,     0,     0,    62,    67,     0,   135,
       0,     0,    32,    34,     0,    61,     0,    55,   140,   134,
       0,    31,     0,     0,    64,    63,   136,    33,     0,   181
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
    -229,  -229,  -229,  -229,   288,  -229,  -229,  -197,  -229,  -229,
      11,   -23,  -229,    21,  -229,    -7,  -229,  -229,  -229,  -229,
    -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
     100,    72,  -229,  -229,    91,  -229,  -229,  -229,   -11,  -229,
    -229,  -229,  -229,    -9,  -229,  -229,  -229,   -93,   -42,   152,
    -177,  -229,  -229,  -229,  -130,  -229,  -227,  -229,  -229,  -228,
    -229,  -229,  -229,   -63,   222,  -229,  -229,   -68,   226,  -229,
      -6,    -5,  -229,  -229,  -229
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    24,    25,    26,   133,   199,    27,    68,   287,
     141,    28,    69,   288,   142,   255,   256,    66,   310,   331,
     332,   146,    98,    99,    30,    67,    31,    32,   269,    33,
     326,   337,   345,    34,    35,    36,    37,   100,   172,    38,
     147,   220,    39,    40,   240,    41,    42,    43,    44,    45,
      46,    47,   126,    48,    49,    50,    51,   130,   178,   206,
     207,   208,   330,   209,   210,   280,   105,   211,    52,    53,
      54,    81,    82,    83,    84,    85,    86,    87,    88,    55,
      56,    57,    58,   224,   292
  };

  const short int
   Parser ::yytable_[] =
  {
      63,    71,    71,    72,    73,    76,    77,   213,    78,    93,
      94,   155,    89,    90,    97,   177,   154,   131,   257,     3,
     278,   281,    29,   279,     4,   284,  -119,   197,   250,   104,
       3,     4,  -144,  -144,   303,  -144,     3,     4,   338,   149,
       3,     4,   193,   274,  -144,  -144,   134,    62,   129,   194,
     194,   150,   198,   251,  -144,  -144,   221,   251,  -144,    61,
    -144,    65,   313,  -144,   304,  -144,   232,  -144,   103,   305,
     228,    79,   282,    89,    90,    18,  -144,    80,   319,  -144,
     195,   302,   174,    80,    95,   169,    62,    80,    96,   311,
     251,  -144,   196,  -144,  -144,   175,   176,   129,   179,   180,
     225,    64,   222,   181,   182,   223,   343,   183,   160,   184,
     185,   186,   187,   188,   189,   190,   276,   277,   216,   170,
     349,   217,   200,    59,  -120,   212,   201,   202,   203,   226,
     204,   -58,    60,   253,   129,   -58,   293,   164,   129,   294,
      74,   251,   267,   339,   160,    75,   247,   164,   248,   127,
     340,   132,   205,   244,   108,    90,   230,  -121,    89,   234,
     135,   136,   238,   266,   265,   156,   157,     3,     4,   341,
       5,   116,   117,   118,   119,   296,   342,   177,   128,     7,
       8,   125,   137,   243,   129,   245,   218,   219,   328,    12,
      13,   -35,   -35,    14,   138,   -35,   261,   139,    16,   145,
      17,   140,    18,   201,   202,   203,   143,    71,   148,   260,
     153,    19,   159,   158,    20,   272,     2,   160,   162,   161,
       3,     4,    90,     5,   129,    89,    21,   163,    22,    23,
       6,   273,     7,     8,     9,    10,    11,   164,   165,   166,
     171,   191,    12,    13,   192,   214,    14,   215,   227,   235,
      15,    16,   231,    17,   236,    18,   239,   129,   241,   298,
      93,    78,   194,   242,    19,   246,   249,    20,   200,   258,
     259,   262,     3,     4,   263,     5,   106,   107,   251,    21,
     264,    22,    23,   157,     7,     8,   156,   268,   286,   285,
     317,   291,   283,   295,    12,    13,   297,   301,    14,   306,
     290,   307,   308,    16,   314,   270,   309,    18,   316,   315,
     321,   109,   110,   318,   320,   333,    19,   322,   112,   271,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,    21,   125,    22,    23,   333,     3,     4,   323,     5,
     324,   325,  -145,  -145,   329,  -145,   335,   334,     7,     8,
     336,   346,   344,   347,  -145,  -145,   348,   144,    12,    13,
     289,   312,    14,   299,  -145,  -145,   254,    16,  -145,    17,
    -145,    18,     0,  -145,     0,   145,     0,  -145,   101,     4,
      19,   102,   229,    20,     3,     4,  -145,   167,     3,     4,
     233,    70,     0,     0,     0,    21,     0,    22,    23,     0,
       0,  -145,   168,  -145,  -145,     0,     0,     0,     0,    91,
      92,    17,    70,    18,     0,     0,     0,    17,     0,    18,
       0,    17,     0,    18,     0,    20,   103,    91,    92,     0,
      70,    20,     0,     0,     0,    20,     0,    21,     0,    22,
      23,     0,    17,    21,    18,    22,    23,    21,     0,    22,
      23,     0,     0,     0,     0,     0,    20,     0,   106,   107,
      17,     0,    18,   108,     0,     0,     0,     0,    21,     0,
      22,    23,     0,     0,     0,     0,   112,   113,   114,   115,
     116,   117,   118,   119,   106,   107,    21,   108,    22,    23,
     125,     0,     0,   109,   110,     0,     0,     0,   111,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   108,   125,   106,   107,     0,     0,   109,
     110,     0,     0,     0,   151,     0,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,     0,
     125,   106,   107,     0,   108,   152,     0,     0,     0,     0,
     109,   110,     0,     0,     0,     0,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     108,   125,   106,   107,     0,     0,   109,   110,     0,     0,
     173,     0,     0,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,   106,   107,
       0,   108,     0,     0,     0,     0,     0,   109,   110,     0,
       0,     0,   237,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   108,   125,   106,
     107,     0,     0,   109,   110,     0,     0,     0,   252,     0,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,     0,   125,   106,   107,     0,   108,     0,
       0,     0,     0,     0,   109,   110,     0,     0,     0,   275,
       0,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   108,   125,   106,   107,     0,     0,
     109,   110,     0,     0,     0,   300,     0,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
       0,   125,   106,   107,     0,   108,     0,     0,     0,     0,
       0,   109,   110,     0,     0,     0,   327,     0,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   108,   125,   106,   107,     0,     0,   109,   110,     0,
       0,     0,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,   125,   106,
     107,     0,   108,     0,     0,     0,     0,     0,   109,   110,
       0,     0,     0,     0,     0,   112,   113,   114,   115,   116,
     117,   118,   119,  -146,   121,   122,   123,   124,   108,   125,
    -122,  -122,     0,     0,   109,   110,     0,     0,     0,     0,
       0,   112,   113,   114,   115,   116,   117,   118,   119,     0,
     121,   122,   123,   124,     0,   125,   -89,   -89,     0,     0,
       0,     0,     0,     0,     0,  -122,  -122,     0,  -122,     0,
       0,     0,  -122,     0,  -122,  -122,  -122,  -122,  -122,  -122,
    -122,  -122,  -122,  -122,  -122,     0,  -122,     0,     0,     0,
       0,   -89,   -89,     0,     0,     0,     0,     0,   -89,     0,
     -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,
     -89,   108,   -89,     0,     0,     0,     0,   109,   110,     0,
       0,     0,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,     0,     0,     0,   123,   124,     0,   125
  };

  const short int
   Parser ::yycheck_[] =
  {
       7,    12,    13,    12,    13,    16,    17,   137,    17,    20,
      21,    79,    18,    18,    37,   108,    79,    59,   215,     4,
       4,   248,     1,     7,     5,   253,    46,     7,   205,    40,
       4,     5,     4,     5,    38,     7,     4,     5,    38,    37,
       4,     5,    38,    38,    16,    17,    47,     7,    59,    45,
      45,    49,    32,    57,    26,    27,   149,    57,    30,     7,
      32,    21,   290,    35,    40,    37,    51,    39,    52,    45,
      51,    39,   249,    79,    79,    39,    48,    51,   305,    51,
      37,    48,   105,    51,    45,    96,     7,    51,    49,   286,
      57,    63,    49,    65,    66,   106,   107,   108,   109,   110,
      40,     7,     4,   114,   115,     7,   334,   118,    48,   120,
     121,   122,   123,   124,   125,   126,   246,   247,   141,    98,
     348,   144,     7,    37,    46,   136,    11,    12,    13,    40,
      15,    45,    46,    49,   145,    49,    40,    48,   149,    40,
       7,    57,   235,    38,    48,     7,    37,    48,    39,    46,
      45,     7,    37,   195,    37,   160,   161,    46,   164,   165,
      48,    49,   173,   231,   227,     4,     5,     4,     5,    38,
       7,    54,    55,    56,    57,   268,    45,   270,    57,    16,
      17,    64,    47,   194,   195,   196,    28,    29,   318,    26,
      27,    28,    29,    30,     7,    32,   219,    48,    35,    37,
      37,     7,    39,    11,    12,    13,     7,   218,    31,   218,
      38,    48,    45,    40,    51,   238,     0,    48,    40,    45,
       4,     5,   227,     7,   235,   231,    63,    45,    65,    66,
      14,   242,    16,    17,    18,    19,    20,    48,    45,     7,
      32,     7,    26,    27,    38,    47,    30,    37,    39,    37,
      34,    35,    39,    37,     7,    39,    32,   268,    38,   270,
     271,   270,    45,    49,    48,    47,    41,    51,     7,    32,
      32,    32,     4,     5,    38,     7,     8,     9,    57,    63,
      47,    65,    66,     5,    16,    17,     4,    37,    45,    38,
     301,     7,    52,    38,    26,    27,    23,    49,    30,    42,
      59,     4,    48,    35,    46,    37,    36,    39,     7,    38,
      38,    43,    44,    37,    37,   322,    48,    37,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,   342,     4,     5,    48,     7,
       7,    24,     4,     5,     7,     7,     7,    49,    16,    17,
      25,     7,    50,   342,    16,    17,    36,    69,    26,    27,
     260,   289,    30,   272,    26,    27,   214,    35,    30,    37,
      32,    39,    -1,    35,    -1,    37,    -1,    39,     4,     5,
      48,     7,   160,    51,     4,     5,    48,     7,     4,     5,
     164,     7,    -1,    -1,    -1,    63,    -1,    65,    66,    -1,
      -1,    63,    22,    65,    66,    -1,    -1,    -1,    -1,     4,
       5,    37,     7,    39,    -1,    -1,    -1,    37,    -1,    39,
      -1,    37,    -1,    39,    -1,    51,    52,     4,     5,    -1,
       7,    51,    -1,    -1,    -1,    51,    -1,    63,    -1,    65,
      66,    -1,    37,    63,    39,    65,    66,    63,    -1,    65,
      66,    -1,    -1,    -1,    -1,    -1,    51,    -1,     8,     9,
      37,    -1,    39,    37,    -1,    -1,    -1,    -1,    63,    -1,
      65,    66,    -1,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,     8,     9,    63,    37,    65,    66,
      64,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    37,    64,     8,     9,    -1,    -1,    43,
      44,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    -1,
      64,     8,     9,    -1,    37,    38,    -1,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      37,    64,     8,     9,    -1,    -1,    43,    44,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64,     8,     9,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
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
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    -1,    64,     8,
       9,    -1,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    37,    64,
       8,     9,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      59,    60,    61,    62,    -1,    64,     8,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    46,    -1,
      -1,    -1,    50,    -1,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    37,    64,    -1,    -1,    -1,    -1,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    61,    62,    -1,    64
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
      48,    45,    40,    45,    48,    45,     7,     7,    22,   115,
      90,    32,   105,    47,    88,   115,   115,   124,   125,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,     7,    38,    38,    45,    37,    49,     7,    32,    73,
       7,    11,    12,    13,    15,    37,   126,   127,   128,   130,
     131,   134,   115,   131,    47,    37,    88,    88,    28,    29,
     108,   124,     4,     7,   150,    40,    40,    39,    51,   141,
     148,    39,    51,   145,   147,    37,     7,    48,   115,    32,
     111,    38,    49,   115,   125,   115,    47,    37,    39,    41,
     127,    57,    48,    49,   126,    82,    83,    84,    32,    32,
     120,    88,    32,    38,    47,   140,   144,   124,    37,    95,
      37,    51,    88,   115,    38,    48,   131,   131,     4,     7,
     132,   133,   127,    52,   136,    38,    45,    76,    80,   107,
      59,     7,   151,    40,    40,    38,   124,    23,   115,   111,
      48,    49,    48,    38,    40,    45,    42,     4,    48,    36,
      85,    84,   108,   136,    46,    38,     7,   115,    37,   133,
      37,    38,    37,    48,     7,    24,    97,    48,   131,     7,
     129,    86,    87,    92,    49,     7,    25,    98,    38,    38,
      45,    38,    45,   136,    50,    99,     7,    87,    36,   136
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    67,    68,    68,    69,    69,    69,    69,    69,    69,
      70,    71,    72,    72,    73,    75,    76,    74,    77,    77,
      79,    80,    78,    81,    82,    82,    83,    83,    84,    84,
      85,    85,    86,    86,    87,    88,    88,    89,    89,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      91,    91,    92,    93,    93,    94,    95,    95,    96,    96,
      97,    97,    98,    98,    99,   100,   101,   101,   102,   103,
     104,   105,   106,   107,   108,   108,   108,   109,   109,   110,
     111,   112,   113,   113,   114,   115,   115,   115,   115,   115,
     115,   115,   116,   117,   118,   118,   118,   118,   118,   118,
     118,   119,   119,   119,   119,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   121,   122,
     122,   122,   123,   123,   124,   124,   125,   125,   126,   126,
     126,   126,   127,   128,   128,   129,   129,   130,   131,   131,
     131,   131,   132,   132,   133,   133,   133,   134,   134,   135,
     136,   136,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   142,   142,   143,   143,   144,   144,   145,
     145,   146,   146,   147,   147,   148,   148,   149,   149,   150,
     150,   151
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     6,     1,     1,
       0,     0,     6,     5,     0,     1,     1,     3,     4,     1,
       0,     4,     1,     3,     1,     0,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     5,     3,     6,     4,     9,     0,     3,     1,     3,
       0,     2,     0,     2,     1,     6,     7,     9,     3,     2,
       1,     1,     5,     1,     0,     2,     4,     4,     6,     3,
       1,     7,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     2,     3,     3,     3,     2,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     1,     1,     3,     1,
       1,     1,     4,     4,     0,     1,     1,     3,     1,     1,
       1,     1,     1,     4,     7,     1,     3,     5,     1,     4,
       7,     2,     1,     3,     1,     1,     1,     1,     1,     1,
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
       0,   286,   286,   288,   291,   292,   300,   308,   309,   310,
     315,   325,   338,   341,   349,   359,   359,   359,   367,   379,
     393,   393,   393,   401,   435,   438,   444,   449,   457,   464,
     469,   472,   478,   483,   491,   497,   500,   507,   508,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     526,   536,   558,   569,   661,   702,   757,   760,   767,   771,
     778,   781,   794,   797,   803,   808,   828,   850,   874,   883,
     890,   896,   902,   915,   921,   924,   929,   941,   949,   959,
     970,   975,  1003,  1006,  1011,  1019,  1020,  1021,  1022,  1023,
    1024,  1025,  1031,  1051,  1060,  1068,  1087,  1155,  1175,  1203,
    1208,  1217,  1218,  1219,  1220,  1226,  1230,  1236,  1242,  1248,
    1254,  1260,  1266,  1272,  1278,  1283,  1289,  1293,  1301,  1335,
    1336,  1337,  1344,  1391,  1412,  1415,  1421,  1427,  1438,  1439,
    1440,  1441,  1445,  1457,  1461,  1471,  1480,  1492,  1504,  1508,
    1511,  1553,  1563,  1568,  1576,  1579,  1593,  1599,  1602,  1652,
    1656,  1657,  1661,  1665,  1672,  1683,  1690,  1694,  1698,  1712,
    1716,  1731,  1735,  1742,  1749,  1753,  1757,  1771,  1775,  1790,
    1794,  1801,  1804,  1810,  1813,  1819,  1822,  1829,  1848,  1872,
    1873,  1881
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



