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

/* Task 9：结构体 DefList → FieldList（保持声明顺序，尾插）*/
FieldList tr_collect_fields(Node *deflist);

/* Task 9：通用左值地址翻译（ID / 数组下标 / 结构体域，任意嵌套）。
   返回地址 Operand（OP_TEMP），out_type 输出元素/域类型。*/
Operand translate_addr_general(Node *exp, Type *out_type);

/* Task 9：Cannot translate prepass。发现结构体变量/参数且未开
   ENABLE_STRUCT 宏时，置全局 cannot_translate=1。*/
void prepass_check(Node *root);

/* Task 9：全局标志，1 表示不可翻译（被 prepass_check 置位）*/
extern int cannot_translate;

#endif /* TRANSLATE_H */
