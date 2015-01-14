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

      case 71: // extern


        { delete (yysym.value.var); }

        break;

      case 72: // element_type_decl


        { delete (yysym.value.type); }

        break;

      case 73: // field_decl_list


        { delete (yysym.value.fields); }

        break;

      case 74: // field_decl


        { delete (yysym.value.field); }

        break;

      case 75: // procedure


        { delete (yysym.value.function); }

        break;

      case 78: // procedure_header


        { delete (yysym.value.function); }

        break;

      case 79: // function


        { delete (yysym.value.function); }

        break;

      case 82: // function_header


        { delete (yysym.value.function); }

        break;

      case 83: // arguments


        { delete (yysym.value.vars); }

        break;

      case 84: // argument_list


        { delete (yysym.value.vars); }

        break;

      case 85: // argument_decl


        { delete (yysym.value.var); }

        break;

      case 86: // results


        { delete (yysym.value.vars); }

        break;

      case 87: // result_list


        { delete (yysym.value.vars); }

        break;

      case 88: // result_decl


        { delete (yysym.value.var); }

        break;

      case 89: // stmt_block


        { delete (yysym.value.stmt); }

        break;

      case 93: // var_decl


        { delete (yysym.value.var); }

        break;

      case 96: // partial_expr_list


        { delete (yysym.value.exprs); }

        break;

      case 97: // idents


        { delete (yysym.value.strings); }

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

      case 110: // else_clauses


        { delete (yysym.value.stmt); }

        break;

      case 116: // for_stmt_header


        { delete (yysym.value.var); }

        break;

      case 119: // expr_stmt


        { delete (yysym.value.stmt); }

        break;

      case 121: // expr


        { delete (yysym.value.expr); }

        break;

      case 122: // ident_expr


        { delete (yysym.value.expr); }

        break;

      case 123: // paren_expr


        { delete (yysym.value.expr); }

        break;

      case 124: // linear_algebra_expr


        { delete (yysym.value.expr); }

        break;

      case 125: // elwise_binary_op


        {}

        break;

      case 126: // boolean_expr


        { delete (yysym.value.expr); }

        break;

      case 127: // field_read_expr


        { delete (yysym.value.expr); }

        break;

      case 128: // set_read_expr


        { delete (yysym.value.expr); }

        break;

      case 129: // call_or_paren_read_expr


        { delete (yysym.value.expr); }

        break;

      case 130: // expr_list_or_empty


        { delete (yysym.value.exprs); }

        break;

      case 131: // expr_list


        { delete (yysym.value.exprs); }

        break;

      case 132: // type


        { delete (yysym.value.type); }

        break;

      case 133: // element_type


        { delete (yysym.value.type); }

        break;

      case 134: // set_type


        { delete (yysym.value.type); }

        break;

      case 135: // endpoints


        { delete (yysym.value.exprs); }

        break;

      case 136: // tuple_type


        { delete (yysym.value.type); }

        break;

      case 137: // tensor_type


        { delete (yysym.value.type); }

        break;

      case 138: // index_sets


        { delete (yysym.value.indexSets); }

        break;

      case 139: // index_set


        { delete (yysym.value.indexSet); }

        break;

      case 140: // component_type


        { delete (yysym.value.scalarType); }

        break;

      case 141: // literal_expr


        { delete (yysym.value.expr); }

        break;

      case 142: // tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 143: // dense_tensor_literal


        { delete (yysym.value.expr); }

        break;

      case 144: // float_dense_tensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 145: // float_dense_ndtensor_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 146: // float_dense_matrix_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 147: // float_dense_vector_literal


        { delete (yysym.value.TensorDoubleValues); }

        break;

      case 148: // int_dense_tensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 149: // int_dense_ndtensor_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 150: // int_dense_matrix_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 151: // int_dense_vector_literal


        { delete (yysym.value.TensorIntValues); }

        break;

      case 152: // scalar_literal


        { delete (yysym.value.expr); }

        break;

      case 153: // signed_int_literal


        {}

        break;

      case 154: // signed_float_literal


        {}

        break;

      case 155: // boolean_literal


        {}

        break;

      case 157: // system_generator


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
        // check if we need to turn this into an index expression
        // TODO: This logic needs to be in all paths that can create tensor
        // reads.
        int numSlices = 0;
        for (auto arg: arguments) {
          if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":")
            numSlices++;
        }
        if (numSlices > 0) {
         // We will construct an index expression.
         // first, we build IndexVars
         std::vector<IndexVar> allivars;
         std::vector<IndexVar> freeVars;
         int i=0;
         for (auto &arg: arguments) {
          if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":") {
            auto iv = IndexVar("tmpfree" + std::to_string(i),
                readExpr.type().toTensor()->dimensions[i]);
            allivars.push_back(iv);
            freeVars.push_back(iv);
          }
          else {
            allivars.push_back(IndexVar("tmpfixed" + std::to_string(i),
                readExpr.type().toTensor()->dimensions[i],
              new Expr(arg)));
          }
          i++;
         }
            // now construct an index expression
         value = IndexExpr::make(freeVars,
            IndexedTensor::make(readExpr, allivars));
        }
        else {
        
          value = TensorRead::make(readExpr, arguments);
        }
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
    string funcName = convertAndFree((yystack_[3].value.string));
    vector<Expr> partialActuals = convertAndDelete((yystack_[2].value.exprs));
    string targetName = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsFunction(funcName)) {
      REPORT_ERROR("undefined function '" + funcName + "'", yystack_[3].location);
    }
    Func func = ctx->getFunction(funcName);

    if (!ctx->hasSymbol(targetName)) {
      REPORT_ERROR("undefined set '" + targetName + "'", yystack_[0].location);
    }
    Expr target = ctx->getSymbol(targetName).getExpr();

    if (!target.type().isSet()) {
      REPORT_ERROR("maps can only be applied to sets", yystack_[0].location);
    }

    // Get endpoints
    std::vector<Expr> endpoints;
    for (Expr *endpoint : target.type().toSet()->endpointSets) {
      endpoints.push_back(*endpoint);
    }

    if (func.getResults().size() != 0) {
      REPORT_ERROR("the number of variables (0) does not match the number of "
                   "results returned by " + func.getName() + " (" +
                   to_string(func.getResults().size()) + ")", yystack_[4].location);
    }

    size_t numFormals = func.getArguments().size();
    size_t numActuals = partialActuals.size() + 1 + (endpoints.size()>0 ?1:0);
    if (!(numFormals == numActuals ||
          (endpoints.size() > 0 && numFormals == numActuals-1))) {
      REPORT_ERROR("the number of actuals (" + to_string(numActuals) +
                   ") does not match the number of formals accepted by " +
                   func.getName() + " (" +
                   to_string(func.getArguments().size()) + ")", yystack_[4].location);
    }

    if (target.type().toSet()->elementType != func.getArguments()[0].getType()){
      REPORT_ERROR("the mapped set's element type is different from the mapped "
                   "function's target argument", yystack_[0].location);
    }

    // We assume the edge set is homogeneous for now
    Expr endpoint = (endpoints.size() > 0) ? endpoints[0] : Expr();

    ctx->addStatement(Map::make({}, func, partialActuals, target, endpoint));
  }

    break;

  case 56:

    {
    auto varNames = unique_ptr<vector<string>>((yystack_[7].value.strings));
    vector<Expr> partialActuals = convertAndDelete((yystack_[3].value.exprs));
    string funcName = convertAndFree((yystack_[4].value.string));
    string targetName = convertAndFree((yystack_[1].value.string));
    ReductionOperator reduction((yystack_[0].value.reductionop));

    if (!ctx->containsFunction(funcName)) {
      REPORT_ERROR("undefined function '" + funcName + "'", yystack_[4].location);
    }
    Func func = ctx->getFunction(funcName);

    if (!ctx->hasSymbol(targetName)) {
      REPORT_ERROR("undefined set '" + targetName + "'", yystack_[1].location);
    }
    Expr target = ctx->getSymbol(targetName).getExpr();

    if (!target.type().isSet()) {
      REPORT_ERROR("maps can only be applied to sets", yystack_[1].location);
    }

    // Get endpoints
    std::vector<Expr> endpoints;
    for (Expr *endpoint : target.type().toSet()->endpointSets) {
      endpoints.push_back(*endpoint);
    }

    if (varNames->size() != func.getResults().size()) {
      REPORT_ERROR("the number of variables (" + to_string(varNames->size()) +
                   ") does not match the number of results returned by " +
                   func.getName() + " (" +
                   to_string(func.getResults().size()) + ")", yystack_[7].location);
    }

    size_t numFormals = func.getArguments().size();
    size_t numActuals = partialActuals.size() + 1 + (endpoints.size()>0 ?1:0);
    if (!(numFormals == numActuals ||
          (endpoints.size() > 0 && numFormals == numActuals-1))) {
      REPORT_ERROR("the number of actuals (" + to_string(numActuals) +
                   ") does not match the number of formals accepted by " +
                   func.getName() + " (" +
                   to_string(func.getArguments().size()) + ")", yystack_[7].location);
    }

    Var targetFormal = func.getArguments()[partialActuals.size()];
    if (target.type().toSet()->elementType != targetFormal.getType()){
      REPORT_ERROR("the mapped set's element type is different from the mapped "
                   "function's target argument", yystack_[1].location);
    }

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

    // We assume the edge set is homogeneous for now
    Expr endpoint = (endpoints.size() > 0) ? endpoints[0] : Expr();

    ctx->addStatement(Map::make(vars, func, partialActuals, target, endpoint,
                                reduction));
  }

    break;

  case 57:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 58:

    {
    (yylhs.value.exprs) = (yystack_[1].value.exprs);
  }

    break;

  case 59:

    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }

    break;

  case 60:

    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
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
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression is null" ;

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
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression is null" ;
 
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
    iassert((yystack_[1].value.expr) != nullptr) << "assigned expression is null" ;

    std::string tensorName = convertAndFree((yystack_[9].value.string));
    vector<Expr> indices1 = convertAndDelete((yystack_[7].value.exprs));
    vector<Expr> indices2 = convertAndDelete((yystack_[4].value.exprs));
    Expr value = convertAndDelete((yystack_[1].value.expr));

    if (!ctx->hasSymbol(tensorName)) {
      REPORT_ERROR(tensorName + " is not defined in scope", yystack_[9].location);
    }

    const Symbol &tensorSymbol = ctx->getSymbol(tensorName);
    if (!tensorSymbol.isWritable()) {
      REPORT_ERROR(tensorName + " is not writable", yystack_[9].location);
    }

    Expr tensorExpr = TensorRead::make(tensorSymbol.getExpr(), indices1);
    ctx->addStatement(TensorWrite::make(tensorExpr, indices2, value));
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
    Stmt ifStmt = convertAndDelete((yystack_[2].value.stmt));
    Stmt elseStmt = convertAndDelete((yystack_[1].value.stmt));
    Stmt *result = new Stmt(IfThenElse::make(cond, ifStmt, elseStmt));
    ctx->addStatement(*result);
    (yylhs.value.stmt) = result;
  }

    break;

  case 73:

    {ctx->scope();}

    break;

  case 74:

    {ctx->unscope();}

    break;

  case 75:

    {
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[1].value.stmt)));
  }

    break;

  case 76:

    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }

    break;

  case 77:

    {ctx->scope();}

    break;

  case 78:

    {ctx->unscope();}

    break;

  case 79:

    {
    (yylhs.value.stmt) = new Stmt(convertAndDelete((yystack_[1].value.stmt)));
  }

    break;

  case 80:

    {ctx->scope();}

    break;

  case 81:

    {ctx->unscope();}

    break;

  case 82:

    {
    Expr cond = convertAndDelete((yystack_[4].value.expr));
    Stmt ifStmt = convertAndDelete((yystack_[2].value.stmt));
    Stmt elseStmt = convertAndDelete((yystack_[0].value.stmt));
    Stmt *result = new Stmt(IfThenElse::make(cond, ifStmt, elseStmt));
    ctx->addStatement(*result);
    (yylhs.value.stmt) = result;
  }

    break;

  case 83:

    {    
    if((yystack_[2].value.indexSet)->getKind()==IndexSet::Set){
      ctx->addStatement(For::make(*(yystack_[3].value.var),ForDomain(*(yystack_[2].value.indexSet)), *(yystack_[1].value.stmt)));
    }
    delete (yystack_[3].value.var);
    delete (yystack_[2].value.indexSet);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 84:

    {    
    ctx->addStatement(ForRange::make(*(yystack_[5].value.var), *(yystack_[4].value.expr), *(yystack_[2].value.expr), *(yystack_[1].value.stmt)));
    delete (yystack_[5].value.var);
    delete (yystack_[4].value.expr);
    delete (yystack_[2].value.expr);
    delete (yystack_[1].value.stmt);
  }

    break;

  case 85:

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

  case 86:

    {
    ctx->unscope();
  }

    break;

  case 87:

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

  case 88:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 89:

    {
    (yylhs.value.stmt) = NULL;
  }

    break;

  case 90:

    {
     Expr expr = Expr(convertAndDelete((yystack_[1].value.expr)));
     ctx->addStatement(Stmt(Print::make(expr)));
  }

    break;

  case 98:

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

  case 99:

    {
    iassert((yystack_[1].value.expr) != nullptr) << "expression in parenthesis is null" ;
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 100:

    {
    iassert((yystack_[0].value.expr) != nullptr) << "negated expression is null" ;

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(IRBuilder::Neg, expr));
  }

    break;

  case 101:

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

  case 102:

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

  case 103:

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

  case 104:

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

  case 105:

    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 106:

    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }

    break;

  case 107:

    { (yylhs.value.binop) = IRBuilder::Add; }

    break;

  case 108:

    { (yylhs.value.binop) = IRBuilder::Sub; }

    break;

  case 109:

    { (yylhs.value.binop) = IRBuilder::Mul; }

    break;

  case 110:

    { (yylhs.value.binop) = IRBuilder::Div; }

    break;

  case 111:

    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }

    break;

  case 112:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Eq::make(l, r));
  }

    break;

  case 113:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ne::make(l, r));
  }

    break;

  case 114:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Gt::make(l, r));
  }

    break;

  case 115:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Lt::make(l, r));
  }

    break;

  case 116:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Ge::make(l, r));
  }

    break;

  case 117:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Le::make(l, r));
  }

    break;

  case 118:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(And::make(l, r));
  }

    break;

  case 119:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Or::make(l, r));
  }

    break;

  case 120:

    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Not::make(r));
  }

    break;

  case 121:

    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));
    (yylhs.value.expr) = new Expr(Xor::make(l, r));
  }

    break;

  case 122:

    {
    bool val = true;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 123:

    {
    bool val = false;
    (yylhs.value.expr) = new Expr(Literal::make(val));
  }

    break;

  case 124:

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

  case 128:

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
        // check if we need to turn this into an index expression
        // TODO: This logic needs to be in all paths that can create tensor
        // reads.
        int numSlices = 0;
        for (auto arg: *arguments) {
          if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":")
            numSlices++;
        }
        if (numSlices > 0) {
         // We will construct an index expression.
         // first, we build IndexVars
         std::vector<IndexVar> allivars;
         std::vector<IndexVar> freeVars;
         int i=0;
         for (auto &arg: *arguments) {
          if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":") {
            auto iv = IndexVar("tmpfree" + std::to_string(i),
                readExpr.type().toTensor()->dimensions[i]);
            allivars.push_back(iv);
            freeVars.push_back(iv);
          }
          else {
            allivars.push_back(IndexVar("tmpfixed" + std::to_string(i),
                readExpr.type().toTensor()->dimensions[i],
              new Expr(arg)));
          }
          i++;
         }
            // now construct an index expression
         (yylhs.value.expr) = new Expr(IndexExpr::make(freeVars,
            IndexedTensor::make(readExpr, allivars)));
        }
        else {
        
          (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, *arguments));
        }
        
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

      if (func.getResults().size() > 1) {
        REPORT_ERROR("unassigned calls must have zero or one result", yystack_[3].location);
      }

      vector<Var> vars;
      if (func.getResults().size() == 1) {
        Var resultFormal = func.getResults()[0];
        Var var = ctx->getBuilder()->temporary(resultFormal.getType());;
        ctx->addSymbol(var.getName(), var, Symbol::ReadWrite);
        ctx->addStatement(VarDecl::make(var));
        (yylhs.value.expr) = new Expr(VarExpr::make(var));

        vars.push_back(var);
      }

      ctx->addStatement(CallStmt::make(vars, func, *arguments));
    }
    else {
      REPORT_ERROR(name + " is not defined in scope", yystack_[3].location);
    }
  }

    break;

  case 129:

    {
    Expr readExpr = convertAndDelete((yystack_[3].value.expr));
    vector<Expr> indices = convertAndDelete((yystack_[1].value.exprs));

    // The parenthesis read can be a read from a tensor or a tuple.
    if (readExpr.type().isTensor()) {
      (yylhs.value.expr) = new Expr(TensorRead::make(readExpr, indices));
    }
    else if (readExpr.type().isTuple()) {
      if (indices.size() != 1) {
        REPORT_ERROR("reading a tuple requires exactly one index", yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(TupleRead::make(readExpr, indices[0]));
    }
    else {
      REPORT_ERROR("can only access components in tensors and tuples", yystack_[3].location);
    }
  }

    break;

  case 130:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 131:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 132:

    {
    iassert((yystack_[0].value.expr));
    (yylhs.value.exprs) = new std::vector<Expr>();
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 133:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    (yylhs.value.exprs)->push_back(*(new Expr(VarExpr::make(Var(":", Type())))));
  }

    break;

  case 134:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(*(new Expr(VarExpr::make(Var(":", Type())))));
  }

    break;

  case 135:

    {
    iassert((yystack_[0].value.expr));
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 140:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 141:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 142:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 143:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 144:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 145:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 146:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 147:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 148:

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

  case 149:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 150:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 151:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 152:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 153:

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

  case 154:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 155:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 156:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 157:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Boolean);
  }

    break;

  case 160:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 162:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values));
  }

    break;

  case 163:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 164:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 166:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 167:

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

  case 168:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 169:

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

  case 170:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 171:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 172:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 174:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 175:

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

  case 176:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 177:

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

  case 178:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 179:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 180:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.num)));
  }

    break;

  case 181:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.fnum)));
  }

    break;

  case 182:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.boolean)));
  }

    break;

  case 183:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 184:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 185:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 186:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 187:

    {
    (yylhs.value.boolean) = true;
  }

    break;

  case 188:

    {
    (yylhs.value.boolean) = false;
  }

    break;

  case 189:

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

  case 190:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::SetBase*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 192:

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


  const signed char  Parser ::yypact_ninf_ = -127;

  const short int  Parser ::yytable_ninf_ = -184;

  const short int
   Parser ::yypact_[] =
  {
    -127,    86,  -127,    12,    58,    15,  -127,  -127,   106,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,    78,   101,   168,
     103,  -127,   184,   189,     7,    11,   245,   239,   142,  -127,
     172,    31,  -127,   172,    31,    36,   197,   166,  -127,  -127,
    -127,  -127,  -127,    37,     6,  -127,  -127,   188,   214,  -127,
    -127,  -127,  -127,   176,   239,    15,  -127,  -127,    87,   236,
     237,   351,   351,   240,   351,   351,    47,  -127,   356,   351,
    -127,  -127,   216,    31,  -127,  -127,  -127,  -127,    32,  -127,
    -127,  -127,    31,  -127,  -127,    56,  -127,  -127,  -127,   389,
     213,   215,  -127,  -127,  -127,   218,   219,  -127,  -127,   203,
    -127,  -127,  -127,  -127,   234,   230,  -127,   684,   232,   226,
    -127,  -127,   225,   245,   245,    13,   165,  -127,   214,   221,
    -127,   241,   229,  -127,    36,   271,    43,   243,   684,   829,
     829,   247,   417,   448,   254,    20,   180,   253,   249,   233,
     250,   256,   252,   255,   257,  -127,  -127,  -127,  -127,   100,
     684,  -127,  -127,   279,   347,  -127,   266,   743,   175,  -127,
     476,    31,   351,   351,    36,   351,   351,  -127,  -127,  -127,
     351,   351,  -127,  -127,   351,  -127,   351,   351,   351,   351,
     351,   351,   351,   295,  -127,  -127,    36,   248,   326,   298,
      -3,   -16,  -127,  -127,    70,  -127,  -127,  -127,   258,   269,
     305,   276,    15,    -7,    74,  -127,   351,    36,   292,   208,
      31,  -127,  -127,  -127,  -127,    69,   145,  -127,  -127,  -127,
     277,    16,    16,  -127,   278,    27,    27,  -127,  -127,   282,
     317,   507,  -127,  -127,   351,   293,   771,   771,   286,   876,
     876,   100,   100,   100,   712,   855,   855,   876,   876,   684,
     684,  -127,  -127,   161,   165,  -127,   684,   306,  -127,  -127,
    -127,   291,    13,  -127,   319,   320,   324,  -127,  -127,   128,
      36,   351,   535,   328,   358,   351,  -127,   335,  -127,  -127,
    -127,    16,   364,   250,  -127,    27,   367,   257,  -127,    36,
     243,  -127,   283,  -127,  -127,  -127,  -127,   323,   366,   245,
    -127,   368,  -127,   236,    36,   351,   170,   566,  -127,  -127,
    -127,   829,    31,  -127,  -127,   146,   148,   173,   352,    36,
     110,   293,  -127,   327,   -11,  -127,   179,   183,  -127,  -127,
     187,   594,   329,  -127,  -127,  -127,  -127,  -127,   802,   373,
     448,  -127,   165,  -127,  -127,   374,  -127,   236,   332,  -127,
     351,  -127,  -127,   357,   349,  -127,  -127,   351,   625,   208,
     333,  -127,   165,   653,  -127,  -127,  -127,  -127,  -127,  -127
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,     0,     0,     0,    15,    20,     0,     3,
       7,     4,     5,     6,     8,     9,    12,     0,     0,     0,
       0,    29,     0,     0,     0,     0,     0,     0,     0,    10,
      18,    35,    19,     0,    35,   130,     0,     0,    11,    13,
     155,   156,   157,     0,     0,   146,   140,     0,     0,    52,
     136,   137,   138,   139,     0,    24,   183,   185,    98,     0,
       0,     0,     0,     0,     0,     0,     0,    89,     0,     0,
     122,   123,     0,    36,    37,    39,    40,    41,     0,    42,
      43,    45,    35,    44,    46,     0,    47,    48,    49,     0,
      91,    92,    94,    95,    96,     0,    97,    93,   158,   159,
     161,   180,   181,   182,     0,    98,   133,   132,     0,   131,
     192,   191,     0,     0,     0,     0,     0,   149,     0,     0,
      28,     0,    25,    26,     0,     0,     0,    57,     0,    69,
      73,     0,     0,     0,    95,     0,     0,     0,   165,   164,
     168,     0,   173,   172,   176,   178,   170,   183,   185,   100,
     120,    16,    38,     0,     0,    70,     0,   152,    98,   154,
       0,    35,     0,     0,   130,     0,     0,    88,   107,   108,
       0,     0,   109,   110,     0,   104,     0,     0,     0,     0,
       0,     0,     0,     0,   160,    21,     0,     0,     0,     0,
       0,     0,   152,   153,     0,   150,   187,   188,     0,     0,
       0,    30,     0,     0,     0,    50,     0,   130,     0,    76,
      35,    85,    90,    99,   111,     0,     0,   184,   186,   162,
       0,     0,     0,   163,     0,     0,     0,    17,    60,    98,
       0,     0,    71,    68,     0,     0,   118,   119,     0,   115,
     114,   102,   103,   105,   106,   112,   113,   117,   116,   121,
     101,   124,    22,     0,     0,   134,   135,     0,   190,    14,
     147,     0,     0,    87,   141,     0,     0,    23,    27,   128,
       0,     0,     0,     0,     0,     0,    77,     0,    74,   166,
     174,     0,     0,   169,   171,     0,     0,   177,   179,     0,
      57,    54,    35,    86,    83,   129,   128,     0,     0,     0,
     151,     0,   145,     0,     0,     0,     0,     0,    51,    58,
      55,    80,    35,    72,    75,     0,     0,     0,     0,   130,
     108,     0,   189,     0,     0,   143,     0,     0,    32,    34,
       0,     0,     0,    64,    73,    78,   167,   175,    53,     0,
     132,    84,     0,   148,   142,     0,    31,     0,     0,    65,
       0,    81,    79,    61,     0,   144,    33,     0,     0,    76,
       0,    56,     0,     0,    66,    82,    63,    62,   193,    67
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,   365,  -127,  -127,   -42,  -127,  -127,
      48,   -29,  -127,   331,  -127,   -58,  -127,  -127,   111,  -127,
    -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,  -127,    66,
    -127,  -127,    46,  -127,  -127,  -127,  -127,  -127,  -127,    81,
     405,  -127,  -127,   -35,  -127,  -127,  -127,  -127,   -55,  -127,
    -127,  -127,   -24,  -116,   353,   -33,  -127,  -127,  -127,   -23,
    -127,  -101,  -127,  -127,  -114,  -127,  -127,  -127,  -119,   191,
    -127,  -127,  -126,   185,  -127,   -62,   -54,  -127,  -127,  -127,
    -127
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,     9,    10,    11,    25,    39,    12,    22,   227,
      31,    13,    23,   252,    32,   121,   122,    20,   267,   327,
     328,    72,    73,    74,    75,    21,    76,    77,   208,    78,
     361,   367,    79,    80,    81,    82,   156,   233,    83,   209,
     210,   314,   277,   312,   352,   334,   359,    84,    85,   294,
      86,    87,    88,    89,    90,    91,    92,   182,    93,    94,
      95,    96,   238,   109,    49,    50,    51,   326,    52,    53,
     194,   161,    45,    97,    98,    99,   137,   138,   139,   140,
     141,   142,   143,   144,   100,   101,   102,   103,    15,   112,
     258
  };

  const short int
   Parser ::yytable_[] =
  {
     107,   126,   198,    44,   145,   104,   129,   130,   203,   216,
     134,   108,   146,   123,   195,   119,   215,   192,    37,    16,
     193,    57,    18,   260,    56,    57,   128,   128,   343,   132,
     133,    56,   269,   149,   150,    56,    57,    19,    58,   188,
      56,    57,   117,   105,    38,    35,   259,   117,    59,     4,
     160,    56,    57,   155,    60,   117,   116,    36,    61,    62,
     157,    57,    63,   158,   117,    17,   159,    64,   282,    65,
     253,    66,   136,   145,    65,   114,    66,   115,   153,   286,
      67,   146,   154,    68,   106,   199,     2,   135,    68,   107,
     190,   191,   205,   206,    65,    69,    66,    70,    71,   136,
      69,     3,    70,    71,     4,     5,     6,     7,    68,   159,
     279,   261,   270,    24,   147,   148,   262,   105,   221,   231,
      69,     8,    70,    71,   271,   124,    26,   236,   237,   107,
     239,   240,   235,   -59,   125,   241,   242,   -59,   164,   243,
     297,   244,   245,   246,   247,   248,   249,   250,    65,    27,
      66,   107,    29,   256,   306,   172,   173,   174,   175,   316,
     268,   300,   315,   145,   288,   181,   304,   146,   284,    56,
      57,   272,   107,   317,    69,    28,    70,    71,   305,  -153,
    -153,   278,  -153,   273,   217,   218,   280,   336,   330,   337,
      54,    30,  -153,  -153,   225,   221,    33,   225,  -153,   292,
     296,   110,  -153,  -153,   111,    66,  -153,   188,  -153,   332,
      55,  -153,   338,   186,   113,  -153,   188,   136,   344,   188,
     311,    46,   346,   145,  -153,   345,   348,   146,   354,   347,
     118,   196,   197,   188,   117,   107,   307,   275,   276,  -153,
     128,  -153,  -153,    18,   127,   329,    46,   131,   368,   151,
      40,    41,    42,    43,   107,    47,    40,    41,    42,    43,
    -125,   184,  -126,   321,   134,   183,  -127,   185,   186,   107,
     331,   187,   188,   189,   200,   202,   324,    48,   204,   211,
     201,   207,   221,   335,   340,   149,   228,    56,    57,   329,
      58,   162,   163,   214,   219,   220,   222,   223,   224,   232,
      59,     4,   251,   226,   225,   257,    60,   263,   254,   265,
      61,    62,   264,   266,    63,   358,   274,   281,   285,    64,
     289,   319,   363,    66,   290,   295,   293,   165,   166,   299,
      56,    57,    67,   105,   168,   320,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,    69,   181,    70,
      71,    56,    57,   298,   229,    56,    57,   301,   105,   302,
     147,   148,   303,   105,    65,   310,    66,   309,   313,   218,
     230,   217,   322,   323,   255,   325,   339,   342,    68,   350,
     353,   355,   357,   360,   366,    65,   362,    66,    34,    65,
      69,    66,    70,    71,    65,   356,    66,   162,   163,    68,
     351,   318,   341,    68,   152,   365,    14,   120,    68,     0,
     287,    69,   283,    70,    71,    69,     0,    70,    71,     0,
      69,     0,    70,    71,     0,   162,   163,   164,     0,     0,
       0,     0,     0,   165,   166,     0,     0,     0,   167,     0,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,     0,   181,   164,   162,   163,     0,     0,
       0,   165,   166,     0,     0,     0,   212,     0,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,     0,   181,     0,   162,   163,   164,   213,     0,     0,
       0,     0,   165,   166,     0,     0,     0,     0,     0,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,     0,   181,   164,   162,   163,     0,     0,     0,
     165,   166,     0,     0,   234,     0,     0,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   181,     0,   162,   163,   164,     0,     0,     0,     0,
       0,   165,   166,     0,     0,     0,   291,     0,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,     0,   181,   164,   162,   163,     0,     0,     0,   165,
     166,     0,     0,     0,   308,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,     0,
     181,     0,   162,   163,   164,     0,     0,     0,     0,     0,
     165,   166,     0,     0,     0,   333,     0,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   181,   164,   162,   163,     0,     0,     0,   165,   166,
       0,     0,     0,   349,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   181,
       0,   162,   163,   164,     0,     0,     0,     0,     0,   165,
     166,     0,     0,     0,   364,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,     0,
     181,   164,   162,   163,     0,     0,     0,   165,   166,     0,
       0,     0,   369,     0,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,     0,   181,     0,
     162,   163,   164,     0,     0,     0,     0,     0,   165,   166,
       0,     0,     0,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   181,
     164,  -183,  -183,     0,     0,     0,   165,   166,     0,     0,
       0,     0,     0,   168,   169,   170,   171,   172,   173,   174,
     175,  -184,   177,   178,   179,   180,     0,   181,     0,   162,
     163,     0,     0,     0,     0,     0,     0,  -183,  -183,     0,
       0,  -183,     0,     0,  -183,     0,  -183,  -183,  -183,  -183,
    -183,  -183,  -183,  -183,  -183,  -183,  -183,     0,  -183,   164,
    -128,  -128,     0,     0,     0,   165,   166,     0,     0,     0,
       0,     0,   168,   169,   170,   171,   172,   173,   174,   175,
       0,   177,   178,   179,   180,     0,   181,   -95,   -95,     0,
       0,     0,     0,     0,     0,     0,  -128,  -128,     0,  -128,
       0,     0,     0,  -128,     0,  -128,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,     0,  -128,     0,     0,
       0,     0,     0,   -95,   -95,     0,     0,     0,     0,     0,
     -95,     0,   -95,   -95,   -95,   -95,   -95,   -95,   -95,   -95,
     -95,   -95,   -95,   164,   -95,     0,     0,     0,     0,   165,
     166,     0,     0,     0,     0,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   164,     0,     0,   179,   180,     0,
     181,     0,     0,     0,     0,     0,     0,   168,   169,   170,
     171,   172,   173,   174,   175,     0,     0,     0,     0,     0,
       0,   181
  };

  const short int
   Parser ::yycheck_[] =
  {
      35,    59,   116,    26,    66,    34,    61,    62,   124,   135,
      65,    35,    66,    55,   115,    48,   135,     4,     7,     7,
       7,     5,     7,    39,     4,     5,    61,    62,    39,    64,
      65,     4,    39,    68,    69,     4,     5,    22,     7,    46,
       4,     5,    58,     7,    33,    38,    49,    58,    17,    18,
      85,     4,     5,    82,    23,    58,    50,    50,    27,    28,
       4,     5,    31,     7,    58,     7,    53,    36,    52,    38,
     186,    40,    52,   135,    38,    38,    40,    40,    46,    52,
      49,   135,    50,    52,    48,   118,     0,    40,    52,   124,
     113,   114,    49,    50,    38,    64,    40,    66,    67,    52,
      64,    15,    66,    67,    18,    19,    20,    21,    52,    53,
      41,    41,    38,     7,     4,     5,    46,     7,    49,   154,
      64,    35,    66,    67,    50,    38,    48,   162,   163,   164,
     165,   166,   161,    46,    47,   170,   171,    50,    38,   174,
     254,   176,   177,   178,   179,   180,   181,   182,    38,    48,
      40,   186,    49,   188,   270,    55,    56,    57,    58,   285,
     202,   262,   281,   225,   226,    65,    38,   221,   222,     4,
       5,   206,   207,   289,    64,     7,    66,    67,    50,     4,
       5,   210,     7,   207,     4,     5,    41,    41,   304,    41,
      48,     7,    17,    18,    49,    49,     7,    49,    23,   234,
      39,     4,    27,    28,     7,    40,    31,    46,    33,    39,
      38,    36,    39,    38,    48,    40,    46,    52,    39,    46,
     275,     7,    39,   285,    49,    46,    39,   281,   342,    46,
      42,    66,    67,    46,    58,   270,   271,    29,    30,    64,
     275,    66,    67,     7,     7,   303,     7,     7,   362,    33,
      11,    12,    13,    14,   289,    16,    11,    12,    13,    14,
      47,    58,    47,   292,   319,    47,    47,    33,    38,   304,
     305,    39,    46,    48,    53,    46,   299,    38,     7,    32,
      39,    38,    49,   312,   319,   320,     7,     4,     5,   347,
       7,     8,     9,    39,    41,    46,    46,    41,    46,    33,
      17,    18,     7,    46,    49,     7,    23,    49,    60,     4,
      27,    28,    43,    37,    31,   350,    24,    40,    40,    36,
      38,    38,   357,    40,     7,    39,    33,    44,    45,    38,
       4,     5,    49,     7,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,     4,     5,    47,     7,     4,     5,    38,     7,    39,
       4,     5,    38,     7,    38,     7,    40,    39,    33,     5,
      23,     4,    49,     7,    48,     7,    24,    50,    52,    50,
       7,     7,    50,    26,    51,    38,    37,    40,    23,    38,
      64,    40,    66,    67,    38,   347,    40,     8,     9,    52,
     334,   290,   321,    52,    73,   359,     1,    54,    52,    -1,
     225,    64,   221,    66,    67,    64,    -1,    66,    67,    -1,
      64,    -1,    66,    67,    -1,     8,     9,    38,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    65,    38,     8,     9,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    65,    -1,     8,     9,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    65,    38,     8,     9,    -1,    -1,    -1,
      44,    45,    -1,    -1,    48,    -1,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    -1,     8,     9,    38,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    65,    38,     8,     9,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      65,    -1,     8,     9,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    38,     8,     9,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    65,
      -1,     8,     9,    38,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      65,    38,     8,     9,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    65,    -1,
       8,     9,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    65,
      38,     8,     9,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    48,    -1,    -1,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    65,    38,
       8,     9,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      -1,    60,    61,    62,    63,    -1,    65,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    47,
      -1,    -1,    -1,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    38,    65,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    38,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    65
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    69,     0,    15,    18,    19,    20,    21,    35,    70,
      71,    72,    75,    79,   118,   156,     7,     7,     7,    22,
      85,    93,    76,    80,     7,    73,    48,    48,     7,    49,
       7,    78,    82,     7,    82,    38,    50,     7,    33,    74,
      11,    12,    13,    14,   137,   140,     7,    16,    38,   132,
     133,   134,   136,   137,    48,    38,     4,     5,     7,    17,
      23,    27,    28,    31,    36,    38,    40,    49,    52,    64,
      66,    67,    89,    90,    91,    92,    94,    95,    97,   100,
     101,   102,   103,   106,   115,   116,   118,   119,   120,   121,
     122,   123,   124,   126,   127,   128,   129,   141,   142,   143,
     152,   153,   154,   155,    89,     7,    48,   121,   130,   131,
       4,     7,   157,    48,    38,    40,    50,    58,    42,   133,
     132,    83,    84,    85,    38,    47,    93,     7,   121,   126,
     126,     7,   121,   121,   126,    40,    52,   144,   145,   146,
     147,   148,   149,   150,   151,   153,   154,     4,     5,   121,
     121,    33,    91,    46,    50,    89,   104,     4,     7,    53,
     121,   139,     8,     9,    38,    44,    45,    49,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    65,   125,    47,    58,    33,    38,    39,    46,    48,
     137,   137,     4,     7,   138,   139,    66,    67,   142,   133,
      53,    39,    46,   131,     7,    49,    50,    38,    96,   107,
     108,    32,    49,    39,    39,   146,   150,     4,     5,    41,
      46,    49,    46,    41,    46,    49,    46,    77,     7,     7,
      23,   121,    33,   105,    48,    89,   121,   121,   130,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,     7,    81,   131,    60,    48,   121,     7,   158,    49,
      39,    41,    46,    49,    43,     4,    37,    86,    85,    39,
      38,    50,   121,   130,    24,    29,    30,   110,    89,    41,
      41,    40,    52,   147,   154,    40,    52,   151,   153,    38,
       7,    49,   121,    33,   117,    39,    39,   142,    47,    38,
     139,    38,    39,    38,    38,    50,   131,   121,    49,    39,
       7,   126,   111,    33,   109,   146,   150,   131,    96,    38,
      52,    89,    49,     7,   137,     7,   135,    87,    88,    93,
     131,   121,    39,    49,   113,    89,    41,    41,    39,    24,
     121,   117,    50,    39,    39,    46,    39,    46,    39,    49,
      50,   107,   112,     7,   142,     7,    88,    50,   121,   114,
      26,    98,    37,   121,    49,   110,    51,    99,   142,    49
  };

  const unsigned char
   Parser ::yyr1_[] =
  {
       0,    68,    69,    69,    70,    70,    70,    70,    70,    70,
      71,    72,    73,    73,    74,    76,    77,    75,    78,    78,
      80,    81,    79,    82,    83,    83,    84,    84,    85,    85,
      86,    86,    87,    87,    88,    89,    89,    90,    90,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      92,    92,    93,    94,    94,    95,    95,    96,    96,    97,
      97,    98,    98,    99,   100,   101,   101,   101,   102,   103,
     104,   105,   106,   108,   109,   107,   110,   111,   112,   110,
     113,   114,   110,   115,   115,   116,   117,   118,   119,   119,
     120,   121,   121,   121,   121,   121,   121,   121,   122,   123,
     124,   124,   124,   124,   124,   124,   124,   125,   125,   125,
     125,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   127,   128,   128,   128,   129,   129,
     130,   130,   131,   131,   131,   131,   132,   132,   132,   132,
     133,   134,   134,   135,   135,   136,   137,   137,   137,   137,
     138,   138,   139,   139,   139,   140,   140,   140,   141,   142,
     142,   142,   143,   143,   144,   144,   145,   145,   146,   146,
     147,   147,   148,   148,   149,   149,   150,   150,   151,   151,
     152,   152,   152,   153,   153,   154,   154,   155,   155,   156,
     156,   157,   157,   158
  };

  const unsigned char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       3,     4,     0,     2,     4,     0,     0,     6,     1,     1,
       0,     0,     6,     5,     0,     1,     1,     3,     4,     1,
       0,     4,     1,     3,     1,     0,     1,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     5,     3,     6,     4,     5,     8,     0,     3,     1,
       3,     0,     2,     1,     6,     7,     9,    10,     3,     2,
       1,     1,     5,     0,     0,     3,     0,     0,     0,     4,
       0,     0,     6,     4,     6,     3,     1,     7,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     3,     3,     3,     2,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     3,     1,     1,     3,     1,     1,     1,     4,     4,
       0,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     4,     7,     1,     3,     5,     1,     4,     7,     2,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     3,     3,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     1,     1,     2,     1,     2,     1,     1,     8,
       6,     1,     1,     7
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "\"int literal\"",
  "\"float literal\"", "\"string literal\"", "\"identifier\"", "\"and\"",
  "\"or\"", "NEG", "\"int\"", "\"float\"", "\"bool\"", "\"tensor\"",
  "\"element\"", "\"set\"", "\"var\"", "\"const\"", "\"extern\"",
  "\"proc\"", "\"func\"", "\"inout\"", "\"map\"", "\"to\"", "\"with\"",
  "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"", "\"else\"", "\"for\"",
  "\"in\"", "\"end\"", "\"return\"", "\"%!\"", "\"print\"", "\"->\"",
  "\"(\"", "\")\"", "\"[\"", "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"",
  "\",\"", "\".\"", "\":\"", "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"",
  "\"/\"", "\".*\"", "\"./\"", "\"^\"", "\"'\"", "\"\\\\\"", "\"==\"",
  "\"!=\"", "\"<=\"", "\">=\"", "\"not\"", "\"xor\"", "\"true\"",
  "\"false\"", "$accept", "program", "program_element", "extern",
  "element_type_decl", "field_decl_list", "field_decl", "procedure", "$@1",
  "$@2", "procedure_header", "function", "$@3", "$@4", "function_header",
  "arguments", "argument_list", "argument_decl", "results", "result_list",
  "result_decl", "stmt_block", "stmts", "stmt", "var_decl_stmt",
  "var_decl", "assign_stmt", "map_stmt", "partial_expr_list", "idents",
  "reduce", "reduce_op", "field_write_stmt", "tensor_write_stmt",
  "while_stmt", "while_stmt_header", "while_body", "while_end", "if_stmt",
  "if_body", "$@5", "$@6", "else_clauses", "$@7", "$@8", "$@9", "$@10",
  "for_stmt", "for_stmt_header", "for_stmt_footer", "const_stmt",
  "expr_stmt", "print_stmt", "expr", "ident_expr", "paren_expr",
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
  "signed_float_literal", "boolean_literal", "test", "system_generator",
  "extern_assert", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   287,   287,   289,   292,   293,   301,   309,   310,   311,
     316,   326,   339,   342,   350,   360,   360,   360,   368,   380,
     394,   394,   394,   402,   436,   439,   445,   450,   458,   465,
     470,   473,   479,   484,   492,   498,   501,   508,   509,   512,
     513,   514,   515,   516,   517,   518,   519,   520,   521,   522,
     527,   537,   559,   568,   695,   736,   787,   867,   870,   877,
     881,   888,   891,   897,   902,   922,   944,   966,   988,   997,
    1004,  1009,  1015,  1026,  1026,  1026,  1032,  1035,  1035,  1035,
    1038,  1038,  1038,  1048,  1056,  1066,  1077,  1082,  1110,  1113,
    1118,  1126,  1127,  1128,  1129,  1130,  1131,  1132,  1138,  1158,
    1167,  1175,  1194,  1262,  1282,  1310,  1315,  1324,  1325,  1326,
    1327,  1333,  1337,  1343,  1349,  1355,  1361,  1367,  1373,  1379,
    1385,  1390,  1396,  1400,  1408,  1442,  1443,  1444,  1451,  1536,
    1557,  1560,  1569,  1575,  1579,  1583,  1593,  1594,  1595,  1596,
    1600,  1612,  1616,  1626,  1635,  1647,  1659,  1663,  1666,  1708,
    1718,  1723,  1731,  1734,  1748,  1754,  1757,  1760,  1810,  1814,
    1815,  1819,  1823,  1830,  1841,  1848,  1852,  1856,  1870,  1874,
    1889,  1893,  1900,  1907,  1911,  1915,  1929,  1933,  1948,  1952,
    1959,  1962,  1965,  1971,  1974,  1980,  1983,  1989,  1992,  1999,
    2018,  2042,  2043,  2051
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
      65,    66,    67
    };
    const unsigned int user_token_number_max_ = 322;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} } //  simit::internal 



