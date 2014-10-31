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

#line 37 "parser.cpp" // lalr1.cc:399

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "parser.h"

// User implementation prologue.

#line 51 "parser.cpp" // lalr1.cc:407
// Unqualified %code blocks.
#line 30 "parser.ypp" // lalr1.cc:408

  #include <stdlib.h>
  #include <cassert>
  #include <iostream>
  #include <map>
  #include <set>
  #include <algorithm>

  #include "program_context.h"
  #include "scanner.h"
  #include "ir_mutator.h"

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

  void transposeVector(Expr vec) {
    assert(vec.type().isTensor());
    const TensorType *ttype = vec.type().toTensor();
    assert(ttype->order() == 1);

    Type transposedVector = TensorType::make(ttype->componentType,
                                             ttype->dimensions,
                                             !ttype->isColumnVector);

    const_cast<ExprNodeBase*>(vec.expr())->type = transposedVector;
  }

  bool compare(const Type &l, const Type &r, ProgramContext *ctx) {
    if (l.kind() != r.kind()) {
      return false;
    }

    if (l.isTensor()) {
      if (l.toTensor()->isColumnVector != r.toTensor()->isColumnVector) {
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
      assert(lt.isTensor() && rt.isTensor());     \
      const TensorType *ltt = lt.toTensor();      \
      const TensorType *rtt = rt.toTensor();      \
      if (ltt->order() > 0 && rtt->order() > 0) { \
        CHECK_TYPE_EQUALITY(lt, rt, loc);         \
      }                                           \
    }                                             \
    while (0)

#line 180 "parser.cpp" // lalr1.cc:408


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

#line 5 "parser.ypp" // lalr1.cc:474
namespace  simit { namespace internal  {
#line 266 "parser.cpp" // lalr1.cc:474

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
            case 4: // "int literal"

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 487 "parser.cpp" // lalr1.cc:602
        break;

      case 5: // "float literal"

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 494 "parser.cpp" // lalr1.cc:602
        break;

      case 6: // "string literal"

#line 187 "parser.ypp" // lalr1.cc:602
        { free((void*)((yysym.value.string))); }
#line 501 "parser.cpp" // lalr1.cc:602
        break;

      case 7: // "identifier"

#line 187 "parser.ypp" // lalr1.cc:602
        { free((void*)((yysym.value.string))); }
#line 508 "parser.cpp" // lalr1.cc:602
        break;

      case 61: // extern

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.var); }
#line 515 "parser.cpp" // lalr1.cc:602
        break;

      case 62: // element_type_decl

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 522 "parser.cpp" // lalr1.cc:602
        break;

      case 63: // field_decl_list

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.fields); }
#line 529 "parser.cpp" // lalr1.cc:602
        break;

      case 64: // field_decl

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.field); }
#line 536 "parser.cpp" // lalr1.cc:602
        break;

      case 65: // procedure

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.function); }
#line 543 "parser.cpp" // lalr1.cc:602
        break;

      case 68: // procedure_header

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.function); }
#line 550 "parser.cpp" // lalr1.cc:602
        break;

      case 69: // function

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.function); }
#line 557 "parser.cpp" // lalr1.cc:602
        break;

      case 72: // function_header

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.function); }
#line 564 "parser.cpp" // lalr1.cc:602
        break;

      case 73: // arguments

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.vars); }
#line 571 "parser.cpp" // lalr1.cc:602
        break;

      case 74: // argument_list

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.vars); }
#line 578 "parser.cpp" // lalr1.cc:602
        break;

      case 75: // argument_decl

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.var); }
#line 585 "parser.cpp" // lalr1.cc:602
        break;

      case 76: // results

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.vars); }
#line 592 "parser.cpp" // lalr1.cc:602
        break;

      case 77: // result_list

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.vars); }
#line 599 "parser.cpp" // lalr1.cc:602
        break;

      case 78: // result_decl

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.var); }
#line 606 "parser.cpp" // lalr1.cc:602
        break;

      case 79: // stmt_block

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.stmt); }
#line 613 "parser.cpp" // lalr1.cc:602
        break;

      case 84: // idents

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.strings); }
#line 620 "parser.cpp" // lalr1.cc:602
        break;

      case 85: // with

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 627 "parser.cpp" // lalr1.cc:602
        break;

      case 86: // reduce

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 634 "parser.cpp" // lalr1.cc:602
        break;

      case 87: // reduce_op

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 641 "parser.cpp" // lalr1.cc:602
        break;

      case 95: // expr_stmt

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.stmt); }
#line 648 "parser.cpp" // lalr1.cc:602
        break;

      case 96: // expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 655 "parser.cpp" // lalr1.cc:602
        break;

      case 97: // ident_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 662 "parser.cpp" // lalr1.cc:602
        break;

      case 98: // paren_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 669 "parser.cpp" // lalr1.cc:602
        break;

      case 99: // linear_algebra_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 676 "parser.cpp" // lalr1.cc:602
        break;

      case 100: // elwise_binary_op

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 683 "parser.cpp" // lalr1.cc:602
        break;

      case 101: // boolean_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 690 "parser.cpp" // lalr1.cc:602
        break;

      case 102: // field_read_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 697 "parser.cpp" // lalr1.cc:602
        break;

      case 103: // set_read_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 704 "parser.cpp" // lalr1.cc:602
        break;

      case 104: // call_or_paren_read_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 711 "parser.cpp" // lalr1.cc:602
        break;

      case 105: // call_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 718 "parser.cpp" // lalr1.cc:602
        break;

      case 106: // expr_list_or_empty

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.exprs); }
#line 725 "parser.cpp" // lalr1.cc:602
        break;

      case 107: // expr_list

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.exprs); }
#line 732 "parser.cpp" // lalr1.cc:602
        break;

      case 108: // type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 739 "parser.cpp" // lalr1.cc:602
        break;

      case 109: // element_type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 746 "parser.cpp" // lalr1.cc:602
        break;

      case 110: // set_type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 753 "parser.cpp" // lalr1.cc:602
        break;

      case 111: // endpoints

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.exprs); }
#line 760 "parser.cpp" // lalr1.cc:602
        break;

      case 112: // tuple_type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 767 "parser.cpp" // lalr1.cc:602
        break;

      case 113: // tensor_type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.type); }
#line 774 "parser.cpp" // lalr1.cc:602
        break;

      case 114: // index_sets

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.indexSets); }
#line 781 "parser.cpp" // lalr1.cc:602
        break;

      case 115: // index_set

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.indexSet); }
#line 788 "parser.cpp" // lalr1.cc:602
        break;

      case 116: // component_type

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.scalarType); }
#line 795 "parser.cpp" // lalr1.cc:602
        break;

      case 117: // literal_expr

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 802 "parser.cpp" // lalr1.cc:602
        break;

      case 118: // tensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 809 "parser.cpp" // lalr1.cc:602
        break;

      case 119: // dense_tensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 816 "parser.cpp" // lalr1.cc:602
        break;

      case 120: // float_dense_tensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorDoubleValues); }
#line 823 "parser.cpp" // lalr1.cc:602
        break;

      case 121: // float_dense_ndtensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorDoubleValues); }
