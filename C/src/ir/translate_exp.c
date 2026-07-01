/*
 * translate_exp.c - 表达式翻译（实践 3 Task 4，PDF 表 4.1）
 *
 * 入口：void translate_exp(Node *exp, Operand place)
 *   把 exp 的值"放到" place。
 *   place 为 OP_CONST 哨兵表示丢弃结果（不需要值的场景，如纯表达式语句）。
 *
 * 本文件覆盖基础产生式（PDF 表 4.1 前 8 行）：
 *   Exp -> INT
 *   Exp -> ID
 *   Exp -> Exp ASSIGNOP Exp   （左值仅 ID，数组/结构体左值后续任务）
 *   Exp -> Exp PLUS/MINUS/STAR/DIV Exp
 *   Exp -> MINUS Exp           （一元负）
 *   Exp -> LP Exp RP
 *
 * 函数调用、数组下标、结构体域、RELOP/AND/OR/NOT 在后续任务实现。
 *
 * 兼容 C99 / GCC 7.5.0
 */
#include "translate.h"
#include "ir.h"
#include <string.h>

/* 判断 place 是否需要结果（非哨兵）。
 * 哨兵约定：place.kind == OP_CONST。
 * 合法的 place 不会是 OP_CONST（结果不会存到常量里）。
 * 不要用 place.name != NULL 判断，OP_CONST 的 name 是 NULL 但某些
 * 路径可能误判。 */
static int place_wants(Operand place)
{
    return place.kind != OP_CONST;
}

/* 判断 Exp 是否为单纯 ID（Exp -> ID）。
 * 此时 children[0] 是 ID token，sval 是变量名。 */
static int is_id_exp(Node *exp)
{
    return exp->nchild >= 1
        && exp->children[0]->is_token
        && strcmp(exp->children[0]->name, "ID") == 0;
}

