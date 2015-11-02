/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 3 "jni.bison.y" /* yacc.c:339  */

#include <set>
#include "parser.hpp"
#include "ast/ast.hpp"

#line 72 "jni.bison.tab.cpp" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "jni.bison.tab.hpp".  */
#ifndef YY_YY_JNI_BISON_TAB_HPP_INCLUDED
# define YY_YY_JNI_BISON_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 9 "jni.bison.y" /* yacc.c:355  */

#include "ast/ast__forward.hpp"

#line 106 "jni.bison.tab.cpp" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_EOF = 0,
    T_PACKAGE = 258,
    T_IMPORT = 259,
    T_BOOLEAN = 260,
    T_BYTE = 261,
    T_FLOAT = 262,
    T_INT = 263,
    T_LONG = 264,
    T_VOID = 265,
    T_PRIVATE = 266,
    T_PROTECTED = 267,
    T_PUBLIC = 268,
    T_ABSTRACT = 269,
    T_FINAL = 270,
    T_STATIC = 271,
    T_CLASS = 272,
    T_INTEGER_LITERAL = 273,
    T_ID = 274
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 37 "jni.bison.y" /* yacc.c:355  */

    int                     num_int;

    bfc::string             *string;
    class int_set           *int_set;

    ast::Annotation           *annotation;
    ast::Annotations          *annotations;
    ast::Argument             *argument;
    ast::ArgumentList         *argument_list;
    ast::Class                *clazz;
    ast::CompilationUnit      *compilation_unit;
    ast::Identifier           *identifier;
    ast::ImportList           *import_list;
    ast::Field                *field;
    ast::MemberList           *member_list;
    ast::Method               *method;
    ast::ModifierSet          *modifier_set;
    ast::Identifier           *qualified_identifier;
    ast::Type                 *type;

#line 161 "jni.bison.tab.cpp" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_JNI_BISON_TAB_HPP_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 178 "jni.bison.tab.cpp" /* yacc.c:358  */

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


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   87

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  47
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  76

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   274

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
      21,    22,     2,     2,    23,     2,    27,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    26,
       2,     2,     2,     2,    20,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    28,     2,    29,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    24,     2,    25,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   114,   114,   119,   127,   131,   141,   150,   154,   160,
     170,   173,   190,   202,   218,   235,   247,   257,   270,   274,
     281,   288,   298,   299,   300,   301,   302,   303,   307,   311,
     321,   325,   329,   332,   339,   343,   347,   357,   362,   365,
     368,   371,   374,   377,   383,   388,   395,   396
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "T_PACKAGE", "T_IMPORT",
  "T_BOOLEAN", "T_BYTE", "T_FLOAT", "T_INT", "T_LONG", "T_VOID",
  "T_PRIVATE", "T_PROTECTED", "T_PUBLIC", "T_ABSTRACT", "T_FINAL",
  "T_STATIC", "T_CLASS", "T_INTEGER_LITERAL", "T_ID", "'@'", "'('", "')'",
  "','", "'{'", "'}'", "';'", "'.'", "'['", "']'", "$accept", "annotation",
  "annotations", "argument", "argument_list", "class_identifier",
  "class_head", "class", "compilation_unit_head", "compilation_unit",
  "field", "method", "member_list", "modifier", "modifier_set", "package",
  "identifier", "qualified_identifier", "import", "import_list",
  "basic_type", "reference_type", "type", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
      64,    40,    41,    44,   123,   125,    59,    46,    91,    93
};
# endif

