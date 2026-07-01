/* ir.c - IR 指令链表实现
 * 使用 POSIX strdup，兼容 GCC 7.5 / C99
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

/* ---- 全局链表 ---- */
IR ir_head = NULL;
IR ir_tail = NULL;

/* ---- 静态辅助：创建节点并追加 ---- */
static IR ir_new_node(void)
{
    IR node = (IR)calloc(1, sizeof(struct IR_));
    if (!node) {
        fprintf(stderr, "ir_new_node: out of memory\n");
        exit(1);
    }
    if (ir_tail) {
        ir_tail->next = node;
    } else {
        ir_head = node;
    }
    ir_tail = node;
    return node;
}

/* ---- Relop 打印 ---- */
const char *relop_str(Relop r)
{
    switch (r) {
    case RELOP_LT: return "<";
    case RELOP_LE: return "<=";
    case RELOP_GT: return ">";
    case RELOP_GE: return ">=";
    case RELOP_EQ: return "==";
    case RELOP_NE: return "!=";
    }
    return "?";
}

/* ---- gen 系列 ---- */

void gen_label(Operand label)
{
    IR node = ir_new_node();
    node->kind  = IR_LABEL;
    node->x     = label;
}

void gen_function(const char *fname)
{
    IR node = ir_new_node();
    node->kind = IR_FUNCTION;
    node->name = strdup(fname);
}

void gen_assign(Operand x, Operand y)
{
    IR node = ir_new_node();
    node->kind = IR_ASSIGN;
    node->x    = x;
    node->y    = y;
}

void gen_binop(IRKind kind, Operand x, Operand y, Operand z)
{
    IR node = ir_new_node();
    node->kind = kind;   /* IR_ADD / IR_SUB / IR_MUL / IR_DIV */
    node->x    = x;
    node->y    = y;
    node->z    = z;
}

void gen_addr(Operand x, Operand y)
{
    IR node = ir_new_node();
    node->kind = IR_ADDR;
    node->x    = x;
    node->y    = y;
}

void gen_load(Operand x, Operand y)
{
    IR node = ir_new_node();
    node->kind = IR_LOAD;
    node->x    = x;
    node->y    = y;
}

void gen_store(Operand x, Operand y)
{
    IR node = ir_new_node();
    node->kind = IR_STORE;
    node->x    = x;
    node->y    = y;
}

void gen_goto(Operand label)
{
    IR node = ir_new_node();
    node->kind = IR_GOTO;
    node->x    = label;
}

void gen_if(Operand x, Relop relop, Operand y, Operand label)
{
    IR node = ir_new_node();
    node->kind  = IR_IF;
    node->x     = x;
    node->relop = relop;
    node->y     = y;
    node->z     = label;
}

void gen_return(Operand x)
{
    IR node = ir_new_node();
    node->kind = IR_RETURN;
    node->x    = x;
}

void gen_dec(Operand x, int size)
{
    IR node = ir_new_node();
    node->kind = IR_DEC;
    node->x    = x;
    node->size = size;
}

void gen_arg(Operand x)
{
    IR node = ir_new_node();
    node->kind = IR_ARG;
    node->x    = x;
}

void gen_call(Operand x, const char *fname)
{
    IR node = ir_new_node();
    node->kind = IR_CALL;
    node->x    = x;
    node->name = strdup(fname);
}

void gen_param(Operand x)
{
    IR node = ir_new_node();
    node->kind = IR_PARAM;
    node->x    = x;
}

void gen_read(Operand x)
{
    IR node = ir_new_node();
    node->kind = IR_READ;
    node->x    = x;
}

void gen_write(Operand x)
{
    IR node = ir_new_node();
    node->kind = IR_WRITE;
    node->x    = x;
}

/* ---- 打印全部 IR ---- */

void ir_print(FILE *out)
{
    IR p;
    for (p = ir_head; p; p = p->next) {
        switch (p->kind) {
        case IR_LABEL:
            fprintf(out, "LABEL %s :\n", p->x.name);
            break;
        case IR_FUNCTION:
            fprintf(out, "FUNCTION %s :\n", p->name);
            break;
        case IR_ASSIGN:
            operand_print(out, p->x);
            fprintf(out, " := ");
            operand_print(out, p->y);
            fprintf(out, "\n");
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV: {
            const char *op_str = "+";
            if (p->kind == IR_SUB) op_str = "-";
            else if (p->kind == IR_MUL) op_str = "*";
            else if (p->kind == IR_DIV) op_str = "/";
            fprintf(out, "%s := ", p->x.name);
            operand_print(out, p->y);
            fprintf(out, " %s ", op_str);
            operand_print(out, p->z);
            fprintf(out, "\n");
            break;
        }
        case IR_ADDR:
            fprintf(out, "%s := &%s\n", p->x.name, p->y.name);
            break;
        case IR_LOAD:
            fprintf(out, "%s := *%s\n", p->x.name, p->y.name);
            break;
        case IR_STORE:
            fprintf(out, "*%s := ", p->x.name);
            operand_print(out, p->y);
            fprintf(out, "\n");
            break;
        case IR_GOTO:
            fprintf(out, "GOTO %s\n", p->x.name);
            break;
        case IR_IF:
            fprintf(out, "IF ");
            operand_print(out, p->x);
            fprintf(out, " %s ", relop_str(p->relop));
            operand_print(out, p->y);
            fprintf(out, " GOTO %s\n", p->z.name);
            break;
        case IR_RETURN:
            fprintf(out, "RETURN ");
            operand_print(out, p->x);
            fprintf(out, "\n");
            break;
        case IR_DEC:
            fprintf(out, "DEC ");
            operand_print(out, p->x);
            fprintf(out, " %d\n", p->size);
            break;
        case IR_ARG:
            fprintf(out, "ARG ");
            operand_print(out, p->x);
            fprintf(out, "\n");
            break;
        case IR_CALL:
            operand_print(out, p->x);
            fprintf(out, " := CALL %s\n", p->name);
            break;
        case IR_PARAM:
            fprintf(out, "PARAM ");
            operand_print(out, p->x);
            fprintf(out, "\n");
            break;
        case IR_READ:
            fprintf(out, "READ ");
            operand_print(out, p->x);
            fprintf(out, "\n");
            break;
        case IR_WRITE:
            fprintf(out, "WRITE ");
            operand_print(out, p->x);
            fprintf(out, "\n");
            break;
        }
    }
}
