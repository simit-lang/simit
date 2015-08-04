// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

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
      errorStr << "type mismatch (" <<                               \
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
                                             ttype->getDimensions(),
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
#define yyclearin       (yyla.clear ())

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
    clear ();
  }

  template <typename Base>
  inline
  void
   Parser ::basic_symbol<Base>::clear ()
  {
    Base::clear ();
  }

  template <typename Base>
  inline
  bool
   Parser ::basic_symbol<Base>::empty () const
  {
    return Base::type_get () == empty_symbol;
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
    : type (empty_symbol)
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
   Parser ::by_type::clear ()
  {
    type = empty_symbol;
  }

  inline
  void
   Parser ::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
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
    : state (empty_state)
  {}

  inline
   Parser ::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
   Parser ::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
   Parser ::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
   Parser ::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
   Parser ::symbol_number_type
   Parser ::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
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
    that.type = empty_symbol;
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
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
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
    if (yyla.empty ())
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
    vector<Var> arguments = convertAndDelete((yystack_[2].value.vars));
    vector<Var> results = convertAndDelete((yystack_[0].value.vars));
    vector<Var> newArguments;

    for (Var &arg : arguments) {
      string argName = arg.getName();

      auto found = argName.find("___inout___");
      if (found != string::npos) {
        // this is an inout param
        string newName = argName.substr(0,found);
        auto newArg = Var(newName, arg.getType());
        ctx->addSymbol(newName, newArg, Symbol::ReadWrite);
        newArguments.push_back(newArg);
      } else {
        ctx->addSymbol(argName, arg, Symbol::Read);
        newArguments.push_back(arg);
      }
    }

    (yylhs.value.function) = new Func(name, newArguments, results, Stmt());

    for (Var &res : results) {
      ctx->addSymbol(res.getName(), res, Symbol::ReadWrite);
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

  case 29:

    {
    (yylhs.value.var) = (yystack_[0].value.var);
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
          vector<IndexVar> allivars;
          vector<IndexVar> freeVars;
          vector<IndexDomain> dimensions =
              readExpr.type().toTensor()->getDimensions();
          int i=0;
          for (auto &arg: arguments) {
            if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":") {
              auto iv = IndexVar("tmpfree" + std::to_string(i), dimensions[i]);
              allivars.push_back(iv);
              freeVars.push_back(iv);
            }
            else {
              allivars.push_back(IndexVar("tmpfixed" + std::to_string(i),
                                          dimensions[i],
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

    if (target.type().toSet()->elementType != func.getArguments()[partialActuals.size()].getType()){
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
    iassert((yystack_[3].value.expr)) << "condition expression is null";

    Expr cond = convertAndDelete((yystack_[3].value.expr));
    if (!isScalar(cond.type()) ||
        !cond.type().toTensor()->componentType.isBoolean()) {
      REPORT_ERROR("conditional expression is not boolean", yystack_[3].location);
    }

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
    iassert((yystack_[4].value.expr)) << "condition expression is null";

    Expr cond = convertAndDelete((yystack_[4].value.expr));
    if (!isScalar(cond.type()) ||
        !cond.type().toTensor()->componentType.isBoolean()) {
      REPORT_ERROR("conditional expression is not boolean", yystack_[4].location);
    }

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
    vector<IndexDomain> ldimensions = ltype->getDimensions();
    vector<IndexDomain> rdimensions = rtype->getDimensions();

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
      if (ldimensions[1] != rdimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      // TODO: Figure out how column vectors should be handled here
      if (ldimensions[0] != rdimensions[0]) {
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gevm(l,r));
    }
    // Matrix-Matrix
    else if (ltype->order() == 2 && rtype->order() == 2) {
      if (ldimensions[1] != rdimensions[0]){
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
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.boolean)));
  }

    break;

  case 123:

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

  case 127:

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
          vector<IndexVar> allivars;
          vector<IndexVar> freeVars;
          vector<IndexDomain> dimensions =
              readExpr.type().toTensor()->getDimensions();

          int i=0;
          for (auto &arg: *arguments) {
            if (isa<VarExpr>(arg) && to<VarExpr>(arg)->var.getName() == ":") {
              auto iv = IndexVar("tmpfree" + std::to_string(i),
                                 dimensions[i]);
              allivars.push_back(iv);
              freeVars.push_back(iv);
            }
            else {
              allivars.push_back(IndexVar("tmpfixed" + std::to_string(i),
                                          dimensions[i], new Expr(arg)));
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

  case 128:

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

  case 129:

    {
    (yylhs.value.exprs) = new vector<Expr>();
  }

    break;

  case 130:

    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }

    break;

  case 131:

    {
    iassert((yystack_[0].value.expr));
    (yylhs.value.exprs) = new std::vector<Expr>();
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 132:

    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    (yylhs.value.exprs)->push_back(*(new Expr(VarExpr::make(Var(":", Type())))));
  }

    break;

  case 133:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(*(new Expr(VarExpr::make(Var(":", Type())))));
  }

    break;

  case 134:

    {
    iassert((yystack_[0].value.expr));
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }

    break;

  case 139:

    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }

    break;

  case 140:

    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }

    break;

  case 141:

    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }

    break;

  case 142:

    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 143:

    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }

    break;

  case 144:

    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }

    break;

  case 145:

    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }

    break;

  case 146:

    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }

    break;

  case 147:

    {
    auto blockTypePtr = convertAndDelete((yystack_[1].value.type));
    const TensorType *blockType = blockTypePtr.toTensor();

    auto componentType = blockType->componentType;

    auto outerDimensions = unique_ptr<vector<IndexSet>>((yystack_[4].value.indexSets));
    auto blockDimensions = blockType->getDimensions();

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

  case 148:

    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->getDimensions();
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }

    break;

  case 149:

    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 150:

    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }

    break;

  case 151:

    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }

    break;

  case 152:

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

  case 153:

    {
    (yylhs.value.indexSet) = new IndexSet();
  }

    break;

  case 154:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }

    break;

  case 155:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }

    break;

  case 156:

    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Boolean);
  }

    break;

  case 159:

    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }

    break;

  case 161:

    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values));
  }

    break;

  case 162:

    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }

    break;

  case 163:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }

    break;

  case 165:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 166:

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

  case 167:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }

    break;

  case 168:

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

  case 169:

    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 170:

    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }

    break;

  case 171:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }

    break;

  case 173:

    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 174:

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

  case 175:

    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }

    break;

  case 176:

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

  case 177:

    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 178:

    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }

    break;

  case 179:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.num)));
  }

    break;

  case 180:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.fnum)));
  }

    break;

  case 181:

    {
    (yylhs.value.expr) = new Expr(Literal::make((yystack_[0].value.boolean)));
  }

    break;

  case 182:

    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }

    break;

  case 183:

    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }

    break;

  case 184:

    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }

    break;

  case 185:

    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }

    break;

  case 186:

    {
    (yylhs.value.boolean) = true;
  }

    break;

  case 187:

    {
    (yylhs.value.boolean) = false;
  }

    break;

  case 188:

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

  case 189:

    {
    std::string setName = convertAndFree((yystack_[4].value.string));
    unique_ptr<System> system((yystack_[2].value.system));

    //std::map<std::string, simit::Set*> externs;
    //externs[setName] = system->elements;
    //ctx->addTest(new ProcedureTest("test", externs));
  }

    break;

  case 191:

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
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
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
    if (!yyla.empty ())
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
        if (!yyla.empty ())
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
   Parser ::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
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
       - The only way there can be no lookahead present (in yyla) is
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
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
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

    std::string yyres;
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


  const signed char  Parser ::yypact_ninf_ = -128;

  const short int  Parser ::yytable_ninf_ = -183;

  const short int
   Parser ::yypact_[] =
  {
    -128,   258,  -128,    51,    69,    53,  -128,  -128,   101,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,    86,   104,   161,
     -24,  -128,   178,   184,   -14,    10,   252,   102,   124,  -128,
     157,    34,  -128,   157,    34,    40,    28,   155,  -128,  -128,
    -128,  -128,  -128,    82,    45,  -128,  -128,   163,   192,  -128,
    -128,  -128,  -128,   156,   102,    53,  -128,  -128,   129,   211,
     214,   350,   350,   219,   350,   350,    44,  -128,   355,   350,
    -128,  -128,   196,    34,  -128,  -128,  -128,  -128,    71,  -128,
    -128,  -128,    34,  -128,  -128,    59,  -128,  -128,  -128,   388,
     191,   195,  -128,  -128,  -128,   200,   203,  -128,  -128,   197,
    -128,  -128,  -128,  -128,   221,   222,  -128,   683,   228,   215,
    -128,  -128,   223,   252,   252,    24,    15,  -128,   192,   217,
    -128,   233,   234,  -128,    40,   267,    88,   243,   683,   828,
     683,   253,   416,   447,   255,    17,   151,   251,   249,   247,
     256,   257,   260,   248,   261,  -128,  -128,  -128,  -128,   175,
     683,  -128,  -128,   294,   346,  -128,   270,   742,   179,  -128,
     475,    34,   350,   350,    40,   350,   350,  -128,  -128,  -128,
     350,   350,  -128,  -128,   350,  -128,   350,   350,   350,   350,
     350,   350,   350,   297,  -128,  -128,    40,   259,   325,   301,
      75,   -21,  -128,  -128,    13,  -128,   262,  -128,   269,   311,
     279,    53,   148,    18,  -128,   350,    40,   293,   152,    34,
    -128,  -128,  -128,  -128,    61,   109,  -128,  -128,  -128,   283,
      35,    35,  -128,   284,    42,    42,  -128,  -128,   287,   321,
     506,  -128,  -128,   350,   319,   770,   770,   317,   875,   875,
     175,   175,   175,   711,   854,   854,   875,   875,   683,   683,
    -128,  -128,   154,    15,  -128,   683,   314,  -128,  -128,  -128,
     320,    24,  -128,   326,   327,   329,  -128,  -128,    55,    40,
     350,   534,   331,   361,   350,  -128,   338,  -128,  -128,  -128,
      35,   367,   256,  -128,    42,   370,   261,  -128,    40,   243,
    -128,   282,  -128,  -128,  -128,  -128,   330,   368,   252,  -128,
     369,  -128,   211,    40,   350,   162,   565,  -128,  -128,  -128,
     683,    34,  -128,  -128,   125,   128,   165,   354,    40,   185,
     319,  -128,   332,   -16,  -128,   170,   181,  -128,  -128,   202,
     593,   333,  -128,  -128,  -128,  -128,  -128,   801,   373,   447,
    -128,    15,  -128,  -128,   374,  -128,   211,   335,  -128,   350,
    -128,  -128,   375,   357,  -128,  -128,   350,   624,   152,   336,
    -128,    15,   652,  -128,  -128,  -128,  -128,  -128,  -128
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,     0,     1,     0,     0,     0,    15,    20,     0,     3,
       7,     4,     5,     6,     8,     9,    12,     0,     0,     0,
       0,    29,     0,     0,     0,     0,     0,     0,     0,    10,
      18,    35,    19,     0,    35,   129,     0,     0,    11,    13,
     154,   155,   156,     0,     0,   145,   139,     0,     0,    52,
     135,   136,   137,   138,     0,    24,   182,   184,    98,     0,
       0,     0,     0,     0,     0,     0,     0,    89,     0,     0,
     186,   187,     0,    36,    37,    39,    40,    41,     0,    42,
      43,    45,    35,    44,    46,     0,    47,    48,    49,     0,
      91,    92,    94,    95,    96,     0,    97,    93,   157,   158,
     160,   179,   180,   122,     0,    98,   132,   131,     0,   130,
     191,   190,     0,     0,     0,     0,     0,   148,     0,     0,
      28,     0,    25,    26,     0,     0,     0,    57,     0,    69,
      73,     0,     0,     0,    95,     0,     0,     0,   164,   163,
     167,     0,   172,   171,   175,   177,   169,   182,   184,   100,
     120,    16,    38,     0,     0,    70,     0,   151,    98,   153,
       0,    35,     0,     0,   129,     0,     0,    88,   107,   108,
       0,     0,   109,   110,     0,   104,     0,     0,     0,     0,
       0,     0,     0,     0,   159,    21,     0,     0,     0,     0,
       0,     0,   151,   152,     0,   149,     0,   181,     0,     0,
      30,     0,     0,     0,    50,     0,   129,     0,    76,    35,
      85,    90,    99,   111,     0,     0,   183,   185,   161,     0,
       0,     0,   162,     0,     0,     0,    17,    60,    98,     0,
       0,    71,    68,     0,     0,   118,   119,     0,   115,   114,
     102,   103,   105,   106,   112,   113,   117,   116,   121,   101,
     123,    22,     0,     0,   133,   134,     0,   189,    14,   146,
       0,     0,    87,   140,     0,     0,    23,    27,   127,     0,
       0,     0,     0,     0,     0,    77,     0,    74,   165,   173,
       0,     0,   168,   170,     0,     0,   176,   178,     0,    57,
      54,    35,    86,    83,   128,   127,     0,     0,     0,   150,
       0,   144,     0,     0,     0,     0,     0,    51,    58,    55,
      80,    35,    72,    75,     0,     0,     0,     0,   129,   108,
       0,   188,     0,     0,   142,     0,     0,    32,    34,     0,
       0,     0,    64,    73,    78,   166,   174,    53,     0,   131,
      84,     0,   147,   141,     0,    31,     0,     0,    65,     0,
      81,    79,    61,     0,   143,    33,     0,     0,    76,     0,
      56,     0,     0,    66,    82,    63,    62,   192,    67
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,   376,  -128,  -128,   -42,  -128,  -128,
      54,   -29,  -128,   342,  -128,   -58,  -128,  -128,   114,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,    72,
    -128,  -128,    46,  -128,  -128,  -128,  -128,  -128,  -128,    89,
     405,  -128,  -128,   -35,  -128,  -128,  -128,  -128,   -59,  -128,
    -128,  -128,   -28,  -115,   364,   -33,  -128,  -128,  -128,   -23,
    -128,  -101,  -128,  -128,  -105,  -128,  -128,  -128,  -119,   188,
    -128,  -128,  -127,   187,  -128,   -62,   -56,  -104,  -128,  -128,
    -128
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,     9,    10,    11,    25,    39,    12,    22,   226,
      31,    13,    23,   251,    32,   121,   122,    20,   266,   326,
     327,    72,    73,    74,    75,    21,    76,    77,   207,    78,
     360,   366,    79,    80,    81,    82,   156,   232,    83,   208,
     209,   313,   276,   311,   351,   333,   358,    84,    85,   293,
      86,    87,    88,    89,    90,    91,    92,   182,    93,    94,
      95,    96,   237,   109,    49,    50,    51,   325,    52,    53,
     194,   161,    45,    97,    98,    99,   137,   138,   139,   140,
     141,   142,   143,   144,   100,   101,   102,   103,    15,   112,
     257
  };

  const short int
   Parser ::yytable_[] =
  {
     107,   126,   129,    44,   145,   104,   134,   108,   215,   202,
     146,   196,   197,   123,   195,   119,   214,    37,   259,    56,
      57,    56,    57,   342,    35,    29,   128,   130,   192,   132,
     133,   193,   110,   149,   150,   111,    36,   117,    56,    57,
      57,    58,   117,    38,    56,    57,    56,   105,    56,    57,
     160,    59,     4,   155,   260,    66,   269,    60,    16,   261,
      18,    61,    62,   157,    57,    63,   158,   136,   270,   136,
      64,   252,    65,   145,    66,    19,    17,   159,    65,   146,
      66,    70,    71,    67,   135,   198,    68,   281,   106,   107,
     190,   191,    68,   303,   285,   116,   136,    65,    69,    66,
      70,    71,   278,   117,    69,   304,    70,    71,    24,    46,
     220,    68,   159,    40,    41,    42,    43,   153,    47,   230,
     114,   154,   115,    69,   258,    70,    71,   235,   236,   107,
     238,   239,   234,   117,    26,   240,   241,   204,   205,   242,
      48,   243,   244,   245,   246,   247,   248,   249,   296,   197,
     279,   107,    27,   255,   305,   216,   217,   315,   224,   267,
     299,   314,   145,   287,   146,   283,   335,   124,    28,   336,
     271,   107,    54,   316,   220,   -59,   125,   224,   272,   -59,
     277,   274,   275,  -152,  -152,    30,  -152,   268,   329,   147,
     148,    33,   105,   295,   188,    55,  -152,  -152,   291,    46,
     188,   331,  -152,   113,   337,   118,  -152,  -152,   188,   343,
    -152,   188,  -152,   164,   117,  -152,   344,   186,    18,  -152,
     345,   127,   145,    65,   146,    66,   131,   346,  -152,   151,
     172,   173,   174,   175,   107,   306,   353,   197,  -124,   310,
     181,   347,  -125,  -152,   328,  -152,  -152,   183,   188,    69,
    -126,    70,    71,   107,   185,   184,   367,   197,     2,   134,
     186,   188,   320,    40,    41,    42,    43,   187,   107,   330,
     199,   189,   200,     3,   203,   323,     4,     5,     6,     7,
     201,   206,   334,   339,   149,   210,    56,    57,   328,    58,
     162,   163,   218,     8,   213,   219,   220,   224,   222,    59,
       4,   227,   221,   231,   250,    60,   223,   225,   256,    61,
      62,   262,   263,    63,   357,   264,   265,   273,    64,   253,
     318,   362,    66,   280,   284,   288,   165,   166,   289,    56,
      57,    67,   105,   168,   319,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,    69,   181,    70,    71,
      56,    57,   292,   228,    56,    57,   294,   105,   298,   147,
     148,   297,   105,    65,   300,    66,   301,   302,   309,   229,
     308,   312,   217,   254,   216,   322,   324,    68,   338,   321,
     352,   354,   341,   349,    65,   356,    66,   365,    65,    69,
      66,    70,    71,    65,   361,    66,   162,   163,    68,    34,
     355,   359,    68,   317,   364,   350,    14,    68,   282,   340,
      69,   286,    70,    71,    69,   152,    70,    71,   120,    69,
       0,    70,    71,     0,   162,   163,   164,     0,     0,     0,
       0,     0,   165,   166,     0,     0,     0,   167,     0,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,     0,   181,   164,   162,   163,     0,     0,     0,
     165,   166,     0,     0,     0,   211,     0,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   181,     0,   162,   163,   164,   212,     0,     0,     0,
       0,   165,   166,     0,     0,     0,     0,     0,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,     0,   181,   164,   162,   163,     0,     0,     0,   165,
     166,     0,     0,   233,     0,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,     0,
     181,     0,   162,   163,   164,     0,     0,     0,     0,     0,
     165,   166,     0,     0,     0,   290,     0,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
       0,   181,   164,   162,   163,     0,     0,     0,   165,   166,
       0,     0,     0,   307,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   181,
       0,   162,   163,   164,     0,     0,     0,     0,     0,   165,
     166,     0,     0,     0,   332,     0,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,     0,
     181,   164,   162,   163,     0,     0,     0,   165,   166,     0,
       0,     0,   348,     0,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,     0,   181,     0,
     162,   163,   164,     0,     0,     0,     0,     0,   165,   166,
       0,     0,     0,   363,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,     0,   181,
     164,   162,   163,     0,     0,     0,   165,   166,     0,     0,
       0,   368,     0,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,     0,   181,     0,   162,
     163,   164,     0,     0,     0,     0,     0,   165,   166,     0,
       0,     0,     0,     0,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,     0,   181,   164,
    -182,  -182,     0,     0,     0,   165,   166,     0,     0,     0,
       0,     0,   168,   169,   170,   171,   172,   173,   174,   175,
    -183,   177,   178,   179,   180,     0,   181,     0,   162,   163,
       0,     0,     0,     0,     0,     0,  -182,  -182,     0,     0,
    -182,     0,     0,  -182,     0,  -182,  -182,  -182,  -182,  -182,
    -182,  -182,  -182,  -182,  -182,  -182,     0,  -182,   164,  -127,
    -127,     0,     0,     0,   165,   166,     0,     0,     0,     0,
       0,   168,   169,   170,   171,   172,   173,   174,   175,     0,
     177,   178,   179,   180,     0,   181,   -95,   -95,     0,     0,
       0,     0,     0,     0,     0,  -127,  -127,     0,  -127,     0,
       0,     0,  -127,     0,  -127,  -127,  -127,  -127,  -127,  -127,
    -127,  -127,  -127,  -127,  -127,     0,  -127,     0,     0,     0,
       0,     0,   -95,   -95,     0,     0,     0,     0,     0,   -95,
       0,   -95,   -95,   -95,   -95,   -95,   -95,   -95,   -95,   -95,
     -95,   -95,   164,   -95,     0,     0,     0,     0,   165,   166,
       0,     0,     0,     0,     0,   168,   169,   170,   171,   172,
     173,   174,   175,   164,     0,     0,   179,   180,     0,   181,
       0,     0,     0,     0,     0,     0,   168,   169,   170,   171,
     172,   173,   174,   175,     0,     0,     0,     0,     0,     0,
     181
  };

  const short int
   Parser ::yycheck_[] =
  {
      35,    59,    61,    26,    66,    34,    65,    35,   135,   124,
      66,   116,   116,    55,   115,    48,   135,     7,    39,     4,
       5,     4,     5,    39,    38,    49,    61,    62,     4,    64,
      65,     7,     4,    68,    69,     7,    50,    58,     4,     5,
       5,     7,    58,    33,     4,     5,     4,     7,     4,     5,
      85,    17,    18,    82,    41,    40,    38,    23,     7,    46,
       7,    27,    28,     4,     5,    31,     7,    52,    50,    52,
      36,   186,    38,   135,    40,    22,     7,    53,    38,   135,
      40,    66,    67,    49,    40,   118,    52,    52,    48,   124,
     113,   114,    52,    38,    52,    50,    52,    38,    64,    40,
      66,    67,    41,    58,    64,    50,    66,    67,     7,     7,
      49,    52,    53,    11,    12,    13,    14,    46,    16,   154,
      38,    50,    40,    64,    49,    66,    67,   162,   163,   164,
     165,   166,   161,    58,    48,   170,   171,    49,    50,   174,
      38,   176,   177,   178,   179,   180,   181,   182,   253,   253,
      41,   186,    48,   188,   269,     4,     5,   284,    49,   201,
     261,   280,   224,   225,   220,   221,    41,    38,     7,    41,
     205,   206,    48,   288,    49,    46,    47,    49,   206,    50,
     209,    29,    30,     4,     5,     7,     7,    39,   303,     4,
       5,     7,     7,    39,    46,    38,    17,    18,   233,     7,
      46,    39,    23,    48,    39,    42,    27,    28,    46,    39,
      31,    46,    33,    38,    58,    36,    46,    38,     7,    40,
      39,     7,   284,    38,   280,    40,     7,    46,    49,    33,
      55,    56,    57,    58,   269,   270,   341,   341,    47,   274,
      65,    39,    47,    64,   302,    66,    67,    47,    46,    64,
      47,    66,    67,   288,    33,    58,   361,   361,     0,   318,
      38,    46,   291,    11,    12,    13,    14,    39,   303,   304,
      53,    48,    39,    15,     7,   298,    18,    19,    20,    21,
      46,    38,   311,   318,   319,    32,     4,     5,   346,     7,
       8,     9,    41,    35,    39,    46,    49,    49,    41,    17,
      18,     7,    46,    33,     7,    23,    46,    46,     7,    27,
      28,    49,    43,    31,   349,     4,    37,    24,    36,    60,
      38,   356,    40,    40,    40,    38,    44,    45,     7,     4,
       5,    49,     7,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
       4,     5,    33,     7,     4,     5,    39,     7,    38,     4,
       5,    47,     7,    38,    38,    40,    39,    38,     7,    23,
      39,    33,     5,    48,     4,     7,     7,    52,    24,    49,
       7,     7,    50,    50,    38,    50,    40,    51,    38,    64,
      40,    66,    67,    38,    37,    40,     8,     9,    52,    23,
     346,    26,    52,   289,   358,   333,     1,    52,   220,   320,
      64,   224,    66,    67,    64,    73,    66,    67,    54,    64,
      -1,    66,    67,    -1,     8,     9,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    65,    38,     8,     9,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    -1,     8,     9,    38,    39,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    65,    38,     8,     9,    -1,    -1,    -1,    44,
      45,    -1,    -1,    48,    -1,    -1,    51,    52,    53,    54,
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
      -1,    -1,    -1,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    65,
      38,     8,     9,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    -1,     8,
       9,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    -1,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    65,    38,
       8,     9,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    65,    -1,     8,     9,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      48,    -1,    -1,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    38,     8,
       9,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    61,    62,    63,    -1,    65,     8,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    47,    -1,
      -1,    -1,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    65,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    38,    65,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    38,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      65
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
     121,     7,   121,   121,   126,    40,    52,   144,   145,   146,
     147,   148,   149,   150,   151,   153,   154,     4,     5,   121,
     121,    33,    91,    46,    50,    89,   104,     4,     7,    53,
     121,   139,     8,     9,    38,    44,    45,    49,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    65,   125,    47,    58,    33,    38,    39,    46,    48,
     137,   137,     4,     7,   138,   139,   142,   155,   133,    53,
      39,    46,   131,     7,    49,    50,    38,    96,   107,   108,
      32,    49,    39,    39,   146,   150,     4,     5,    41,    46,
      49,    46,    41,    46,    49,    46,    77,     7,     7,    23,
     121,    33,   105,    48,    89,   121,   121,   130,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
       7,    81,   131,    60,    48,   121,     7,   158,    49,    39,
      41,    46,    49,    43,     4,    37,    86,    85,    39,    38,
      50,   121,   130,    24,    29,    30,   110,    89,    41,    41,
      40,    52,   147,   154,    40,    52,   151,   153,    38,     7,
      49,   121,    33,   117,    39,    39,   142,    47,    38,   139,
      38,    39,    38,    38,    50,   131,   121,    49,    39,     7,
     121,   111,    33,   109,   146,   150,   131,    96,    38,    52,
      89,    49,     7,   137,     7,   135,    87,    88,    93,   131,
     121,    39,    49,   113,    89,    41,    41,    39,    24,   121,
     117,    50,    39,    39,    46,    39,    46,    39,    49,    50,
     107,   112,     7,   142,     7,    88,    50,   121,   114,    26,
      98,    37,   121,    49,   110,    51,    99,   142,    49
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
     126,   126,   126,   127,   128,   128,   128,   129,   129,   130,
     130,   131,   131,   131,   131,   132,   132,   132,   132,   133,
     134,   134,   135,   135,   136,   137,   137,   137,   137,   138,
     138,   139,   139,   139,   140,   140,   140,   141,   142,   142,
     142,   143,   143,   144,   144,   145,   145,   146,   146,   147,
     147,   148,   148,   149,   149,   150,   150,   151,   151,   152,
     152,   152,   153,   153,   154,   154,   155,   155,   156,   156,
     157,   157,   158
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
       2,     3,     1,     3,     1,     1,     1,     4,     4,     0,
       1,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       4,     7,     1,     3,     5,     1,     4,     7,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     3,     3,     1,     1,     3,     5,     1,     3,     1,
       3,     1,     1,     3,     5,     1,     3,     1,     3,     1,
       1,     1,     1,     2,     1,     2,     1,     1,     8,     6,
       1,     1,     7
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
     394,   394,   394,   402,   433,   436,   442,   447,   455,   462,
     469,   472,   478,   483,   491,   497,   500,   507,   508,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     526,   536,   558,   567,   695,   736,   787,   867,   870,   877,
     881,   888,   891,   897,   902,   922,   944,   966,   988,   997,
    1004,  1009,  1015,  1033,  1033,  1033,  1039,  1042,  1042,  1042,
    1045,  1045,  1045,  1062,  1070,  1080,  1091,  1096,  1124,  1127,
    1132,  1140,  1141,  1142,  1143,  1144,  1145,  1146,  1152,  1172,
    1181,  1189,  1208,  1278,  1298,  1326,  1331,  1340,  1341,  1342,
    1343,  1349,  1353,  1359,  1365,  1371,  1377,  1383,  1389,  1395,
    1401,  1406,  1412,  1419,  1453,  1454,  1455,  1462,  1547,  1568,
    1571,  1580,  1586,  1590,  1594,  1604,  1605,  1606,  1607,  1611,
    1623,  1627,  1637,  1646,  1658,  1670,  1674,  1677,  1719,  1729,
    1734,  1742,  1745,  1759,  1765,  1768,  1771,  1821,  1825,  1826,
    1830,  1834,  1841,  1852,  1859,  1863,  1867,  1881,  1885,  1900,
    1904,  1911,  1918,  1922,  1926,  1940,  1944,  1959,  1963,  1970,
    1973,  1976,  1982,  1985,  1991,  1994,  2000,  2003,  2010,  2029,
    2053,  2054,  2062
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