#define YYPACT_NINF -12

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-12)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      11,     7,   -12,    18,   -12,   -12,   -12,   -10,    25,   -12,
      38,   -12,    35,   -12,     7,    27,   -12,    16,    22,   -12,
       7,   -12,   -12,    28,   -12,   -12,   -12,   -12,   -12,   -12,
       7,   -12,   -12,    25,   -12,   -12,   -12,    10,    30,    26,
      24,    29,    -4,   -12,    31,   -12,    23,   -12,   -12,   -12,
     -12,   -12,   -12,    33,    32,    29,    36,   -12,   -12,    15,
      34,    14,   -12,    19,    37,   -12,    15,   -12,    39,    15,
     -12,    21,   -12,   -12,    40,   -12
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     4,     0,    35,    31,    32,     0,    28,    18,
       0,     1,    13,    30,     0,     0,     5,     0,     4,    14,
       0,    36,    33,     2,    24,    25,    26,    22,    23,    27,
       0,    29,    12,    28,    19,    20,    21,     0,     0,     0,
      45,    10,     0,    34,     0,    11,     0,    38,    39,    40,
      41,    42,    43,    31,    46,    47,     0,     3,    44,     7,
       0,     0,     8,     0,     0,    37,     7,    15,     0,     0,
       6,     0,    17,     9,     0,    16
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -12,   -12,    41,   -11,    -5,   -12,   -12,    44,   -12,   -12,
     -12,   -12,   -12,   -12,    42,   -12,    50,    -1,   -12,   -12,
     -12,    43,    45
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    16,     8,    62,    63,    39,     9,    10,     2,     3,
      35,    36,    18,    31,    17,     4,     6,    40,    21,    12,
      54,    55,    64
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
       7,    47,    48,    49,    50,    51,    52,    24,    25,    26,
      27,    28,    29,    30,     1,    53,    13,    14,    11,    37,
      47,    48,    49,    50,    51,    52,     5,    24,    25,    26,
      27,    28,    29,    30,     5,    66,    43,    14,    19,    20,
      67,    68,    69,    74,    69,    15,    23,    32,    44,    38,
      45,    14,    58,    57,    59,    61,    70,    46,    73,    33,
      60,    71,    34,    65,    22,    72,    75,     0,     0,     0,
       0,     0,     0,    41,     0,    42,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56
};

static const yytype_int8 yycheck[] =
{
       1,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     3,    19,    26,    27,     0,    20,
       5,     6,     7,     8,     9,    10,    19,    11,    12,    13,
      14,    15,    16,    17,    19,    21,    26,    27,     0,     4,
      26,    22,    23,    22,    23,    20,    19,    25,    18,    21,
      24,    27,    29,    22,    21,    19,    19,    28,    69,    18,
      28,    66,    18,    29,    14,    26,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    38,    39,    45,    19,    46,    47,    32,    36,
      37,     0,    49,    26,    27,    20,    31,    44,    42,     0,
       4,    48,    46,    19,    11,    12,    13,    14,    15,    16,
      17,    43,    25,    32,    37,    40,    41,    47,    21,    35,
      47,    51,    44,    26,    18,    24,    28,     5,     6,     7,
       8,     9,    10,    19,    50,    51,    52,    22,    29,    21,
      28,    19,    33,    34,    52,    29,    21,    26,    22,    23,
      19,    34,    26,    33,    22,    26
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    31,    32,    32,    33,    34,    34,    34,
      35,    36,    37,    38,    39,    40,    41,    41,    42,    42,
      42,    42,    43,    43,    43,    43,    43,    43,    44,    44,
      45,    46,    47,    47,    48,    49,    49,    50,    50,    50,
      50,    50,    50,    50,    51,    51,    52,    52
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     5,     0,     2,     2,     0,     1,     3,
       1,     5,     3,     2,     3,     5,     8,     7,     0,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     0,     2,
       3,     1,     1,     3,     3,     0,     2,     3,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1
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
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
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
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
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
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 18: /* T_INTEGER_LITERAL  */
#line 84 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).string)); }
#line 1055 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 19: /* T_ID  */
#line 83 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).string)); }
#line 1061 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 31: /* annotation  */
#line 87 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).annotation)); }
#line 1067 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 32: /* annotations  */
#line 88 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).annotations)); }
#line 1073 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 33: /* argument  */
#line 89 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).argument)); }
#line 1079 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 34: /* argument_list  */
#line 90 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).argument_list)); }
#line 1085 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 35: /* class_identifier  */
#line 94 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).identifier)); }
#line 1091 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 36: /* class_head  */
#line 93 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).clazz)); }
#line 1097 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 37: /* class  */
#line 92 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).clazz)); }
#line 1103 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 39: /* compilation_unit  */
#line 95 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).compilation_unit)); }
#line 1109 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 40: /* field  */
#line 99 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).field)); }
#line 1115 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 41: /* method  */
#line 101 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).method)); }
#line 1121 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 42: /* member_list  */
#line 100 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).member_list)); }
#line 1127 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 43: /* modifier  */
#line 102 "jni.bison.y" /* yacc.c:1257  */
      { ((*yyvaluep).num_int) = 0;          }
#line 1133 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 44: /* modifier_set  */
#line 103 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).modifier_set)); }
#line 1139 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 45: /* package  */
#line 104 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).qualified_identifier)); }
#line 1145 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 46: /* identifier  */
#line 96 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).identifier)); }
#line 1151 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 47: /* qualified_identifier  */
#line 105 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).qualified_identifier)); }
#line 1157 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 48: /* import  */
#line 97 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).qualified_identifier)); }
#line 1163 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 49: /* import_list  */
#line 98 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).import_list)); }
#line 1169 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 50: /* basic_type  */
#line 91 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).type)); }
#line 1175 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 51: /* reference_type  */
#line 106 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).type)); }
#line 1181 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;

    case 52: /* type  */
#line 107 "jni.bison.y" /* yacc.c:1257  */
      { BFC_RELEASE(((*yyvaluep).type)); }
#line 1187 "jni.bison.tab.cpp" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

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
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

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
      yychar = yylex ();
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
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
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

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 115 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("annotation: %s\n", (yyvsp[0].string)->c_str());
          (yyval.annotation) = ast::Annotation::make_ptr((yyvsp[0].string)).detach();
      }
