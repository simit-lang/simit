/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */


  #include <stdlib.h>
  #include <assert.h>
  #include <iostream>
  #include <algorithm>

  #include "Logger.h"
  #include "Frontend.h"
  #include "Util.h"
  #include "Errors.h"
  #include "Test.h"
  using namespace std;
  using namespace simit;

  #define REPORT_ERROR(msg, loc) \
    yyerror(&loc, symtable, errors, tests, std::string(msg).c_str())





/* Copy the first part of user declarations.  */



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "Tokens.h".  */
#ifndef YY_YY_USERS_FRED_PROJECTS_SIM_SIMIT_SRC_TOKENS_H_INCLUDED
# define YY_YY_USERS_FRED_PROJECTS_SIM_SIMIT_SRC_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


  #include <vector>

  #include "IR.h"
  #include "Types.h"


  #include <assert.h>
  namespace {
    template <typename T>
    class TensorValues {
     public:
      TensorValues() : dimSizes(1) {};

      void addValue(const T &val) {
        values.push_back(val);
        dimSizes[dimSizes.size()-1]++;
      }
      void addDimension() { dimSizes.push_back(1); }

      bool dimensionsMatch(const TensorValues<T> &other, std::string *errors) {
        assert(errors != NULL);
        std::string mismatchError = "error, missmatched dimension sizes";
        if (dimSizes.size()-1 != other.dimSizes.size()) {
          *errors = mismatchError;
          return false;
        }

        for (unsigned int i=0; i<dimSizes.size()-1; ++i) {
          if (dimSizes[i] != other.dimSizes[i]) {
            *errors = mismatchError;
            return false;
          }
        }
        return true;
      }

      void merge(const TensorValues<T> &other) {
        values.insert(values.end(), other.values.begin(), other.values.end());
        dimSizes[dimSizes.size()-1]++;
      }

      std::vector<unsigned int> dimSizes;
      std::vector<T>            values;
    };
  }



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    UNKNOWN = 258,
    NEG = 259,
    INT_LITERAL = 260,
    FLOAT_LITERAL = 261,
    STRING_LITERAL = 262,
    IDENT = 263,
    INT = 264,
    FLOAT = 265,
    STRUCT = 266,
    CONST = 267,
    EXTERN = 268,
    PROC = 269,
    FUNC = 270,
    TENSOR = 271,
    MAP = 272,
    TO = 273,
    WITH = 274,
    REDUCE = 275,
    WHILE = 276,
    IF = 277,
    ELIF = 278,
    ELSE = 279,
    BLOCKEND = 280,
    RETURN = 281,
    TEST = 282,
    RARROW = 283,
    LP = 284,
    RP = 285,
    LB = 286,
    RB = 287,
    LC = 288,
    RC = 289,
    LA = 290,
    RA = 291,
    COMMA = 292,
    PERIOD = 293,
    COL = 294,
    SEMICOL = 295,
    ASSIGN = 296,
    PLUS = 297,
    MINUS = 298,
    STAR = 299,
    SLASH = 300,
    EXP = 301,
    TRANSPOSE = 302,
    BACKSLASH = 303,
    EQ = 304,
    NE = 305,
    LE = 306,
    GE = 307
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{


  // Primitive literals
  int                               num;
  double                            fnum;
  const char                       *string;

  // Values
  simit::Value                     *value;


  simit::Type                      *type;
  simit::ElementType               *element_type;
  simit::TensorType                *tensor_type;
  simit::TensorType::ComponentType  scalar_type;
  std::vector<simit::Shape*>       *shapes;
  simit::Shape                     *shape;
  std::vector<simit::Dimension*>   *dimensions;
  simit::Dimension                 *dimension;


  simit::LiteralTensor      *literal_tensor;
  simit::DenseLiteralTensor *dense_literal_tensor;
  TensorValues<double>      *float_values;
  TensorValues<int>         *int_values;


};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests);

#endif /* !YY_YY_USERS_FRED_PROJECTS_SIM_SIMIT_SRC_TOKENS_H_INCLUDED  */

/* Copy the second part of user declarations.  */


