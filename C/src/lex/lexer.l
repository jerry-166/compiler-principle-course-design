/*
 * lexer.l - C-- 词法分析器（实践 1.2：作为 Bison 的子模块）
 *
 * 与 1.1 的区别：本文件不再是独立程序，而是被 Bison 生成的 parser 调用。
 *   - 每条规则的 action 用 return TOKEN; 返回 Token 编号（Token 编号由 parser.tab.h 定义）
 *   - Token 的语义值通过 yylval.tok 传给语法分析器：
 *       INT     → yylval.tok.ival（已按进制转十进制的 long）
 *       FLOAT   → yylval.tok.fval（strtof 转 float32 后提升为 double，打印用 %f）
 *       ID/TYPE → yylval.tok.sval（原始词素字符串）
 *       所有 Token → yylval.tok.line（行号，由 YY_USER_ACTION 统一设置）
 *   - 不含 main（main 由 parser.y 提供）。
 *   - type A 词法错误：用 out() 打印错误信息后不 return Token（继续扫描），
 *     这样语法分析看到的还是干净的 Token 流，词法错误不影响后续匹配。
 *
 * 编译（项目根目录，WSL2）：
 *   flex  -o C/src/lex/lex.yy.c C/src/lex/lexer.l
 *   （配合 bison 生成的 parser.tab.c 一起编译，见 docs/语法分析.md）
 *
 * 详细原理见 docs/词法分析.md 和 docs/语法分析.md。
 */

%option yylineno
%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"         /* Node 类型 + out()/flush_output() */
#include "parser.tab.h"   /* Bison 生成的 Token 编号定义 和 yylval 类型 */

/* 把当前词素按进制转十进制 long。第三个参数 8/10/16。 */
#define TO_LONG(base)  strtol(yytext, NULL, base)

/* 复制当前词素（yytext 长度 yyleng）。替代 strdup，避免 -std=c99 严格模式下
   strdup 未声明导致返回 int 截断指针（GCC 7.5 + 64 位会段错误）。*/
#define DUP_YYTEXT()  dup_yytext(yytext, yyleng)

static char *dup_yytext(const char *s, int len)
{
    char *p = (char *)malloc((size_t)len + 1);
    if (p) { memcpy(p, s, (size_t)len); p[len] = '\0'; }
    return p;
}

/* YY_USER_ACTION：flex 每次成功匹配一条规则、执行 action 之前自动调用。
   统一在这里设置当前 Token 的行号，避免每条规则都写一遍。 */
#define YY_USER_ACTION  yylval.tok.line = yylineno;
%}

digit       [0-9]
letter      [_a-zA-Z]
nonzero     [1-9]
hexdigit    [0-9A-Fa-f]
/* base：指数浮点的基数部分，三种写法。点要转义。 */
base        ({digit}+\.{digit}*|\.{digit}+)

%%

 /* ===== 注释（选做 2.3）：丢弃，不产生 Token ===== */

"//".*              { /* 单行注释，整段丢弃 */ }

"/*"                {
                        /* 块注释：读到最先遇到的星斜杠，整段丢弃。不支持嵌套。 */
                        int c, prev = 0;
                        while ((c = input()) != 0) {
                            if (prev == '*' && c == '/') break;
                            prev = c;
                        }
                        if (c == 0) {
                            out("Error type A at Line %d: Unclosed comment.\n", yylineno);
                        }
                    }

 /* ===== 空白符：丢弃 ===== */

[ \t\r\n]+          { }

 /* ===== 复合赋值运算符（Tests1 A-8：作为整体识别，parser 报 type B）=====
    单独给一个 Token，配合 parser 的错误产生式报 "Unsupported compound assignment"。
    必须排在单字符 "+" "-" "*" "/" 和 "=" 之前（最长匹配 + 等长顺序）。 */

"+="                { yylval.tok.sval = DUP_YYTEXT(); return COMPOUND_ASSIGN; }
"-="                { yylval.tok.sval = DUP_YYTEXT(); return COMPOUND_ASSIGN; }
"*="                { yylval.tok.sval = DUP_YYTEXT(); return COMPOUND_ASSIGN; }
"/="                { yylval.tok.sval = DUP_YYTEXT(); return COMPOUND_ASSIGN; }

 /* ===== 多字符运算符（必须排在单字符前）===== */

