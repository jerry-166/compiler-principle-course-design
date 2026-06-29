/*
 * node.h - C-- 语法树节点定义（实践 1.2 语法分析）
 *
 * 语法树节点分两类：
 *   1. 非终结符节点：如 Program/ExtDef/Stmt/Exp。打印格式 "Name (lineno)"，
 *      lineno 取自它的第一个子节点（详见 tree.c 的 addChild 逻辑）。
 *   2. 终结符节点（Token）：如 INT/FLOAT/ID/SEMI。打印格式：
 *        - 有值的（INT/FLOAT/ID/TYPE）："TOKEN: value"
 *        - 无值的（SEMI/LP/IF 等）："TOKEN"
 *      终结符节点不带行号。
 *
 * 配合 parser.y 使用：parser.y 的每条产生式 action 调用 newNode/newTokenNode
 * 建节点，最后把根节点 Program 交给 printTree 输出。
 */

#ifndef NODE_H
#define NODE_H

/* 子节点最大数量。C-- 产生式右侧最多符号数（如 Stmt → IF LP Exp RP Stmt ELSE Stmt = 7），
   再加一些冗余，够用。超出会断言失败提醒加。 */
#define MAX_CHILDREN 8

/* 终结符的值类型标记。newTokenNode 的 val_kind 参数用这几个。
   NO_VAL=无值（SEMI/LP 等）；VAL_INT=整数值；VAL_FLT=浮点值；VAL_STR=字符串值。 */
#define NO_VAL   0
#define VAL_INT  1
#define VAL_FLT  2
#define VAL_STR  3

typedef struct Node {
    char *name;          /* 节点名：非终结符名（"Program"）或 Token 名（"INT"） */
    int   lineno;        /* 行号：终结符=出现行；非终结符=首个子节点行号 */
    int   is_token;      /* 1=终结符，0=非终结符 */
    int   val_kind;      /* 终结符的值类型：NO_VAL/INT_VAL/FLT_VAL/STR_VAL */
    long  ival;          /* INT 的十进制值 */
    double fval;         /* FLOAT 的值 */
    char *sval;          /* ID/FLOAT/TYPE 的原始文本（FLOAT 存原始词素用于 %f 打印） */
    struct Node *children[MAX_CHILDREN];
    int   nchild;
} Node;

/* 新建非终结符节点：name=节点名，lineno 一般传 0（addChild 后由首个子节点决定）。
   调用 newNode 后用 addChild 挂子节点。若所有子节点都为 NULL（空产生式场景），
   返回 NULL 表示"不产生节点"。 */
Node *newNode(const char *name);

/* 新建终结符节点：name=Token 名，lineno=yylineno，val_kind 区分有无值及值类型。
   ival/fval/sval 按需传入。 */
Node *newTokenNode(const char *name, int lineno, int val_kind,
                   long ival, double fval, const char *sval);

/* 把 child 挂到 parent 下。child 为 NULL 时跳过（不挂空）。
   首个被挂的子节点决定 parent 的 lineno。返回 parent。 */
Node *addChild(Node *parent, Node *child);

/* 缩进打印整棵树。每层 2 空格。根节点不带额外缩进。 */
void printTree(Node *root);

/* 统一输出入口：lexer/parser/树的打印都调用它，写入内部缓冲。
   flush_output 在分析结束后统一输出到 stdout。 */
void out(const char *fmt, ...);
void flush_output(int strip_trailing_newline);

/* 全局错误标志：任何 type B 语法错误置 1。main 中据此决定是否打印树。 */
extern int has_error;

#endif /* NODE_H */