void translate_exp(Node *exp, Operand place)
{
    if (!exp || exp->nchild < 1) return;
    Node *c0 = exp->children[0];

    /* -----------------------------------------------------------------
     * Exp -> INT
     * 翻译：place := #ival（仅当 place 需要结果时）
     * ----------------------------------------------------------------- */
    if (c0->is_token && strcmp(c0->name, "INT") == 0) {
        if (place_wants(place))
            gen_assign(place, new_const((int)c0->ival));
        return;
    }

    /* -----------------------------------------------------------------
     * Exp -> ID LP RP        （无参函数调用）
     * Exp -> ID LP Args RP   （带参函数调用）
     * 必须在普通 ID 分支（is_id_exp）之前判断：两者都是 children[0]==ID，
     * 但函数调用多了 LP。带 LP 的在此 return 后，普通 ID 分支只剩纯 ID。
     * ----------------------------------------------------------------- */
    if (c0->is_token && strcmp(c0->name, "ID") == 0
        && exp->nchild >= 3 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "LP") == 0) {
        const char *fname = c0->sval;

        /* 特判 read：无参。Exp -> ID LP RP，READ place。*/
        if (strcmp(fname, "read") == 0) {
            if (place_wants(place))
                gen_read(place);
            return;
        }

        /* 特判 write：1 个参数，WRITE t，place := #0。*/
        if (strcmp(fname, "write") == 0) {
            Node *args = exp->children[2];   /* ID LP Args RP */
            Operand t = new_temp();
            translate_exp(args->children[0], t);
            gen_write(t);
            if (place_wants(place))
                gen_assign(place, new_const(0));
            return;
        }

        /* 通用函数调用 */
        {
            Operand arg_list[32];
            int argc = 0;
            int i;
            if (exp->nchild == 4) {  /* ID LP Args RP */
                translate_args(exp->children[2], arg_list, &argc);
                /* ARG 正序：arg_list[0] 是第一个实参，先 ARG。
                 * 与 irvm 语义一致（PARAM[0] 取第一个 ARG）。*/
                for (i = 0; i < argc; i++) {
                    gen_arg(arg_list[i]);
                }
            }
            if (place_wants(place)) {
                gen_call(place, fname);
            } else {
                /* 调用方丢弃结果：仍要 CALL（函数可能有副作用），
                 * 用临时变量接住返回值。*/
                Operand tmp = new_temp();
                gen_call(tmp, fname);
            }
            return;
        }
    }

    /* -----------------------------------------------------------------
     * Exp -> ID
     * 翻译：查符号表得 ir_name；place := v_<ir_name>
     * ----------------------------------------------------------------- */
    if (is_id_exp(exp)) {
        if (place_wants(place)) {
            VarEntry *e = translate_lookup_var(c0->sval);
            const char *irn = e ? e->ir_name : c0->sval;
            gen_assign(place, new_var_from_ir(irn));
        }
        return;
    }

    /* -----------------------------------------------------------------
     * Exp -> Exp LB Exp RB（数组下标读）  或  Exp -> Exp DOT ID（结构体域读）
     * 翻译（place 需要结果时）：
     *   addr = translate_addr_general(exp)   # 元素/域地址
     *   place := *addr                       # 读出元素/域
     * （左值赋值在下方 ASSIGNOP 分支处理。）
     * ----------------------------------------------------------------- */
    if ((exp->nchild == 4 && exp->children[1] && exp->children[1]->is_token
         && strcmp(exp->children[1]->name, "LB") == 0)
     || (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token
         && strcmp(exp->children[1]->name, "DOT") == 0)) {
        if (place_wants(place)) {
            Operand addr = translate_addr_general(exp, NULL);
            gen_load(place, addr);   /* place := *addr */
        }
        return;
    }

    /* -----------------------------------------------------------------
     * 二元运算 / 赋值：看 children[1] 的 token 名
     * 产生式 Exp -> Exp <OP> Exp 或 Exp -> Exp ASSIGNOP Exp
     * ----------------------------------------------------------------- */
    if (exp->nchild >= 3 && exp->children[1] && exp->children[1]->is_token) {
        Node *op_node = exp->children[1];
        const char *op = op_node->name;

        /* ---- Exp -> Exp ASSIGNOP Exp（左值）---- */
        if (strcmp(op, "ASSIGNOP") == 0) {
            Node *lhs = exp->children[0];
            Node *rhs = exp->children[2];
            if (is_id_exp(lhs)) {
                Operand t = new_temp();
                translate_exp(rhs, t);
                /* 查左值的符号表条目 */
                VarEntry *e = translate_lookup_var(lhs->children[0]->sval);
                const char *irn = e ? e->ir_name : lhs->children[0]->sval;
                Operand vlhs = new_var_from_ir(irn);
                /* 赋值（有副作用，始终执行） */
                gen_assign(vlhs, t);
                /* 若调用方需要结果，再复制到 place */
                if (place_wants(place))
                    gen_assign(place, vlhs);
            } else if ((lhs->nchild == 4 && lhs->children[1]
                        && lhs->children[1]->is_token
                        && strcmp(lhs->children[1]->name, "LB") == 0)
                    || (lhs->nchild == 3 && lhs->children[1]
                        && lhs->children[1]->is_token
                        && strcmp(lhs->children[1]->name, "DOT") == 0)) {
                /* 数组元素 / 结构体域左值：a[i] = rhs 或 s.f = rhs：
                 *   addr = 元素/域地址；*addr := rhs_val */
                Operand addr = translate_addr_general(lhs, NULL);
                Operand t = new_temp();
                translate_exp(rhs, t);
                gen_store(addr, t);   /* *addr := t */
                /* 赋值表达式作为右值时取赋的值：place := t */
                if (place_wants(place))
                    gen_assign(place, t);
            }
            /* 其他左值形式：不处理 */
            return;
        }

        /* ---- Exp -> Exp PLUS/MINUS/STAR/DIV Exp ---- */
        if (strcmp(op, "PLUS") == 0 || strcmp(op, "MINUS") == 0
         || strcmp(op, "STAR") == 0  || strcmp(op, "DIV") == 0) {
            IRKind k;
            if      (strcmp(op, "PLUS") == 0) k = IR_ADD;
            else if (strcmp(op, "MINUS") == 0) k = IR_SUB;
            else if (strcmp(op, "STAR") == 0)  k = IR_MUL;
            else                                k = IR_DIV;

            if (place_wants(place)) {
                Operand t1 = new_temp();
                Operand t2 = new_temp();
                translate_exp(exp->children[0], t1);
                translate_exp(exp->children[2], t2);
                gen_binop(k, place, t1, t2);
            } else {
                /* 仍翻译子表达式以保留副作用（如函数调用），丢弃结果 */
                translate_exp(exp->children[0], place);
                translate_exp(exp->children[2], place);
            }
            return;
        }
    }

    /* -----------------------------------------------------------------
     * Exp -> MINUS Exp（一元负）
     * 翻译：t := Exp; place := #0 - t
     * ----------------------------------------------------------------- */
    if (c0->is_token && strcmp(c0->name, "MINUS") == 0) {
        Node *sub = exp->children[1];
        if (place_wants(place)) {
            Operand t = new_temp();
            translate_exp(sub, t);
            gen_binop(IR_SUB, place, new_const(0), t);
        } else {
            translate_exp(sub, place);
        }
        return;
    }

    /* -----------------------------------------------------------------
     * Exp -> LP Exp RP
     * 翻译：递归翻译内部 Exp，括号本身不产生代码
     * ----------------------------------------------------------------- */
    if (c0->is_token && strcmp(c0->name, "LP") == 0) {
        translate_exp(exp->children[1], place);
        return;
    }

    /* -----------------------------------------------------------------
     * Exp -> Exp RELOP/AND/OR Exp  或  NOT Exp
     * 出现在"表达式上下文"（place 需要结果，而非控制流条件上下文）时，
     * 数值化为 0/1：先令 place := #0，若条件成立跳到 l_true 处置 place := #1。
     * （条件上下文由 translate_cond 直接处理短路跳转，不走这里。）
     * PDF 表 4.1 RELOP 行。
     * ----------------------------------------------------------------- */
    if (place_wants(place)) {
        int is_logic = 0;
        if (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token) {
            const char *opn = exp->children[1]->name;
            if (strcmp(opn, "RELOP") == 0
             || strcmp(opn, "AND") == 0
             || strcmp(opn, "OR") == 0)
                is_logic = 1;
        }
        if (!is_logic && c0->is_token && strcmp(c0->name, "NOT") == 0)
            is_logic = 1;
        if (is_logic) {
            Operand l_true = new_label(), l_false = new_label();
            gen_assign(place, new_const(0));
            translate_cond(exp, l_true, l_false);
            gen_label(l_true);
            gen_assign(place, new_const(1));
            gen_label(l_false);
            return;
        }
    }

    /* -----------------------------------------------------------------
     * 其余产生式（函数调用、数组下标、结构体域）
     * 由后续 Task 实现，此处不做任何事。
     * ----------------------------------------------------------------- */
}
