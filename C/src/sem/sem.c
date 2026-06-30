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
static int  is_lvalue(Node *exp);
static Node *array_base_id(Node *exp);
static void visit_stmt(Node *stmt);
static void visit_compst(Node *compst, Type ret);
static void visit_declist(Node *dl, Type base, int is_global);
static void visit_extdeclist(Node *edl, Type base);
static void visit_fundef(Node *spec, Node *fundec, Node *compst);
static void visit_fundecl(Node *spec, Node *fundec);
static void prepass_extdef(Node *extdef);

/* ===== 错误打印 =====
   说明文字照搬 Tests2 .exp（逐字）。*/
static void sem_error(int type, int lineno, const char *msg)
{
    out("Error type %d at Line %d: %s\n", type, lineno, msg);
}

/* 当前函数返回类型（return 检查用）。NULL=不在函数体内。*/
static Type current_return_type = NULL;

/* 语法嵌套深度（仅用于错误3 文字区分 A-3 vs D-2）。
   进函数体 = 1，进 Stmt→CompSt 嵌套块再 +1。*/
static int nest_depth = 0;

#ifdef ENABLE_FUNC_DECL
/* 要求3.1：先扫一遍 ExtDefList，标记所有会被定义的函数名。
   详见 sem_analyze 和 visit_fundecl。*/
#define MAX_DEFINED_FUNCS 256
static char *defined_func_names[MAX_DEFINED_FUNCS];
static int   defined_func_count = 0;

static void mark_defined(const char *name)
{
    int i;
    for (i = 0; i < defined_func_count; i++)
        if (strcmp(defined_func_names[i], name) == 0) return;
    if (defined_func_count < MAX_DEFINED_FUNCS) {
        size_t len = strlen(name);
        char *cp = (char *)malloc(len + 1);
        memcpy(cp, name, len + 1);
        defined_func_names[defined_func_count++] = cp;
    }
}

static int func_will_be_defined(const char *name)
{
    int i;
    for (i = 0; i < defined_func_count; i++)
        if (strcmp(defined_func_names[i], name) == 0) return 1;
    return 0;
}

static void prepass_extdef(Node *extdef)
{
    if (!extdef || extdef->nchild < 3) return;
    Node *second = extdef->children[1];
    Node *third  = extdef->children[2];
    if (!second || second->is_token || strcmp(second->name, "FunDec") != 0) return;
    if (!third  || third->is_token  || strcmp(third->name,  "CompSt") != 0) return;
    Node *idn = (second->nchild >= 1) ? second->children[0] : NULL;
    if (idn && idn->is_token && strcmp(idn->name, "ID") == 0) {
        mark_defined(idn->sval);
    }
}
#endif

/* ===== 入口 ===== */
void sem_analyze(Node *root)
{
    if (!root) return;
    symtab_init();
#ifdef ENABLE_FUNC_DECL
    /* 要求3.1：先扫一遍 ExtDefList，标记所有会被定义的函数名。
       这样主遍历中遇到函数声明时，能就地判定"最终是否被定义"，
       按声明位置立即报错误18（保证行号顺序与其他错误一致）。*/
    Node *edl0 = (root->nchild > 0) ? root->children[0] : NULL;
    while (edl0 && edl0->nchild >= 1) {
        Node *extdef = edl0->children[0];
        if (extdef) prepass_extdef(extdef);
        edl0 = (edl0->nchild >= 2) ? edl0->children[1] : NULL;
    }
#endif
    /* Program → ExtDefList 正式遍历 */
    Node *edl = (root->nchild > 0) ? root->children[0] : NULL;
    while (edl && edl->nchild >= 1) {
        Node *extdef = edl->children[0];
        if (extdef) visit_extdef(extdef);
        edl = (edl->nchild >= 2) ? edl->children[1] : NULL;
    }
}

/* ===== Specifier → Type ===== */

/* StructSpecifier 处理：
   形式1: STRUCT OptTag LC DefList RC  —— 结构体【定义】，注册结构体名，返回新 Type
   形式2: STRUCT Tag                    —— 结构体【引用】，查表返回已注册 Type
   返回 NULL 表示定义出错或引用未定义（调用方报错）。*/
