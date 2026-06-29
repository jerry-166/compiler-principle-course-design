/*
 * main_tree.c - 实践1 语法分析程序的入口（建树+打印树）
 *
 * 从 parser.y 移出，使实践1(parser) 与实践2(cmmc) 各自带 main，共享 yyparse。
 * 行为与原 parser.y 中的 main 完全一致。
 */
#include <stdio.h>
#include "node.h"

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
    if (!has_error && root) {
        printTree(root);
        flush_output(0);
    } else {
        flush_output(1);
    }
    return 0;
}
