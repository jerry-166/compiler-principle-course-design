/*
 * parser.y - C-- 语法分析器（实践 1.2，Bison）
 *
 * 任务：调用 lexer 的 yylex() 拿 Token，按 C-- 文法（Appendix.pdf 附录 A）组装语法树，
 *       打印缩进树。遇到语法错误（type B）打印精确错误信息，且不打印树。
 *
 * 与 lexer.l 的接口：
 *   - yylex() 返回 Token 编号（下面 %token 声明的那些）
 *   - yylval 传语义值：ival（INT 十进制值）、fval（FLOAT double 值）、sval（ID/TYPE 词素）、line（行号）
 *
 * 文法来源：Appendix.pdf 附录 A，中文讲解见 docs/C--词法语法分析.md 第 3 节。
 * 输出格式规则、错误信息清单见 docs/语法分析.md。
 *
 * 编译（项目根目录，WSL2）：
 *   bison -d -o C/src/syntax/parser.tab.c C/src/syntax/parser.y
 *   flex  -o C/src/lex/lex.yy.c C/src/lex/lexer.l
 *   gcc -o C/bin/parser C/src/syntax/parser.tab.c C/src/syntax/tree.c C/src/lex/lex.yy.c -I C/src/syntax -lm
 */

%{
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
%}

/* ===== yylval 类型 =====
   设计：终结符(Token)统一用 TokVal 结构体，里面 line 字段总是被 lexer 设为 yylineno，
   ival/fval/sval 按 Token 类型填写。这样 action 里能可靠拿到每个终结符的行号
   （不依赖 %locations，因为 flex 默认不维护 yylloc）。
   非终结符用 node 指针。 */
%union {
    struct TokVal {
        long   ival;    /* INT：十进制值 */
        double fval;    /* FLOAT：double 值 */
        char  *sval;    /* ID/TYPE：词素 */
        int    line;    /* 该 Token 出现的行号 */
    } tok;
    Node *node;
}

/* ===== Token 声明（终结符）。全部用 <tok> 类型，lexer 每个 return 前设 yylval.tok.line。 ===== */
%token <tok> INT FLOAT ID TYPE
%token <tok> SEMI COMMA ASSIGNOP RELOP
%token <tok> PLUS MINUS STAR DIV
%token <tok> AND OR DOT NOT
%token <tok> STRUCT RETURN IF ELSE WHILE
%token <tok> LP RP LB RB LC RC
%token <tok> COMPOUND_ASSIGN   /* += -= *= /= ：Tests1 A-8 报 Unsupported compound assignment */

%type <node> Program ExtDef ExtDefList Specifier StructSpecifier OptTag Tag
%type <node> VarDec ExtDecList FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args

/* ===== 运算符优先级与结合性（Appendix A 表1，从低到高）=====
   COMPOUND_ASSIGN 与 ASSIGNOP 同优先级：消除 Exp 中 COMPOUND_ASSIGN 与其他
   二元运算符之间的 shift/reduce 冲突（Bison 默认 shift，行为正确，但消除警告）。
   ELSE 优先级低于所有二元运算符：解决经典 dangling else（ELSE 作为 Stmt 的
   结束标记，优先级低于 IF/Exp 等，使 Bison 先归约 Stmt 再 shift ELSE）。
   CONFLICT_DEF_STMT 是伪 token，低于 TYPE/STRUCT/error，使 CompSt 中 DefList→ε
   归约优先级低于 shift，消除 DefList vs StmtList 歧义。 */
%right ASSIGNOP COMPOUND_ASSIGN
%left  OR
%left  AND
%left  RELOP
%left  PLUS MINUS
%left  STAR DIV
%right NOT UMINUS
%left  LP RP LB RB DOT
%precedence ELSE CONFLICT_DEF_STMT

%expect 8
/* 8 个已确认安全的 shift/reduce 冲突（Bison 默认 shift 行为正确）：
   - 5 个：CompSt 中 DefList→ε vs StmtList，look-ahead TYPE/STRUCT/error
     （Bison 默认 shift 进入 Def，正确：声明优先于语句中的声明当语句错误）
   - 3 个：FunDec 中 ParamDec 后 COMMA 的 look-ahead 歧义
     （Bison 默认 shift 进 VarList 继续匹配，正确：正常多参数优先） */


%%

/* ============ 高层定义 ============ */

Program    : ExtDefList                  { root = NT("Program"); addChild(root, $1); }
           ;