#line 830 "parser.cpp" // lalr1.cc:602
        break;

      case 122: // float_dense_matrix_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorDoubleValues); }
#line 837 "parser.cpp" // lalr1.cc:602
        break;

      case 123: // float_dense_vector_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorDoubleValues); }
#line 844 "parser.cpp" // lalr1.cc:602
        break;

      case 124: // int_dense_tensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorIntValues); }
#line 851 "parser.cpp" // lalr1.cc:602
        break;

      case 125: // int_dense_ndtensor_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorIntValues); }
#line 858 "parser.cpp" // lalr1.cc:602
        break;

      case 126: // int_dense_matrix_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorIntValues); }
#line 865 "parser.cpp" // lalr1.cc:602
        break;

      case 127: // int_dense_vector_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.TensorIntValues); }
#line 872 "parser.cpp" // lalr1.cc:602
        break;

      case 128: // scalar_literal

#line 188 "parser.ypp" // lalr1.cc:602
        { delete (yysym.value.expr); }
#line 879 "parser.cpp" // lalr1.cc:602
        break;

      case 129: // signed_int_literal

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 886 "parser.cpp" // lalr1.cc:602
        break;

      case 130: // signed_float_literal

#line 186 "parser.ypp" // lalr1.cc:602
        {}
#line 893 "parser.cpp" // lalr1.cc:602
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
#line 278 "parser.ypp" // lalr1.cc:847
    {
    Func func = convertAndDelete((yystack_[0].value.function));
    std::string name = func.getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("procedure redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(func);
  }
#line 1142 "parser.cpp" // lalr1.cc:847
    break;

  case 6:
#line 286 "parser.ypp" // lalr1.cc:847
    {
    Func func = convertAndDelete((yystack_[0].value.function));
    std::string name = func.getName();
    if (ctx->containsFunction(name)) {
      REPORT_ERROR("function redefinition (" + name + ")", yystack_[0].location);
    }
    ctx->addFunction(func);
  }
#line 1155 "parser.cpp" // lalr1.cc:847
    break;

  case 10:
#line 301 "parser.ypp" // lalr1.cc:847
    {
    Var externVar = convertAndDelete((yystack_[1].value.var));
    ctx->addExtern(externVar);
    ctx->addSymbol(externVar);
  }
#line 1165 "parser.cpp" // lalr1.cc:847
    break;

  case 11:
#line 311 "parser.ypp" // lalr1.cc:847
    {
    string name = convertAndFree((yystack_[2].value.string));
    unique_ptr<std::map<string,Field>> fields((yystack_[1].value.fields));

    if (ctx->containsElementType(name)) {
      REPORT_ERROR("struct redefinition (" + name + ")", yylhs.location);
    }

    ctx->addElementType(ElementType::make(name, *fields));
  }
#line 1180 "parser.cpp" // lalr1.cc:847
    break;

  case 12:
#line 324 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.fields) = new map<string,Field>;
  }
#line 1188 "parser.cpp" // lalr1.cc:847
    break;

  case 13:
#line 327 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.fields) = (yystack_[1].value.fields);
    (yystack_[0].value.field)->second.location = (yylhs.value.fields)->size();
    (yylhs.value.fields)->insert(*(yystack_[0].value.field));
    delete (yystack_[0].value.field);
  }
#line 1199 "parser.cpp" // lalr1.cc:847
    break;

  case 14:
#line 336 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[3].value.string));
    auto tensorType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.field) = new pair<string,Field>(name, tensorType);
  }
#line 1209 "parser.cpp" // lalr1.cc:847
    break;

  case 15:
#line 346 "parser.ypp" // lalr1.cc:847
    {ctx->scope();}
#line 1215 "parser.cpp" // lalr1.cc:847
    break;

  case 16:
#line 346 "parser.ypp" // lalr1.cc:847
    {ctx->unscope();}
#line 1221 "parser.cpp" // lalr1.cc:847
    break;

  case 17:
#line 346 "parser.ypp" // lalr1.cc:847
    {
    Func func = convertAndDelete((yystack_[3].value.function));
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = new Func(func.getName(), func.getArguments(), func.getResults(), body);
  }
#line 1231 "parser.cpp" // lalr1.cc:847
    break;

  case 18:
#line 354 "parser.ypp" // lalr1.cc:847
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
#line 1250 "parser.cpp" // lalr1.cc:847
    break;

  case 19:
#line 371 "parser.ypp" // lalr1.cc:847
    {ctx->scope();}
#line 1256 "parser.cpp" // lalr1.cc:847
    break;

  case 20:
#line 371 "parser.ypp" // lalr1.cc:847
    {ctx->unscope();}
#line 1262 "parser.cpp" // lalr1.cc:847
    break;

  case 21:
#line 371 "parser.ypp" // lalr1.cc:847
    {
    Func func = convertAndDelete((yystack_[3].value.function));
    Stmt body = convertAndDelete((yystack_[2].value.stmt));
    (yylhs.value.function) = new Func(func.getName(), func.getArguments(), func.getResults(), body);
  }
#line 1272 "parser.cpp" // lalr1.cc:847
    break;

  case 22:
#line 379 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[4].value.string));
    auto arguments = unique_ptr<vector<Var>>((yystack_[2].value.vars));
    auto results = unique_ptr<vector<Var>>((yystack_[0].value.vars));
    (yylhs.value.function) = new Func(name, *arguments, *results, Stmt());

    std::set<std::string> argNames;
    for (Var &arg : *arguments) {
      ctx->addSymbol(arg.name, arg, Symbol::Read);
      argNames.insert(arg.name);
    }

    for (Var &res : *results) {
      Symbol::Access access = (argNames.find(res.name) != argNames.end())
                              ? Symbol::ReadWrite : Symbol::ReadWrite;
      ctx->addSymbol(res.name, res, access);
    }
  }
#line 1295 "parser.cpp" // lalr1.cc:847
    break;

  case 23:
#line 400 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.vars) = new vector<Var>;
  }
#line 1303 "parser.cpp" // lalr1.cc:847
    break;

  case 24:
#line 403 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.vars) = (yystack_[0].value.vars);
 }
#line 1311 "parser.cpp" // lalr1.cc:847
    break;

  case 25:
#line 409 "parser.ypp" // lalr1.cc:847
    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(argument);
  }
#line 1321 "parser.cpp" // lalr1.cc:847
    break;

  case 26:
#line 414 "parser.ypp" // lalr1.cc:847
    {
    auto argument = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(argument);
  }
#line 1331 "parser.cpp" // lalr1.cc:847
    break;

  case 27:
#line 422 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[2].value.string));

    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }
#line 1342 "parser.cpp" // lalr1.cc:847
    break;

  case 28:
#line 431 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.vars) = new vector<Var>;
  }
#line 1350 "parser.cpp" // lalr1.cc:847
    break;

  case 29:
#line 434 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.vars) = (yystack_[1].value.vars);
  }
#line 1358 "parser.cpp" // lalr1.cc:847
    break;

  case 30:
#line 440 "parser.ypp" // lalr1.cc:847
    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = new vector<Var>;
    (yylhs.value.vars)->push_back(result);
  }
#line 1368 "parser.cpp" // lalr1.cc:847
    break;

  case 31:
#line 445 "parser.ypp" // lalr1.cc:847
    {
    auto result = convertAndDelete((yystack_[0].value.var));
    (yylhs.value.vars) = (yystack_[2].value.vars);
    (yylhs.value.vars)->push_back(result);
  }
#line 1378 "parser.cpp" // lalr1.cc:847
    break;

  case 32:
#line 453 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[2].value.string));
    auto type = convertAndDelete((yystack_[0].value.type));
    (yylhs.value.var) = new Var(name, type);
  }
#line 1388 "parser.cpp" // lalr1.cc:847
    break;

  case 33:
#line 463 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.stmt) = new Stmt(Pass::make());
  }
#line 1396 "parser.cpp" // lalr1.cc:847
    break;

  case 34:
#line 466 "parser.ypp" // lalr1.cc:847
    {
    vector<Stmt> stmts = *ctx->getStatements();
    if (stmts.size() == 0) {(yylhs.value.stmt) = new Stmt(Pass::make()); break;} // TODO: remove
    (yylhs.value.stmt) = new Stmt((stmts.size() == 1) ? stmts[0] : Block::make(stmts));
  }
#line 1406 "parser.cpp" // lalr1.cc:847
    break;

  case 45:
#line 489 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[1].value.expr) == nullptr) { break; } // TODO: Remove check

    auto varNames = unique_ptr<vector<string>>((yystack_[3].value.strings));
    if (varNames->size() > 1) {
      REPORT_ERROR("can only assign to one value in a non-map statement", yystack_[3].location);
    }

    string varName = (*varNames)[0];
    Expr value = convertAndDelete((yystack_[1].value.expr));

    Var var;
    if (ctx->hasSymbol(varName)) {
      Symbol symbol = ctx->getSymbol(varName);

      if (!symbol.isWritable()) {
        REPORT_ERROR(varName + " is not writable", yystack_[3].location);
      }

      var = symbol.getVar();
    }
    else {
      var = Var(varName, value.type());
      ctx->addSymbol(varName, var, Symbol::ReadWrite);
    }

    ctx->addStatement(AssignStmt::make(var, value));
  }
#line 1439 "parser.cpp" // lalr1.cc:847
    break;

  case 46:
#line 520 "parser.ypp" // lalr1.cc:847
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
        var = Var(varName, results[i].type);
        ctx->addSymbol(varName, var, Symbol::ReadWrite);
      }
      vars.push_back(var);
    }

    ctx->addStatement(Map::make(vars, func, targets, neighbor, reduction));
  }
#line 1493 "parser.cpp" // lalr1.cc:847
    break;

  case 47:
#line 572 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.strings) = new vector<string>;
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }
#line 1502 "parser.cpp" // lalr1.cc:847
    break;

  case 48:
#line 576 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.strings) = (yystack_[2].value.strings);
    (yylhs.value.strings)->push_back(convertAndFree((yystack_[0].value.string)));
  }
#line 1511 "parser.cpp" // lalr1.cc:847
    break;

  case 49:
#line 583 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.expr) = new Expr();
  }
#line 1519 "parser.cpp" // lalr1.cc:847
    break;

  case 50:
#line 586 "parser.ypp" // lalr1.cc:847
    {
    std::string neighborsName = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(neighborsName)) {
      REPORT_ERROR("undefined set '" + neighborsName + "'", yystack_[0].location);
    }
    Expr neighbors = ctx->getSymbol(neighborsName).getExpr();

    (yylhs.value.expr) = new Expr(neighbors);
  }
#line 1534 "parser.cpp" // lalr1.cc:847
    break;

  case 51:
#line 599 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.reductionop) =  ReductionOperator::Undefined;
  }
#line 1542 "parser.cpp" // lalr1.cc:847
    break;

  case 52:
#line 602 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.reductionop) =  (yystack_[0].value.reductionop);
  }
#line 1550 "parser.cpp" // lalr1.cc:847
    break;

  case 53:
#line 608 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.reductionop) = ReductionOperator::Sum;
  }
#line 1558 "parser.cpp" // lalr1.cc:847
    break;

  case 54:
#line 613 "parser.ypp" // lalr1.cc:847
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
#line 1581 "parser.cpp" // lalr1.cc:847
    break;

  case 55:
#line 633 "parser.ypp" // lalr1.cc:847
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
#line 1606 "parser.cpp" // lalr1.cc:847
    break;

  case 56:
#line 653 "parser.ypp" // lalr1.cc:847
    {
    // TODO
  }
#line 1614 "parser.cpp" // lalr1.cc:847
    break;

  case 57:
#line 659 "parser.ypp" // lalr1.cc:847
    {
    delete (yystack_[3].value.expr);
    delete (yystack_[2].value.stmt);
  }
#line 1623 "parser.cpp" // lalr1.cc:847
    break;

  case 59:
#line 666 "parser.ypp" // lalr1.cc:847
    {
    delete (yystack_[0].value.stmt);
  }
#line 1631 "parser.cpp" // lalr1.cc:847
    break;

  case 61:
#line 672 "parser.ypp" // lalr1.cc:847
    {
    delete (yystack_[1].value.expr);
    delete (yystack_[0].value.stmt);
  }
#line 1640 "parser.cpp" // lalr1.cc:847
    break;

  case 63:
#line 681 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[5].value.string));
    auto type = convertAndDelete((yystack_[3].value.type));
    const TensorType *tensorType = type.toTensor();

    Expr literalExpr = convertAndDelete((yystack_[1].value.expr));

    assert(literalExpr.type().isTensor() &&
           "Only tensor literals are currently supported");
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
#line 1670 "parser.cpp" // lalr1.cc:847
    break;

  case 64:
#line 709 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.stmt) = NULL;
  }
#line 1678 "parser.cpp" // lalr1.cc:847
    break;

  case 65:
#line 712 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.stmt) = NULL;
  }
#line 1686 "parser.cpp" // lalr1.cc:847
    break;

  case 73:
#line 732 "parser.ypp" // lalr1.cc:847
    {
    string ident = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(ident)) { (yylhs.value.expr)=NULL; break; } // TODO: Remove check

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
#line 1708 "parser.cpp" // lalr1.cc:847
    break;

  case 74:
#line 754 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = (yystack_[1].value.expr);
  }
#line 1717 "parser.cpp" // lalr1.cc:847
    break;

  case 75:
#line 763 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr expr = convertAndDelete((yystack_[0].value.expr));
    CHECK_IS_TENSOR(expr, yystack_[0].location);

    IRBuilder::UnaryOperator op = IRBuilder::UnaryOperator::Neg;
    (yylhs.value.expr) = new Expr(ctx->getBuilder()->unaryElwiseExpr(op, expr));
  }
#line 1731 "parser.cpp" // lalr1.cc:847
    break;

  case 76:
#line 772 "parser.ypp" // lalr1.cc:847
    {  // + - .* ./
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    BINARY_ELWISE_TYPE_CHECK(l.type(), r.type(), yystack_[1].location);
    (yylhs.value.expr) = new Expr(ctx->getBuilder()->binaryElwiseExpr(l, (yystack_[1].value.binop), r));
  }
#line 1748 "parser.cpp" // lalr1.cc:847
    break;

  case 77:
#line 784 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    IRBuilder *builder = ctx->getBuilder();

    Expr l = convertAndDelete((yystack_[2].value.expr));
    Expr r = convertAndDelete((yystack_[0].value.expr));

    CHECK_IS_TENSOR(l, yystack_[2].location);
    CHECK_IS_TENSOR(r, yystack_[0].location);

    const TensorType *ltype = l.type().toTensor();
    const TensorType *rtype = r.type().toTensor();

    // Scale
    if (ltype->order()==0 || rtype->order()==0) {
      IRBuilder::BinaryOperator mulOp = IRBuilder::BinaryOperator::Mul;
      (yylhs.value.expr) = new Expr(builder->binaryElwiseExpr(l, mulOp, r));
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
      if (ltype->dimensions[1] != rtype->dimensions[0]){
        REPORT_TYPE_MISSMATCH(l.type(), r.type(), yystack_[1].location);
      }
      (yylhs.value.expr) = new Expr(builder->gemv(l, r));
    }
    // Vector-Matrix
    else if (ltype->order() == 1 && rtype->order() == 2) {
      if (ltype->dimensions[0] != rtype->dimensions[0]){
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
#line 1820 "parser.cpp" // lalr1.cc:847
    break;

  case 78:
#line 851 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1831 "parser.cpp" // lalr1.cc:847
    break;

  case 79:
#line 857 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[1].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    IRBuilder *builder = ctx->getBuilder();
    IRBuilder::UnaryOperator noneOp = IRBuilder::UnaryOperator::None;

    Expr expr = convertAndDelete((yystack_[1].value.expr));

    CHECK_IS_TENSOR(expr, yystack_[1].location);

    const TensorType *type = expr.type().toTensor();
    switch (type->order()) {
      case 0:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(noneOp, expr));
        break;
      case 1:
        // OPT: This might lead to redundant code to be removed in later pass
        (yylhs.value.expr) = new Expr(builder->unaryElwiseExpr(noneOp, expr));
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
#line 1866 "parser.cpp" // lalr1.cc:847
    break;

  case 80:
#line 887 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1876 "parser.cpp" // lalr1.cc:847
    break;

  case 81:
#line 892 "parser.ypp" // lalr1.cc:847
    {  // Solve
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1887 "parser.cpp" // lalr1.cc:847
    break;

  case 82:
#line 901 "parser.ypp" // lalr1.cc:847
    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Add; }
#line 1893 "parser.cpp" // lalr1.cc:847
    break;

  case 83:
#line 902 "parser.ypp" // lalr1.cc:847
    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Sub; }
#line 1899 "parser.cpp" // lalr1.cc:847
    break;

  case 84:
#line 903 "parser.ypp" // lalr1.cc:847
    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Mul; }
#line 1905 "parser.cpp" // lalr1.cc:847
    break;

  case 85:
#line 904 "parser.ypp" // lalr1.cc:847
    { (yylhs.value.binop) = IRBuilder::BinaryOperator::Div; }
#line 1911 "parser.cpp" // lalr1.cc:847
    break;

  case 86:
#line 910 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1922 "parser.cpp" // lalr1.cc:847
    break;

  case 87:
#line 916 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1933 "parser.cpp" // lalr1.cc:847
    break;

  case 88:
#line 922 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1944 "parser.cpp" // lalr1.cc:847
    break;

  case 89:
#line 928 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1955 "parser.cpp" // lalr1.cc:847
    break;

  case 90:
#line 934 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1966 "parser.cpp" // lalr1.cc:847
    break;

  case 91:
#line 940 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    (yylhs.value.expr) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 1977 "parser.cpp" // lalr1.cc:847
    break;

  case 92:
#line 951 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.string) == NULL) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check
    if (!(*(yystack_[2].value.expr)).type().defined()) { (yylhs.value.expr) = NULL; break; } // TODO: Remove check

    Expr elemOrSet = convertAndDelete((yystack_[2].value.expr));
    std::string fieldName = convertAndFree((yystack_[0].value.string));

    Type type = elemOrSet.type();
    if (!(type.isElement() || type.isSet())) {
      std::stringstream errorStr;
      errorStr << "only elements and sets have fields";
      REPORT_ERROR(errorStr.str(), yystack_[2].location);
    }

    (yylhs.value.expr) = new Expr(FieldRead::make(elemOrSet, fieldName));
  }
#line 1998 "parser.cpp" // lalr1.cc:847
    break;

  case 96:
#line 978 "parser.ypp" // lalr1.cc:847
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
#line 2038 "parser.cpp" // lalr1.cc:847
    break;

  case 97:
#line 1013 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.expr) = NULL;
  }
#line 2046 "parser.cpp" // lalr1.cc:847
    break;

  case 98:
#line 1019 "parser.ypp" // lalr1.cc:847
    {
    std::string funcName = convertAndFree((yystack_[3].value.string));
    auto actuals = unique_ptr<vector<Expr>>((yystack_[1].value.exprs));

    if (!ctx->containsFunction(funcName)) {(yylhs.value.expr) = NULL; break;} // TODO: Remove check

    if (!ctx->containsFunction(funcName)) {
      REPORT_ERROR("undefined function '" + funcName + "'", yystack_[3].location);
    }
    Func func = ctx->getFunction(funcName);

    (yylhs.value.expr) = new Expr(Call::make(func, *actuals));
  }
#line 2064 "parser.cpp" // lalr1.cc:847
    break;

  case 99:
#line 1035 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = new vector<Expr>();
  }
#line 2072 "parser.cpp" // lalr1.cc:847
    break;

  case 100:
#line 1038 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = (yystack_[0].value.exprs);
  }
#line 2080 "parser.cpp" // lalr1.cc:847
    break;

  case 101:
#line 1044 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = new std::vector<Expr>();
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }
#line 2091 "parser.cpp" // lalr1.cc:847
    break;

  case 102:
#line 1050 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    if ((yystack_[0].value.expr) == NULL) break;  // TODO: Remove check
    (yylhs.value.exprs)->push_back(*(yystack_[0].value.expr));
    delete (yystack_[0].value.expr);
  }
#line 2102 "parser.cpp" // lalr1.cc:847
    break;

  case 107:
#line 1068 "parser.ypp" // lalr1.cc:847
    {
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->containsElementType(name)) {
      REPORT_ERROR("undefined element type '" + name + "'" , yystack_[0].location);
    }

    (yylhs.value.type) = new Type(ctx->getElementType(name));
  }
#line 2116 "parser.cpp" // lalr1.cc:847
    break;

  case 108:
#line 1080 "parser.ypp" // lalr1.cc:847
    {
    auto elementType = convertAndDelete((yystack_[1].value.type));
    (yylhs.value.type) = new Type(SetType::make(elementType, {}));
  }