/* Unqualified %code blocks.  */


  #include "Scanner.h"
  void yyerror(YYLTYPE                                  *loc,
               simit::SymbolTable                       &symtable,
               std::list<std::shared_ptr<simit::Error>> &errors,
               std::list<std::shared_ptr<simit::Test>>  &tests,
               const char                               *errorStr) {
    errors.push_back(shared_ptr<Error>(new Error(loc->first_line,
                                                 loc->first_column,
                                                 loc->last_line,
                                                 loc->last_column,
                                                 errorStr)));
  }


  Shape *dimSizesToShape(const vector<unsigned int> &dimSizes) {
    auto dims = std::vector<Dimension*>();
    for (auto rit=dimSizes.rbegin(); rit!=dimSizes.rend(); ++rit) {
      dims.push_back(new Dimension(*rit));
    }
    return new Shape(dims);
  }



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
# define YYCOPY_NEEDED 1
#endif


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   386

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  134
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  238

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   137,   137,   139,   142,   143,   144,   145,   146,   147,
     151,   155,   161,   164,   171,   175,   177,   179,   181,   184,
     189,   194,   197,   201,   203,   205,   207,   210,   211,   215,
     217,   220,   221,   222,   223,   224,   227,   251,   257,   259,
     261,   263,   265,   268,   271,   274,   275,   280,   283,   284,
     285,   287,   288,   289,   290,   291,   292,   293,   294,   296,
     297,   298,   299,   300,   301,   303,   304,   305,   306,   307,
     308,   311,   314,   315,   318,   319,   324,   329,   331,   335,
     337,   342,   344,   346,   349,   352,   355,   361,   362,   367,
     395,   398,   403,   409,   412,   422,   425,   431,   437,   441,
     447,   450,   454,   459,   462,   533,   534,   536,   540,   541,
     554,   560,   568,   575,   578,   582,   596,   600,   615,   619,
     625,   632,   635,   639,   652,   656,   669,   673,   679,   682,
     688,   692,   694,   697,   698
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "UNKNOWN", "NEG",
  "\"int literal\"", "\"float literal\"", "\"string literal\"",
  "\"identifier\"", "\"int\"", "\"float\"", "\"struct\"", "\"const\"",
  "\"extern\"", "\"proc\"", "\"func\"", "\"Tensor\"", "\"map\"", "\"to\"",
  "\"with\"", "\"reduce\"", "\"while\"", "\"if\"", "\"elif\"", "\"else\"",
  "\"end\"", "\"return\"", "\"test\"", "\"->\"", "\"(\"", "\")\"", "\"[\"",
  "\"]\"", "\"{\"", "\"}\"", "\"<\"", "\">\"", "\",\"", "\".\"", "\":\"",
  "\";\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"^\"", "\"'\"",
  "\"\\\\\"", "\"==\"", "\"!=\"", "\"<=\"", "\">=\"", "$accept", "program",
  "program_element", "extern", "endpoints", "struct", "struct_decl_block",
  "struct_decl_list", "field_decl", "procedure", "function",
  "function_header", "params", "results", "param_list", "stmt_block",
  "stmt", "const_stmt", "if_stmt", "else_clauses", "elif_clauses",
  "return_stmt", "assign_stmt", "expr_stmt", "expr", "field_expr",
  "call_expr", "expr_list", "map_expr", "with", "reduce", "index_expr",
  "reduction_indices", "reduction_index", "reduction_op", "var_decl",
  "type", "element_type", "tensor_type", "shapes", "shape", "dimensions",
  "dimension", "scalar_type", "literal", "element_literal",
  "tensor_literal", "dense_tensor_literal", "float_dense_tensor_literal",
  "float_dense_ndtensor_literal", "float_dense_matrix_literal",
  "float_dense_vector_literal", "int_dense_tensor_literal",
  "int_dense_ndtensor_literal", "int_dense_matrix_literal",
  "int_dense_vector_literal", "scalar_literal", "test", "test_stmt_block",
  "test_stmt", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307
};
# endif

#define YYPACT_NINF -184

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-184)))