ExtDefList : ExtDef ExtDefList           { Node *n = NT("ExtDefList"); addChild(n, $1); addChild(n, $2); $$ = n; }
           | /* 空 */                    { $$ = NULL; }
           ;

ExtDef     : Specifier ExtDecList SEMI   { Node *n = NT("ExtDef"); addChild(n,$1); addChild(n,$2);
                                           Node *s = TKS("SEMI", $3, NO_VAL); addChild(n, s); $$ = n; }
           | Specifier SEMI              { if (last_struct_anonymous) {
                                               /* Tests1 B-1: struct { int tag; }; 匿名结构体声明 */
                                               out("Error type B at Line %d: Anonymous struct declaration.\n", $1->lineno);
                                               has_error = 1; $$ = NULL;
                                             } else {
                                               Node *n = NT("ExtDef"); addChild(n, $1);
                                               Node *s = TKS("SEMI", $2, NO_VAL); addChild(n, s); $$ = n;
                                             }
                                             last_struct_anonymous = 0; }
           | Specifier FunDec CompSt     { Node *n = NT("ExtDef"); addChild(n,$1); addChild(n,$2); addChild(n,$3); $$ = n; }
           | Specifier FunDec SEMI       {
#ifdef ENABLE_FUNC_DECL
                                          Node *n = NT("ExtDef"); addChild(n,$1);
                                          addChild(n, $2);
                                          Node *s = TKS("SEMI", $3, NO_VAL); addChild(n, s);
                                          $$ = n;
#else
                                          /* 未启用函数声明（要求3.1）：报 type B，
                                             文字与 D-1.exp 一致。$2 是 FunDec，行号=ID 行号。 */
                                          out("Error type B at Line %d: Syntax error.\n", $2->lineno);
                                          has_error = 1; $$ = NULL;
#endif
                                          }
           /* Tests1 A-4: 全局变量不允许初始化，如 float ratio = 1.5; */
           | Specifier ExtDecList ASSIGNOP Exp SEMI  { out("Error type B at Line %d: Global variable initialization not allowed.\n", $3.line);
                                           has_error = 1; $$ = NULL; }
           | error SEMI                  { yyerrok; has_error = 1; $$ = NULL; }
           ;

ExtDecList : VarDec                      { Node *n = NT("ExtDecList"); addChild(n, $1); $$ = n; }
           | VarDec COMMA ExtDecList     { Node *n = NT("ExtDecList"); addChild(n,$1);
                                           Node *c = TKS("COMMA", $2, NO_VAL); addChild(n,c);
                                           addChild(n, $3); $$ = n; }
           ;

/* ============ 类型描述符 ============ */

Specifier       : TYPE                   { Node *n = NT("Specifier");
                                           Node *t = TKS("TYPE", $1, VAL_STR); addChild(n, t); $$ = n; }
                | StructSpecifier        { Node *n = NT("Specifier"); addChild(n, $1); $$ = n; }
                ;

StructSpecifier : STRUCT OptTag LC DefList RC
                                           { Node *n = NT("StructSpecifier");
                                             Node *s = TKS("STRUCT", $1, NO_VAL); addChild(n,s);
                                             addChild(n, $2);
                                             Node *lc = TKS("LC", $3, NO_VAL); addChild(n,lc);
                                             addChild(n, $4);
                                             Node *rc = TKS("RC", $5, NO_VAL); addChild(n,rc);
                                             /* 记录是否匿名（OptTag 为空）。ExtDef 据此报 B-1 错误。 */
                                             last_struct_anonymous = ($2 == NULL);
                                             $$ = n; }
                | STRUCT Tag              { Node *n = NT("StructSpecifier");
                                           Node *s = TKS("STRUCT", $1, NO_VAL); addChild(n,s);
                                           addChild(n, $2);
                                           last_struct_anonymous = 0;
                                           $$ = n; }
                ;

OptTag      : ID                          { Node *n = NT("OptTag");
                                            Node *i = TKS("ID", $1, VAL_STR); addChild(n, i); $$ = n; }
            | /* 空 */                    { $$ = NULL; }
            ;

Tag         : ID                          { Node *n = NT("Tag");
                                            Node *i = TKS("ID", $1, VAL_STR); addChild(n, i); $$ = n; }
            ;

/* ============ 声明器 ============ */

