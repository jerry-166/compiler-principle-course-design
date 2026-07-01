/* operand.c - IR 操作数实现
 * 使用 POSIX strdup，兼容 GCC 7.5 / C99
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operand.h"

/* ---- 静态辅助 ---- */

/* snprintf 到栈缓冲再 strdup，避免调用方持有原始缓冲 */
static char *fmt_strdup(const char *fmt, int num)
{
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, num);
    return strdup(buf);
}

/* ---- 全局计数器 ---- */
static int g_temp_counter = 0;
static int g_label_counter = 0;

/* ---- 生成器 ---- */

Operand new_temp(void)
{
    Operand op;
    op.kind = OP_TEMP;
    op.name = fmt_strdup("t%d", ++g_temp_counter);
    op.value = 0;
    return op;
}

Operand new_label(void)
{
    Operand op;
    op.kind = OP_LABEL;
    op.name = fmt_strdup("label%d", ++g_label_counter);
    op.value = 0;
    return op;
}

Operand new_const(int value)
{
    Operand op;
    op.kind = OP_CONST;
    op.name = NULL;
    op.value = value;
    return op;
}

Operand new_var(const char *src_name)
{
    Operand op;
    op.kind = OP_VAR;
    /* "v_" 前缀 + src_name */
    size_t len = strlen(src_name) + 3; /* "v_" + name + '\0' */
    char *buf = (char *)malloc(len);
    sprintf(buf, "v_%s", src_name);
    op.name = buf;
    op.value = 0;
    return op;
}

Operand new_addr(const char *src_name)
{
    Operand op;
    op.kind = OP_ADDR;
    /* 与 new_var 同样加 "v_" 前缀 */
    size_t len = strlen(src_name) + 3;
    char *buf = (char *)malloc(len);
    sprintf(buf, "v_%s", src_name);
    op.name = buf;
    op.value = 0;
    return op;
}

Operand new_var_from_ir(const char *ir_name)
{
    Operand op;
    op.kind = OP_VAR;
    op.name = strdup(ir_name);
    op.value = 0;
    return op;
}

/* ---- 打印 ---- */

void operand_print(FILE *out, Operand op)
{
    switch (op.kind) {
    case OP_VAR:
    case OP_TEMP:
    case OP_LABEL:
    case OP_ADDR:
        fprintf(out, "%s", op.name);
        break;
    case OP_CONST:
        fprintf(out, "#%d", op.value);
        break;
    }
}
