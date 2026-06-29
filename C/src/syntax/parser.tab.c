/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 20 "C/src/syntax/parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

extern int yylex(void);
extern int yylineno;
void yyerror(const char *s);

Node *root = NULL;   /* 语法树根（Program）。分析成功后由 main 打印。 */

/* 标记：刚归约的 StructSpecifier 是否匿名（OptTag 为空）。供 ExtDef 判断
   "struct {...}; 这种匿名结构体声明"（Tests1 B-1）。每次归约 StructSpecifier 前清零。 */
int last_struct_anonymous = 0;

/* 当前产生式是否已经报过 type B 错误。某些错误产生式报错后置 has_error。
   详见各错误产生式注释。 */

/* 新建终结符节点的简写宏：name=Token 名，pos=该终结符的 $n 位置（取行号），kind/ival/fval/sval 见 node.h */
#define TKS(name, t, kind) newTokenNode(name, (t).line, kind, (t).ival, (t).fval, (t).sval)
/* 非终结符节点简写：name + 0 行号（addChild 会从首个子节点修正） */
#define NT(name) newNode(name)

#line 96 "C/src/syntax/parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_FLOAT = 4,                      /* FLOAT  */
  YYSYMBOL_ID = 5,                         /* ID  */
  YYSYMBOL_TYPE = 6,                       /* TYPE  */
  YYSYMBOL_SEMI = 7,                       /* SEMI  */
  YYSYMBOL_COMMA = 8,                      /* COMMA  */
  YYSYMBOL_ASSIGNOP = 9,                   /* ASSIGNOP  */
  YYSYMBOL_RELOP = 10,                     /* RELOP  */
  YYSYMBOL_PLUS = 11,                      /* PLUS  */
  YYSYMBOL_MINUS = 12,                     /* MINUS  */
  YYSYMBOL_STAR = 13,                      /* STAR  */
  YYSYMBOL_DIV = 14,                       /* DIV  */
  YYSYMBOL_AND = 15,                       /* AND  */
  YYSYMBOL_OR = 16,                        /* OR  */
  YYSYMBOL_DOT = 17,                       /* DOT  */
  YYSYMBOL_NOT = 18,                       /* NOT  */
  YYSYMBOL_STRUCT = 19,                    /* STRUCT  */
  YYSYMBOL_RETURN = 20,                    /* RETURN  */
  YYSYMBOL_IF = 21,                        /* IF  */
  YYSYMBOL_ELSE = 22,                      /* ELSE  */
  YYSYMBOL_WHILE = 23,                     /* WHILE  */
  YYSYMBOL_LP = 24,                        /* LP  */
  YYSYMBOL_RP = 25,                        /* RP  */
  YYSYMBOL_LB = 26,                        /* LB  */
  YYSYMBOL_RB = 27,                        /* RB  */
  YYSYMBOL_LC = 28,                        /* LC  */
  YYSYMBOL_RC = 29,                        /* RC  */
  YYSYMBOL_COMPOUND_ASSIGN = 30,           /* COMPOUND_ASSIGN  */
  YYSYMBOL_UMINUS = 31,                    /* UMINUS  */
  YYSYMBOL_CONFLICT_DEF_STMT = 32,         /* CONFLICT_DEF_STMT  */
  YYSYMBOL_YYACCEPT = 33,                  /* $accept  */
  YYSYMBOL_Program = 34,                   /* Program  */
  YYSYMBOL_ExtDefList = 35,                /* ExtDefList  */
  YYSYMBOL_ExtDef = 36,                    /* ExtDef  */
  YYSYMBOL_ExtDecList = 37,                /* ExtDecList  */
  YYSYMBOL_Specifier = 38,                 /* Specifier  */
  YYSYMBOL_StructSpecifier = 39,           /* StructSpecifier  */
  YYSYMBOL_OptTag = 40,                    /* OptTag  */
  YYSYMBOL_Tag = 41,                       /* Tag  */
  YYSYMBOL_VarDec = 42,                    /* VarDec  */
  YYSYMBOL_FunDec = 43,                    /* FunDec  */
  YYSYMBOL_VarList = 44,                   /* VarList  */
  YYSYMBOL_ParamDec = 45,                  /* ParamDec  */
  YYSYMBOL_CompSt = 46,                    /* CompSt  */
  YYSYMBOL_StmtList = 47,                  /* StmtList  */
  YYSYMBOL_Stmt = 48,                      /* Stmt  */
  YYSYMBOL_DefList = 49,                   /* DefList  */
  YYSYMBOL_Def = 50,                       /* Def  */
  YYSYMBOL_DecList = 51,                   /* DecList  */
  YYSYMBOL_Dec = 52,                       /* Dec  */
  YYSYMBOL_Exp = 53,                       /* Exp  */
  YYSYMBOL_Args = 54                       /* Args  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   447

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  76
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  143

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   287


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      25,    26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   104,   104,   107,   108,   111,   113,   122,   124,   126,
     129,   130,   137,   139,   142,   152,   159,   161,   164,   170,
     172,   177,   180,   184,   189,   196,   198,   202,   205,   208,
     213,   219,   220,   223,   225,   226,   230,   236,   244,   251,
     254,   257,   259,   264,   265,   268,   271,   280,   283,   284,
     289,   290,   317,   319,   325,   328,   331,   334,   337,   340,
     343,   346,   349,   353,   356,   359,   364,   369,   371,   375,
     379,   382,   385,   387,   389,   393,   396
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT", "FLOAT", "ID",
  "TYPE", "SEMI", "COMMA", "ASSIGNOP", "RELOP", "PLUS", "MINUS", "STAR",
  "DIV", "AND", "OR", "DOT", "NOT", "STRUCT", "RETURN", "IF", "ELSE",
  "WHILE", "LP", "RP", "LB", "RB", "LC", "RC", "COMPOUND_ASSIGN", "UMINUS",
  "CONFLICT_DEF_STMT", "$accept", "Program", "ExtDefList", "ExtDef",
  "ExtDecList", "Specifier", "StructSpecifier", "OptTag", "Tag", "VarDec",
  "FunDec", "VarList", "ParamDec", "CompSt", "StmtList", "Stmt", "DefList",
  "Def", "DecList", "Dec", "Exp", "Args", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-58)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-45)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      54,    -4,   -58,     6,    30,   -58,    54,    97,   -58,   -58,
       8,    22,   -58,   -58,   -58,    47,   -58,   137,    31,    46,
      34,    53,   -58,   406,    75,    11,     4,   -58,    82,   140,
      66,     4,   -58,    75,    69,    93,   -58,   -58,   102,   406,
     406,   406,   176,   -58,   -58,    86,    90,    64,   -58,   -58,
      77,   122,   133,   -58,   -58,   139,   105,   -58,    56,    94,
     -13,   -13,   298,   -58,   406,   406,   406,   406,   406,   406,
     406,   406,   162,   406,   406,   142,   -58,   166,   401,   153,
     155,    75,   -58,   165,    64,   198,   156,   -58,    75,   -58,
     -58,   -58,   174,   -58,   243,   170,   -58,   386,   126,   149,
     149,   -13,   -13,   277,   421,   -58,   320,   386,   -58,   -58,
     -58,   220,   406,   406,   189,   -58,   -58,   -58,   152,   386,
     -58,    60,   281,   -58,   -58,   -58,   342,   364,   -58,   -58,
     266,   168,   -58,   -58,   130,   104,   406,   -58,   177,   -58,
     -58,   130,   -58
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,    12,    17,     0,     2,     0,     0,    13,     9,
      18,     0,    15,     1,     3,    19,     6,     0,    10,     0,
       0,     0,     5,     0,     0,     0,     0,     7,     0,     0,
       0,     0,    24,     0,     0,    28,    73,    74,    72,     0,
       0,     0,     0,    19,    11,     0,     0,     0,    47,    46,
      50,     0,    48,    14,    43,    29,     0,    23,     0,     0,
      63,    64,     0,     8,     0,     0,     0,     0,     0,     0,
       0,     0,    70,     0,     0,    20,    21,     0,     0,     0,
       0,     0,    34,     0,     0,     0,     0,    45,     0,    26,
      25,    27,    28,    66,    76,     0,    62,    54,    57,    58,
      59,    60,    61,    55,    56,    69,     0,    71,    22,    42,
      40,     0,     0,     0,     0,    30,    31,    33,     0,    51,
      49,     0,     0,    65,    68,    35,     0,     0,    41,    53,
      76,     0,    67,    75,     0,     0,     0,    52,    36,    39,
      38,     0,    37
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -58,   -58,   192,   -58,   179,     0,   -58,   -58,   -58,     5,
     -58,   180,   183,   181,   132,    37,    89,   -58,   -47,   -58,
     -22,   -57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     4,     5,     6,    17,    81,     8,    11,    12,    50,
      19,    91,    92,    82,    83,    84,    30,    31,    51,    52,
      85,   133
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,    42,    95,     9,    72,    28,     7,   -44,   -44,   -44,
       2,    10,    18,    73,    45,    46,   -44,    60,    61,    62,
      29,    33,   -44,     3,   -44,   -44,    29,   -44,   -44,    18,
      13,    29,   -44,   -44,   114,    28,   -16,    94,    55,    24,
       2,   120,    97,    98,    99,   100,   101,   102,   103,   104,
      20,   106,   107,     3,    -4,     1,   111,    25,    33,     2,
       2,   131,     2,   -44,   119,    77,     2,    36,    37,    38,
       2,    21,     3,     3,    26,     3,    39,    56,    32,     3,
      43,    90,    40,     3,    78,    79,    86,    80,    41,    48,
     126,   127,    26,   -32,    57,    53,   130,    36,    37,    38,
     130,    58,    15,    25,    16,    77,    39,    36,    37,    38,
       2,   139,    40,    75,   130,    47,    39,    76,    41,    93,
      54,    33,    40,     3,    78,    79,    59,    80,    41,    87,
      89,    77,    26,    36,    37,    38,     2,    66,    67,    68,
      69,    88,    39,    72,    22,    43,    23,    49,    40,     3,
      78,    79,    73,    80,    41,    36,    37,    38,    26,    36,
      37,    38,    68,    69,    39,    25,    72,   105,    39,   108,
      40,   138,   140,   109,    40,    73,    41,   112,   142,   113,
      41,   129,   121,    63,   118,    64,    65,    66,    67,    68,
      69,    70,    71,    72,   115,   123,   128,   137,    14,   141,
      27,    34,    73,    44,    35,   117,    74,    64,    65,    66,
      67,    68,    69,    70,    71,    72,   116,     0,     0,     0,
       0,     0,     0,     0,    73,     0,     0,   125,    74,    64,
      65,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,    73,     0,     0,     0,
      74,   122,    64,    65,    66,    67,    68,    69,    70,    71,
      72,     0,     0,     0,     0,     0,     0,     0,     0,    73,
       0,     0,     0,    74,   136,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    36,    37,    38,    65,    66,    67,
      68,    69,    73,    39,    72,     0,    74,     0,     0,    40,
       0,     0,     0,    73,     0,    41,   132,    64,    65,    66,
      67,    68,    69,    70,    71,    72,     0,     0,     0,     0,
       0,     0,     0,    96,    73,     0,     0,     0,    74,    64,
      65,    66,    67,    68,    69,    70,    71,    72,     0,     0,
       0,     0,     0,     0,     0,     0,    73,   124,     0,     0,
      74,    64,    65,    66,    67,    68,    69,    70,    71,    72,
       0,     0,     0,     0,     0,     0,     0,   134,    73,     0,
       0,     0,    74,    64,    65,    66,    67,    68,    69,    70,
      71,    72,     0,     0,     0,     0,     0,     0,     0,   135,
      73,     0,     0,     0,    74,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    36,    37,    38,     0,   110,    36,
      37,    38,    73,    39,     0,     0,    74,     0,    39,    40,
       0,     0,     0,     0,    40,    41,     0,     0,     0,     0,
      41,    65,    66,    67,    68,    69,    70,     0,    72,     0,
       0,     0,     0,     0,     0,     0,     0,    73
};

static const yytype_int16 yycheck[] =
{
       0,    23,    59,     7,    17,     1,     6,     3,     4,     5,
       6,     5,     7,    26,     3,     4,    12,    39,    40,    41,
      20,    21,    18,    19,    20,    21,    26,    23,    24,    24,
       0,    31,    28,    29,    81,     1,    28,    59,    33,     8,
       6,    88,    64,    65,    66,    67,    68,    69,    70,    71,
      28,    73,    74,    19,     0,     1,    78,    26,    58,     6,
       6,   118,     6,    29,    86,     1,     6,     3,     4,     5,
       6,    24,    19,    19,    28,    19,    12,     8,    25,    19,
       5,    25,    18,    19,    20,    21,     9,    23,    24,     7,
     112,   113,    28,    29,    25,    29,   118,     3,     4,     5,
     122,     8,     5,    26,     7,     1,    12,     3,     4,     5,
       6,     7,    18,    27,   136,    26,    12,    27,    24,    25,
      31,   121,    18,    19,    20,    21,    24,    23,    24,     7,
      25,     1,    28,     3,     4,     5,     6,    11,    12,    13,
      14,     8,    12,    17,     7,     5,     9,     7,    18,    19,
      20,    21,    26,    23,    24,     3,     4,     5,    28,     3,
       4,     5,    13,    14,    12,    26,    17,     5,    12,    27,
      18,   134,   135,     7,    18,    26,    24,    24,   141,    24,
      24,    29,     8,     7,    28,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    29,    25,     7,    29,     6,    22,
      19,    21,    26,    24,    21,     7,    30,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    84,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,     7,    30,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,
      30,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    30,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     3,     4,     5,    10,    11,    12,
      13,    14,    26,    12,    17,    -1,    30,    -1,    -1,    18,
      -1,    -1,    -1,    26,    -1,    24,    25,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    -1,    30,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,    -1,
      30,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    -1,    30,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    -1,    30,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     3,     4,     5,    -1,     7,     3,
       4,     5,    26,    12,    -1,    -1,    30,    -1,    12,    18,
      -1,    -1,    -1,    -1,    18,    24,    -1,    -1,    -1,    -1,
      24,    10,    11,    12,    13,    14,    15,    -1,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     6,    19,    34,    35,    36,    38,    39,     7,
       5,    40,    41,     0,    35,     5,     7,    37,    42,    43,
      28,    24,     7,     9,     8,    26,    28,    46,     1,    38,
      49,    50,    25,    38,    44,    45,     3,     4,     5,    12,
      18,    24,    53,     5,    37,     3,     4,    49,     7,     7,
      42,    51,    52,    29,    49,    42,     8,    25,     8,    24,
      53,    53,    53,     7,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    26,    30,    27,    27,     1,    20,    21,
      23,    38,    46,    47,    48,    53,     9,     7,     8,    25,
      25,    44,    45,    25,    53,    54,    25,    53,    53,    53,
      53,    53,    53,    53,    53,     5,    53,    53,    27,     7,
       7,    53,    24,    24,    51,    29,    47,     7,    28,    53,
      51,     8,     8,    25,    27,     7,    53,    53,     7,    29,
      53,    54,    25,    54,    25,    25,     8,    29,    48,     7,
      48,    22,    48
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    33,    34,    35,    35,    36,    36,    36,    36,    36,
      37,    37,    38,    38,    39,    39,    40,    40,    41,    42,
      42,    42,    42,    43,    43,    43,    43,    44,    44,    45,
      46,    47,    47,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    49,    49,    50,    50,    50,    51,    51,
      52,    52,    52,    52,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    54,    54
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     0,     3,     2,     3,     5,     2,
       1,     3,     1,     1,     5,     2,     1,     0,     1,     1,
       4,     4,     5,     4,     3,     5,     5,     3,     1,     2,
       4,     2,     0,     2,     1,     3,     5,     7,     5,     5,
       2,     3,     2,     2,     0,     3,     2,     2,     1,     3,
       1,     3,     5,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     4,     3,     5,     4,     3,
       2,     3,     1,     1,     1,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 2: /* Program: ExtDefList  */
