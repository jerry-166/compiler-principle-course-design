/*
 * translate.c - 翻译器顶层结构（实践 3 Task 3）
 *
 * 负责：
 *   A. 轻量符号表（pjw 哈希，每个函数内独立）
 *   B. 类型推导工具（从 sem.c 移植，去掉错误检查）
 *   C. 顶层翻译：Program / ExtDef / FunDec / CompSt / DefList
 *   D. 占位函数：Stmt / Cond / Exp / Args（后续 Task 实现）
 *
 * 重要：addChild 跳过 NULL 子节点，因此语法树中 DefList/StmtList 等
 * 为空时不存在于 children 数组中，不能按固定索引访问，需按名字查找。
 *
 * 兼容 C99 / GCC 7.5.0
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "translate.h"
#include "ir.h"

/* ==================================================================
 * 部分 A：轻量符号表
 * ================================================================== */

#define NHASH 0x3fff
static VarEntry *vtab[NHASH];

/* 标准 PJW 哈希（与 sem.c 一致） */
static unsigned pjw_hash(const char *s)
{
    unsigned h = 0;
    for (; *s; s++) {
        h = (h << 4) + (unsigned char)*s;
        unsigned g = h & 0xf0000000u;
        if (g) {
            h ^= g >> 24;
            h &= ~g;
        }
    }
    return h % NHASH;
}

VarEntry *translate_lookup_var(const char *src_name)
{
    unsigned idx = pjw_hash(src_name);
    VarEntry *e;
    for (e = vtab[idx]; e; e = e->next) {
        if (strcmp(e->src_name, src_name) == 0) return e;
    }
    return NULL;
}

void translate_insert_var(const char *src_name, Type type)
{
    /* 已存在则跳过（假设 4 不重名） */
    if (translate_lookup_var(src_name)) return;
    VarEntry *e = (VarEntry *)malloc(sizeof(VarEntry));
    if (!e) { fprintf(stderr, "translate: out of memory\n"); exit(1); }
    e->src_name = strdup(src_name);
    {
        /* ir_name = "v_" + src_name */
        size_t len = strlen(src_name) + 3;
        char *buf = (char *)malloc(len);
        sprintf(buf, "v_%s", src_name);
        e->ir_name = buf;
    }
    e->type = type;
    unsigned idx = pjw_hash(src_name);
    e->next = vtab[idx];
    vtab[idx] = e;
}

/* ==================================================================
 * 部分 B：类型推导工具（从 sem.c 移植，去掉错误检查）
 * ================================================================== */

/* Specifier -> TYPE  |  StructSpecifier */
Type tr_specifier_to_type(Node *spec)
{
    if (!spec || spec->nchild < 1) return NULL;
    Node *c = spec->children[0];
    if (!c) return NULL;
    if (c->is_token && strcmp(c->name, "TYPE") == 0) {
        return new_basic(strcmp(c->sval, "float") == 0 ? 1 : 0);
    }
    /* StructSpecifier：Task 9 处理结构体。本任务（必做）用例不含结构体变量，
       但可能出现 ExtDef -> Specifier SEMI 的纯结构体定义（不产生 IR，
       translate_extdef 会跳过）。这里返回 new_basic(0) 占位，避免崩溃。
       Task 9 替换为真正的结构体类型构造。 */
    return new_basic(0);
}

/* 取 VarDec 最内层 ID 节点 */
Node *tr_get_vardec_id(Node *vardec)
{
    Node *p = vardec;
    while (p && !p->is_token) {
        int k;
        for (k = 0; k < p->nchild; k++) {
            Node *ck = p->children[k];
            if (ck && ck->is_token && strcmp(ck->name, "ID") == 0) return ck;
        }
        p = p->children[0];
    }
    return NULL;
}

/* VarDec -> ID  |  VarDec LB INT RB（nchild==4）
   递归：最外层是第一维。int a[10][3] -> array(10, array(3, int))。 */
Type tr_vardec_to_type(Node *vardec, Type base)
{
    if (!vardec) return base;
    if (vardec->nchild == 4) {
        Node *inner = vardec->children[0];
        Node *sz = vardec->children[2];
        int size = sz ? (int)sz->ival : 0;
        Type inner_t = tr_vardec_to_type(inner, base);
        return new_array(inner_t, size);
    }
    return base;
}

/* ==================================================================
 * 部分 C：顶层翻译
 * ================================================================== */

void translate_program(Node *root)
{
    if (!root) return;
    Node *extdeflist = root->children[0];
    while (extdeflist && extdeflist->nchild >= 1 && extdeflist->children[0]) {
        translate_extdef(extdeflist->children[0]);
        extdeflist = (extdeflist->nchild >= 2) ? extdeflist->children[1] : NULL;
    }
}

