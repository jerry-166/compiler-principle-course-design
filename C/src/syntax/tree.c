/*
 * tree.c - C-- 语法树建树与打印实现（实践 1.2 语法分析）
 *
 * 设计要点：
 *   1. 非终结符节点的行号 = 它第一个非空子节点的行号（见 addChild）。
 *      这对应 Tests1/expects 的规则：Program (4) 取自首个 ExtDef 的 Specifier 行号，
 *      ExtDef (7) 取自它 Specifier 子节点（即 TYPE int）的行号。
 *   2. 终结符节点：有值的 Token 打印 "TOKEN: value"，无值的打印 "TOKEN"。
 *      INT 按十进制整数打印（词法层已做进制转换），FLOAT 按 %f（6 位小数）打印（float32 语义）。
 *   3. 空产生式（如 ExtDefList→空、StmtList→空）在 action 里返回 NULL，
 *      newNode+addChild 全 NULL 时本函数也返回 NULL，不产生节点（符合 Tests1 规则）。
 *   4. 输出缓冲：所有输出（树、错误信息）先进 open_memstream 内存流，
 *      最后在 flush_output 时统一写出。这样能控制"错误用例末尾不带 \n"——
 *      Tests1 的错误 .exp 文件末尾无换行，树 .exp 末尾有换行。
 *
 * 配套文档见 docs/语法分析.md。
 */

#define _GNU_SOURCE   /* open_memstream 需要 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "node.h"

int has_error = 0;

/* ===== 全局输出缓冲 =====
   用 open_memstream 把所有输出收集到内存，最后统一 flush。
   flush_output 的 strip_trailing_newline 参数控制是否去掉末尾换行
   （错误用例去换行，树用例保留）。 */
static FILE   *out_stream = NULL;
static char   *out_buf = NULL;
static size_t  out_size = 0;

static void ensure_stream(void)
{
    if (!out_stream) {
        out_stream = open_memstream(&out_buf, &out_size);
        if (!out_stream) { fprintf(stderr, "open_memstream failed\n"); exit(1); }
    }
}

/* 统一输出入口：lexer/parser/树的打印都调用它 */
void out(const char *fmt, ...)
{
    ensure_stream();
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out_stream, fmt, ap);
    va_end(ap);
}

/* 分析结束后调用：把缓冲内容写到真正的 stdout。
   strip_trailing_newline=1 时去掉末尾单个 \n（错误用例的期望行为）。 */
void flush_output(int strip_trailing_newline)
{
    if (out_stream) {
        fflush(out_stream);
        size_t len = out_size;
        if (strip_trailing_newline && len > 0 && out_buf[len-1] == '\n') {
            len -= 1;
        }
        if (len > 0) fwrite(out_buf, 1, len, stdout);
        fclose(out_stream);
        out_stream = NULL;
        free(out_buf);
        out_buf = NULL;
        out_size = 0;
    }
}

/* ===== 节点创建 ===== */

static Node *allocNode(void)
{
    Node *n = (Node *)calloc(1, sizeof(Node));
    if (!n) { fprintf(stderr, "out of memory\n"); exit(1); }
    return n;
}

Node *newNode(const char *name)
{
    Node *n = allocNode();
    n->name = strdup(name);
    n->is_token = 0;
    n->lineno = 0;
    n->val_kind = NO_VAL;
    return n;
}

Node *newTokenNode(const char *name, int lineno, int val_kind,
                   long ival, double fval, const char *sval)
{
    Node *n = allocNode();
    n->name = strdup(name);
    n->is_token = 1;
    n->lineno = lineno;
    n->val_kind = val_kind;
    n->ival = ival;
    n->fval = fval;
    n->sval = sval ? strdup(sval) : NULL;
    return n;
}

Node *addChild(Node *parent, Node *child)
{
    if (!parent) return child;
    if (!child)  return parent;
    if (parent->nchild >= MAX_CHILDREN) {
        fprintf(stderr, "too many children for %s (raise MAX_CHILDREN)\n", parent->name);
        exit(1);
    }
    if (parent->nchild == 0) {
        parent->lineno = child->lineno;
    }
    parent->children[parent->nchild++] = child;
    return parent;
}

/* 递归打印到 out 流 */
static void printRec(Node *n, int depth)
{
    if (!n) return;
    int i;
    for (i = 0; i < depth; i++) out("  ");

    if (n->is_token) {
        switch (n->val_kind) {
            case VAL_INT:
                out("%s: %ld\n", n->name, n->ival);
                break;
            case VAL_FLT:
                out("%s: %f\n", n->name, n->fval);
                break;
            case VAL_STR:
                out("%s: %s\n", n->name, n->sval ? n->sval : "");
                break;
            default:
                out("%s\n", n->name);
                break;
        }
    } else {
        out("%s (%d)\n", n->name, n->lineno);
        for (i = 0; i < n->nchild; i++) {
            printRec(n->children[i], depth + 1);
        }
    }
}

void printTree(Node *root)
{
    printRec(root, 0);
}