#define YYTABLE_NINF -89

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-89)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -184,    60,  -184,  -184,  -184,  -184,  -184,    12,    29,    86,
      89,   127,   130,   208,   115,  -184,   208,  -184,   208,  -184,
    -184,  -184,  -184,  -184,  -184,  -184,  -184,  -184,  -184,  -184,
    -184,   238,  -184,  -184,    55,  -184,  -184,   122,  -184,     8,
     129,    96,  -184,   140,   157,   286,  -184,   114,   262,    95,
     141,   191,   208,   208,   208,   208,  -184,   208,   208,   208,
     208,   208,  -184,   208,   208,   208,   208,   208,   208,   208,
     143,  -184,   195,   286,  -184,    -2,   138,  -184,   154,   172,
     142,    96,  -184,  -184,  -184,  -184,    92,  -184,  -184,  -184,
     166,   172,   177,    73,  -184,   214,  -184,  -184,  -184,  -184,
    -184,   286,   -22,   -17,   -17,   286,   286,   201,   201,    95,
      95,    95,   310,   334,   334,   -17,   -17,   286,    71,   178,
    -184,   179,    96,  -184,  -184,  -184,   148,   149,   121,   185,
     160,  -184,   174,   168,  -184,   188,   186,    46,    30,  -184,
    -184,   176,   183,  -184,  -184,    30,   150,     5,  -184,  -184,
     -14,   182,   190,   172,   211,   203,  -184,   208,  -184,  -184,
    -184,    57,   187,  -184,  -184,  -184,  -184,  -184,  -184,   189,
     192,   196,  -184,  -184,  -184,   100,  -184,   198,   225,  -184,
     206,  -184,  -184,  -184,    67,  -184,   286,   114,  -184,  -184,
     156,   204,   207,   215,   231,   209,   232,   230,   234,  -184,
    -184,  -184,  -184,  -184,     5,   241,  -184,   172,  -184,  -184,
     114,   -23,    44,  -184,   248,   266,   287,  -184,   263,   290,
     291,  -184,   259,    77,  -184,  -184,   266,   231,  -184,   290,
     234,  -184,  -184,  -184,    51,    61,  -184,  -184
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,    82,     1,    49,    48,    50,    47,     0,     0,     0,
       0,     0,     0,    82,     0,   131,    82,    45,    82,     3,
       8,     4,     5,     6,    29,     7,    31,    32,    33,    34,
      35,    74,    68,    67,     0,    70,    69,     0,     9,    15,
       0,     0,    29,     0,     0,    29,    43,   130,     0,    56,
      82,    82,    82,    82,    82,    82,    46,    82,    82,    82,
      82,    82,    57,    82,    82,    82,    82,    82,    82,    82,
      47,    87,    82,    81,    83,     0,     0,    14,     0,    17,
       0,     0,    92,   103,   104,    95,     0,    90,    91,    93,
      82,    23,     0,    41,   133,    74,   132,    65,    21,    30,
      72,    74,     0,    61,    62,    71,    66,    52,    53,    54,
      55,    58,    51,    59,    60,    63,    64,    75,     0,     0,
      84,     0,     0,    16,    18,    19,     0,     0,     0,     0,
       0,    20,     0,    24,    27,    77,     0,     0,   107,    73,
      44,     0,     0,    89,   107,     0,     0,     0,    96,    12,
       0,     0,    25,     0,     0,    79,    38,    82,    29,   128,
     129,     0,     0,   105,   106,   108,   109,    86,    85,     0,
       0,     0,   100,   101,   102,     0,    98,     0,     0,    10,
       0,    22,    28,    78,     0,    76,    29,    40,   126,   118,
       0,     0,   113,   112,   116,     0,   121,   120,   124,   134,
      37,    36,    94,    97,     0,     0,    13,     0,    88,    80,
      42,     0,     0,   110,     0,     0,     0,   111,     0,     0,
       0,    99,     0,     0,   114,   122,     0,   117,   119,     0,
     125,   127,    11,    26,     0,     0,   115,   123
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -184,  -184,  -184,  -184,  -184,  -184,  -184,   223,  -184,  -184,
    -184,  -184,  -184,  -184,   109,   -41,    10,  -184,  -184,  -184,
    -184,  -184,  -184,  -184,   -13,  -184,  -184,   -37,  -184,  -184,
    -184,  -184,  -184,  -184,   119,   -89,   197,   236,   237,  -184,
    -184,  -184,   116,   180,  -184,   199,   202,  -184,  -184,  -184,
    -183,   108,  -184,  -184,  -171,   123,  -184,  -184,  -184,  -184
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    19,    20,   150,    21,    77,    78,    79,    22,
      23,    24,   132,   181,   133,    50,    99,    26,    27,   136,
     137,    28,    29,    30,    31,    32,    33,    34,    35,   155,
     185,    36,    37,    74,    75,    80,    86,    87,    88,   128,
     148,   175,   176,    89,   162,   163,   164,   165,   191,   192,
     193,   194,   195,   196,   197,   198,   166,    38,    47,    96
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      45,    90,   134,    48,    93,    49,   120,   211,   139,   224,
     172,    25,    51,   173,   102,    68,    76,   215,   177,   212,
      39,    54,    55,   178,    73,    57,    58,    59,    60,    61,
      62,   121,   118,   -17,    95,   159,   160,    40,   101,   103,
     104,   105,   106,   234,   107,   108,   109,   110,   111,   174,
     112,   113,   114,   115,   116,   117,   101,    94,   235,    49,
       2,   161,   188,   189,   182,     3,     4,     5,     6,   157,
     158,     7,     8,     9,    10,    11,   225,    12,     3,     4,
       5,     6,    13,   236,   219,     8,    14,    15,   190,    16,
      12,   215,    68,   237,    41,    13,    69,    42,   -39,    14,
      17,   219,    16,    18,    82,    83,    84,   233,    68,    71,
     208,   140,    85,    17,   153,   -82,    18,   187,   134,     3,
       4,     5,     6,   129,    51,   130,     8,     3,     4,     5,
      70,    12,   203,    54,    55,    43,    13,   204,    44,    12,
      14,    61,    62,    16,   186,   210,     3,     4,     5,     6,
     146,    16,   147,     8,    17,    46,   -82,    18,    12,    83,
      84,   188,   189,    13,    71,    72,    98,    14,    81,    91,
      16,     3,     4,     5,     6,    92,   119,   122,     8,   123,
      76,    17,   125,    12,    18,   135,   141,   142,    13,   144,
     145,   131,    14,   149,   151,    16,     3,     4,     5,     6,
       3,     4,     5,     6,   152,   153,    17,   154,    12,    18,
     167,   156,    12,     3,     4,     5,     6,   168,   180,   183,
      16,   100,   179,   184,    16,    12,   202,   199,   -88,   200,
      51,   205,   201,   206,    18,   207,   213,    16,    18,    54,
      55,   217,   138,    51,   214,    59,    60,    61,    62,    52,
      53,    18,    54,    55,    56,   215,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    51,   216,   218,
     219,   220,   189,    52,    53,   222,    54,    55,    56,   226,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    51,    97,   228,   229,   188,   231,    52,    53,   232,
      54,    55,   124,   209,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    51,   223,   126,   127,   143,
     221,    52,    53,   227,    54,    55,   171,     0,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    51,
       0,     0,   230,   169,     0,    52,    53,   170,    54,    55,
       0,     0,    57,    58,    59,    60,    61,    62,   -89,    64,
      65,    66,    67,    51,     0,     0,     0,     0,     0,    52,
      53,     0,    54,    55,     0,     0,    57,    58,    59,    60,
      61,    62,     0,     0,     0,    66,    67
};

