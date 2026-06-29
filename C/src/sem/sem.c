/*
 * sem.c - 语义分析实现（实践 2）
 * 后序遍历语法树，遇 ExtDef/Def 填符号表，遇 Exp 查表+类型检查。
 * 对应 Project_2.pdf 3.2.5。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "sem.h"
#include "type.h"
#include "symtab.h"

/* ===== 前置声明（互相递归）===== */
static Node *get_vardec_id(Node *vardec);
static Type vardec_to_type(Node *vardec, Type base);
static FieldList collect_fields(Node *deflist);
static Type struct_specifier_type(Node *ss);
static Type specifier_to_type(Node *spec);
static void report_undeclared(Symbol s, void *arg);
static void visit_extdef(Node *extdef);
static void visit_def(Node *def, int is_global);
static Type check_exp(Node *exp);
static void visit_stmt(Node *stmt);
static void visit_compst(Node *compst, Type ret);
static void visit_declist(Node *dl, Type base, int is_global);
static void visit_fundef(Node *spec, Node *fundec, Node *compst);
static void visit_fundecl(Node *spec, Node *fundec);

/* ===== 错误打印 =====
   说明文字照搬 Tests2 .exp（逐字）。*/
static void sem_error(int type, int lineno, const char *msg)
{
    out("Error type %d at Line %d: %s\n", type, lineno, msg);
}

/* 当前函数返回类型（return 检查用）。NULL=不在函数体内。*/
static Type current_return_type = NULL;

/* ===== 入口 ===== */
void sem_analyze(Node *root)
{
    if (!root) return;
    symtab_init();
    /* Program → ExtDefList */
    Node *edl = (root->nchild > 0) ? root->children[0] : NULL;
    while (edl && edl->nchild >= 1) {
        Node *extdef = edl->children[0];
        if (extdef) visit_extdef(extdef);
        edl = (edl->nchild >= 2) ? edl->children[1] : NULL;
    }
#ifdef ENABLE_FUNC_DECL
    /* 要求3.1：扫描未定义的函数声明（错误18）*/
    symtab_for_undeclared(report_undeclared, NULL);
#endif
}

#ifdef ENABLE_FUNC_DECL
static void report_undeclared(Symbol s, void *arg)
{
    /* 错误18 文字与 E-1.exp 一致：Undefined function. */
    sem_error(18, s->lineno, "Undefined function.");
    (void)arg;
}
#endif

/* ===== Specifier → Type ===== */

/* StructSpecifier 处理：
   形式1: STRUCT OptTag LC DefList RC  —— 结构体【定义】，注册结构体名，返回新 Type
   形式2: STRUCT Tag                    —— 结构体【引用】，查表返回已注册 Type
   返回 NULL 表示定义出错或引用未定义（调用方报错）。*/
static Type struct_specifier_type(Node *ss)
{
    if (!ss) return NULL;
    Node *second = (ss->nchild >= 2) ? ss->children[1] : NULL;
    if (ss->nchild == 5) {
        /* 定义：STRUCT OptTag LC DefList RC */
        Node *deflist = ss->children[3];
        FieldList fields = collect_fields(deflist);
        Type t = new_structure(fields);
        /* 注册结构体名（OptTag→ID） */
        if (second && second->nchild > 0) {
            Node *idnode = second->children[0];
            if (idnode && idnode->is_token && strcmp(idnode->name, "ID") == 0) {
                if (symtab_lookup(idnode->sval, -1)) {
                    /* 名字已被占用：错误16 */
                    sem_error(16, idnode->lineno, "Duplicated name.");
                }
                Symbol sym = (Symbol)calloc(1, sizeof(struct Symbol_));
                size_t len = strlen(idnode->sval);
                sym->name = (char *)malloc(len + 1);
                memcpy(sym->name, idnode->sval, len + 1);
                sym->kind = SYM_STRUCT;
                sym->type = t;
                sym->lineno = idnode->lineno;
                symtab_insert(sym);
            }
        }
        return t;
    } else if (ss->nchild == 2) {
        /* 引用：STRUCT Tag，Tag → ID */
        Node *idnode = (second && second->nchild > 0) ? second->children[0] : NULL;
        if (idnode && idnode->is_token && strcmp(idnode->name, "ID") == 0) {
            Symbol s = symtab_lookup(idnode->sval, SYM_STRUCT);
            if (!s) {
                /* 未定义结构体：错误17（A-17.exp 文字 Undefined structure3.）*/
                sem_error(17, idnode->lineno, "Undefined structure3.");
                return NULL;
            }
            return s->type;
        }
    }
    return NULL;
}

static Type specifier_to_type(Node *spec)
{
    if (!spec || spec->nchild < 1) return NULL;
    Node *c = spec->children[0];
    if (!c) return NULL;
    if (c->is_token && strcmp(c->name, "TYPE") == 0) {
        return new_basic(strcmp(c->sval, "float") == 0 ? 1 : 0);
    }
    if (!c->is_token && strcmp(c->name, "StructSpecifier") == 0) {
        return struct_specifier_type(c);
    }
    return NULL;
}