void translate_extdef(Node *extdef)
{
    if (!extdef || extdef->nchild < 2) return;
    Node *second = extdef->children[1];
    if (second && strcmp(second->name, "FunDec") == 0) {
        translate_fundef(extdef);
    }
    /* Specifier SEMI（结构体定义）/ Specifier ExtDecList SEMI（全局变量）：
       假设 4 不出现，跳过 */
}

/* 本地辅助：递归计算类型占用字节数。
   BASIC = 4；ARRAY = elem_size * size；STRUCTURE 由 Task 9 完善。
   后续 Task 7 会被 translate_addr.c 的正式 type_size 替代。 */
static int compute_size_simple(Type t)
{
    if (!t) return 4;
    switch (t->kind) {
    case BASIC:
        return 4;
    case ARRAY:
        return compute_size_simple(t->u.array.elem) * t->u.array.size;
    case STRUCTURE:
        /* Task 9 处理，目前简化为 0 */
        return 0;
    }
    return 4;
}

void translate_fundef(Node *extdef)
{
    Node *fundec = extdef->children[1];
    Node *compst = extdef->children[2];
    Node *idnode = fundec->children[0];  /* ID */
    gen_function(idnode->sval);

    /* 形参：填表 + PARAM v_<name> */
    if (fundec->nchild == 4) {  /* ID LP VarList RP */
        Node *varlist = fundec->children[2];
        while (varlist) {
            Node *paramdec = varlist->children[0];  /* ParamDec -> Specifier VarDec */
            Node *pspec = paramdec->children[0];
            Node *pvardec = paramdec->children[1];
            Node *pid = tr_get_vardec_id(pvardec);
            Type ptype = tr_vardec_to_type(pvardec, tr_specifier_to_type(pspec));
            translate_insert_var(pid->sval, ptype);
            gen_param(new_var(pid->sval));
            varlist = (varlist->nchild == 3) ? varlist->children[2] : NULL;
        }
    }

    translate_compst(compst);
}

void translate_compst(Node *compst)
{
    /* CompSt -> LC DefList StmtList RC
       注意：addChild 跳过 NULL 子节点。当 DefList 为空时返回 NULL，
       CompSt 的 children 中不含 DefList 节点，索引会偏移。
       因此不能按固定索引取，必须按名字查找。 */
    Node *deflist = NULL;
    Node *stmtlist = NULL;
    int i;
    for (i = 0; i < compst->nchild; i++) {
        Node *c = compst->children[i];
        if (!c) continue;
        if (strcmp(c->name, "DefList") == 0) deflist = c;
        else if (strcmp(c->name, "StmtList") == 0) stmtlist = c;
    }

    /* DefList：局部变量。数组输出 DEC v_<name> <size> */
    while (deflist && deflist->nchild >= 1 && deflist->children[0]) {
        Node *def = deflist->children[0];  /* Def -> Specifier DecList SEMI */
        Node *spec = def->children[0];
        Node *declist = def->children[1];
        Type base = tr_specifier_to_type(spec);
        while (declist && declist->nchild >= 1 && declist->children[0]) {
            Node *dec = declist->children[0];  /* Dec -> VarDec [ASSIGNOP Exp] */
            Node *vardec = dec->children[0];
            Node *idnode = tr_get_vardec_id(vardec);
            Type vtype = tr_vardec_to_type(vardec, base);
            translate_insert_var(idnode->sval, vtype);
            /* 数组输出 DEC；普通变量不需要 DEC */
            if (vtype && vtype->kind == ARRAY) {
                int sz = compute_size_simple(vtype);
                gen_dec(new_var(idnode->sval), sz);
            }
            /* STRUCTURE: Task 9 处理，必做无结构体变量，跳过 */
            declist = (declist->nchild == 3) ? declist->children[2] : NULL;
        }
        deflist = (deflist->nchild >= 2) ? deflist->children[1] : NULL;
    }

    /* StmtList：逐个翻译 */
    while (stmtlist && stmtlist->nchild >= 1 && stmtlist->children[0]) {
        translate_stmt(stmtlist->children[0]);
        stmtlist = (stmtlist->nchild >= 2) ? stmtlist->children[1] : NULL;
    }
}

/* ==================================================================
 * 部分 D：控制流语句 / 条件表达式翻译（Task 5，PDF 表 4.2 / 4.3）
 *
 *   translate_stmt  : Stmt 节点 → IR 控制流
 *   translate_cond  : 条件表达式 → 真假跳转（短路求值）
 *
 * translate_exp 的真正实现在 translate_exp.c（本文件末尾 extern 声明）。
 * ================================================================== */