static const yytype_int16 yycheck[] =
{
      13,    42,    91,    16,    45,    18,     8,   190,    30,    32,
       5,     1,    29,     8,    51,    37,     8,    40,    32,   190,
       8,    38,    39,    37,    37,    42,    43,    44,    45,    46,
      47,    33,    69,    25,    47,     5,     6,     8,    51,    52,
      53,    54,    55,   226,    57,    58,    59,    60,    61,    44,
      63,    64,    65,    66,    67,    68,    69,    47,   229,    72,
       0,    31,     5,     6,   153,     5,     6,     7,     8,    23,
      24,    11,    12,    13,    14,    15,    32,    17,     5,     6,
       7,     8,    22,    32,    40,    12,    26,    27,    31,    29,
      17,    40,    37,    32,     8,    22,    41,     8,    25,    26,
      40,    40,    29,    43,     8,     9,    10,    30,    37,    42,
      43,    40,    16,    40,    37,    42,    43,   158,   207,     5,
       6,     7,     8,    31,    29,    33,    12,     5,     6,     7,
       8,    17,    32,    38,    39,     8,    22,    37,     8,    17,
      26,    46,    47,    29,   157,   186,     5,     6,     7,     8,
      29,    29,    31,    12,    40,    40,    42,    43,    17,     9,
      10,     5,     6,    22,    42,    43,    25,    26,    39,    29,
      29,     5,     6,     7,     8,    18,    33,    39,    12,    25,
       8,    40,    40,    17,    43,     8,     8,     8,    22,    41,
      41,    25,    26,     8,    34,    29,     5,     6,     7,     8,
       5,     6,     7,     8,    30,    37,    40,    19,    17,    43,
      34,    25,    17,     5,     6,     7,     8,    34,    28,     8,
      29,    30,    40,    20,    29,    17,    30,    40,    33,    40,
      29,    33,    40,     8,    43,    29,    32,    29,    43,    38,
      39,    32,    28,    29,    37,    44,    45,    46,    47,    35,
      36,    43,    38,    39,    40,    40,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    29,    37,    37,
      40,    37,     6,    35,    36,    34,    38,    39,    40,    31,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    29,    30,     6,    31,     5,     5,    35,    36,    40,
      38,    39,    79,   184,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    29,   207,    81,    81,   122,
     204,    35,    36,   215,    38,    39,   146,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    29,
      -1,    -1,   219,   144,    -1,    35,    36,   145,    38,    39,
      -1,    -1,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    29,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    38,    39,    -1,    -1,    42,    43,    44,    45,
      46,    47,    -1,    -1,    -1,    51,    52
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    54,     0,     5,     6,     7,     8,    11,    12,    13,
      14,    15,    17,    22,    26,    27,    29,    40,    43,    55,
      56,    58,    62,    63,    64,    69,    70,    71,    74,    75,
      76,    77,    78,    79,    80,    81,    84,    85,   110,     8,
       8,     8,     8,     8,     8,    77,    40,   111,    77,    77,
      68,    29,    35,    36,    38,    39,    40,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    37,    41,
       8,    42,    43,    77,    86,    87,     8,    59,    60,    61,
      88,    39,     8,     9,    10,    16,    89,    90,    91,    96,
      68,    29,    18,    68,    69,    77,   112,    30,    25,    69,
      30,    77,    80,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    80,    33,
       8,    33,    39,    25,    60,    40,    90,    91,    92,    31,
      33,    25,    65,    67,    88,     8,    72,    73,    28,    30,
      40,     8,     8,    89,    41,    41,    29,    31,    93,     8,
      57,    34,    30,    37,    19,    82,    25,    23,    24,     5,
       6,    31,    97,    98,    99,   100,   109,    34,    34,    98,
      99,    96,     5,     8,    44,    94,    95,    32,    37,    40,
      28,    66,    88,     8,    20,    83,    77,    68,     5,     6,
      31,   101,   102,   103,   104,   105,   106,   107,   108,    40,
      40,    40,    30,    32,    37,    33,     8,    29,    43,    87,
      68,   103,   107,    32,    37,    40,    37,    32,    37,    40,
      37,    95,    34,    67,    32,    32,    31,   104,     6,    31,
     108,     5,    40,    30,   103,   107,    32,    32
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    54,    54,    55,    55,    55,    55,    55,    55,
      56,    56,    57,    57,    58,    59,    59,    60,    60,    61,
      62,    63,    64,    65,    65,    66,    66,    67,    67,    68,
      68,    69,    69,    69,    69,    69,    70,    70,    71,    72,
      72,    73,    73,    74,    75,    76,    76,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    78,    79,    79,    80,    80,    81,    82,    82,    83,
      83,    84,    85,    85,    86,    86,    86,    87,    87,    88,
      89,    89,    90,    91,    91,    92,    92,    93,    94,    94,
      95,    95,    95,    96,    96,    97,    97,    98,    99,    99,
     100,   100,   101,   101,   102,   102,   103,   103,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   109,
     110,   111,   111,   112,   112
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       6,     9,     1,     3,     3,     0,     2,     0,     2,     2,
       4,     3,     6,     0,     1,     0,     4,     1,     3,     0,
       2,     1,     1,     1,     1,     1,     7,     7,     5,     0,
       3,     0,     4,     2,     4,     1,     2,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     3,     3,     4,     1,     3,     6,     0,     2,     0,
       2,     2,     0,     2,     2,     4,     4,     1,     1,     3,
       1,     1,     1,     1,     5,     0,     2,     3,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     1,
       3,     3,     1,     1,     3,     5,     1,     3,     1,     3,
       1,     1,     3,     5,     1,     3,     1,     3,     1,     1,
       2,     0,     2,     1,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      YY_LAC_DISCARD ("YYBACKUP");                              \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, symtable, errors, tests, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, symtable, errors, tests); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (symtable);
  YYUSE (errors);
  YYUSE (tests);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, symtable, errors, tests);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , symtable, errors, tests);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, symtable, errors, tests); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

/* Given a state stack such that *YYBOTTOM is its bottom, such that
   *YYTOP is either its top or is YYTOP_EMPTY to indicate an empty
   stack, and such that *YYCAPACITY is the maximum number of elements it
   can hold without a reallocation, make sure there is enough room to
   store YYADD more elements.  If not, allocate a new stack using
   YYSTACK_ALLOC, copy the existing elements, and adjust *YYBOTTOM,
   *YYTOP, and *YYCAPACITY to reflect the new capacity and memory
   location.  If *YYBOTTOM != YYBOTTOM_NO_FREE, then free the old stack
   using YYSTACK_FREE.  Return 0 if successful or if no reallocation is
   required.  Return 1 if memory is exhausted.  */