#line 1454 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 3:
#line 120 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("annotation: %s(%s)\n", (yyvsp[-3].string)->c_str(), (yyvsp[-1].string)->c_str());
          (yyval.annotation) = ast::Annotation::make_ptr((yyvsp[-3].string), (yyvsp[-1].string)).detach();
      }
#line 1463 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 4:
#line 127 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("annotations: NULL\n");
          (yyval.annotations) = ast::Annotations::make_ptr().detach();
      }
#line 1472 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 5:
#line 133 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("annotations: ++\n");
          (yyval.annotations) = (yyvsp[-1].annotations);
          (yyval.annotations)->insert((yyvsp[0].annotation));
      }
#line 1482 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 6:
#line 142 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("argument: %s %s\n", (yyvsp[-1].type)->get_name()->c_str(), (yyvsp[0].string)->c_str());
          (yyval.argument) = ast::Argument::make_ptr((yyvsp[0].string)).detach();
          (yyval.argument)->set_type((yyvsp[-1].type));
      }
#line 1492 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 7:
#line 150 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("argument_list: NULL\n");
          (yyval.argument_list) = ast::ArgumentList::make_ptr().detach();
      }
#line 1501 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 8:
#line 155 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("argument_list: << %s %s\n", (yyvsp[0].argument)->get_type()->get_name()->c_str(), (yyvsp[0].argument)->get_name()->c_str());
          (yyval.argument_list) = ast::ArgumentList::make_ptr().detach();
          (yyval.argument_list)->push_back((yyvsp[0].argument));
      }
#line 1511 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 9:
#line 162 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("argument_list: << , %s %s\n", (yyvsp[0].argument)->get_type()->get_name()->c_str(), (yyvsp[0].argument)->get_name()->c_str());
          (yyval.argument_list) = (yyvsp[-2].argument_list);
          (yyval.argument_list)->push_back((yyvsp[0].argument));
      }
#line 1521 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 10:
#line 170 "jni.bison.y" /* yacc.c:1661  */
    {(yyval.identifier) = ast::ClassIdentifier::make_ptr((yyvsp[0].type)).detach();}
#line 1527 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 11:
#line 176 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("class\n");

          (yyvsp[-1].identifier)->set_prefix(ast::Context::instance()->get_local_namespace()->get_identifier());
          ast::Context::instance()->get_local_namespace()->add_class_identifier((yyvsp[-1].identifier));

          (yyval.clazz) = ast::Class::make_ptr((yyvsp[-1].identifier)).detach();
          (yyval.clazz)->set_annotations((yyvsp[-4].annotations));
          (yyval.clazz)->set_modifier_set((yyvsp[-3].modifier_set));
          (yyval.clazz)->get_local_namespace()->set_identifier((yyvsp[-1].identifier));
          ast::Context::instance()->push_local_namespace((yyval.clazz)->get_local_namespace());
      }
#line 1544 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 12:
#line 192 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("class_body\n");
          (yyval.clazz) = (yyvsp[-2].clazz);
          (yyval.clazz)->set_member_list((yyvsp[-1].member_list));

          ast::Context::instance()->pop_local_namespace();
      }