static Type struct_specifier_type(Node *ss)
{
    if (!ss) return NULL;
    /* STRUCT OptTag LC DefList RC  或  STRUCT Tag
       DefList 为空时会被 addChild 跳过，所以按节点名识别。*/
    Node *opttag_or_tag = NULL;
    Node *deflist = NULL;
    int i;
    int has_lc = 0;
    for (i = 0; i < ss->nchild; i++) {
        Node *c = ss->children[i];
        if (!c) continue;
        if (c->is_token) {
            if (strcmp(c->name, "LC") == 0) has_lc = 1;
            continue;
        }
        if (strcmp(c->name, "OptTag") == 0 || strcmp(c->name, "Tag") == 0)
            opttag_or_tag = c;
        else if (strcmp(c->name, "DefList") == 0)
            deflist = c;
    }
    if (has_lc) {
        /* 定义：STRUCT OptTag LC DefList RC */
        FieldList fields = collect_fields(deflist);
        Type t = new_structure(fields);
        /* 注册结构体名（OptTag→ID） */
        if (opttag_or_tag && opttag_or_tag->nchild > 0) {
            Node *idnode = opttag_or_tag->children[0];
            if (idnode && idnode->is_token && strcmp(idnode->name, "ID") == 0) {
                if (symtab_lookup(idnode->sval, -1)) {
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
    } else {
        /* 引用：STRUCT Tag，Tag → ID */
        if (opttag_or_tag && opttag_or_tag->nchild > 0) {
            Node *idnode = opttag_or_tag->children[0];
            if (idnode && idnode->is_token && strcmp(idnode->name, "ID") == 0) {
                Symbol s = symtab_lookup(idnode->sval, SYM_STRUCT);
                if (!s) {
                    sem_error(17, idnode->lineno, "Undefined structure3.");
                    return NULL;
                }
                return s->type;
            }
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
        visit_extdeclist(second, base);
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

/* ===== 变量/函数定义填表 + 语句遍历（Task 6）===== */

/* 把一个 VarDec 对应的变量填入当前作用域符号表。
   错误3 文字按 depth 区分：必做模式下 depth>=2 的嵌套块用 D-2 措辞，
   否则用 A-3 措辞。*/
static void insert_var(Node *vardec, Type base)
{
    Type t = vardec_to_type(vardec, base);
    Node *idn = get_vardec_id(vardec);
    if (!idn) return;
    Symbol exist = symtab_lookup(idn->sval, -1);
    if (exist && exist->depth == symtab_cur_depth()) {
        const char *msg;
#ifndef ENABLE_SCOPE
        /* 必做模式：嵌套块内（nest_depth>=2）重定义外层变量 → D-2 措辞；
           函数体顶层（nest_depth==1）同作用域重定义 → A-3 措辞。*/
        msg = (nest_depth >= 2) ? "Redefined variable or structure conflict."
                                : "Redefined Variable.";
#else
        msg = "Redefined Variable.";
#endif
        sem_error(3, idn->lineno, msg);
        return;
    }
    Symbol sym = (Symbol)calloc(1, sizeof(struct Symbol_));
    size_t len = strlen(idn->sval);
    sym->name = (char *)malloc(len + 1);
    memcpy(sym->name, idn->sval, len + 1);
    sym->kind = SYM_VAR;
    sym->type = t;
    sym->lineno = idn->lineno;
    symtab_insert(sym);
}

/* 遍历 DecList（局部变量定义，Dec → VarDec | VarDec ASSIGNOP Exp）。*/
static void visit_declist(Node *dl, Type base, int is_global)
{
    (void)is_global;
    while (dl && dl->nchild >= 1) {
        Node *dec = dl->children[0];
        if (!dec || dec->nchild < 1) {
            dl = (dl->nchild >= 3) ? dl->children[2] : NULL;
            continue;
        }
        Node *vardec = dec->children[0];
        insert_var(vardec, base);
        if (dec->nchild == 3) {
            /* VarDec ASSIGNOP Exp：局部变量初始化，遍历触发右边表达式检查 */
            (void)check_exp(dec->children[2]);
        }
        dl = (dl->nchild >= 3) ? dl->children[2] : NULL;
    }
}

/* 遍历 ExtDecList（全局变量定义，直接是 VarDec 链表，无 Dec 包装）。*/
static void visit_extdeclist(Node *edl, Type base)
{
    while (edl && edl->nchild >= 1) {
        Node *vardec = edl->children[0];
        if (vardec) insert_var(vardec, base);
        edl = (edl->nchild >= 3) ? edl->children[2] : NULL;
    }
}

/* 签名匹配：返回类型 + 参数个数 + 逐参数 type_equal。错误19 用。*/
static int func_signature_match(Symbol sym, Type ret, FieldList params, int nparam)
{
    if (sym->nparam != nparam) return 0;
    if (!type_equal(sym->type, ret)) return 0;
    FieldList a = sym->params, b = params;
    while (a && b) {
        if (!type_equal(a->type, b->type)) return 0;
        a = a->tail; b = b->tail;
    }
    return 1;
}

/* 收集 FunDec 的形参为 FieldList（同时返回个数）。
   FunDec → ID LP VarList RP  |  ID LP RP
   VarList → ParamDec COMMA VarList | ParamDec
   ParamDec → Specifier VarDec */
static FieldList collect_params(Node *fundec, int *nparam)
{
    *nparam = 0;
    FieldList head = NULL;
    if (!fundec || fundec->nchild != 4) return head;
    /* ID LP VarList RP */
    Node *vl = fundec->children[2];
    while (vl && vl->nchild >= 1) {
        Node *paramdec = vl->children[0];
        if (paramdec && paramdec->nchild >= 2) {
            Type base = specifier_to_type(paramdec->children[0]);
            Type t = vardec_to_type(paramdec->children[1], base);
            Node *idn = get_vardec_id(paramdec->children[1]);
            if (idn) head = new_field(idn->sval, t, head);
            (*nparam)++;
        }
        vl = (vl->nchild >= 3) ? vl->children[2] : NULL;
    }
    return head;
}

static void visit_fundef(Node *spec, Node *fundec, Node *compst)
{
    Type ret_type = specifier_to_type(spec);
    Node *idn = (fundec && fundec->nchild >= 1) ? fundec->children[0] : NULL;
    if (!idn || !idn->is_token) return;
    int nparam;
    FieldList params = collect_params(fundec, &nparam);

    int proceed = 1;   /* 是否继续进入函数体 */
    Symbol exist = symtab_lookup(idn->sval, SYM_FUNC);
    if (exist) {
        if (exist->defined) {
            /* 已定义过：错误4 */
            sem_error(4, idn->lineno, "Redefined Function.");
            proceed = 0;
        } else {
#ifdef ENABLE_FUNC_DECL
            /* 之前声明过，现在定义：检查一致性（错误19）*/
            if (!func_signature_match(exist, ret_type, params, nparam)) {
                sem_error(19, idn->lineno, "Inconsistent declaration.");
            }
            exist->defined = 1;
#else
            sem_error(4, idn->lineno, "Redefined Function.");
            proceed = 0;
#endif
        }
    } else {
        if (symtab_lookup(idn->sval, -1)) {
            /* 名字被其他 kind 占用：按错误4 */
            sem_error(4, idn->lineno, "Redefined Function.");
            proceed = 0;
        } else {
            Symbol sym = (Symbol)calloc(1, sizeof(struct Symbol_));
            size_t len = strlen(idn->sval);
            sym->name = (char *)malloc(len + 1);
            memcpy(sym->name, idn->sval, len + 1);
            sym->kind = SYM_FUNC;
            sym->type = ret_type;
            sym->params = params;
            sym->nparam = nparam;
            sym->defined = 1;
            sym->lineno = idn->lineno;
            symtab_insert(sym);
        }
    }

    if (!proceed) return;

    /* 进入函数体作用域，形参填表，遍历 CompSt */
    symtab_enter_scope();
    int saved_nest = nest_depth;
    nest_depth = 1;
    FieldList p = params;
    while (p) {
        Symbol psym = (Symbol)calloc(1, sizeof(struct Symbol_));
        size_t len = strlen(p->name);
        psym->name = (char *)malloc(len + 1);
        memcpy(psym->name, p->name, len + 1);
        psym->kind = SYM_VAR;
        psym->type = p->type;
        psym->lineno = idn->lineno;
        symtab_insert(psym);
        p = p->tail;
    }
    current_return_type = ret_type;
    visit_compst(compst, ret_type);
    current_return_type = NULL;
    nest_depth = saved_nest;
    symtab_leave_scope();
}

#ifdef ENABLE_FUNC_DECL
static void visit_fundecl(Node *spec, Node *fundec)
{
    Type ret_type = specifier_to_type(spec);
    Node *idn = (fundec && fundec->nchild >= 1) ? fundec->children[0] : NULL;
    if (!idn || !idn->is_token) return;
    int nparam;
    FieldList params = collect_params(fundec, &nparam);
    Symbol exist = symtab_lookup(idn->sval, SYM_FUNC);
    if (exist) {
        /* 之前声明过或定义过：检查一致（错误19）*/
        if (!func_signature_match(exist, ret_type, params, nparam)) {
            sem_error(19, idn->lineno, "Inconsistent declaration.");
        }
    } else {
        if (symtab_lookup(idn->sval, -1)) {
            sem_error(4, idn->lineno, "Redefined Function.");
            return;
        }
        Symbol sym = (Symbol)calloc(1, sizeof(struct Symbol_));
        size_t len = strlen(idn->sval);
        sym->name = (char *)malloc(len + 1);
        memcpy(sym->name, idn->sval, len + 1);
        sym->kind = SYM_FUNC;
        sym->type = ret_type;
        sym->params = params;
        sym->nparam = nparam;
        sym->declared_only = 1;
        sym->defined = 0;
        sym->lineno = idn->lineno;
        symtab_insert(sym);
        /* 错误18：声明但全文件中从未定义 → 就地按声明行号报（保证行号顺序）*/
        if (!func_will_be_defined(idn->sval)) {
            sem_error(18, idn->lineno, "Undefined function.");
        }
    }
}
#else
static void visit_fundecl(Node *spec, Node *fundec) { (void)spec; (void)fundec; }
#endif

/* CompSt → LC DefList StmtList RC
   注意：DefList 或 StmtList 为空时 parser 返回 NULL，addChild 不加空槽位，
   导致 children 索引不固定。这里按节点名识别。*/
static void visit_compst(Node *compst, Type ret)
{
    if (!compst) return;
    Node *deflist = NULL, *stmtlist = NULL;
    int i;
    for (i = 0; i < compst->nchild; i++) {
        Node *c = compst->children[i];
        if (!c || c->is_token) continue;
        if (strcmp(c->name, "DefList") == 0) deflist = c;
        else if (strcmp(c->name, "StmtList") == 0) stmtlist = c;
    }
    while (deflist && deflist->nchild >= 1) {
        Node *def = deflist->children[0];
        if (def && def->nchild >= 2) {
            Type base = specifier_to_type(def->children[0]);
            visit_declist(def->children[1], base, 0);
        }
        deflist = (deflist->nchild >= 2) ? deflist->children[1] : NULL;
    }
    while (stmtlist && stmtlist->nchild >= 1) {
        visit_stmt(stmtlist->children[0]);
        stmtlist = (stmtlist->nchild >= 2) ? stmtlist->children[1] : NULL;
    }
    (void)ret;
}

static void visit_stmt(Node *stmt)
{
    if (!stmt || stmt->nchild < 1) return;
    Node *c0 = stmt->children[0];
    if (!c0) return;
    /* Exp SEMI */
    if (!c0->is_token && strcmp(c0->name, "Exp") == 0) {
        check_exp(c0);
        return;
    }
    /* CompSt */
    if (!c0->is_token && strcmp(c0->name, "CompSt") == 0) {
        symtab_enter_scope();
        int saved_nest = nest_depth;
        nest_depth++;
        visit_compst(c0, current_return_type);
        nest_depth = saved_nest;
        symtab_leave_scope();
        return;
    }
    /* RETURN Exp SEMI */
    if (c0->is_token && strcmp(c0->name, "RETURN") == 0) {
        Node *ret_exp = (stmt->nchild >= 2) ? stmt->children[1] : NULL;
        Type t = check_exp(ret_exp);
        if (current_return_type && t && !type_equal(current_return_type, t)) {
            sem_error(8, ret_exp->lineno, "Type mismatched for return.");
        }
        return;
    }
    /* IF LP Exp RP Stmt [ELSE Stmt] */
    if (c0->is_token && strcmp(c0->name, "IF") == 0) {
        Node *cond = (stmt->nchild >= 3) ? stmt->children[2] : NULL;
        Type t = check_exp(cond);
        /* 条件须 BASIC（int 或 float；与 RELOP 一致放宽）*/
        if (t && t->kind != BASIC)
            sem_error(7, cond->lineno, "Type mismatched for operands.");
        if (stmt->nchild >= 5) visit_stmt(stmt->children[4]);
        if (stmt->nchild >= 7) visit_stmt(stmt->children[6]);
        return;
    }
    /* WHILE LP Exp RP Stmt */
    if (c0->is_token && strcmp(c0->name, "WHILE") == 0) {
        Node *cond = (stmt->nchild >= 3) ? stmt->children[2] : NULL;
        Type t = check_exp(cond);
        if (t && t->kind != BASIC)
            sem_error(7, cond->lineno, "Type mismatched for operands.");
        if (stmt->nchild >= 5) visit_stmt(stmt->children[4]);
        return;
    }
}

/* ===== 表达式类型推导（Task 7）===== */

/* 判断 Exp 是否左值：ID / Exp[Exp] / Exp.ID 三种产生式。*/
static int is_lvalue(Node *exp)
{
    if (!exp || exp->nchild < 1) return 0;
    Node *c0 = exp->children[0];
    if (c0 && c0->is_token && strcmp(c0->name, "ID") == 0 && exp->nchild == 1)
        return 1;
    if (c0 && !c0->is_token && strcmp(c0->name, "Exp") == 0) {
        Node *op = (exp->nchild >= 2) ? exp->children[1] : NULL;
        if (op && op->is_token &&
            (strcmp(op->name, "LB") == 0 || strcmp(op->name, "DOT") == 0))
            return 1;
    }
    return 0;
}

/* 取数组访问表达式最左的 ID 节点（如 a[b][c] → a），用于错误10/13 的行号。*/
static Node *array_base_id(Node *exp)
{
    if (!exp) return NULL;
    Node *c0 = (exp->nchild >= 1) ? exp->children[0] : NULL;
    if (c0 && c0->is_token && strcmp(c0->name, "ID") == 0) return c0;
    if (c0 && !c0->is_token && strcmp(c0->name, "Exp") == 0) return array_base_id(c0);
    return NULL;
}

static Type check_exp(Node *exp)
{
    if (!exp || exp->nchild < 1) return NULL;
    Node *c0 = exp->children[0];
    if (!c0) return NULL;

    /* ID（单个变量）*/
    if (c0->is_token && strcmp(c0->name, "ID") == 0 && exp->nchild == 1) {
        Symbol s = symtab_lookup(c0->sval, SYM_VAR);
        if (!s) {
            Symbol fn = symtab_lookup(c0->sval, SYM_FUNC);
            if (!fn) {
                /* 若有同名结构体也不报错误1（让结构体误用走错误 17/13）*/
                Symbol st = symtab_lookup(c0->sval, SYM_STRUCT);
                if (!st) sem_error(1, c0->lineno, "Undefined Variable.");
            }
            return NULL;
        }
        return s->type;
    }

    /* INT / FLOAT */
    if (c0->is_token && strcmp(c0->name, "INT") == 0) return new_basic(0);
    if (c0->is_token && strcmp(c0->name, "FLOAT") == 0) return new_basic(1);

    /* ID LP ... RP：函数调用 */
    if (c0->is_token && strcmp(c0->name, "ID") == 0 && exp->nchild >= 3) {
        Node *lp = exp->children[1];
        if (lp && lp->is_token && strcmp(lp->name, "LP") == 0) {
            Symbol fn = symtab_lookup(c0->sval, SYM_FUNC);
            if (!fn) {
                Symbol v = symtab_lookup(c0->sval, SYM_VAR);
                if (!v) sem_error(2, c0->lineno, "Undefined function.");
                else    sem_error(11, c0->lineno, "Not a function.");
                return NULL;
            }
            /* 收集实参类型（头插）*/
            FieldList args = NULL;
            int nargs = 0;
            if (exp->nchild == 4) {
                Node *argsnode = exp->children[2];
                while (argsnode && argsnode->nchild >= 1) {
                    Type at = check_exp(argsnode->children[0]);
                    args = new_field("a", at, args);
                    nargs++;
                    argsnode = (argsnode->nchild >= 3) ? argsnode->children[2] : NULL;
                }
            }
            /* 错误9：实参数目或类型不匹配 */
            if (fn->nparam != nargs) {
                sem_error(9, c0->lineno, "Function is not applicable for arguments.");
            } else {
                FieldList p = fn->params;
                FieldList a = args;
                int mismatch = 0;
                while (p && a) {
                    if (!type_equal(p->type, a->type)) { mismatch = 1; break; }
                    p = p->tail; a = a->tail;
                }
                if (mismatch)
                    sem_error(9, c0->lineno, "Function is not applicable for arguments.");
            }
            return fn->type;
        }
    }

    /* LP Exp RP */
    if (c0->is_token && strcmp(c0->name, "LP") == 0) {
        return check_exp(exp->children[1]);
    }

    /* MINUS Exp / NOT Exp（一元）*/
    if (c0->is_token && (strcmp(c0->name, "MINUS") == 0 || strcmp(c0->name, "NOT") == 0)) {
        Type t = check_exp(exp->children[1]);
        if (strcmp(c0->name, "NOT") == 0) {
            if (t && !(t->kind == BASIC && t->u.basic == 0))
                sem_error(7, c0->lineno, "Type mismatched for operands.");
            return new_basic(0);
        }
        if (t && t->kind != BASIC)
            sem_error(7, c0->lineno, "Type mismatched for operands.");
        return t;
    }

    /* 二元：Exp OP Exp / Exp LB Exp RB / Exp DOT ID */
    if (!c0->is_token && strcmp(c0->name, "Exp") == 0) {
        Node *op = (exp->nchild >= 2) ? exp->children[1] : NULL;
        const char *opn = (op && op->is_token) ? op->name : "";

        if (strcmp(opn, "ASSIGNOP") == 0) {
            Node *lhs = c0, *rhs = exp->children[2];
            if (!is_lvalue(lhs)) {
                sem_error(6, op->lineno, "The left-hand side of an assignment must be a variable.");
            }
            Type tl = check_exp(lhs);
            Type tr = check_exp(rhs);
            if (tl && tr && !type_equal(tl, tr))
                sem_error(5, op->lineno, "Type mismatched for assignment.");
            return tl;
        }
        if (strcmp(opn, "AND") == 0 || strcmp(opn, "OR") == 0 || strcmp(opn, "RELOP") == 0) {
            Type tl = check_exp(c0), tr = check_exp(exp->children[2]);
            /* AND/OR 两边须 int（布尔运算）；RELOP 两边须同型 BASIC（int/float 均可）*/
            if (strcmp(opn, "AND") == 0 || strcmp(opn, "OR") == 0) {
                if (tl && !(tl->kind == BASIC && tl->u.basic == 0))
                    sem_error(7, op->lineno, "Type mismatched for operands.");
                if (tr && !(tr->kind == BASIC && tr->u.basic == 0))
                    sem_error(7, op->lineno, "Type mismatched for operands.");
            } else {
                /* RELOP：两边都 BASIC（int 或 float）即可 */
                if (tl && tl->kind != BASIC)
                    sem_error(7, op->lineno, "Type mismatched for operands.");
                if (tr && tr->kind != BASIC)
                    sem_error(7, op->lineno, "Type mismatched for operands.");
            }
            return new_basic(0);
        }
        if (strcmp(opn, "PLUS") == 0 || strcmp(opn, "MINUS") == 0 ||
            strcmp(opn, "STAR") == 0 || strcmp(opn, "DIV") == 0) {
            Type tl = check_exp(c0), tr = check_exp(exp->children[2]);
            if (tl && tr) {
                if (tl->kind != BASIC || tr->kind != BASIC || tl->u.basic != tr->u.basic) {
                    sem_error(7, op->lineno, "Type mismatched for operands.");
                    return NULL;   /* 短路：避免上层基于错误类型继续报错误5 */
                }
            }
            return (tl && tr) ? tl : (tl ? tl : tr);
        }
        if (strcmp(opn, "LB") == 0) {
            /* 数组访问 a[i] */
            Type arr = check_exp(c0);
            Node *idxnode = exp->children[2];
            Type idx = check_exp(idxnode);
            if (arr && arr->kind != ARRAY) {
                Node *idn = array_base_id(c0);
                sem_error(10, idn ? idn->lineno : op->lineno, "Not an array.");
                return NULL;
            }
            if (idx && !(idx->kind == BASIC && idx->u.basic == 0)) {
                sem_error(12, idxnode->lineno, "Not an integer.");
            }
            return arr ? arr->u.array.elem : NULL;
        }
        if (strcmp(opn, "DOT") == 0) {
            /* 结构体访问 a.f */
            Type st = check_exp(c0);
            Node *idn = exp->children[2];
            if (st && st->kind != STRUCTURE) {
                sem_error(13, op->lineno, "Illegal use of \".\".");
                return NULL;
            }
            if (st && idn && idn->is_token) {
                FieldList f = st->u.structure;
                while (f) {
                    if (strcmp(f->name, idn->sval) == 0) return f->type;
                    f = f->tail;
                }
                sem_error(14, idn->lineno, "Not-existen field.");
            }
            return NULL;
        }
    }
    return NULL;
}
