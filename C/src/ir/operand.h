/*
 * operand.h - IR 操作数（实践 3 中间代码生成）
 * PDF 4.2.4 的 place 可以是变量/临时变量/立即数/标号/地址。统一为 Operand。
 */
#ifndef OPERAND_H
#define OPERAND_H

#include <stdio.h>

typedef enum {
    OP_VAR,       /* 源码变量：name = "v_a" */
    OP_TEMP,      /* 临时变量：name = "t1" */
    OP_CONST,     /* 立即数：value，打印为 "#value" */
    OP_LABEL,     /* 标号：name = "label1"（仅 IR_IF/GOTO/LABEL 用） */
    OP_ADDR       /* &name：变量地址（数组/结构体参数传引用） */
} OperandKind;

typedef struct {
    OperandKind kind;
    char *name;     /* OP_VAR/OP_TEMP/OP_LABEL/OP_ADDR 的名字 */
    int   value;    /* OP_CONST 的值 */
} Operand;

/* 生成器（内部用全局计数器） */
Operand new_temp(void);                       /* OP_TEMP，name=t1,t2,... */
Operand new_label(void);                      /* OP_LABEL，name=label1,... */
Operand new_const(int value);                 /* OP_CONST */
Operand new_var(const char *src_name);        /* OP_VAR，name=v_src_name */
Operand new_addr(const char *src_name);       /* OP_ADDR，name=v_src_name */
Operand new_var_from_ir(const char *ir_name); /* 已知完整 IR 名（如 "v_a"），直接造 OP_VAR */

/* 打印到 FILE*，不含前后空格 */
void operand_print(FILE *out, Operand op);

#endif /* OPERAND_H */