#line 1556 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 13:
#line 203 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("compilation_unit_head:\n");
          (yyval.compilation_unit) = ast::CompilationUnit::make_ptr().detach();
          (yyval.compilation_unit)->set_package((yyvsp[-1].qualified_identifier));
          (yyval.compilation_unit)->set_import_list((yyvsp[0].import_list));

          (yyval.compilation_unit)->get_local_namespace()->set_identifier((yyvsp[-1].qualified_identifier));
          (yyval.compilation_unit)->get_local_namespace()->add_class_identifiers((yyvsp[0].import_list)->begin(), (yyvsp[0].import_list)->end());
          (yyval.compilation_unit)->get_local_namespace()->add_package_identifier((yyvsp[-1].qualified_identifier));

          ast::Context::instance()->push_local_namespace((yyval.compilation_unit)->get_local_namespace());
      }
#line 1573 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 14:
#line 220 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("compilation_unit_body:\n");
          (yyval.compilation_unit) = (yyvsp[-2].compilation_unit);
          (yyval.compilation_unit)->set_clazz((yyvsp[-1].clazz));

          ast::Context::instance()->pop_local_namespace();

          printf("----------\n");
          (yyval.compilation_unit)->debug_print(0);
          printf("----------\n");
          (yyval.compilation_unit)->build();
      }
#line 1590 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 15:
#line 237 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("method:\n");
          (yyval.field) = ast::Field::make_ptr((yyvsp[-1].string)).detach();
          (yyval.field)->set_annotations((yyvsp[-4].annotations));
          (yyval.field)->set_modifier_set((yyvsp[-3].modifier_set));
          (yyval.field)->set_type((yyvsp[-2].type));
      }
#line 1602 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 16:
#line 249 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("method:\n");
          (yyval.method) = ast::Method::make_ptr((yyvsp[-4].string)).detach();
          (yyval.method)->set_annotations((yyvsp[-7].annotations));
          (yyval.method)->set_modifier_set((yyvsp[-6].modifier_set));
          (yyval.method)->set_type((yyvsp[-5].type));
          (yyval.method)->set_argument_list((yyvsp[-2].argument_list));
      }
#line 1615 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 17:
#line 259 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("method:\n");
          (yyval.method) = ast::Constructor::make_ptr((yyvsp[-4].string)).detach();
          (yyval.method)->set_annotations((yyvsp[-6].annotations));
          (yyval.method)->set_modifier_set((yyvsp[-5].modifier_set));
          (yyval.method)->set_type(ast::ReferenceType::make_ptr((yyval.method)->get_name()).get());
          (yyval.method)->set_argument_list((yyvsp[-2].argument_list));
      }
#line 1628 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 18:
#line 270 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("member_list: NULL\n");
          (yyval.member_list) = ast::MemberList::make_ptr().detach();
      }
#line 1637 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 19:
#line 276 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("member_list: + class: %s\n", (yyvsp[0].clazz)->get_name()->c_str());
          (yyval.member_list) = (yyvsp[-1].member_list);
          (yyval.member_list)->push_back((yyvsp[0].clazz));
      }
#line 1647 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 20:
#line 283 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("member_list: + field: %s\n", (yyvsp[0].field)->get_name()->c_str());
          (yyval.member_list) = (yyvsp[-1].member_list);
          (yyval.member_list)->push_back((yyvsp[0].field));
      }
#line 1657 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 21:
#line 290 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("member_list: + method: %s\n", (yyvsp[0].method)->get_name()->c_str());
          (yyval.member_list) = (yyvsp[-1].member_list);
          (yyval.member_list)->push_back((yyvsp[0].method));
      }
#line 1667 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 22:
#line 298 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_ABSTRACT\n");  (yyval.num_int) = T_ABSTRACT;}
#line 1673 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 23:
#line 299 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_FINAL\n");     (yyval.num_int) = T_FINAL;}
#line 1679 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 24:
#line 300 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_PRIVATE\n");   (yyval.num_int) = T_PRIVATE;}
#line 1685 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 25:
#line 301 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_PROTECTED\n"); (yyval.num_int) = T_PROTECTED;}
#line 1691 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 26:
#line 302 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_PUBLIC\n");    (yyval.num_int) = T_PUBLIC;}
#line 1697 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 27:
#line 303 "jni.bison.y" /* yacc.c:1661  */
    {BISON_LOGF("modifier: T_STATIC\n");    (yyval.num_int) = T_STATIC;}
