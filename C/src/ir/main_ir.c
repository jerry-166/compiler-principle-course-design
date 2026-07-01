/*
 * main_ir.c - 实践 3 IR 生成程序入口。
 * 用法：./cmmc_ir <input.cmm> [output.ir]
 * 不给 output.ir 则打印到 stdout。
 */
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "ir.h"

/* parser.tab.c 提供 */
extern FILE *yyin;
extern int yyparse(void);
extern Node *root;
extern int has_error;

/* translate.c 提供 */
void translate_program(Node *root);

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <input.cmm> [output.ir]\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("fopen input"); return 1; }
    yyparse();
    fclose(yyin);
    if (has_error || !root) {
        fprintf(stderr, "parse error, skip translate\n");
        return 1;
    }
    translate_program(root);
    FILE *out = (argc >= 3) ? fopen(argv[2], "w") : stdout;
    if (!out) { perror("fopen output"); return 1; }
    ir_print(out);
    if (argc >= 3) fclose(out);
    return 0;
}