static int
yy_lac_stack_realloc (YYSIZE_T *yycapacity, YYSIZE_T yyadd,
#if YYDEBUG
                      char const *yydebug_prefix,
                      char const *yydebug_suffix,
#endif
                      yytype_int16 **yybottom,
                      yytype_int16 *yybottom_no_free,
                      yytype_int16 **yytop, yytype_int16 *yytop_empty)
{
  YYSIZE_T yysize_old =
    *yytop == yytop_empty ? 0 : *yytop - *yybottom + 1;
  YYSIZE_T yysize_new = yysize_old + yyadd;
  if (*yycapacity < yysize_new)
    {
      YYSIZE_T yyalloc = 2 * yysize_new;
      yytype_int16 *yybottom_new;
      /* Use YYMAXDEPTH for maximum stack size given that the stack
         should never need to grow larger than the main state stack
         needs to grow without LAC.  */
      if (YYMAXDEPTH < yysize_new)
        {
          YYDPRINTF ((stderr, "%smax size exceeded%s", yydebug_prefix,
                      yydebug_suffix));
          return 1;
        }
      if (YYMAXDEPTH < yyalloc)
        yyalloc = YYMAXDEPTH;
      yybottom_new =
        (yytype_int16*) YYSTACK_ALLOC (yyalloc * sizeof *yybottom_new);
      if (!yybottom_new)
        {
          YYDPRINTF ((stderr, "%srealloc failed%s", yydebug_prefix,
                      yydebug_suffix));
          return 1;
        }
      if (*yytop != yytop_empty)
        {
          YYCOPY (yybottom_new, *yybottom, yysize_old);
          *yytop = yybottom_new + (yysize_old - 1);
        }
      if (*yybottom != yybottom_no_free)
        YYSTACK_FREE (*yybottom);
      *yybottom = yybottom_new;
      *yycapacity = yyalloc;
    }
  return 0;
}

/* Establish the initial context for the current lookahead if no initial
   context is currently established.

   We define a context as a snapshot of the parser stacks.  We define
   the initial context for a lookahead as the context in which the
   parser initially examines that lookahead in order to select a
   syntactic action.  Thus, if the lookahead eventually proves
   syntactically unacceptable (possibly in a later context reached via a
   series of reductions), the initial context can be used to determine
   the exact set of tokens that would be syntactically acceptable in the
   lookahead's place.  Moreover, it is the context after which any
   further semantic actions would be erroneous because they would be
   determined by a syntactically unacceptable token.

   YY_LAC_ESTABLISH should be invoked when a reduction is about to be
   performed in an inconsistent state (which, for the purposes of LAC,
   includes consistent states that don't know they're consistent because
   their default reductions have been disabled).  Iff there is a
   lookahead token, it should also be invoked before reporting a syntax
   error.  This latter case is for the sake of the debugging output.

   For parse.lac=full, the implementation of YY_LAC_ESTABLISH is as
   follows.  If no initial context is currently established for the
   current lookahead, then check if that lookahead can eventually be
   shifted if syntactic actions continue from the current context.
   Report a syntax error if it cannot.  */
#define YY_LAC_ESTABLISH                                         \
do {                                                             \
  if (!yy_lac_established)                                       \
    {                                                            \
      YYDPRINTF ((stderr,                                        \
                  "LAC: initial context established for %s\n",   \
                  yytname[yytoken]));                            \
      yy_lac_established = 1;                                    \
      {                                                          \
        int yy_lac_status =                                      \
          yy_lac (yyesa, &yyes, &yyes_capacity, yyssp, yytoken); \
        if (yy_lac_status == 2)                                  \
          goto yyexhaustedlab;                                   \
        if (yy_lac_status == 1)                                  \
          goto yyerrlab;                                         \
      }                                                          \
    }                                                            \
} while (0)

/* Discard any previous initial lookahead context because of Event,
   which may be a lookahead change or an invalidation of the currently
   established initial context for the current lookahead.

   The most common example of a lookahead change is a shift.  An example
   of both cases is syntax error recovery.  That is, a syntax error
   occurs when the lookahead is syntactically erroneous for the
   currently established initial context, so error recovery manipulates
   the parser stacks to try to find a new initial context in which the
   current lookahead is syntactically acceptable.  If it fails to find
   such a context, it discards the lookahead.  */
#if YYDEBUG
# define YY_LAC_DISCARD(Event)                                           \
do {                                                                     \
  if (yy_lac_established)                                                \
    {                                                                    \
      if (yydebug)                                                       \
        YYFPRINTF (stderr, "LAC: initial context discarded due to "      \
                   Event "\n");                                          \
      yy_lac_established = 0;                                            \
    }                                                                    \
} while (0)
#else
# define YY_LAC_DISCARD(Event) yy_lac_established = 0
#endif

/* Given the stack whose top is *YYSSP, return 0 iff YYTOKEN can
   eventually (after perhaps some reductions) be shifted, return 1 if
   not, or return 2 if memory is exhausted.  As preconditions and
   postconditions: *YYES_CAPACITY is the allocated size of the array to
   which *YYES points, and either *YYES = YYESA or *YYES points to an
   array allocated with YYSTACK_ALLOC.  yy_lac may overwrite the
   contents of either array, alter *YYES and *YYES_CAPACITY, and free
   any old *YYES other than YYESA.  */