#line 2125 "parser.cpp" // lalr1.cc:847
    break;

  case 109:
#line 1084 "parser.ypp" // lalr1.cc:847
    {
    auto elementType = convertAndDelete((yystack_[4].value.type));
    auto endpoints = convertAndDelete((yystack_[1].value.exprs));

    // TODO: Add endpoint information to set type
    (yylhs.value.type) = new Type(SetType::make(elementType, endpoints));
  }
#line 2137 "parser.cpp" // lalr1.cc:847
    break;

  case 110:
#line 1094 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = new vector<Expr>;
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[0].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }
#line 2151 "parser.cpp" // lalr1.cc:847
    break;

  case 111:
#line 1103 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.exprs) = (yystack_[2].value.exprs);
    std::string name = convertAndFree((yystack_[0].value.string));

    if (!ctx->hasSymbol(name)) {
      REPORT_ERROR("undefined set type '" + name + "'" , yystack_[2].location);
    }
    (yylhs.value.exprs)->push_back(ctx->getSymbol(name).getExpr());
  }
#line 2165 "parser.cpp" // lalr1.cc:847
    break;

  case 112:
#line 1115 "parser.ypp" // lalr1.cc:847
    {
    auto elementType = convertAndDelete((yystack_[3].value.type));

    if ((yystack_[1].value.num)<1) {
      REPORT_ERROR("Must be 1 or greater", yystack_[2].location);
    }

    (yylhs.value.type) = new Type(TupleType::make(elementType, (yystack_[1].value.num)));
  }
#line 2179 "parser.cpp" // lalr1.cc:847
    break;

  case 113:
#line 1127 "parser.ypp" // lalr1.cc:847
    {
    auto componentType = convertAndDelete((yystack_[0].value.scalarType));
    (yylhs.value.type) = new Type(TensorType::make(componentType));
  }
#line 2188 "parser.cpp" // lalr1.cc:847
    break;

  case 114:
#line 1131 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.type) = (yystack_[1].value.type);
  }
#line 2196 "parser.cpp" // lalr1.cc:847
    break;

  case 115:
#line 1134 "parser.ypp" // lalr1.cc:847
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
      if (blockType->order() != outerDimensions->size()) {
        REPORT_ERROR("Blocktype order (" + to_string(blockType->order()) +
                     ") differ from number of dimensions", yystack_[4].location);
      }

      assert(blockDimensions.size() == outerDimensions->size());
      for (size_t i=0; i < outerDimensions->size(); ++i) {
        vector<IndexSet> dimension;
        dimension.push_back((*outerDimensions)[i]);
        dimension.insert(dimension.begin(),
                         blockDimensions[i].getIndexSets().begin(),
                         blockDimensions[i].getIndexSets().end());

        dimensions.push_back(IndexDomain(dimension));
      }
    }

    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions));
  }
#line 2243 "parser.cpp" // lalr1.cc:847
    break;

  case 116:
#line 1176 "parser.ypp" // lalr1.cc:847
    {
    auto type = convertAndDelete((yystack_[1].value.type));
    const TensorType *tensorType = type.toTensor();
    auto dimensions = tensorType->dimensions;
    auto componentType = tensorType->componentType;
    (yylhs.value.type) = new Type(TensorType::make(componentType, dimensions, true));
  }
#line 2255 "parser.cpp" // lalr1.cc:847
    break;

  case 117:
#line 1186 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.indexSets) = new std::vector<IndexSet>();
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }
#line 2265 "parser.cpp" // lalr1.cc:847
    break;

  case 118:
#line 1191 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.indexSets) = (yystack_[2].value.indexSets);
    (yylhs.value.indexSets)->push_back(*(yystack_[0].value.indexSet));
    delete (yystack_[0].value.indexSet);
  }
#line 2275 "parser.cpp" // lalr1.cc:847
    break;

  case 119:
#line 1199 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.indexSet) = new IndexSet((yystack_[0].value.num));
  }
#line 2283 "parser.cpp" // lalr1.cc:847
    break;

  case 120:
#line 1202 "parser.ypp" // lalr1.cc:847
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
#line 2302 "parser.cpp" // lalr1.cc:847
    break;

  case 121:
#line 1216 "parser.ypp" // lalr1.cc:847
    {
    if ((yystack_[2].value.expr) == NULL || (yystack_[0].value.expr) == NULL) { (yylhs.value.indexSet) = NULL; break; } // TODO: Remove check
    (yylhs.value.indexSet) = NULL;
    delete (yystack_[2].value.expr);
    delete (yystack_[0].value.expr);
  }
#line 2313 "parser.cpp" // lalr1.cc:847
    break;

  case 122:
#line 1222 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.indexSet) = new IndexSet();
  }
#line 2321 "parser.cpp" // lalr1.cc:847
    break;

  case 123:
#line 1228 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Int);
  }
#line 2329 "parser.cpp" // lalr1.cc:847
    break;

  case 124:
#line 1231 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.scalarType) = new ScalarType(ScalarType::Float);
  }
#line 2337 "parser.cpp" // lalr1.cc:847
    break;

  case 127:
#line 1286 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.expr) = (yystack_[1].value.expr);
    transposeVector(*(yylhs.value.expr));
  }
#line 2346 "parser.cpp" // lalr1.cc:847
    break;

  case 129:
#line 1294 "parser.ypp" // lalr1.cc:847
    {
    auto values = unique_ptr<TensorValues<double>>((yystack_[1].value.TensorDoubleValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Float), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }
#line 2358 "parser.cpp" // lalr1.cc:847
    break;

  case 130:
#line 1301 "parser.ypp" // lalr1.cc:847
    {
    auto values = unique_ptr<TensorValues<int>>((yystack_[1].value.TensorIntValues));
    auto idoms = std::vector<IndexDomain>(values->dimSizes.rbegin(),
                                          values->dimSizes.rend());
    Type type = TensorType::make(ScalarType(ScalarType::Int), idoms);
    (yylhs.value.expr) = new Expr(Literal::make(type, values->values.data()));
  }
#line 2370 "parser.cpp" // lalr1.cc:847
    break;

  case 131:
#line 1312 "parser.ypp" // lalr1.cc:847
    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorDoubleValues)->dimSizes[(yystack_[0].value.TensorDoubleValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorDoubleValues)->dimSizes.pop_back();
    }
  }
#line 2382 "parser.cpp" // lalr1.cc:847
    break;

  case 133:
#line 1323 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorDoubleValues) = (yystack_[1].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }
#line 2391 "parser.cpp" // lalr1.cc:847
    break;

  case 134:
#line 1327 "parser.ypp" // lalr1.cc:847
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
#line 2407 "parser.cpp" // lalr1.cc:847
    break;

  case 135:
#line 1341 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorDoubleValues) = (yystack_[0].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addDimension();
  }
#line 2416 "parser.cpp" // lalr1.cc:847
    break;

  case 136:
#line 1345 "parser.ypp" // lalr1.cc:847
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
#line 2433 "parser.cpp" // lalr1.cc:847
    break;

  case 137:
#line 1360 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorDoubleValues) = new TensorValues<double>();
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }
#line 2442 "parser.cpp" // lalr1.cc:847
    break;

  case 138:
#line 1364 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorDoubleValues) = (yystack_[2].value.TensorDoubleValues);
    (yylhs.value.TensorDoubleValues)->addValue((yystack_[0].value.fnum));
  }
#line 2451 "parser.cpp" // lalr1.cc:847
    break;

  case 139:
#line 1371 "parser.ypp" // lalr1.cc:847
    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yystack_[0].value.TensorIntValues)->dimSizes[(yystack_[0].value.TensorIntValues)->dimSizes.size()-1] == 1) {
      (yystack_[0].value.TensorIntValues)->dimSizes.pop_back();
    }
  }
#line 2463 "parser.cpp" // lalr1.cc:847
    break;

  case 141:
#line 1382 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorIntValues) = (yystack_[1].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }
#line 2472 "parser.cpp" // lalr1.cc:847
    break;

  case 142:
#line 1386 "parser.ypp" // lalr1.cc:847
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
#line 2488 "parser.cpp" // lalr1.cc:847
    break;

  case 143:
#line 1400 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorIntValues) = (yystack_[0].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addDimension();
  }
#line 2497 "parser.cpp" // lalr1.cc:847
    break;

  case 144:
#line 1404 "parser.ypp" // lalr1.cc:847
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
#line 2514 "parser.cpp" // lalr1.cc:847
    break;

  case 145:
#line 1419 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorIntValues) = new TensorValues<int>();
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }
#line 2523 "parser.cpp" // lalr1.cc:847
    break;

  case 146:
#line 1423 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.TensorIntValues) = (yystack_[2].value.TensorIntValues);
    (yylhs.value.TensorIntValues)->addValue((yystack_[0].value.num));
  }
#line 2532 "parser.cpp" // lalr1.cc:847
    break;

  case 147:
#line 1430 "parser.ypp" // lalr1.cc:847
    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Int));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.num)));
  }
#line 2541 "parser.cpp" // lalr1.cc:847
    break;

  case 148:
#line 1434 "parser.ypp" // lalr1.cc:847
    {
    auto scalarTensorType = TensorType::make(ScalarType(ScalarType::Float));
    (yylhs.value.expr) = new Expr(Literal::make(scalarTensorType, &(yystack_[0].value.fnum)));
  }
#line 2550 "parser.cpp" // lalr1.cc:847
    break;

  case 149:
#line 1441 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.num) = (yystack_[0].value.num);
  }
#line 2558 "parser.cpp" // lalr1.cc:847
    break;

  case 150:
#line 1444 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.num) = -(yystack_[0].value.num);
  }
#line 2566 "parser.cpp" // lalr1.cc:847
    break;

  case 151:
#line 1450 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.fnum) = (yystack_[0].value.fnum);
  }
#line 2574 "parser.cpp" // lalr1.cc:847
    break;

  case 152:
#line 1453 "parser.ypp" // lalr1.cc:847
    {
    (yylhs.value.fnum) = -(yystack_[0].value.fnum);
  }
#line 2582 "parser.cpp" // lalr1.cc:847
    break;

  case 153:
#line 1460 "parser.ypp" // lalr1.cc:847
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
    ctx->addTest(new Test(name, literalArgs, expecteds));
  }
#line 2605 "parser.cpp" // lalr1.cc:847
    break;


