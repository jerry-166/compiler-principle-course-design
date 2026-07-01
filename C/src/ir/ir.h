/*
 * ir.h - IR 指令链表（实践 3），按 PDF 表 4.6 的 17 种语句。
 */
#ifndef IR_H
#define IR_H

#include <stdio.h>
#include "operand.h"

typedef enum {
    IR_LABEL, IR_FUNCTION, IR_ASSIGN,
    IR_ADD, IR_SUB, IR_MUL, IR_DIV,
    IR_ADDR, IR_LOAD, IR_STORE,
    IR_GOTO, IR_IF, IR_RETURN,
    IR_DEC, IR_ARG, IR_CALL, IR_PARAM,
    IR_READ, IR_WRITE
} IRKind;

typedef enum {
    RELOP_LT, RELOP_LE, RELOP_GT, RELOP_GE, RELOP_EQ, RELOP_NE
} Relop;

typedef struct IR_ *IR;
struct IR_ {
    IRKind kind;
    Operand x, y, z;   /* 操作数（按 kind 用不同字段；不用全） */
    char  *name;       /* FUNCTION/CALL 的函数名；GOTO/LABEL/IF 的标号名 */
    Relop  relop;      /* IF 的关系运算符 */
    int    size;       /* DEC 的字节数 */
    struct IR_ *next;
};

/* 全局链表头/尾 */
extern IR ir_head, ir_tail;

/* gen 系列：构造节点并追加到链表尾部 */
void gen_label(Operand label);                  /* LABEL label : */
void gen_function(const char *fname);           /* FUNCTION fname : */
void gen_assign(Operand x, Operand y);          /* x := y */
void gen_binop(IRKind kind, Operand x, Operand y, Operand z); /* x := y op z，kind=IR_ADD/SUB/MUL/DIV */
void gen_addr(Operand x, Operand y);            /* x := &y（y 是变量名） */
void gen_load(Operand x, Operand y);            /* x := *y */
void gen_store(Operand x, Operand y);           /* *x := y */
void gen_goto(Operand label);                   /* GOTO label */
void gen_if(Operand x, Relop relop, Operand y, Operand label); /* IF x relop y GOTO label */
void gen_return(Operand x);                     /* RETURN x */
void gen_dec(Operand x, int size);              /* DEC x size */
void gen_arg(Operand x);                        /* ARG x */
void gen_call(Operand x, const char *fname);    /* x := CALL fname */
void gen_param(Operand x);                      /* PARAM x */
void gen_read(Operand x);                       /* READ x */
void gen_write(Operand x);                      /* WRITE x */

/* 遍历 ir_head，每条指令按 PDF 表 4.6 格式打印一行到 out */
void ir_print(FILE *out);

/* 把 Relop 打印成文字 "<" "<=" ">" ">=" "==" "!=" */
const char *relop_str(Relop r);

#endif /* IR_H */