static int
yy_lac (yytype_int16 *yyesa, yytype_int16 **yyes,
        YYSIZE_T *yyes_capacity, yytype_int16 *yyssp, int yytoken)
{
  yytype_int16 *yyes_prev = yyssp;
  yytype_int16 *yyesp = yyes_prev;
  YYDPRINTF ((stderr, "LAC: checking lookahead %s:", yytname[yytoken]));
  if (yytoken == YYUNDEFTOK)
    {
      YYDPRINTF ((stderr, " Always Err\n"));
      return 1;
    }
  while (1)
    {
      int yyrule = yypact[*yyesp];
      if (yypact_value_is_default (yyrule)
          || (yyrule += yytoken) < 0 || YYLAST < yyrule
          || yycheck[yyrule] != yytoken)
        {
          yyrule = yydefact[*yyesp];
          if (yyrule == 0)
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
        }
      else
        {
          yyrule = yytable[yyrule];
          if (yytable_value_is_error (yyrule))
            {
              YYDPRINTF ((stderr, " Err\n"));
              return 1;
            }
          if (0 < yyrule)
            {
              YYDPRINTF ((stderr, " S%d\n", yyrule));
              return 0;
            }
          yyrule = -yyrule;
        }
      {
        YYSIZE_T yylen = yyr2[yyrule];
        YYDPRINTF ((stderr, " R%d", yyrule - 1));
        if (yyesp != yyes_prev)
          {
            YYSIZE_T yysize = yyesp - *yyes + 1;
            if (yylen < yysize)
              {
                yyesp -= yylen;
                yylen = 0;
              }
            else
              {
                yylen -= yysize;
                yyesp = yyes_prev;
              }
          }
        if (yylen)
          yyesp = yyes_prev -= yylen;
      }
      {
        int yystate;
        {
          int yylhs = yyr1[yyrule] - YYNTOKENS;
          yystate = yypgoto[yylhs] + *yyesp;
          if (yystate < 0 || YYLAST < yystate
              || yycheck[yystate] != *yyesp)
            yystate = yydefgoto[yylhs];
          else
            yystate = yytable[yystate];
        }
        if (yyesp == yyes_prev)
          {
            yyesp = *yyes;
            *yyesp = yystate;
          }
        else
          {
            if (yy_lac_stack_realloc (yyes_capacity, 1,
#if YYDEBUG
                                      " (", ")",
#endif
                                      yyes, yyesa, &yyesp, yyes_prev))
              {
                YYDPRINTF ((stderr, "\n"));
                return 2;
              }
            *++yyesp = yystate;
          }
        YYDPRINTF ((stderr, " G%d", yystate));
      }
    }
}


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.  In order to see if a particular token T is a
   valid looakhead, invoke yy_lac (YYESA, YYES, YYES_CAPACITY, YYSSP, T).

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store or if
   yy_lac returned 2.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyesa, yytype_int16 **yyes,
                YYSIZE_T *yyes_capacity, yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      YYDPRINTF ((stderr, "Constructing syntax error message\n"));
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          int yyx;

          for (yyx = 0; yyx < YYNTOKENS; ++yyx)
            if (yyx != YYTERROR && yyx != YYUNDEFTOK)
              {
                {
                  int yy_lac_status = yy_lac (yyesa, yyes, yyes_capacity,
                                              yyssp, yyx);
                  if (yy_lac_status == 2)
                    return 2;
                  if (yy_lac_status == 1)
                    continue;
                }
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
# if YYDEBUG
      else if (yydebug)
        YYFPRINTF (stderr, "No expected tokens.\n");
# endif
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (symtable);
  YYUSE (errors);
  YYUSE (tests);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 5: /* "int literal"  */

      {}

        break;

    case 6: /* "float literal"  */

      {}

        break;

    case 7: /* "string literal"  */

      { free((void*)(((*yyvaluep).string))); }

        break;

    case 8: /* "identifier"  */

      { free((void*)(((*yyvaluep).string))); }

        break;

    case 89: /* type  */

      { delete ((*yyvaluep).type); }

        break;

    case 90: /* element_type  */

      { delete ((*yyvaluep).element_type); }

        break;

    case 91: /* tensor_type  */

      { delete ((*yyvaluep).tensor_type); }

        break;

    case 92: /* shapes  */

      { delete ((*yyvaluep).shapes); }

        break;

    case 93: /* shape  */

      { delete ((*yyvaluep).shape); }

        break;

    case 94: /* dimensions  */

      { delete ((*yyvaluep).dimensions); }

        break;

    case 95: /* dimension  */

      { delete ((*yyvaluep).dimension); }

        break;

    case 96: /* scalar_type  */

      {}

        break;

    case 97: /* literal  */

      { delete ((*yyvaluep).value); }

        break;

    case 99: /* tensor_literal  */

      { delete ((*yyvaluep).literal_tensor); }

        break;

    case 100: /* dense_tensor_literal  */

      { delete ((*yyvaluep).dense_literal_tensor); }

        break;

    case 101: /* float_dense_tensor_literal  */

      { delete ((*yyvaluep).float_values); }

        break;

    case 102: /* float_dense_ndtensor_literal  */

      { delete ((*yyvaluep).float_values); }

        break;

    case 103: /* float_dense_matrix_literal  */

      { delete ((*yyvaluep).float_values); }

        break;

    case 104: /* float_dense_vector_literal  */

      { delete ((*yyvaluep).float_values); }

        break;

    case 105: /* int_dense_tensor_literal  */

      { delete ((*yyvaluep).int_values); }

        break;

    case 106: /* int_dense_ndtensor_literal  */

      { delete ((*yyvaluep).int_values); }

        break;

    case 107: /* int_dense_matrix_literal  */

      { delete ((*yyvaluep).int_values); }

        break;

    case 108: /* int_dense_vector_literal  */

      { delete ((*yyvaluep).int_values); }

        break;

    case 109: /* scalar_literal  */

      { delete ((*yyvaluep).dense_literal_tensor); }

        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (simit::SymbolTable &symtable, std::list<std::shared_ptr<simit::Error>> &errors, std::list<std::shared_ptr<simit::Test>> &tests)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

    yytype_int16 yyesa[20];
    yytype_int16 *yyes;
    YYSIZE_T yyes_capacity;

  int yy_lac_established = 0;
  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  yyes = yyesa;
  yyes_capacity = sizeof yyesa / sizeof *yyes;
  if (YYMAXDEPTH < yyes_capacity)
    yyes_capacity = YYMAXDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    {
      YY_LAC_ESTABLISH;
      goto yydefault;
    }
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      YY_LAC_ESTABLISH;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  YY_LAC_DISCARD ("shift");

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  {
    int yychar_backup = yychar;
    switch (yyn)
      {
          case 10:

    {
    free((void*)(yyvsp[-4].string));
    delete (yyvsp[-3].type);
  }

    break;

  case 11:

    {
    free((void*)(yyvsp[-7].string));
    delete (yyvsp[-6].type);
  }

    break;

  case 12:

    {
    free((void*)(yyvsp[0].string));
  }

    break;

  case 13:

    {
    free((void*)(yyvsp[0].string));
  }

    break;

  case 14:

    {
    free((void*)(yyvsp[-1].string));
  }

    break;

  case 20:

    {
    free((void*)(yyvsp[-2].string));
  }

    break;

  case 22:

    {
    free((void*)(yyvsp[-4].string));
  }

    break;

  case 36:

    {
    (yyvsp[-1].literal_tensor)->setName((yyvsp[-5].string));
    free((void*)(yyvsp[-5].string));

    // If $type is a 1xn matrix and $tensor_literal is a vector then we cast
    // $tensor_literal to a 1xn matrix.
    bool casted = false;
    if ((yyvsp[-3].tensor_type)->getOrder() == 2 && (yyvsp[-1].literal_tensor)->getOrder() == 1) {
      (yyvsp[-1].literal_tensor)->cast((yyvsp[-3].tensor_type));
      casted = true;
    }

    // Typecheck: value and literal types must be equivalent.
    if (*(yyvsp[-3].tensor_type) != *(yyvsp[-1].literal_tensor)->getType()) {
      REPORT_ERROR("error, value type does not match literal type", (yylsp[-2]));
      delete (yyvsp[-1].literal_tensor);
      YYERROR;
    }

    if (!casted) {
      delete (yyvsp[-3].tensor_type);
    }
    symtable.addNode((yyvsp[-1].literal_tensor));
  }

    break;

  case 37:

    {
    free((void*)(yyvsp[-5].string));
    delete (yyvsp[-3].element_type);
  }

    break;

  case 47:

    {
    free((void*)(yyvsp[0].string));
  }

    break;

  case 76:

    {
    free((void*)(yyvsp[-4].string));
    free((void*)(yyvsp[-2].string));
  }

    break;

  case 78:

    {
    free((void*)(yyvsp[0].string));
  }

    break;

  case 84:

    {
    free((void*)(yyvsp[0].string));
  }

    break;

  case 85:

    {
    free((void*)(yyvsp[-1].string));
  }

    break;

  case 86:

    {
    free((void*)(yyvsp[-3].string));
    free((void*)(yyvsp[-1].string));
  }

    break;

  case 89:

    {
    delete (yyvsp[0].type);
    free((void*)(yyvsp[-2].string));
  }

    break;

  case 90:

    {
    (yyval.type) = (yyvsp[0].element_type);
  }

    break;

  case 91:

    {
    (yyval.type) = (yyvsp[0].tensor_type);
  }

    break;

  case 92:

    {
    (yyval.element_type) = new ElementType();
    free((void*)(yyvsp[0].string));
  }

    break;

  case 93:

    {
    (yyval.tensor_type) = new ScalarType((yyvsp[0].scalar_type));
  }

    break;

  case 94:

    {
    (yyval.tensor_type) = new ScalarType((yyvsp[-1].scalar_type));
    typedef vector<Shape*>::reverse_iterator shapes_rit_t;
    for (shapes_rit_t rit = (yyvsp[-3].shapes)->rbegin(); rit != (yyvsp[-3].shapes)->rend(); ++rit) {
      (yyval.tensor_type) = new NDTensorType(*rit, (yyval.tensor_type));
    }
    delete (yyvsp[-3].shapes);
  }

    break;

  case 95:

    {
    (yyval.shapes) = new vector<Shape*>();
  }

    break;

  case 96:

    {
    (yyval.shapes) = (yyvsp[-1].shapes);
    (yyval.shapes)->push_back((yyvsp[0].shape));
  }

    break;

  case 97:

    {
    (yyval.shape) = new Shape(*(yyvsp[-1].dimensions));
    delete (yyvsp[-1].dimensions);
  }

    break;

  case 98:

    {
    (yyval.dimensions) = new vector<Dimension*>();
    (yyval.dimensions)->push_back((yyvsp[0].dimension));
  }

    break;

  case 99:

    {
    (yyval.dimensions) = (yyvsp[-2].dimensions);
    (yyval.dimensions)->push_back((yyvsp[0].dimension));
  }

    break;

  case 100:

    {
    (yyval.dimension) = new Dimension((yyvsp[0].num));
  }

    break;

  case 101:

    {
    free((void*)(yyvsp[0].string));
    (yyval.dimension) = new Dimension(123456789);  // TODO: This needs to be a set dimension
  }

    break;

  case 102:

    {
    (yyval.dimension) = new Dimension();
  }

    break;

  case 103:

    {
    (yyval.scalar_type) = ScalarType::INT;
  }

    break;

  case 104:

    {
    (yyval.scalar_type) = ScalarType::FLOAT;
  }

    break;

  case 110:

    {
    Shape *shape = dimSizesToShape((yyvsp[-1].float_values)->dimSizes);
    auto type = new NDTensorType(shape, new ScalarType(ScalarType::FLOAT));
    (yyval.dense_literal_tensor) = new DenseLiteralTensor(type, (yyvsp[-1].float_values)->values.data());
    delete (yyvsp[-1].float_values);
  }

    break;

  case 111:

    {
    Shape *shape = dimSizesToShape((yyvsp[-1].int_values)->dimSizes);
    auto type = new NDTensorType(shape, new ScalarType(ScalarType::INT));
    (yyval.dense_literal_tensor) = new DenseLiteralTensor(type, (yyvsp[-1].int_values)->values.data());
    delete (yyvsp[-1].int_values);
  }

    break;

  case 112:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yyvsp[0].float_values)->dimSizes[(yyvsp[0].float_values)->dimSizes.size()-1] == 1) {
      (yyvsp[0].float_values)->dimSizes.pop_back();
    }
  }

    break;

  case 114:

    {
    (yyval.float_values) = (yyvsp[-1].float_values);
    (yyval.float_values)->addDimension();
  }

    break;

  case 115:

    {
    string errorStr;
    if(!(yyvsp[-4].float_values)->dimensionsMatch(*(yyvsp[-1].float_values), &errorStr)) {
      REPORT_ERROR(errorStr, (yylsp[-3]));
      delete (yyvsp[-4].float_values);
      delete (yyvsp[-1].float_values);
      YYERROR;
    }
    (yyval.float_values) = (yyvsp[-4].float_values);
    (yyvsp[-4].float_values)->merge(*(yyvsp[-1].float_values));
    delete (yyvsp[-1].float_values);
  }

    break;

  case 116:

    {
    (yyval.float_values) = (yyvsp[0].float_values);
    (yyval.float_values)->addDimension();
  }

    break;

  case 117:

    {
    string errorStr;
    if(!(yyvsp[-2].float_values)->dimensionsMatch(*(yyvsp[0].float_values), &errorStr)) {
      REPORT_ERROR(errorStr, (yylsp[-1]));
      delete (yyvsp[-2].float_values);
      delete (yyvsp[0].float_values);
      YYERROR;
    }

    (yyval.float_values) = (yyvsp[-2].float_values);
    (yyvsp[-2].float_values)->merge(*(yyvsp[0].float_values));
    delete (yyvsp[0].float_values);
  }

    break;

  case 118:

    {
    (yyval.float_values) = new TensorValues<double>();
    (yyval.float_values)->addValue((yyvsp[0].fnum));
  }

    break;

  case 119:

    {
    (yyval.float_values) = (yyvsp[-2].float_values);
    (yyval.float_values)->addValue((yyvsp[0].fnum));
  }

    break;

  case 120:

    {
    // If the matrix has only one column then we discard that dimension and
    // treat it as a vector.
    if ((yyvsp[0].int_values)->dimSizes[(yyvsp[0].int_values)->dimSizes.size()-1] == 1) {
      (yyvsp[0].int_values)->dimSizes.pop_back();
    }
  }

    break;

  case 122:

    {
    (yyval.int_values) = (yyvsp[-1].int_values);
    (yyval.int_values)->addDimension();
  }

    break;

  case 123:

    {
    string errorStr;
    if(!(yyvsp[-4].int_values)->dimensionsMatch(*(yyvsp[-1].int_values), &errorStr)) {
      REPORT_ERROR(errorStr, (yylsp[-3]));
      YYERROR;
    }

    (yyval.int_values) = (yyvsp[-4].int_values);
    (yyvsp[-4].int_values)->merge(*(yyvsp[-1].int_values));
    delete (yyvsp[-1].int_values);
  }

    break;

  case 124:

    {
    (yyval.int_values) = (yyvsp[0].int_values);
    (yyval.int_values)->addDimension();
  }

    break;

  case 125:

    {
    string errorStr;
    if(!(yyvsp[-2].int_values)->dimensionsMatch(*(yyvsp[0].int_values), &errorStr)) {
      REPORT_ERROR(errorStr, (yylsp[-1]));
      YYERROR;
    }

    (yyval.int_values) = (yyvsp[-2].int_values);
    (yyvsp[-2].int_values)->merge(*(yyvsp[0].int_values));
    delete (yyvsp[0].int_values);
  }

    break;

  case 126:

    {
    (yyval.int_values) = new TensorValues<int>();
    (yyval.int_values)->addValue((yyvsp[0].num));
  }

    break;

  case 127:

    {
    (yyval.int_values) = (yyvsp[-2].int_values);
    (yyval.int_values)->addValue((yyvsp[0].num));
  }

    break;

  case 128:

    {
    (yyval.dense_literal_tensor) = new DenseLiteralTensor(new ScalarType(ScalarType::INT), &(yyvsp[0].num));
  }

    break;

  case 129:

    {
    (yyval.dense_literal_tensor) = new DenseLiteralTensor(new ScalarType(ScalarType::FLOAT), &(yyvsp[0].fnum));
  }

    break;

  case 130:

    {
    tests.push_back(shared_ptr<Test>(new simit::Test("MyTest")));
  }

    break;

  case 134:

    {
    delete (yyvsp[-1].value);
  }

    break;



        default: break;
      }
    if (yychar_backup != yychar)
      YY_LAC_DISCARD ("yychar change");
  }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, symtable, errors, tests, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyesa, &yyes, &yyes_capacity, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        if (yychar != YYEMPTY)
          YY_LAC_ESTABLISH;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, symtable, errors, tests, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc, symtable, errors, tests);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, symtable, errors, tests);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  /* If the stack popping above didn't lose the initial context for the
     current lookahead token, the shift below will for sure.  */
  YY_LAC_DISCARD ("error recovery");

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, symtable, errors, tests, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, symtable, errors, tests);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, symtable, errors, tests);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yyes != yyesa)
    YYSTACK_FREE (yyes);
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}


