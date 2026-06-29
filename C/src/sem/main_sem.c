/*
 * main_sem.c - 实践2 语义分析程序入口
 * 建树 → sem_analyze（语义检查）→ flush_output。不打印语法树。
 */
#include <stdio.h>
#include "node.h"
#include "sem.h"

extern int yyparse(void);
extern Node *root;
extern int has_error;

int main(int argc, char **argv)
{
    if (argc > 1) {
        extern FILE *yyin;
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror(argv[1]); return 1; }
    }
    yyparse();
    /* 语法错误（has_error）或建树失败时跳过语义分析（D-1 直接输出 type B）。*/
    if (!has_error && root) {
        sem_analyze(root);
    }
    /* 语义错误由 sem.c 用 out() 写缓冲，这里 flush，保留末尾换行。*/
    flush_output(0);
    return 0;
}
