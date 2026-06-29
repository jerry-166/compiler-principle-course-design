/*
 * lexer.l - C-- 词法分析器（实践 1.1 必做 + 选做 2.1/2.2/2.3）
 *
 * 必做：十进制 INT、基础 FLOAT、ID、7关键字、20类符号、错误A。
 * 选做 2.1：八进制(0开头)、十六进制(0x开头)整数，含非法形式报错。
 * 选做 2.2：指数形式浮点数(1.05e-4、.5E03、43.e-4)，含非法形式报错。
 * 选做 2.3：双斜线单行注释、斜杠星 多行注释完整识别丢弃（不支持嵌套）。
 *
 * 编译：
 *   flex -o C/src/lex/lex.yy.c C/src/lex/lexer.l
 *   gcc C/src/lex/lex.yy.c -o C/bin/scanner
 * 运行：
 *   ./C/bin/scanner test.cmm
 *
 * 详细原理见 docs/词法分析.md。
 */

%option yylineno

%{
#include <stdio.h>
#include <stdlib.h>

/* 把当前词素(yytext)按指定进制转成十进制 long。
   strtol 第二个参数 NULL 表示不关心剩余字符，第三个是进制(8/10/16)。 */
#define INT_VAL(base)  strtol(yytext, NULL, base)
%}

digit       [0-9]
letter      [_a-zA-Z]
nonzero     [1-9]
hexdigit    [0-9A-Fa-f]
/* base：指数浮点的"基数"部分，三种写法(数字.数字 / .数字 / 数字.)。
   注意点是元字符，这里要匹配字面小数点，用 \. 转义。 */
base        ({digit}+\.{digit}*|\.{digit}+)

%%

 /* ===== 注释（选做 2.3）===== */

"//".*              { // 单行注释：// 后到行尾，整段丢弃，不产生 Token
                    }

"/*"                {
                        // 块注释：从 /* 开始，读到最先遇到的 */ 为止，整段丢弃。
                        // C-- 不支持嵌套，遇到第一个 */ 就结束。
                        int c, prev = 0;
                        while ((c = input()) != 0) {
                            if (prev == '*' && c == '/') {
                                break;  // 凑成星斜杠，注释闭合
                            }
                            prev = c;
                        }
                        // 若读到 EOF(c==0) 说明注释没闭合，扫描随之结束
                    }

 /* ===== 空白符：丢弃 ===== */

[ \t\r\n]+          { }

 /* ===== 多字符运算符（必须排在单字符前）===== */

"&&"                { printf("AND\n"); }
"||"                { printf("OR\n"); }
">="                { printf("RELOP\n"); }
"<="                { printf("RELOP\n"); }
"=="                { printf("RELOP\n"); }
"!="                { printf("RELOP\n"); }

 /* ===== 关键字（必须排在 ID 前）===== */

"int"               { printf("TYPE\n"); }
"float"             { printf("TYPE\n"); }
"struct"            { printf("STRUCT\n"); }
"return"            { printf("RETURN\n"); }
"if"                { printf("IF\n"); }
"else"              { printf("ELSE\n"); }
"while"             { printf("WHILE\n"); }

 /* ===== 标识符 ID ===== */

{letter}({letter}|{digit})*  { printf("ID: %s\n", yytext); }

 /* ===== 整型常数 INT（选做 2.1：十进制/八进制/十六进制）=====
    排列依据前缀区分，非法形式靠"最长匹配"自然优先于合法形式：
      - 0x[hex]+[非hex字母] 形如 0x3G：非法规则能吃整串，比合法 0x3 长，自动胜出。
      - 0[0-7]*[89]        形如 09：非法规则吃整串，比合法八进制 0 长，自动胜出。
    所以"非法"规则写在"合法"前面更清晰，但顺序不强制（最长匹配保证）。 */

0[xX][0-9A-Fa-f]*[G-Zg-z_][0-9A-Za-z_]*  {
                        /* 非法十六进制：0x 后出现非十六进制字符(G~Z/g~z/_等)。
                           如 0x3G、0xGH。整体报错，避免拆成 0x3 + ID:G。 */
                        printf("Error type A at Line %d: Illegal hexadecimal number \"%s\".\n", yylineno, yytext);
                    }