"&&"                { return AND; }
"||"                { return OR; }
">="                { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }
"<="                { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }
"=="                { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }
"!="                { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }

 /* ===== 关键字（必须排在 ID 前）===== */

"int"               { yylval.tok.sval = DUP_YYTEXT(); return TYPE; }
"float"             { yylval.tok.sval = DUP_YYTEXT(); return TYPE; }
"struct"            { return STRUCT; }
"return"            { return RETURN; }
"if"                { return IF; }
"else"              { return ELSE; }
"while"             { return WHILE; }

 /* ===== 标识符 ID ===== */

{letter}({letter}|{digit})*  { yylval.tok.sval = DUP_YYTEXT(); return ID; }

 /* ===== 整型常数 INT（选做 2.1：十/八/十六进制）===== */

0[xX][0-9A-Fa-f]*[G-Zg-z_][0-9A-Za-z_]*  {
                        /* 非法十六进制：0x 后出现非 hex 字符（如 0x3G）。整体报错。 */
                        out("Error type A at Line %d: Undefined character in number \"%s\".\n", yylineno, yytext);
                    }

0[xX]              {
                        /* 单独 0x/0X 后无数字，非法。 */
                        out("Error type A at Line %d: Undefined character in number \"%s\".\n", yylineno, yytext);
                    }

0[xX][0-9A-Fa-f]+  {
                        /* 合法十六进制：转十进制存入 yylval.tok.ival。 */
                        yylval.tok.ival = TO_LONG(16);
                        return INT;
                    }

0[0-7]*[89][0-9]*  {
                        /* 非法八进制：0 开头但出现 8/9（如 09）。整体报错。 */
                        out("Error type A at Line %d: Undefined character in number \"%s\".\n", yylineno, yytext);
                    }

0                  { yylval.tok.ival = 0; return INT; }
0[0-7]+            { yylval.tok.ival = TO_LONG(8);  return INT; }
{nonzero}{digit}*  { yylval.tok.ival = TO_LONG(10); return INT; }

 /* ===== 浮点常数 FLOAT（选做 2.2：含指数形式）===== */

{base}[eE][+-]?{digit}+   {
                        /* 合法指数浮点：如 1.05e-4、43.e-4、.5E03。
                           strtof 转 float32（C-- FLOAT 是 IEEE754 单精度），再提升 double 存入 fval。 */
                        yylval.tok.fval = (double)strtof(yytext, NULL);
                        return FLOAT;
                    }

{base}[eE]                {
                        /* 非法指数浮点：e 后无数字（如 1.05e）。整体报错。 */
                        out("Error type A at Line %d: Undefined character in number \"%s\".\n", yylineno, yytext);
                    }

 /* Tests1 A-5：多个小数点的非法浮点（如 3.14.15）。
    规则比合法浮点长（多吃 .15），靠最长匹配优先，整体报错。 */
{digit}+\.{digit}+\.{digit}({digit}|\.{digit})*  {
                        out("Error type A at Line %d: Invalid floating point literal '%s'.\n", yylineno, yytext);
                    }

{digit}+"."{digit}+       {
                        /* 基础浮点（无指数）：小数点前后都有数字。如 3.14。 */
                        yylval.tok.fval = (double)strtof(yytext, NULL);
                        return FLOAT;
                    }

 /* Tests1 A-2：数字开头后跟字母的非法标识符（如 2x）。
    规则比合法 INT（只吃 2）长（多吃 x），靠最长匹配优先，整体报错。 */
{digit}+{letter}({letter}|{digit})*  {
                        out("Error type A at Line %d: Invalid identifier starting with digit '%s'.\n", yylineno, yytext);
                    }

 /* ===== 单字符符号 ===== */

";"                 { return SEMI; }
","                 { return COMMA; }
"="                 { return ASSIGNOP; }
">"                 { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }
"<"                 { yylval.tok.sval = DUP_YYTEXT(); return RELOP; }
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"*"                 { return STAR; }
"/"                 { return DIV; }
"."                 { return DOT; }
"!"                 { return NOT; }
"("                 { return LP; }
")"                 { return RP; }
"["                 { return LB; }
"]"                 { return RB; }
"{"                 { return LC; }
"}"                 { return RC; }

 /* ===== 兜底：未定义字符（错误类型 A，Tests1 A-1 的 '$'）===== */

.                   {
                        out("Error type A at Line %d: Undefined character '%s'.\n", yylineno, yytext);
                    }

%%