VarDec      : ID                          { Node *n = NT("VarDec");
                                            Node *i = TKS("ID", $1, VAL_STR); addChild(n, i); $$ = n; }
            | VarDec LB INT RB            { Node *n = NT("VarDec"); addChild(n, $1);
                                            Node *lb = TKS("LB", $2, NO_VAL); addChild(n,lb);
                                            Node *in = TKS("INT", $3, VAL_INT); addChild(n,in);
                                            Node *rb = TKS("RB", $4, NO_VAL); addChild(n,rb); $$ = n; }
            /* Tests1 A-6/B-1: 数组维度是 FLOAT 而非 INT */
            | VarDec LB FLOAT RB          { out("Error type B at Line %d: Array dimension must be integer.\n", $2.line);
                                            has_error = 1; $$ = NULL; }
            /* Tests1 B-2: 数组声明多余 ]，如 int data[5]] */
            | VarDec LB INT RB RB         { out("Error type B at Line %d: Extra closing bracket in array declaration.\n", $4.line);
                                            has_error = 1; $$ = NULL; }
            ;

FunDec      : ID LP VarList RP            { Node *n = NT("FunDec");
                                            Node *i = TKS("ID", $1, VAL_STR); addChild(n,i);
                                            Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                            addChild(n, $3);
                                            Node *rp = TKS("RP", $4, NO_VAL); addChild(n,rp); $$ = n; }
            | ID LP RP                    { Node *n = NT("FunDec");
                                            Node *i = TKS("ID", $1, VAL_STR); addChild(n,i);
                                            Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                            Node *rp = TKS("RP", $3, NO_VAL); addChild(n,rp); $$ = n; }
            /* Tests1 B-1: 形参列表尾逗号，如 int f(int a,) */
            /* Tests1 B-1: 形参列表尾逗号，如 int f(int a,)。直接用 ParamDec 避免
               VarList 归约时的 shift 冲突；look-ahead COMMA 时 Bison shift 进此分支。 */
            | ID LP ParamDec COMMA RP   { out("Error type B at Line %d: Trailing comma in function parameters.\n", $2.line);
                                          has_error = 1; $$ = NULL; }
            | ID LP VarList COMMA RP      { out("Error type B at Line %d: Trailing comma in function parameters.\n", $2.line);
                                            has_error = 1; $$ = NULL; }
            ;

VarList     : ParamDec COMMA VarList      { Node *n = NT("VarList"); addChild(n,$1);
                                            Node *c = TKS("COMMA", $2, NO_VAL); addChild(n,c);
                                            addChild(n, $3); $$ = n; }
            | ParamDec                    { Node *n = NT("VarList"); addChild(n, $1); $$ = n; }
            ;

ParamDec    : Specifier VarDec            { Node *n = NT("ParamDec"); addChild(n,$1); addChild(n,$2); $$ = n; }
            ;

/* ============ 语句 ============ */

CompSt      : LC DefList StmtList RC     { Node *n = NT("CompSt");
                                           Node *lc = TKS("LC", $1, NO_VAL); addChild(n,lc);
                                           addChild(n, $2); addChild(n, $3);
                                           Node *rc = TKS("RC", $4, NO_VAL); addChild(n,rc); $$ = n; }
            ;

StmtList    : Stmt StmtList              { Node *n = NT("StmtList"); addChild(n,$1); addChild(n,$2); $$ = n; }
            | /* 空 */                   { $$ = NULL; }
            ;