/* 取 VarDec 最内层 ID 节点（用于行号/名字）*/
static Node *get_vardec_id(Node *vardec)
{
    Node *p = vardec;
    while (p && !p->is_token) {
        int k;
        for (k = 0; k < p->nchild; k++) {
            Node *ck = p->children[k];
            if (ck && ck->is_token && strcmp(ck->name, "ID") == 0) {
                return ck;
            }
        }
        p = p->children[0];
    }
    return NULL;
}

/* VarDec → ID  |  VarDec LB INT RB
   递归处理：最外层是第一维。int a[10][3] → array(10, array(3, int))。*/
static Type vardec_to_type(Node *vardec, Type base)
{
    if (!vardec) return base;
    if (vardec->nchild == 4) {
        Node *inner = vardec->children[0];
        Node *sz = vardec->children[2];
        int size = sz ? (int)sz->ival : 0;
        Type inner_t = vardec_to_type(inner, base);
        return new_array(inner_t, size);
    }
    return base;
}

/* 从 DefList 收集结构体域。
   每条 Def → Specifier DecList SEMI；DecList→Dec...；Dec→VarDec (ASSIGNOP Exp)?
   返回域链表（头插）。错误15：域重名 或 域被初始化。*/
static FieldList collect_fields(Node *deflist)
{
    FieldList head = NULL;
    while (deflist && deflist->nchild >= 1) {
        Node *def = deflist->children[0];
        if (!def || def->nchild < 2) {
            deflist = (deflist->nchild >= 2) ? deflist->children[1] : NULL;
            continue;
        }
        Node *spec = def->children[0];
        Node *declist = def->children[1];
        Type base = specifier_to_type(spec);
        Node *dl = declist;
        while (dl && dl->nchild >= 1) {
            Node *dec = dl->children[0];
            if (!dec || dec->nchild < 1) {
                dl = (dl->nchild >= 3) ? dl->children[2] : NULL;
                continue;
            }
            Node *vardec = dec->children[0];
            if (dec->nchild == 3) {
                /* 域初始化：错误15（A-18.exp 文字 Field can't be inited.）*/
                Node *idnode = get_vardec_id(vardec);
                sem_error(15, idnode ? idnode->lineno : vardec->lineno,
                          "Field can't be inited.");
            }
            char *name = NULL;
            Node *idn = get_vardec_id(vardec);
            if (idn) name = idn->sval;
            Type ftype = vardec_to_type(vardec, base);
            /* 域重名检查（错误15：Redefined field.）*/
            int dup = 0;
            if (name) {
                FieldList f = head;
                while (f) {
                    if (strcmp(f->name, name) == 0) {
                        sem_error(15, idn ? idn->lineno : vardec->lineno,
                                  "Redefined field.");
                        dup = 1;
                        break;
                    }
                    f = f->tail;
                }
            }
            if (name && !dup) {
                head = new_field(name, ftype, head);
            } else {
                head = new_field("__err__", ftype ? ftype : new_basic(0), head);
            }
            dl = (dl->nchild >= 3) ? dl->children[2] : NULL;
        }
        deflist = (deflist->nchild >= 2) ? deflist->children[1] : NULL;
    }
    return head;
}

/* ===== visit_extdef：处理 ExtDef 四种形式 =====
   形式1: Specifier ExtDecList SEMI（全局变量）
   形式2: Specifier SEMI（仅结构体定义，副作用在 specifier_to_type 完成）
   形式3: Specifier FunDec CompSt（函数定义）
   形式4(选做3.1): Specifier FunDec SEMI（函数声明）*/
static void visit_extdef(Node *extdef)
{
    if (!extdef || extdef->nchild < 1) return;
    Node *spec = extdef->children[0];
    Node *second = (extdef->nchild >= 2) ? extdef->children[1] : NULL;

    /* 先把 Specifier 副作用触发（结构体定义要在变量前注册）*/
    Type base = specifier_to_type(spec);

    if (second && !second->is_token && strcmp(second->name, "ExtDecList") == 0) {
        visit_declist(second, base, 1);
    } else if (second && !second->is_token && strcmp(second->name, "FunDec") == 0) {
        if (extdef->nchild == 3) {
            Node *third = extdef->children[2];
            if (third && !third->is_token && strcmp(third->name, "CompSt") == 0) {
                visit_fundef(spec, second, third);
            } else {
#ifdef ENABLE_FUNC_DECL
                visit_fundecl(spec, second);
#else
                (void)base;
#endif
            }
        }
    }
    (void)base;
}

/* ===== 占位（Task 6/7 替换）===== */
static void visit_def(Node *def, int is_global) { (void)def; (void)is_global; }
static void visit_declist(Node *dl, Type base, int is_global) { (void)dl; (void)base; (void)is_global; }
static void visit_fundef(Node *spec, Node *fundec, Node *compst) { (void)spec; (void)fundec; (void)compst; }
static void visit_fundecl(Node *spec, Node *fundec) { (void)spec; (void)fundec; }
static void visit_stmt(Node *stmt) { (void)stmt; }
static Type check_exp(Node *exp) { (void)exp; return NULL; }
static void visit_compst(Node *compst, Type ret) { (void)compst; (void)ret; }
