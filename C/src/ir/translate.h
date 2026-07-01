/*
 * translate.h - 翻译主入口与对外接口（实践 3 中间代码生成）
 */
#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "node.h"
#include "type.h"
#include "operand.h"

/* 主入口：遍历 Program -> ExtDefList */
void translate_program(Node *root);

/* 顶层结构 */
void translate_extdef(Node *extdef);
void translate_fundef(Node *extdef);   /* Specifier FunDec CompSt */
void translate_compst(Node *compst);

/* 语句（Task 5 实现，本任务先放占位） */
void translate_stmt(Node *stmt);
/* 条件（Task 5 实现，本任务先放占位） */
void translate_cond(Node *exp, Operand l_true, Operand l_false);
/* 表达式（Task 4 实现，本任务先放占位） */
void translate_exp(Node *exp, Operand place);
/* 函数调用参数（Task 6 实现，本任务先放占位） */
void translate_args(Node *args, Operand *arg_list, int *arg_count);

/* 轻量符号表（src_name -> ir_name + type） */
typedef struct VarEntry {
    char *src_name;
    char *ir_name;     /* "v_a" */
    Type  type;        /* 数组/结构体寻址用；普通 int 是 new_basic(0) */
    struct VarEntry *next;
} VarEntry;

VarEntry *translate_lookup_var(const char *src_name);
void      translate_insert_var(const char *src_name, Type type);

/* 从 sem.c 复制的类型推导工具（translate.c 内定义，非 static 供其他文件用） */
Type   tr_specifier_to_type(Node *spec);
Type   tr_vardec_to_type(Node *vardec, Type base);
Node  *tr_get_vardec_id(Node *vardec);

#endif /* TRANSLATE_H */