Stmt        : Exp SEMI                   { Node *n = NT("Stmt"); addChild(n,$1);
                                           Node *s = TKS("SEMI", $2, NO_VAL); addChild(n,s); $$ = n; }
            | CompSt                     { Node *n = NT("Stmt"); addChild(n, $1); $$ = n; }
            | RETURN Exp SEMI            { Node *n = NT("Stmt");
                                           Node *r = TKS("RETURN", $1, NO_VAL); addChild(n,r);
                                           addChild(n, $2);
                                           Node *s = TKS("SEMI", $3, NO_VAL); addChild(n,s); $$ = n; }
            | IF LP Exp RP Stmt          { Node *n = NT("Stmt");
                                           Node *i = TKS("IF", $1, NO_VAL); addChild(n,i);
                                           Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                           addChild(n, $3);
                                           Node *rp = TKS("RP", $4, NO_VAL); addChild(n,rp);
                                           addChild(n, $5); $$ = n; }
            | IF LP Exp RP Stmt ELSE Stmt %prec ELSE { Node *n = NT("Stmt");
                                           Node *i = TKS("IF", $1, NO_VAL); addChild(n,i);
                                           Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                           addChild(n, $3);
                                           Node *rp = TKS("RP", $4, NO_VAL); addChild(n,rp);
                                           addChild(n, $5);
                                           Node *e = TKS("ELSE", $6, NO_VAL); addChild(n,e);
                                           addChild(n, $7); $$ = n; }
            | WHILE LP Exp RP Stmt       { Node *n = NT("Stmt");
                                           Node *w = TKS("WHILE", $1, NO_VAL); addChild(n,w);
                                           Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                           addChild(n, $3);
                                           Node *rp = TKS("RP", $4, NO_VAL); addChild(n,rp);
                                           addChild(n, $5); $$ = n; }
            /* Tests1 B-1: while (c); 空循环体 —— 直接分号 */
            | WHILE LP Exp RP SEMI       { out("Error type B at Line %d: Empty while statement body.\n", $1.line);
                                           has_error = 1; $$ = NULL; }
            /* Tests1 A-3: return; 非void函数缺返回值 */
            | RETURN SEMI                { out("Error type B at Line %d: Missing return value in non-void function.\n", $1.line);
                                           has_error = 1; $$ = NULL; }
            /* Tests1 A-9/B-2: 语句块中语句之后又出现变量声明 */
            | Specifier DecList SEMI     { out("Error type B at Line %d: Variable declaration after statements in block.\n", $1->lineno);
                                           has_error = 1; $$ = NULL; }
            | error SEMI                 { yyerrok; has_error = 1; $$ = NULL; }
            ;

/* ============ 局部定义 ============ */

DefList     : Def DefList                { Node *n = NT("DefList"); addChild(n,$1); addChild(n,$2); $$ = n; }
            | /* 空 */                   { $$ = NULL; }
            ;

Def         : Specifier DecList SEMI     { Node *n = NT("Def"); addChild(n,$1); addChild(n,$2);
                                           Node *s = TKS("SEMI", $3, NO_VAL); addChild(n,s);
                                           last_struct_anonymous = 0; $$ = n; }
            | Specifier SEMI             { if (last_struct_anonymous) {
                                               /* Tests1 B-1: 局部的 struct {...}; 匿名结构体声明 */
                                               out("Error type B at Line %d: Anonymous struct declaration.\n", $1->lineno);
                                               has_error = 1; $$ = NULL;
                                             } else {
                                               Node *n = NT("Def"); addChild(n,$1);
                                               Node *s = TKS("SEMI", $2, NO_VAL); addChild(n,s); $$ = n;
                                             }
                                             last_struct_anonymous = 0; }
            | error SEMI                 { yyerrok; has_error = 1; $$ = NULL; }
            ;

DecList     : Dec                        { Node *n = NT("DecList"); addChild(n, $1); $$ = n; }
            | Dec COMMA DecList          { Node *n = NT("DecList"); addChild(n,$1);
                                           Node *c = TKS("COMMA", $2, NO_VAL); addChild(n,c);
                                           addChild(n, $3); $$ = n; }
            ;

Dec         : VarDec                     { Node *n = NT("Dec"); addChild(n, $1); $$ = n; }
            | VarDec ASSIGNOP Exp        { /* Tests1 B-1: 数组初始化 a[4] = ... 不支持。
                                              检查 VarDec 是否含数组维度（递归找 LB 子节点）。 */
                                           int is_array = 0;
                                           Node *p = $1;
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
                                               out("Error type B at Line %d: Array initialization not supported.\n", $1->lineno);
                                               has_error = 1; $$ = NULL;
                                           } else {
                                               Node *n = NT("Dec"); addChild(n,$1);
                                               Node *a = TKS("ASSIGNOP", $2, NO_VAL); addChild(n,a);
                                               addChild(n, $3); $$ = n;
                                           } }
            /* Tests1 B-1: 数组花括号初始化，如 float w[4] = {0.25, 0.5, 1.0};。
               { Exp, Exp, ... } 复用 Args 语法。 */
            | VarDec ASSIGNOP LC Args RC  { out("Error type B at Line %d: Array initialization not supported.\n", $1->lineno);
                                           has_error = 1; $$ = NULL; }
            | VarDec ASSIGNOP LC RC       { out("Error type B at Line %d: Array initialization not supported.\n", $1->lineno);
                                           has_error = 1; $$ = NULL; }
            ;

/* ============ 表达式 ============ */