void translate_stmt(Node *stmt)
{
    if (!stmt || stmt->nchild < 1) return;
    Node *c0 = stmt->children[0];

    /* Stmt -> Exp SEMI：表达式语句，结果丢弃（哨兵） */
    if (!c0->is_token && strcmp(c0->name, "Exp") == 0) {
        Operand none;
        none.kind = OP_CONST;
        none.name = NULL;
        none.value = 0;
        translate_exp(c0, none);
        return;
    }

    /* Stmt -> CompSt：复合语句，进入新作用域（变量在 compst 内填表） */
    if (!c0->is_token && strcmp(c0->name, "CompSt") == 0) {
        translate_compst(c0);
        return;
    }

    /* Stmt -> RETURN Exp SEMI */
    if (c0->is_token && strcmp(c0->name, "RETURN") == 0) {
        Operand t = new_temp();
        translate_exp(stmt->children[1], t);
        gen_return(t);
        return;
    }

    /* Stmt -> IF LP Exp RP Stmt  或  IF LP Exp RP Stmt ELSE Stmt */
    if (c0->is_token && strcmp(c0->name, "IF") == 0) {
        Operand l1 = new_label();   /* 条件为真入口 */
        Operand l2 = new_label();   /* 条件为假入口（无 else = 整体出口） */
        translate_cond(stmt->children[2], l1, l2);
        gen_label(l1);
        translate_stmt(stmt->children[4]);
        if (stmt->nchild == 5) {
            /* 无 ELSE：假分支直接落到出口 l2 */
            gen_label(l2);
        } else {
            /* IF LP Exp RP Stmt ELSE Stmt（nchild==7）：
               then 分支结束后跳过 else，故加 l3 作整体出口 */
            Operand l3 = new_label();
            gen_goto(l3);
            gen_label(l2);
            translate_stmt(stmt->children[6]);
            gen_label(l3);
        }
        return;
    }

    /* Stmt -> WHILE LP Exp RP Stmt */
    if (c0->is_token && strcmp(c0->name, "WHILE") == 0) {
        Operand l1 = new_label();   /* 循环头 */
        Operand l2 = new_label();   /* 循环体入口 */
        Operand l3 = new_label();   /* 循环退出 */
        gen_label(l1);
        translate_cond(stmt->children[2], l2, l3);
        gen_label(l2);
        translate_stmt(stmt->children[4]);
        gen_goto(l1);
        gen_label(l3);
        return;
    }

    /* 其他未知语句形式：不处理 */
}

void translate_cond(Node *exp, Operand l_true, Operand l_false)
{
    if (!exp || exp->nchild < 1) return;

    /* Exp -> Exp RELOP Exp */
    if (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "RELOP") == 0) {
        Operand t1 = new_temp(), t2 = new_temp();
        translate_exp(exp->children[0], t1);
        translate_exp(exp->children[2], t2);
        const char *r = exp->children[1]->sval;
        Relop rk;
        if      (strcmp(r, "<")  == 0) rk = RELOP_LT;
        else if (strcmp(r, "<=") == 0) rk = RELOP_LE;
        else if (strcmp(r, ">")  == 0) rk = RELOP_GT;
        else if (strcmp(r, ">=") == 0) rk = RELOP_GE;
        else if (strcmp(r, "==") == 0) rk = RELOP_EQ;
        else                            rk = RELOP_NE;  /* "!=" */
        gen_if(t1, rk, t2, l_true);
        gen_goto(l_false);
        return;
    }

    /* Exp -> NOT Exp：真假标号互换 */
    if (exp->children[0]->is_token && strcmp(exp->children[0]->name, "NOT") == 0) {
        translate_cond(exp->children[1], l_false, l_true);
        return;
    }

    /* Exp -> Exp AND Exp：短路求值，左侧假直接跳 l_false，否则求右侧 */
    if (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "AND") == 0) {
        Operand l = new_label();
        translate_cond(exp->children[0], l, l_false);
        gen_label(l);
        translate_cond(exp->children[2], l_true, l_false);
        return;
    }

    /* Exp -> Exp OR Exp：短路求值，左侧真直接跳 l_true，否则求右侧 */
    if (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "OR") == 0) {
        Operand l = new_label();
        translate_cond(exp->children[0], l_true, l);
        gen_label(l);
        translate_cond(exp->children[2], l_true, l_false);
        return;
    }

    /* 其他（数值表达式当条件）：IF t != #0 GOTO l_true; GOTO l_false */
    {
        Operand t = new_temp();
        translate_exp(exp, t);
        gen_if(t, RELOP_NE, new_const(0), l_true);
        gen_goto(l_false);
    }
}

void translate_args(Node *args, Operand *arg_list, int *arg_count) {
    /* Args -> Exp COMMA Args  |  Exp
       正序：当前 Exp 放 arg_list[0]，后面 Args 的结果放 arg_list[1..]。
       返回时 *arg_count = 实参总数。
       调用方需分配足够大的数组（固定 32，C89 兼容）。*/
    Operand t = new_temp();
    translate_exp(args->children[0], t);
    if (args->nchild == 1) {
        arg_list[0] = t;
        *arg_count = 1;
    } else {
        int rest;
        translate_args(args->children[2], arg_list + 1, &rest);
        arg_list[0] = t;
        *arg_count = rest + 1;
    }
}