#line 104 "C/src/syntax/parser.y"
                                         { root = NT("Program"); addChild(root, (yyvsp[0].node)); }
#line 1287 "C/src/syntax/parser.tab.c"
    break;

  case 3: /* ExtDefList: ExtDef ExtDefList  */
#line 107 "C/src/syntax/parser.y"
                                         { Node *n = NT("ExtDefList"); addChild(n, (yyvsp[-1].node)); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1293 "C/src/syntax/parser.tab.c"
    break;

  case 4: /* ExtDefList: %empty  */
#line 108 "C/src/syntax/parser.y"
                                          { (yyval.node) = NULL; }
#line 1299 "C/src/syntax/parser.tab.c"
    break;

  case 5: /* ExtDef: Specifier ExtDecList SEMI  */
#line 111 "C/src/syntax/parser.y"
                                         { Node *n = NT("ExtDef"); addChild(n,(yyvsp[-2].node)); addChild(n,(yyvsp[-1].node));
                                           Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n, s); (yyval.node) = n; }
#line 1306 "C/src/syntax/parser.tab.c"
    break;

  case 6: /* ExtDef: Specifier SEMI  */
#line 113 "C/src/syntax/parser.y"
                                         { if (last_struct_anonymous) {
                                               /* Tests1 B-1: struct { int tag; }; 匿名结构体声明 */
                                               out("Error type B at Line %d: Anonymous struct declaration.\n", (yyvsp[-1].node)->lineno);
                                               has_error = 1; (yyval.node) = NULL;
                                             } else {
                                               Node *n = NT("ExtDef"); addChild(n, (yyvsp[-1].node));
                                               Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n, s); (yyval.node) = n;
                                             }
                                             last_struct_anonymous = 0; }