0[xX]              {
                        /* 单独的 0x / 0X 后没有任何十六进制数字，非法。如 0x; */
                        printf("Error type A at Line %d: Illegal hexadecimal number \"%s\".\n", yylineno, yytext);
                    }

0[xX][0-9A-Fa-f]+  {
                        /* 合法十六进制：0x 后至少一个十六进制数字。转十进制输出。 */
                        printf("INT: %ld\n", INT_VAL(16));
                    }

0[0-7]*[89][0-9]*  {
                        /* 非法八进制：0 开头但某位出现 8 或 9。如 09、0128。整体报错。 */
                        printf("Error type A at Line %d: Illegal octal number \"%s\".\n", yylineno, yytext);
                    }

0                  { printf("INT: 0\n"); }            /* 单独的 0 */
0[0-7]+            { printf("INT: %ld\n", INT_VAL(8)); }   /* 合法八进制：0 后至少一个 0~7 */
{nonzero}{digit}*  { printf("INT: %ld\n", INT_VAL(10)); }  /* 合法十进制：1~9 开头 */

 /* ===== 浮点常数 FLOAT（选做 2.2：含指数形式）=====
    指数形式三部分：基数(数字+小数点，点可任意位置) + E/e + 指数(可选符号+数字串)。
    基础形式(无指数)：小数点前后都要有数字。

    用两条规则区分合法/非法指数浮点：
      合法：base + e + 可选符号 + 至少一位数字  ({base}[eE][+-]?{digit}+)
      非法：base + e，但 e 后没有数字          ({base}[eE])
    对 1.05e-4：合法规则吃整串(更长)，自动胜出。
    对 1.05e：合法规则匹配不了(e后无数字)，非法规则吃 1.05e(比基础浮点 1.05 多1字符)，胜出。
    合法规则必须写在非法规则前面，否则对 1.05e-4 两条等长会乱。
    base 三种写法：数字.数字(1.05)、.数字(.5)、数字.(43.) */

{base}[eE][+-]?{digit}+   {
                        /* 合法指数浮点：e 后有可选符号和至少一位数字。如 1.05e-4、43.e-4、.5E03 */
                        printf("FLOAT: %s\n", yytext);
                    }

{base}[eE]                {
                        /* 非法指数浮点：有 e/E 但后面没跟合法指数数字(如 1.05e、1.05e+)。
                           这条规则匹配"基数+e"，长度比单纯的基础浮点多1，自动优先；
                           而合法指数浮点比它更长，会先被上一条规则匹配走。 */
                        printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext);
                    }

{digit}+"."{digit}+       {
                        /* 基础浮点(无指数)：小数点前后都有数字。如 3.14、0.7、12.43 */
                        printf("FLOAT: %s\n", yytext);
                    }

 /* ===== 单字符符号 ===== */

";"                 { printf("SEMI\n"); }
","                 { printf("COMMA\n"); }
"="                 { printf("ASSIGNOP\n"); }
">"                 { printf("RELOP\n"); }
"<"                 { printf("RELOP\n"); }
"+"                 { printf("PLUS\n"); }
"-"                 { printf("MINUS\n"); }
"*"                 { printf("STAR\n"); }
"/"                 { printf("DIV\n"); }
"."                 { printf("DOT\n"); }
"!"                 { printf("NOT\n"); }
"("                 { printf("LP\n"); }
")"                 { printf("RP\n"); }
"["                 { printf("LB\n"); }
"]"                 { printf("RB\n"); }
"{"                 { printf("LC\n"); }
"}"                 { printf("RC\n"); }

 /* ===== 兜底：未定义字符（错误类型 A）===== */

.                   {
                        printf("Error type A at Line %d: Mysterious characters \"%s\".\n", yylineno, yytext);
                    }

%%

int main(int argc, char **argv)
{
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror(argv[1]);
            return 1;
        }
        yyin = f;
    }
    yylex();
    return 0;
}

int yywrap(void)
{
    return 1;
}