Exp         : Exp ASSIGNOP Exp           { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *a = TKS("ASSIGNOP", $2, NO_VAL); addChild(n,a);
                                           addChild(n, $3); $$ = n; }
            | Exp AND Exp                { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("AND", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | Exp OR Exp                 { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("OR", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | Exp RELOP Exp              { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *r = TKS("RELOP", $2, NO_VAL); addChild(n,r);
                                           addChild(n, $3); $$ = n; }
            | Exp PLUS Exp               { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("PLUS", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | Exp MINUS Exp              { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("MINUS", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | Exp STAR Exp               { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("STAR", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | Exp DIV Exp                { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *o = TKS("DIV", $2, NO_VAL); addChild(n,o);
                                           addChild(n, $3); $$ = n; }
            | LP Exp RP                  { Node *n = NT("Exp");
                                           Node *lp = TKS("LP", $1, NO_VAL); addChild(n,lp);
                                           addChild(n, $2);
                                           Node *rp = TKS("RP", $3, NO_VAL); addChild(n,rp); $$ = n; }
            | MINUS Exp  %prec UMINUS    { Node *n = NT("Exp");
                                           Node *m = TKS("MINUS", $1, NO_VAL); addChild(n,m);
                                           addChild(n, $2); $$ = n; }
            | NOT Exp                    { Node *n = NT("Exp");
                                           Node *nt = TKS("NOT", $1, NO_VAL); addChild(n,nt);
                                           addChild(n, $2); $$ = n; }
            | ID LP Args RP              { Node *n = NT("Exp");
                                           Node *i = TKS("ID", $1, VAL_STR); addChild(n,i);
                                           Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                           addChild(n, $3);
                                           Node *rp = TKS("RP", $4, NO_VAL); addChild(n,rp); $$ = n; }
            | ID LP RP                   { Node *n = NT("Exp");
                                           Node *i = TKS("ID", $1, VAL_STR); addChild(n,i);
                                           Node *lp = TKS("LP", $2, NO_VAL); addChild(n,lp);
                                           Node *rp = TKS("RP", $3, NO_VAL); addChild(n,rp); $$ = n; }
            /* Tests1 A-7: 函数调用实参列表尾逗号，如 add(3,) */
            | ID LP Exp COMMA RP         { out("Error type B at Line %d: Trailing comma in function call arguments.\n", $2.line);
                                           has_error = 1; $$ = NULL; }
            | Exp LB Exp RB              { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *lb = TKS("LB", $2, NO_VAL); addChild(n,lb);
                                           addChild(n, $3);
                                           Node *rb = TKS("RB", $4, NO_VAL); addChild(n,rb); $$ = n; }
            | Exp DOT ID                 { Node *n = NT("Exp"); addChild(n,$1);
                                           Node *d = TKS("DOT", $2, NO_VAL); addChild(n,d);
                                           Node *i = TKS("ID", $3, VAL_STR); addChild(n,i); $$ = n; }
            /* Tests1 A-10: a. 后缺成员名（DOT 后非 ID，如跟 SEMI） */
            | Exp DOT                     { out("Error type B at Line %d: Missing member name after '.'.\n", $2.line);
                                           has_error = 1; $$ = NULL; }
            /* Tests1 A-8: += -= *= /= 复合赋值，C-- 不支持 */
            | Exp COMPOUND_ASSIGN Exp     { out("Error type B at Line %d: Unsupported compound assignment operator '%s'.\n",
                                                $2.line, $2.sval ? $2.sval : "+=");
                                           has_error = 1; $$ = NULL; }
            | ID                         { Node *n = NT("Exp");
                                           Node *i = TKS("ID", $1, VAL_STR); addChild(n,i); $$ = n; }
            | INT                        { Node *n = NT("Exp");
                                           Node *in = TKS("INT", $1, VAL_INT); addChild(n,in); $$ = n; }
            | FLOAT                      { Node *n = NT("Exp");
                                           Node *f = TKS("FLOAT", $1, VAL_FLT); addChild(n,f); $$ = n; }
            ;

Args        : Exp COMMA Args             { Node *n = NT("Args"); addChild(n,$1);
                                           Node *c = TKS("COMMA", $2, NO_VAL); addChild(n,c);
                                           addChild(n, $3); $$ = n; }
            | Exp                        { Node *n = NT("Args"); addChild(n, $1); $$ = n; }
            ;

%%

void yyerror(const char *s)
{
    /* Bison 默认错误信息。本阶段我们靠手写的错误产生式报精确信息，
       这里作为兜底：解析失败且没有错误产生式命中时，置 has_error 抑制树输出，
       不打印默认 "syntax error"（Tests1 要求精确信息）。 */
    has_error = 1;
}