#line 1320 "C/src/syntax/parser.tab.c"
    break;

  case 7: /* ExtDef: Specifier FunDec CompSt  */
#line 122 "C/src/syntax/parser.y"
                                         { Node *n = NT("ExtDef"); addChild(n,(yyvsp[-2].node)); addChild(n,(yyvsp[-1].node)); addChild(n,(yyvsp[0].node)); (yyval.node) = n; }
#line 1326 "C/src/syntax/parser.tab.c"
    break;

  case 8: /* ExtDef: Specifier ExtDecList ASSIGNOP Exp SEMI  */
#line 124 "C/src/syntax/parser.y"
                                                     { out("Error type B at Line %d: Global variable initialization not allowed.\n", (yyvsp[-2].tok).line);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1333 "C/src/syntax/parser.tab.c"
    break;

  case 9: /* ExtDef: error SEMI  */
#line 126 "C/src/syntax/parser.y"
                                         { yyerrok; has_error = 1; (yyval.node) = NULL; }
#line 1339 "C/src/syntax/parser.tab.c"
    break;

  case 10: /* ExtDecList: VarDec  */
#line 129 "C/src/syntax/parser.y"
                                         { Node *n = NT("ExtDecList"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1345 "C/src/syntax/parser.tab.c"
    break;

  case 11: /* ExtDecList: VarDec COMMA ExtDecList  */
#line 130 "C/src/syntax/parser.y"
                                         { Node *n = NT("ExtDecList"); addChild(n,(yyvsp[-2].node));
                                           Node *c = TKS("COMMA", (yyvsp[-1].tok), NO_VAL); addChild(n,c);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1353 "C/src/syntax/parser.tab.c"
    break;

  case 12: /* Specifier: TYPE  */
#line 137 "C/src/syntax/parser.y"
                                         { Node *n = NT("Specifier");
                                           Node *t = TKS("TYPE", (yyvsp[0].tok), VAL_STR); addChild(n, t); (yyval.node) = n; }
#line 1360 "C/src/syntax/parser.tab.c"
    break;

  case 13: /* Specifier: StructSpecifier  */
#line 139 "C/src/syntax/parser.y"
                                         { Node *n = NT("Specifier"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1366 "C/src/syntax/parser.tab.c"
    break;

  case 14: /* StructSpecifier: STRUCT OptTag LC DefList RC  */
#line 143 "C/src/syntax/parser.y"
                                           { Node *n = NT("StructSpecifier");
                                             Node *s = TKS("STRUCT", (yyvsp[-4].tok), NO_VAL); addChild(n,s);
                                             addChild(n, (yyvsp[-3].node));
                                             Node *lc = TKS("LC", (yyvsp[-2].tok), NO_VAL); addChild(n,lc);
                                             addChild(n, (yyvsp[-1].node));
                                             Node *rc = TKS("RC", (yyvsp[0].tok), NO_VAL); addChild(n,rc);
                                             /* 记录是否匿名（OptTag 为空）。ExtDef 据此报 B-1 错误。 */
                                             last_struct_anonymous = ((yyvsp[-3].node) == NULL);
                                             (yyval.node) = n; }
#line 1380 "C/src/syntax/parser.tab.c"
    break;

  case 15: /* StructSpecifier: STRUCT Tag  */
#line 152 "C/src/syntax/parser.y"
                                          { Node *n = NT("StructSpecifier");
                                           Node *s = TKS("STRUCT", (yyvsp[-1].tok), NO_VAL); addChild(n,s);
                                           addChild(n, (yyvsp[0].node));
                                           last_struct_anonymous = 0;
                                           (yyval.node) = n; }
#line 1390 "C/src/syntax/parser.tab.c"
    break;

  case 16: /* OptTag: ID  */
#line 159 "C/src/syntax/parser.y"
                                          { Node *n = NT("OptTag");
                                            Node *i = TKS("ID", (yyvsp[0].tok), VAL_STR); addChild(n, i); (yyval.node) = n; }
#line 1397 "C/src/syntax/parser.tab.c"
    break;

  case 17: /* OptTag: %empty  */
#line 161 "C/src/syntax/parser.y"
                                           { (yyval.node) = NULL; }
#line 1403 "C/src/syntax/parser.tab.c"
    break;

  case 18: /* Tag: ID  */
#line 164 "C/src/syntax/parser.y"
                                          { Node *n = NT("Tag");
                                            Node *i = TKS("ID", (yyvsp[0].tok), VAL_STR); addChild(n, i); (yyval.node) = n; }
#line 1410 "C/src/syntax/parser.tab.c"
    break;

  case 19: /* VarDec: ID  */
#line 170 "C/src/syntax/parser.y"
                                          { Node *n = NT("VarDec");
                                            Node *i = TKS("ID", (yyvsp[0].tok), VAL_STR); addChild(n, i); (yyval.node) = n; }
#line 1417 "C/src/syntax/parser.tab.c"
    break;

  case 20: /* VarDec: VarDec LB INT RB  */
#line 172 "C/src/syntax/parser.y"
                                          { Node *n = NT("VarDec"); addChild(n, (yyvsp[-3].node));
                                            Node *lb = TKS("LB", (yyvsp[-2].tok), NO_VAL); addChild(n,lb);
                                            Node *in = TKS("INT", (yyvsp[-1].tok), VAL_INT); addChild(n,in);
                                            Node *rb = TKS("RB", (yyvsp[0].tok), NO_VAL); addChild(n,rb); (yyval.node) = n; }
#line 1426 "C/src/syntax/parser.tab.c"
    break;

  case 21: /* VarDec: VarDec LB FLOAT RB  */
#line 177 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Array dimension must be integer.\n", (yyvsp[-2].tok).line);
                                            has_error = 1; (yyval.node) = NULL; }
#line 1433 "C/src/syntax/parser.tab.c"
    break;

  case 22: /* VarDec: VarDec LB INT RB RB  */
#line 180 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Extra closing bracket in array declaration.\n", (yyvsp[-1].tok).line);
                                            has_error = 1; (yyval.node) = NULL; }
#line 1440 "C/src/syntax/parser.tab.c"
    break;

  case 23: /* FunDec: ID LP VarList RP  */
#line 184 "C/src/syntax/parser.y"
                                          { Node *n = NT("FunDec");
                                            Node *i = TKS("ID", (yyvsp[-3].tok), VAL_STR); addChild(n,i);
                                            Node *lp = TKS("LP", (yyvsp[-2].tok), NO_VAL); addChild(n,lp);
                                            addChild(n, (yyvsp[-1].node));
                                            Node *rp = TKS("RP", (yyvsp[0].tok), NO_VAL); addChild(n,rp); (yyval.node) = n; }
#line 1450 "C/src/syntax/parser.tab.c"
    break;

  case 24: /* FunDec: ID LP RP  */
#line 189 "C/src/syntax/parser.y"
                                          { Node *n = NT("FunDec");
                                            Node *i = TKS("ID", (yyvsp[-2].tok), VAL_STR); addChild(n,i);
                                            Node *lp = TKS("LP", (yyvsp[-1].tok), NO_VAL); addChild(n,lp);
                                            Node *rp = TKS("RP", (yyvsp[0].tok), NO_VAL); addChild(n,rp); (yyval.node) = n; }
#line 1459 "C/src/syntax/parser.tab.c"
    break;

  case 25: /* FunDec: ID LP ParamDec COMMA RP  */
#line 196 "C/src/syntax/parser.y"
                                        { out("Error type B at Line %d: Trailing comma in function parameters.\n", (yyvsp[-3].tok).line);
                                          has_error = 1; (yyval.node) = NULL; }
#line 1466 "C/src/syntax/parser.tab.c"
    break;

  case 26: /* FunDec: ID LP VarList COMMA RP  */
#line 198 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Trailing comma in function parameters.\n", (yyvsp[-3].tok).line);
                                            has_error = 1; (yyval.node) = NULL; }
#line 1473 "C/src/syntax/parser.tab.c"
    break;

  case 27: /* VarList: ParamDec COMMA VarList  */
#line 202 "C/src/syntax/parser.y"
                                          { Node *n = NT("VarList"); addChild(n,(yyvsp[-2].node));
                                            Node *c = TKS("COMMA", (yyvsp[-1].tok), NO_VAL); addChild(n,c);
                                            addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1481 "C/src/syntax/parser.tab.c"
    break;

  case 28: /* VarList: ParamDec  */
#line 205 "C/src/syntax/parser.y"
                                          { Node *n = NT("VarList"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1487 "C/src/syntax/parser.tab.c"
    break;

  case 29: /* ParamDec: Specifier VarDec  */
#line 208 "C/src/syntax/parser.y"
                                          { Node *n = NT("ParamDec"); addChild(n,(yyvsp[-1].node)); addChild(n,(yyvsp[0].node)); (yyval.node) = n; }
#line 1493 "C/src/syntax/parser.tab.c"
    break;

  case 30: /* CompSt: LC DefList StmtList RC  */
#line 213 "C/src/syntax/parser.y"
                                         { Node *n = NT("CompSt");
                                           Node *lc = TKS("LC", (yyvsp[-3].tok), NO_VAL); addChild(n,lc);
                                           addChild(n, (yyvsp[-2].node)); addChild(n, (yyvsp[-1].node));
                                           Node *rc = TKS("RC", (yyvsp[0].tok), NO_VAL); addChild(n,rc); (yyval.node) = n; }
#line 1502 "C/src/syntax/parser.tab.c"
    break;

  case 31: /* StmtList: Stmt StmtList  */
#line 219 "C/src/syntax/parser.y"
                                         { Node *n = NT("StmtList"); addChild(n,(yyvsp[-1].node)); addChild(n,(yyvsp[0].node)); (yyval.node) = n; }
#line 1508 "C/src/syntax/parser.tab.c"
    break;

  case 32: /* StmtList: %empty  */
#line 220 "C/src/syntax/parser.y"
                                          { (yyval.node) = NULL; }
#line 1514 "C/src/syntax/parser.tab.c"
    break;

  case 33: /* Stmt: Exp SEMI  */
#line 223 "C/src/syntax/parser.y"
                                         { Node *n = NT("Stmt"); addChild(n,(yyvsp[-1].node));
                                           Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n,s); (yyval.node) = n; }
#line 1521 "C/src/syntax/parser.tab.c"
    break;

  case 34: /* Stmt: CompSt  */
#line 225 "C/src/syntax/parser.y"
                                         { Node *n = NT("Stmt"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1527 "C/src/syntax/parser.tab.c"
    break;

  case 35: /* Stmt: RETURN Exp SEMI  */
#line 226 "C/src/syntax/parser.y"
                                         { Node *n = NT("Stmt");
                                           Node *r = TKS("RETURN", (yyvsp[-2].tok), NO_VAL); addChild(n,r);
                                           addChild(n, (yyvsp[-1].node));
                                           Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n,s); (yyval.node) = n; }
#line 1536 "C/src/syntax/parser.tab.c"
    break;

  case 36: /* Stmt: IF LP Exp RP Stmt  */
#line 230 "C/src/syntax/parser.y"
                                         { Node *n = NT("Stmt");
                                           Node *i = TKS("IF", (yyvsp[-4].tok), NO_VAL); addChild(n,i);
                                           Node *lp = TKS("LP", (yyvsp[-3].tok), NO_VAL); addChild(n,lp);
                                           addChild(n, (yyvsp[-2].node));
                                           Node *rp = TKS("RP", (yyvsp[-1].tok), NO_VAL); addChild(n,rp);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1547 "C/src/syntax/parser.tab.c"
    break;

  case 37: /* Stmt: IF LP Exp RP Stmt ELSE Stmt  */
#line 236 "C/src/syntax/parser.y"
                                                     { Node *n = NT("Stmt");
                                           Node *i = TKS("IF", (yyvsp[-6].tok), NO_VAL); addChild(n,i);
                                           Node *lp = TKS("LP", (yyvsp[-5].tok), NO_VAL); addChild(n,lp);
                                           addChild(n, (yyvsp[-4].node));
                                           Node *rp = TKS("RP", (yyvsp[-3].tok), NO_VAL); addChild(n,rp);
                                           addChild(n, (yyvsp[-2].node));
                                           Node *e = TKS("ELSE", (yyvsp[-1].tok), NO_VAL); addChild(n,e);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1560 "C/src/syntax/parser.tab.c"
    break;

  case 38: /* Stmt: WHILE LP Exp RP Stmt  */
#line 244 "C/src/syntax/parser.y"
                                         { Node *n = NT("Stmt");
                                           Node *w = TKS("WHILE", (yyvsp[-4].tok), NO_VAL); addChild(n,w);
                                           Node *lp = TKS("LP", (yyvsp[-3].tok), NO_VAL); addChild(n,lp);
                                           addChild(n, (yyvsp[-2].node));
                                           Node *rp = TKS("RP", (yyvsp[-1].tok), NO_VAL); addChild(n,rp);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1571 "C/src/syntax/parser.tab.c"
    break;

  case 39: /* Stmt: WHILE LP Exp RP SEMI  */
#line 251 "C/src/syntax/parser.y"
                                         { out("Error type B at Line %d: Empty while statement body.\n", (yyvsp[-4].tok).line);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1578 "C/src/syntax/parser.tab.c"
    break;

  case 40: /* Stmt: RETURN SEMI  */
#line 254 "C/src/syntax/parser.y"
                                         { out("Error type B at Line %d: Missing return value in non-void function.\n", (yyvsp[-1].tok).line);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1585 "C/src/syntax/parser.tab.c"
    break;

  case 41: /* Stmt: Specifier DecList SEMI  */
#line 257 "C/src/syntax/parser.y"
                                         { out("Error type B at Line %d: Variable declaration after statements in block.\n", (yyvsp[-2].node)->lineno);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1592 "C/src/syntax/parser.tab.c"
    break;

  case 42: /* Stmt: error SEMI  */
#line 259 "C/src/syntax/parser.y"
                                         { yyerrok; has_error = 1; (yyval.node) = NULL; }
#line 1598 "C/src/syntax/parser.tab.c"
    break;

  case 43: /* DefList: Def DefList  */
#line 264 "C/src/syntax/parser.y"
                                         { Node *n = NT("DefList"); addChild(n,(yyvsp[-1].node)); addChild(n,(yyvsp[0].node)); (yyval.node) = n; }
#line 1604 "C/src/syntax/parser.tab.c"
    break;

  case 44: /* DefList: %empty  */
#line 265 "C/src/syntax/parser.y"
                                          { (yyval.node) = NULL; }
#line 1610 "C/src/syntax/parser.tab.c"
    break;

  case 45: /* Def: Specifier DecList SEMI  */
#line 268 "C/src/syntax/parser.y"
                                         { Node *n = NT("Def"); addChild(n,(yyvsp[-2].node)); addChild(n,(yyvsp[-1].node));
                                           Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n,s);
                                           last_struct_anonymous = 0; (yyval.node) = n; }
#line 1618 "C/src/syntax/parser.tab.c"
    break;

  case 46: /* Def: Specifier SEMI  */
#line 271 "C/src/syntax/parser.y"
                                         { if (last_struct_anonymous) {
                                               /* Tests1 B-1: 局部的 struct {...}; 匿名结构体声明 */
                                               out("Error type B at Line %d: Anonymous struct declaration.\n", (yyvsp[-1].node)->lineno);
                                               has_error = 1; (yyval.node) = NULL;
                                             } else {
                                               Node *n = NT("Def"); addChild(n,(yyvsp[-1].node));
                                               Node *s = TKS("SEMI", (yyvsp[0].tok), NO_VAL); addChild(n,s); (yyval.node) = n;
                                             }
                                             last_struct_anonymous = 0; }
#line 1632 "C/src/syntax/parser.tab.c"
    break;

  case 47: /* Def: error SEMI  */
#line 280 "C/src/syntax/parser.y"
                                         { yyerrok; has_error = 1; (yyval.node) = NULL; }
#line 1638 "C/src/syntax/parser.tab.c"
    break;

  case 48: /* DecList: Dec  */
#line 283 "C/src/syntax/parser.y"
                                         { Node *n = NT("DecList"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1644 "C/src/syntax/parser.tab.c"
    break;

  case 49: /* DecList: Dec COMMA DecList  */
#line 284 "C/src/syntax/parser.y"
                                         { Node *n = NT("DecList"); addChild(n,(yyvsp[-2].node));
                                           Node *c = TKS("COMMA", (yyvsp[-1].tok), NO_VAL); addChild(n,c);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1652 "C/src/syntax/parser.tab.c"
    break;

  case 50: /* Dec: VarDec  */
#line 289 "C/src/syntax/parser.y"
                                         { Node *n = NT("Dec"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1658 "C/src/syntax/parser.tab.c"
    break;

  case 51: /* Dec: VarDec ASSIGNOP Exp  */
#line 290 "C/src/syntax/parser.y"
                                         { /* Tests1 B-1: 数组初始化 a[4] = ... 不支持。
                                              检查 VarDec 是否含数组维度（递归找 LB 子节点）。 */
                                           int is_array = 0;
                                           Node *p = (yyvsp[-2].node);
                                           while (p && !p->is_token) {
                                               int has_lb = 0, k;
                                               for (k = 0; k < p->nchild; k++) {
                                                   if (p->children[k] && p->children[k]->is_token
                                                       && strcmp(p->children[k]->name, "LB") == 0) {
                                                       has_lb = 1; break;
                                                   }
                                               }
                                               if (has_lb) { is_array = 1; break; }
                                               /* VarDec 嵌套：第一个子节点是内层 VarDec */
                                               p = (p->nchild > 0) ? p->children[0] : NULL;
                                               if (p && p->is_token) break;
                                           }
                                           if (is_array) {
                                               out("Error type B at Line %d: Array initialization not supported.\n", (yyvsp[-2].node)->lineno);
                                               has_error = 1; (yyval.node) = NULL;
                                           } else {
                                               Node *n = NT("Dec"); addChild(n,(yyvsp[-2].node));
                                               Node *a = TKS("ASSIGNOP", (yyvsp[-1].tok), NO_VAL); addChild(n,a);
                                               addChild(n, (yyvsp[0].node)); (yyval.node) = n;
                                           } }
#line 1688 "C/src/syntax/parser.tab.c"
    break;

  case 52: /* Dec: VarDec ASSIGNOP LC Args RC  */
#line 317 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Array initialization not supported.\n", (yyvsp[-4].node)->lineno);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1695 "C/src/syntax/parser.tab.c"
    break;

  case 53: /* Dec: VarDec ASSIGNOP LC RC  */
#line 319 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Array initialization not supported.\n", (yyvsp[-3].node)->lineno);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1702 "C/src/syntax/parser.tab.c"
    break;

  case 54: /* Exp: Exp ASSIGNOP Exp  */
#line 325 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *a = TKS("ASSIGNOP", (yyvsp[-1].tok), NO_VAL); addChild(n,a);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1710 "C/src/syntax/parser.tab.c"
    break;

  case 55: /* Exp: Exp AND Exp  */
#line 328 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("AND", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1718 "C/src/syntax/parser.tab.c"
    break;

  case 56: /* Exp: Exp OR Exp  */
#line 331 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("OR", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1726 "C/src/syntax/parser.tab.c"
    break;

  case 57: /* Exp: Exp RELOP Exp  */
#line 334 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *r = TKS("RELOP", (yyvsp[-1].tok), NO_VAL); addChild(n,r);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1734 "C/src/syntax/parser.tab.c"
    break;

  case 58: /* Exp: Exp PLUS Exp  */
#line 337 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("PLUS", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1742 "C/src/syntax/parser.tab.c"
    break;

  case 59: /* Exp: Exp MINUS Exp  */
#line 340 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("MINUS", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1750 "C/src/syntax/parser.tab.c"
    break;

  case 60: /* Exp: Exp STAR Exp  */
#line 343 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("STAR", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1758 "C/src/syntax/parser.tab.c"
    break;

  case 61: /* Exp: Exp DIV Exp  */
#line 346 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *o = TKS("DIV", (yyvsp[-1].tok), NO_VAL); addChild(n,o);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1766 "C/src/syntax/parser.tab.c"
    break;

  case 62: /* Exp: LP Exp RP  */
#line 349 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *lp = TKS("LP", (yyvsp[-2].tok), NO_VAL); addChild(n,lp);
                                           addChild(n, (yyvsp[-1].node));
                                           Node *rp = TKS("RP", (yyvsp[0].tok), NO_VAL); addChild(n,rp); (yyval.node) = n; }
#line 1775 "C/src/syntax/parser.tab.c"
    break;

  case 63: /* Exp: MINUS Exp  */
#line 353 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *m = TKS("MINUS", (yyvsp[-1].tok), NO_VAL); addChild(n,m);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1783 "C/src/syntax/parser.tab.c"
    break;

  case 64: /* Exp: NOT Exp  */
#line 356 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *nt = TKS("NOT", (yyvsp[-1].tok), NO_VAL); addChild(n,nt);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1791 "C/src/syntax/parser.tab.c"
    break;

  case 65: /* Exp: ID LP Args RP  */
#line 359 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *i = TKS("ID", (yyvsp[-3].tok), VAL_STR); addChild(n,i);
                                           Node *lp = TKS("LP", (yyvsp[-2].tok), NO_VAL); addChild(n,lp);
                                           addChild(n, (yyvsp[-1].node));
                                           Node *rp = TKS("RP", (yyvsp[0].tok), NO_VAL); addChild(n,rp); (yyval.node) = n; }
#line 1801 "C/src/syntax/parser.tab.c"
    break;

  case 66: /* Exp: ID LP RP  */
#line 364 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *i = TKS("ID", (yyvsp[-2].tok), VAL_STR); addChild(n,i);
                                           Node *lp = TKS("LP", (yyvsp[-1].tok), NO_VAL); addChild(n,lp);
                                           Node *rp = TKS("RP", (yyvsp[0].tok), NO_VAL); addChild(n,rp); (yyval.node) = n; }
#line 1810 "C/src/syntax/parser.tab.c"
    break;

  case 67: /* Exp: ID LP Exp COMMA RP  */
#line 369 "C/src/syntax/parser.y"
                                         { out("Error type B at Line %d: Trailing comma in function call arguments.\n", (yyvsp[-3].tok).line);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1817 "C/src/syntax/parser.tab.c"
    break;

  case 68: /* Exp: Exp LB Exp RB  */
#line 371 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-3].node));
                                           Node *lb = TKS("LB", (yyvsp[-2].tok), NO_VAL); addChild(n,lb);
                                           addChild(n, (yyvsp[-1].node));
                                           Node *rb = TKS("RB", (yyvsp[0].tok), NO_VAL); addChild(n,rb); (yyval.node) = n; }
#line 1826 "C/src/syntax/parser.tab.c"
    break;

  case 69: /* Exp: Exp DOT ID  */
#line 375 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp"); addChild(n,(yyvsp[-2].node));
                                           Node *d = TKS("DOT", (yyvsp[-1].tok), NO_VAL); addChild(n,d);
                                           Node *i = TKS("ID", (yyvsp[0].tok), VAL_STR); addChild(n,i); (yyval.node) = n; }
#line 1834 "C/src/syntax/parser.tab.c"
    break;

  case 70: /* Exp: Exp DOT  */
#line 379 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Missing member name after '.'.\n", (yyvsp[0].tok).line);
                                           has_error = 1; (yyval.node) = NULL; }
#line 1841 "C/src/syntax/parser.tab.c"
    break;

  case 71: /* Exp: Exp COMPOUND_ASSIGN Exp  */
#line 382 "C/src/syntax/parser.y"
                                          { out("Error type B at Line %d: Unsupported compound assignment operator '%s'.\n",
                                                (yyvsp[-1].tok).line, (yyvsp[-1].tok).sval ? (yyvsp[-1].tok).sval : "+=");
                                           has_error = 1; (yyval.node) = NULL; }
#line 1849 "C/src/syntax/parser.tab.c"
    break;

  case 72: /* Exp: ID  */
#line 385 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *i = TKS("ID", (yyvsp[0].tok), VAL_STR); addChild(n,i); (yyval.node) = n; }
#line 1856 "C/src/syntax/parser.tab.c"
    break;

  case 73: /* Exp: INT  */
#line 387 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *in = TKS("INT", (yyvsp[0].tok), VAL_INT); addChild(n,in); (yyval.node) = n; }
#line 1863 "C/src/syntax/parser.tab.c"
    break;

  case 74: /* Exp: FLOAT  */
#line 389 "C/src/syntax/parser.y"
                                         { Node *n = NT("Exp");
                                           Node *f = TKS("FLOAT", (yyvsp[0].tok), VAL_FLT); addChild(n,f); (yyval.node) = n; }
#line 1870 "C/src/syntax/parser.tab.c"
    break;

  case 75: /* Args: Exp COMMA Args  */
#line 393 "C/src/syntax/parser.y"
                                         { Node *n = NT("Args"); addChild(n,(yyvsp[-2].node));
                                           Node *c = TKS("COMMA", (yyvsp[-1].tok), NO_VAL); addChild(n,c);
                                           addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1878 "C/src/syntax/parser.tab.c"
    break;

  case 76: /* Args: Exp  */
#line 396 "C/src/syntax/parser.y"
                                         { Node *n = NT("Args"); addChild(n, (yyvsp[0].node)); (yyval.node) = n; }
#line 1884 "C/src/syntax/parser.tab.c"
    break;


#line 1888 "C/src/syntax/parser.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 399 "C/src/syntax/parser.y"


void yyerror(const char *s)
{
    /* Bison 默认错误信息。本阶段我们靠手写的错误产生式报精确信息，
       这里作为兜底：解析失败且没有错误产生式命中时，置 has_error 抑制树输出，
       不打印默认 "syntax error"（Tests1 要求精确信息）。 */
    has_error = 1;
}