#line 2609 "parser.cpp" // lalr1.cc:847
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


  const short int  Parser ::yypact_ninf_ = -180;

  const signed char  Parser ::yytable_ninf_ = -121;

  const short int
   Parser ::yypact_[] =
  {
    -180,    13,  -180,  -180,  -180,   209,    12,    37,    47,    63,
      48,    84,    63,    29,  -180,   117,  -180,  -180,  -180,  -180,
      78,  -180,    98,  -180,  -180,  -180,   165,  -180,  -180,  -180,
    -180,  -180,  -180,   284,    75,    95,  -180,  -180,    93,   101,
    -180,  -180,  -180,  -180,   108,  -180,  -180,  -180,  -180,    63,
     132,  -180,   134,   141,   123,   167,   174,   170,   147,   172,
     264,    11,    27,   176,   178,   191,   207,   217,   216,   214,
     218,  -180,  -180,  -180,  -180,   142,   200,   228,   252,   228,
     254,   164,    63,    63,  -180,  -180,  -180,    63,    63,  -180,
    -180,    63,  -180,    63,    63,    63,    63,    63,    63,    63,
     256,  -180,   384,   232,     8,   222,    14,   229,   276,  -180,
      63,   182,   240,   228,  -180,    63,    19,    63,  -180,    69,
      91,  -180,  -180,  -180,   236,    30,    30,  -180,   238,    18,
      18,  -180,   249,   246,   251,  -180,   273,   304,   437,   437,
     142,   142,   142,   404,   424,   424,   437,   437,   384,   255,
      87,  -180,   250,   248,    63,    63,   253,  -180,  -180,  -180,
    -180,    71,    88,  -180,  -180,   257,   275,  -180,  -180,  -180,
    -180,   242,   258,   262,   219,  -180,   124,   241,  -180,   324,
     228,   263,  -180,  -180,    30,   294,   207,  -180,    18,   296,
     218,  -180,  -180,    47,  -180,   282,  -180,  -180,  -180,    63,
     384,   344,   229,   229,    19,   139,  -180,   275,   259,  -180,
      63,   228,    63,   277,   270,   118,   122,  -180,   271,   267,
    -180,  -180,   318,   364,  -180,    -6,    25,   140,  -180,   283,
     291,   340,   174,  -180,   384,  -180,   139,  -180,  -180,   314,
      47,   326,  -180,  -180,  -180,   316,    19,  -180,   332,   334,
    -180,   322,   336,  -180,  -180,   377,   365,   229,  -180,   378,
    -180,  -180,   381,  -180,   359,  -180,    59,  -180,    97,   363,
     115,  -180,  -180,  -180,  -180,  -180,   399,   276,  -180,   381,
    -180,  -180,  -180
  };

  const unsigned char
   Parser ::yydefact_[] =
  {
       2,    15,     1,   149,   151,    73,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     3,     7,     4,     5,
       0,     6,     0,     8,    37,    38,     0,    39,    40,    41,
      42,    43,    44,     0,    66,    67,    69,    70,    71,     0,
      72,    95,    68,   125,   126,   128,   147,   148,     9,    99,
       0,    12,     0,     0,     0,    73,    33,    71,     0,     0,
       0,     0,     0,     0,   132,   131,   135,     0,   140,   139,
     143,   145,   137,   149,   151,    75,     0,    33,     0,    33,
       0,     0,     0,     0,    64,    82,    83,     0,     0,    84,
      85,     0,    79,     0,     0,     0,     0,     0,     0,    99,
       0,   127,   101,     0,     0,     0,     0,     0,     0,    10,
      99,    83,    60,    34,    35,    99,     0,    99,    74,     0,
       0,   150,   152,   129,     0,     0,     0,   130,     0,     0,
       0,    18,     0,     0,     0,    48,     0,     0,    88,    89,
      77,    78,    80,    81,    86,    87,    90,    91,    76,     0,
       0,    92,    96,     0,     0,     0,     0,    11,    13,   123,
     124,     0,     0,   113,   107,     0,     0,    27,   103,   104,
     105,   106,   100,     0,     0,    36,   149,    73,   122,     0,
      33,     0,   133,   141,     0,     0,   136,   138,     0,     0,
     144,   146,    16,    23,    20,     0,    45,    97,    56,     0,
     102,     0,     0,     0,     0,     0,   116,     0,     0,    57,
       0,    33,     0,     0,     0,     0,     0,    17,     0,    24,
      25,    21,     0,     0,    54,     0,     0,     0,   117,     0,
       0,     0,    33,    59,   121,    62,     0,   134,   142,    28,
       0,    49,    55,    14,   114,     0,     0,    63,   108,     0,
      61,     0,     0,    22,    26,     0,    51,     0,   118,     0,
     112,   153,     0,    50,     0,    46,     0,   110,     0,     0,
       0,    30,    53,    52,   115,   109,     0,     0,    29,     0,
     111,    32,    31
  };

  const short int
   Parser ::yypgoto_[] =
  {
    -180,  -180,  -180,  -180,  -180,  -180,  -180,  -180,  -180,  -180,
    -180,  -180,  -180,  -180,  -180,  -180,  -180,  -179,  -180,  -180,
     129,   -70,  -180,    10,  -180,  -180,  -180,  -180,  -180,  -180,
    -180,  -180,  -180,  -180,  -180,  -180,  -180,  -180,    -9,  -180,
    -180,  -180,  -180,  -180,     4,  -180,  -180,  -180,   -19,   -37,
     148,  -114,  -180,  -180,  -180,  -103,  -180,  -175,  -180,  -180,
    -167,  -180,  -180,  -180,   -51,   299,  -180,  -180,   -53,   297,
    -180,   -11,   -12,  -180
  };

  const short int
   Parser ::yydefgoto_[] =
  {
      -1,     1,    16,    17,    18,   106,   158,    19,    20,   217,
      77,    21,    22,   221,    79,   218,   219,    54,   253,   270,
     271,   112,   113,   114,    24,    25,    26,   256,   265,   273,
      27,    28,    29,   173,   174,    30,    31,    32,    33,    34,
      35,    36,    98,    37,    57,    39,    40,    41,   103,   172,
     167,   168,   169,   268,   170,   171,   227,   180,   163,    42,
      43,    44,    63,    64,    65,    66,    67,    68,    69,    70,
      45,    46,    47,    48
  };

  const short int
   Parser ::yytable_[] =
  {
      56,    72,    71,    60,   162,    38,    75,   132,   120,   134,
     119,    23,   104,     2,   220,     3,     4,     3,     4,    51,
       5,   156,     3,   176,     4,     6,   177,     7,     8,   228,
     -19,   121,   122,     3,     4,     4,     9,   243,   229,    10,
     102,   153,   157,    11,    52,    12,   206,    13,   154,    72,
      71,    12,   208,    13,    53,    58,    14,    62,   244,    15,
      38,   254,   150,    61,   189,    15,   178,     3,     4,   251,
      55,   258,   137,   138,   139,    62,   185,   206,   140,   141,
     149,    38,   142,    38,   143,   144,   145,   146,   147,   148,
     102,    59,   274,   230,    76,    12,   149,    13,   181,   225,
     226,   102,    75,   203,   182,   204,   102,   179,   102,    15,
     213,   206,   125,    72,   187,    78,   -93,    38,    71,   191,
     198,    73,    74,   175,    55,    99,   183,   154,  -119,  -119,
     275,  -119,   205,   215,   129,   216,   -94,   276,  -119,   105,
     206,   233,   100,     3,     4,   200,   201,  -119,   278,    12,
    -119,    13,  -119,   237,   266,   279,  -119,   238,  -119,  -119,
     101,   125,   250,    15,  -119,   129,   109,  -119,     3,     4,
    -119,    55,    72,    13,   116,   245,   107,    71,     3,     4,
     246,     5,   136,   108,    38,    62,    73,    74,     7,    55,
     223,    89,    90,    91,    92,   179,    12,     9,    13,   110,
      10,   232,   115,   234,   117,    80,    12,   131,    13,    81,
      15,   123,    82,    83,    12,    38,    13,    14,   124,    85,
     111,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,     3,     4,   125,     5,    38,   179,   159,   160,
     161,    49,     7,   210,   211,  -120,  -120,   126,  -120,   -47,
      50,     9,   127,   -47,    10,  -120,   128,   129,   130,   133,
      12,   135,    13,   151,  -120,   152,   155,  -120,   -58,  -120,
     184,    14,   188,   110,    15,  -120,  -120,   192,   193,   194,
     195,  -120,   164,   164,  -120,   159,   160,   161,   197,   165,
     209,   -98,   199,   207,   206,   202,   214,   118,   154,   122,
     121,   222,    82,    83,   239,   235,   231,   240,   166,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   236,   241,   247,    84,   248,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   249,   252,   255,   196,   257,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   259,   261,   212,   260,   262,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   263,   267,   264,   224,   269,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   272,   277,   280,   242,   282,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,   186,   281,   190,     0,     0,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    82,    83,     0,     0,     0,     0,     0,    85,
      86,    87,    88,    89,    90,    91,    92,  -121,    94,    95,
      96,    97,    82,    83,     0,     0,     0,     0,     0,    85,
      86,    87,    88,    89,    90,    91,    92,     0,     0,     0,
      96,    97,    85,    86,    87,    88,    89,    90,    91,    92
  };

  const short int
   Parser ::yycheck_[] =
  {
       9,    13,    13,    12,   107,     1,    15,    77,    61,    79,
      61,     1,    49,     0,   193,     4,     5,     4,     5,     7,
       7,     7,     4,     4,     5,    12,     7,    14,    15,   204,
      17,     4,     5,     4,     5,     5,    23,    43,   205,    26,
      49,    33,    28,    30,     7,    32,    52,    34,    40,    61,
      61,    32,   166,    34,     7,     7,    43,    46,    33,    46,
      56,   240,    99,    34,    46,    46,    47,     4,     5,   236,
       7,   246,    81,    82,    83,    46,    46,    52,    87,    88,
      99,    77,    91,    79,    93,    94,    95,    96,    97,    98,
      99,     7,    33,   207,    16,    32,   115,    34,   117,   202,
     203,   110,   111,    32,    35,    34,   115,   116,   117,    46,
     180,    52,    43,   125,   126,    17,    41,   113,   129,   130,
      33,     4,     5,   113,     7,    32,    35,    40,     4,     5,
      33,     7,    44,   184,    43,   188,    41,    40,    14,     7,
      52,   211,    41,     4,     5,   154,   155,    23,    33,    32,
      26,    34,    28,    35,   257,    40,    32,    35,    34,    35,
      52,    43,   232,    46,    40,    43,    43,    43,     4,     5,
      46,     7,   184,    34,    27,    35,    42,   188,     4,     5,
      40,     7,    18,    42,   180,    46,     4,     5,    14,     7,
     199,    49,    50,    51,    52,   204,    32,    23,    34,    32,
      26,   210,    32,   212,    32,    40,    32,     7,    34,    44,
      46,    35,    38,    39,    32,   211,    34,    43,    40,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,     4,     5,    43,     7,   232,   246,     9,    10,
      11,    32,    14,    24,    25,     4,     5,    40,     7,    40,
      41,    23,    35,    44,    26,    14,    40,    43,    40,     7,
      32,     7,    34,     7,    23,    33,    44,    26,    28,    28,
      34,    43,    34,    32,    46,    34,    35,    28,    32,    28,
       7,    40,     7,     7,    43,     9,    10,    11,    33,    13,
      28,    41,    44,    36,    52,    42,    33,    33,    40,     5,
       4,    19,    38,    39,    33,    28,    47,    40,    32,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,    54,     7,    43,    43,    37,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,     4,    31,    20,    43,    32,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,    32,    43,    42,    33,    32,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,     7,     7,    21,    43,     7,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,    45,    42,     7,    43,   279,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,   125,   277,   129,    -1,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    38,    39,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    48,    49,    50,    51,    52,    -1,    -1,    -1,
      56,    57,    45,    46,    47,    48,    49,    50,    51,    52
  };

  const unsigned char
   Parser ::yystos_[] =
  {
       0,    59,     0,     4,     5,     7,    12,    14,    15,    23,
      26,    30,    32,    34,    43,    46,    60,    61,    62,    65,
      66,    69,    70,    81,    82,    83,    84,    88,    89,    90,
      93,    94,    95,    96,    97,    98,    99,   101,   102,   103,
     104,   105,   117,   118,   119,   128,   129,   130,   131,    32,
      41,     7,     7,     7,    75,     7,    96,   102,     7,     7,
      96,    34,    46,   120,   121,   122,   123,   124,   125,   126,
     127,   129,   130,     4,     5,    96,    16,    68,    17,    72,
      40,    44,    38,    39,    43,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,   100,    32,
      41,    52,    96,   106,   107,     7,    63,    42,    42,    43,
      32,    46,    79,    80,    81,    32,    27,    32,    33,   122,
     126,     4,     5,    35,    40,    43,    40,    35,    40,    43,
      40,     7,    79,     7,    79,     7,    18,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,   106,
     107,     7,    33,    33,    40,    44,     7,    28,    64,     9,
      10,    11,   113,   116,     7,    13,    32,   108,   109,   110,
     112,   113,   107,    91,    92,    81,     4,     7,    47,    96,
     115,   106,    35,    35,    34,    46,   123,   130,    34,    46,
     127,   129,    28,    32,    28,     7,    43,    33,    33,    44,
      96,    96,    42,    32,    34,    44,    52,    36,   109,    28,
      24,    25,    42,    79,    33,   122,   126,    67,    73,    74,
      75,    71,    19,    96,    43,   113,   113,   114,   115,   118,
     109,    47,    96,    79,    96,    28,    54,    35,    35,    33,
      40,     7,    43,    43,    33,    35,    40,    43,    37,     4,
      79,   118,    31,    76,    75,    20,    85,    32,   115,    32,
      33,    43,    32,     7,    21,    86,   113,     7,   111,     7,
      77,    78,    45,    87,    33,    33,    40,    42,    33,    40,
       7,   108,    78
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
      92,    92,    93,    94,    95,    95,    96,    96,    96,    96,
      96,    96,    96,    97,    98,    99,    99,    99,    99,    99,
      99,    99,   100,   100,   100,   100,   101,   101,   101,   101,
     101,   101,   102,   103,   103,   103,   104,   104,   105,   106,
     106,   107,   107,   108,   108,   108,   108,   109,   110,   110,
     111,   111,   112,   113,   113,   113,   113,   114,   114,   115,
     115,   115,   115,   116,   116,   117,   118,   118,   118,   119,
     119,   120,   120,   121,   121,   122,   122,   123,   123,   124,
     124,   125,   125,   126,   126,   127,   127,   128,   128,   129,
     129,   130,   130,   131
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
       0,     4,     6,     7,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     3,     2,
       3,     3,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     4,     4,     4,     0,
       1,     1,     3,     1,     1,     1,     1,     1,     4,     7,
       1,     3,     5,     1,     4,     7,     2,     1,     3,     1,
       1,     3,     1,     1,     1,     1,     1,     2,     1,     3,
       3,     1,     1,     3,     5,     1,     3,     1,     3,     1,
       1,     3,     5,     1,     3,     1,     3,     1,     1,     1,
       2,     1,     2,     8
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
  "elif_clauses", "for_stmt", "const_stmt", "expr_stmt", "expr",
  "ident_expr", "paren_expr", "linear_algebra_expr", "elwise_binary_op",
  "boolean_expr", "field_read_expr", "set_read_expr",
  "call_or_paren_read_expr", "call_expr", "expr_list_or_empty",
  "expr_list", "type", "element_type", "set_type", "endpoints",
  "tuple_type", "tensor_type", "index_sets", "index_set", "component_type",
  "literal_expr", "tensor_literal", "dense_tensor_literal",
  "float_dense_tensor_literal", "float_dense_ndtensor_literal",
  "float_dense_matrix_literal", "float_dense_vector_literal",
  "int_dense_tensor_literal", "int_dense_ndtensor_literal",
  "int_dense_matrix_literal", "int_dense_vector_literal", "scalar_literal",
  "signed_int_literal", "signed_float_literal", "test", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
   Parser ::yyrline_[] =
  {
       0,   272,   272,   274,   277,   278,   286,   294,   295,   296,
     301,   311,   324,   327,   336,   346,   346,   346,   354,   371,
     371,   371,   379,   400,   403,   409,   414,   422,   431,   434,
     440,   445,   453,   463,   466,   473,   474,   477,   478,   479,
     480,   481,   482,   483,   484,   489,   520,   572,   576,   583,
     586,   599,   602,   608,   613,   633,   653,   659,   664,   666,
     670,   672,   678,   681,   709,   712,   720,   721,   722,   723,
     724,   725,   726,   732,   754,   763,   772,   784,   851,   857,
     887,   892,   901,   902,   903,   904,   910,   916,   922,   928,
     934,   940,   951,   970,   971,   972,   978,  1013,  1019,  1035,
    1038,  1044,  1050,  1061,  1062,  1063,  1064,  1068,  1080,  1084,
    1094,  1103,  1115,  1127,  1131,  1134,  1176,  1186,  1191,  1199,
    1202,  1216,  1222,  1228,  1231,  1281,  1285,  1286,  1290,  1294,
    1301,  1312,  1319,  1323,  1327,  1341,  1345,  1360,  1364,  1371,
    1378,  1382,  1386,  1400,  1404,  1419,  1423,  1430,  1434,  1441,
    1444,  1450,  1453,  1460
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

#line 5 "parser.ypp" // lalr1.cc:1155
} } //  simit::internal 
#line 3289 "parser.cpp" // lalr1.cc:1155
#line 1479 "parser.ypp" // lalr1.cc:1156