#line 1703 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 28:
#line 307 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("modifier_set: NULL\n");
          (yyval.modifier_set) = ast::ModifierSet::make_ptr().detach();
      }
#line 1712 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 29:
#line 313 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("modifier_set: ++\n");
          (yyval.modifier_set) = (yyvsp[-1].modifier_set);
          (yyval.modifier_set)->insert_token((yyvsp[0].num_int));
      }
#line 1722 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 30:
#line 321 "jni.bison.y" /* yacc.c:1661  */
    {(yyval.qualified_identifier) = (yyvsp[-1].qualified_identifier);}
#line 1728 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 31:
#line 325 "jni.bison.y" /* yacc.c:1661  */
    {(yyval.identifier) = ast::Identifier::make_ptr((yyvsp[0].string)).detach();}
#line 1734 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 32:
#line 329 "jni.bison.y" /* yacc.c:1661  */
    {
          (yyval.qualified_identifier) = (yyvsp[0].identifier);
      }
#line 1742 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 33:
#line 332 "jni.bison.y" /* yacc.c:1661  */
    {
          (yyval.qualified_identifier) = (yyvsp[0].identifier);
          (yyval.qualified_identifier)->set_prefix((yyvsp[-2].qualified_identifier));
      }
#line 1751 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 34:
#line 339 "jni.bison.y" /* yacc.c:1661  */
    {(yyval.qualified_identifier) = (yyvsp[-1].qualified_identifier);}
#line 1757 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 35:
#line 343 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("import_list: NULL\n");
          (yyval.import_list) = ast::ImportList::make_ptr().detach();
      }
#line 1766 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 36:
#line 349 "jni.bison.y" /* yacc.c:1661  */
    {
          BISON_LOGF("import_list: ++\n");
          (yyval.import_list) = (yyvsp[-1].import_list);
          (yyval.import_list)->push_back((yyvsp[0].qualified_identifier));
      }
#line 1776 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 37:
#line 357 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = (yyvsp[-2].type);
        (yyval.type)->set_is_array(true);
        BISON_LOGF("basic_type: %s[]\n", (yyval.type)->get_name()->c_str());
      }
#line 1786 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 38:
#line 362 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::BooleanType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1794 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 39:
#line 365 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::ByteType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1802 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 40:
#line 368 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::FloatType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1810 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 41:
#line 371 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::IntType::make_ptr().detach();
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1818 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 42:
#line 374 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::LongType::make_ptr().detach();   
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1826 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 43:
#line 377 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::VoidType::make_ptr().detach();   
        BISON_LOGF("basic_type: %s\n", (yyval.type)->get_name()->c_str()); }
#line 1834 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 44:
#line 383 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = (yyvsp[-2].type);
        (yyval.type)->set_is_array(true);
        BISON_LOGF("reference_type: %s[]\n", (yyval.type)->get_name()->c_str());
      }
#line 1844 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 45:
#line 388 "jni.bison.y" /* yacc.c:1661  */
    {
        (yyval.type) = ast::ReferenceType::make_ptr((yyvsp[0].qualified_identifier)).detach();
        BISON_LOGF("reference_type: %s\n", (yyval.type)->get_name()->c_str());
    }
#line 1853 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 46:
#line 395 "jni.bison.y" /* yacc.c:1661  */
    { (yyval.type) = (yyvsp[0].type); }
#line 1859 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;

  case 47:
#line 396 "jni.bison.y" /* yacc.c:1661  */
    { (yyval.type) = (yyvsp[0].type); }
#line 1865 "jni.bison.tab.cpp" /* yacc.c:1661  */
    break;


#line 1869 "jni.bison.tab.cpp" /* yacc.c:1661  */
      default: break;
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
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
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
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



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
                      yytoken, &yylval);
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
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
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 398 "jni.bison.y" /* yacc.c:1906  */


int bison_main(int argc, char **argv)
{
    if ( argc <= 1 ) {
        printf(
            " usage:\n"
            "   jjk <input_file>\n");
        return -1;
    }

    yyin = fopen( argv[1], "r" );
    return yyparse();
}
