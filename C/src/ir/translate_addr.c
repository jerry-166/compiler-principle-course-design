/*
 * translate_addr.c - 数组 / 结构体寻址（实践 3 Task 7，PDF 4.2.7）
 *
 * 提供三个对外函数：
 *   - int type_size(Type t)
 *       递归计算类型字节宽度。int = 4；数组 = 元素宽度 × size；
 *       结构体 = 逐域累加（Task 9 用，本任务一并写出备用）。
 *   - Type struct_field_offset(FieldList, const char *, int *)
 *       结构体域偏移（Task 9 用，本任务写出备用）。
 *   - Operand translate_addr_elem(Node *exp, Type *out_elem_type)
 *       翻译数组元素地址，返回地址 Operand（OP_TEMP，内容是地址）。
 *       递归处理多维（Task 10 用），本任务只用一维。
 *
 * 算法（PDF 4.2.7）：
 *   一维 a[i]：
 *     t_idx  = translate_exp(i)              # 下标值
 *     t_off  = t_idx * #elem_size            # 偏移 = 下标 × 元素宽度
 *     t_addr = base_addr + t_off            # 地址 = 数组首地址 + 偏移
 *   其中 base_addr = &v_a（数组变量首地址）。
 *   读：result := *t_addr；写：*t_addr := value。
 *
 * 兼容 C99 / GCC 7.5.0
 */
#include "translate.h"
#include "ir.h"
#include <string.h>

/* ==================================================================
 * 类型字节宽度
 * ================================================================== */

/* 类型字节宽度。int = 4；数组 = 元素宽度 × size；结构体 = 逐域累加。
   假设 1：C-- 必做部分只有 int（无 float 统一按 4 处理）。*/
int type_size(Type t)
{
    if (!t) return 0;
    if (t->kind == BASIC) return 4;
    if (t->kind == ARRAY)
        return type_size(t->u.array.elem) * t->u.array.size;
    if (t->kind == STRUCTURE) {
        int sum = 0;
        FieldList f = t->u.structure;
        while (f) {
            sum += type_size(f->type);
            f = f->tail;
        }
        return sum;
    }
    return 0;
}

/* ==================================================================
 * 结构体域偏移（Task 9 用，本任务备用）
 * ================================================================== */

/* 在 fields 中找名为 fname 的域：找到则通过 out_offset 输出它相对结构体
   首地址的偏移（字节），并返回该域类型；找不到返回 NULL。*/
Type struct_field_offset(FieldList fields, const char *fname, int *out_offset)
{
    int off = 0;
    while (fields) {
        if (strcmp(fields->name, fname) == 0) {
            if (out_offset) *out_offset = off;
            return fields->type;
        }
        off += type_size(fields->type);
        fields = fields->tail;
    }
    return NULL;
}

/* ==================================================================
 * 数组元素寻址
 * ================================================================== */

/* 翻译数组元素地址 a[idx]。
   exp 必须是 Exp -> Exp LB Exp RB（nchild == 4）。
   返回：地址 Operand（OP_TEMP，其值是元素首地址）。
   out_elem_type：输出该下标对应的元素类型（一维时是 BASIC int）。

   递归处理多维：
     a[i][j] 的外层 base 是 a[i]（一个 LB 节点）→ 递归调用本函数得到
     a[i] 元素地址及其元素类型（数组类型），再用本层下标 j 乘以"内层
     元素宽度"得到偏移。本任务只用一维，但实现保持递归结构。*/
Operand translate_addr_elem(Node *exp, Type *out_elem_type)
{
    /* exp -> Exp LB Exp RB */
    Node *base = exp->children[0];   /* 数组名 ID 或更深的 LB 下标 */
    Node *idx  = exp->children[2];   /* 当前维下标 */

    Operand base_addr;
    Type elem_type;

    /* 判断 base 是数组变量 ID（一维）还是更深的 LB（多维）。
       多维判定：base 自身 nchild==4 且 base->children[1] 是 LB token。*/
    if (base->nchild == 4 && base->children[1] && base->children[1]->is_token
        && strcmp(base->children[1]->name, "LB") == 0) {
        /* base 是 Exp -> Exp LB Exp RB → 多维，递归求 a[i] 的地址 */
        base_addr = translate_addr_elem(base, &elem_type);
        /* elem_type 此时是"下一维数组类型"，其 u.array.elem 是再内层元素 */
    } else if (base->children[0] && base->children[0]->is_token
               && strcmp(base->children[0]->name, "ID") == 0) {
        /* base 是数组变量 ID：base_addr := &v_<ir_name> */
        VarEntry *e = translate_lookup_var(base->children[0]->sval);
        if (e) {
            base_addr = new_temp();
            gen_addr(base_addr, new_var_from_ir(e->ir_name));
            elem_type = e->type->u.array.elem;
        } else {
            /* 未在符号表（理论上不应发生）：返回占位地址，元素类型 NULL */
            base_addr = new_temp();
            elem_type = NULL;
        }
    } else {
        /* 结构体域数组等其他情况，Task 9/10 处理；本任务返回占位 */
        base_addr = new_temp();
        elem_type = NULL;
    }

    /* offset = idx * type_size(elem_type)；地址 = base_addr + offset */
    {
        Operand t_idx  = new_temp();
        Operand t_off  = new_temp();
        Operand t_addr = new_temp();
        int esz = type_size(elem_type);
        translate_exp(idx, t_idx);
        gen_binop(IR_MUL, t_off, t_idx, new_const(esz));
        gen_binop(IR_ADD, t_addr, base_addr, t_off);
        if (out_elem_type) *out_elem_type = elem_type;
        return t_addr;
    }
}

/* ==================================================================
 * Task 9：通用左值地址翻译（结构体 + 数组 + 任意嵌套）
 *
 *   translate_addr_general(exp, &out_type)
 *
 *   处理三种左值形式，统一返回"地址"Operand（OP_TEMP，其值是元素/域首地址）：
 *     ID（普通/结构体/数组变量）   →  t := &v_<ir_name>
 *     Exp LB Exp RB（数组下标）   →  base_addr + idx * elem_size
 *     Exp DOT ID（结构体域）      →  base_addr + field_offset
 *
 *   递归处理 base，因此支持任意嵌套：结构体数组 a[i].b、数组结构体 s.f[i]、
 *   嵌套结构体 w.u.p 等。
 * ================================================================== */

Operand translate_addr_general(Node *exp, Type *out_type)
{
    Node *c0 = exp->nchild >= 1 ? exp->children[0] : NULL;

    /* 纯 ID 左值（不是 LB/DOT 的左操作数）：地址 = &v_<ir_name> */
    if (c0 && c0->is_token && strcmp(c0->name, "ID") == 0
        && !(exp->nchild >= 2 && exp->children[1] && exp->children[1]->is_token
             && (strcmp(exp->children[1]->name, "LB") == 0
              || strcmp(exp->children[1]->name, "DOT") == 0))) {
        VarEntry *e = translate_lookup_var(c0->sval);
        Operand a = new_temp();
        if (e) {
            gen_addr(a, new_var_from_ir(e->ir_name));
            if (out_type) *out_type = e->type;
        } else {
            if (out_type) *out_type = NULL;
        }
        return a;
    }

    /* Exp LB Exp RB（数组下标）：base_addr + idx * elem_size */
    if (exp->nchild == 4 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "LB") == 0) {
        Type base_type = NULL;
        Operand base_addr = translate_addr_general(exp->children[0], &base_type);
        Type elem_type = base_type ? base_type->u.array.elem : NULL;
        Operand t_idx = new_temp();
        Operand t_off = new_temp();
        Operand a = new_temp();
        translate_exp(exp->children[2], t_idx);
        gen_binop(IR_MUL, t_off, t_idx, new_const(type_size(elem_type)));
        gen_binop(IR_ADD, a, base_addr, t_off);
        if (out_type) *out_type = elem_type;
        return a;
    }

    /* Exp DOT ID（结构体域）：base_addr + field_offset */
    if (exp->nchild == 3 && exp->children[1] && exp->children[1]->is_token
        && strcmp(exp->children[1]->name, "DOT") == 0) {
        Type base_type = NULL;
        Operand base_addr = translate_addr_general(exp->children[0], &base_type);
        int off = 0;
        Type ftype = NULL;
        if (base_type && base_type->kind == STRUCTURE) {
            Node *idnode = exp->children[2];
            ftype = struct_field_offset(base_type->u.structure, idnode->sval, &off);
        }
        Operand a = new_temp();
        gen_binop(IR_ADD, a, base_addr, new_const(off));
        if (out_type) *out_type = ftype;
        return a;
    }

    /* 兜底 */
    {
        Operand a = new_temp();
        if (out_type) *out_type = NULL;
        return a;
    }
}

/* ==================================================================
 * Task 9：Cannot translate prepass
 *
 *   扫描所有函数的形参和局部变量类型。未开 ENABLE_STRUCT 时，
 *   发现结构体类型 → cannot_translate = 1（之后 translate_program
 *   不产生任何 IR，只输出标准提示）。
 *
 *   注意：高维数组 / 数组参数的检测属于 Task 10，本任务只检测结构体。
 * ================================================================== */

/* 判断类型是否含结构体（递归，数组元素可能是结构体）。
   仅在未开 ENABLE_STRUCT 时由 prepass_check 使用。*/
#ifndef ENABLE_STRUCT
static int type_has_struct(Type t)
{
    if (!t) return 0;
    if (t->kind == BASIC) return 0;
    if (t->kind == ARRAY) return type_has_struct(t->u.array.elem);
    if (t->kind == STRUCTURE) return 1;
    return 0;
}
#endif

void prepass_check(Node *root)
{
#ifdef ENABLE_STRUCT
    (void)root;  /* 开宏：结构体正常翻译，不检测 */
    return;
#else
    Node *extdeflist = root->children[0];
    /* 第一遍：先把所有结构体定义（ExtDef -> Specifier SEMI）注册到表，
       这样引用形式 STRUCT Tag 才能查到类型。tr_specifier_to_type 会副作用注册。*/
    {
        Node *p = extdeflist;
        while (p && p->nchild >= 1 && p->children[0]) {
            Node *extdef = p->children[0];
            if (extdef && extdef->nchild >= 1 && extdef->children[0]) {
                (void)tr_specifier_to_type(extdef->children[0]);
            }
            p = (p->nchild >= 2) ? p->children[1] : NULL;
        }
    }
    while (extdeflist && extdeflist->nchild >= 1 && extdeflist->children[0]) {
        Node *extdef = extdeflist->children[0];
        Node *second = (extdef->nchild >= 2) ? extdef->children[1] : NULL;
        if (second && strcmp(second->name, "FunDec") == 0) {
            /* 形参：ID LP VarList RP（nchild==4）*/
            if (second->nchild == 4) {
                Node *vl = second->children[2];
                while (vl && vl->nchild >= 1 && vl->children[0]) {
                    Node *pd = vl->children[0];  /* ParamDec -> Specifier VarDec */
                    Type t = tr_vardec_to_type(pd->children[1],
                                               tr_specifier_to_type(pd->children[0]));
                    if (type_has_struct(t)) { cannot_translate = 1; return; }
                    vl = (vl->nchild == 3) ? vl->children[2] : NULL;
                }
            }
            /* 局部变量：CompSt -> LC DefList StmtList RC，按名找 DefList */
            Node *compst = (extdef->nchild >= 3) ? extdef->children[2] : NULL;
            Node *deflist = NULL;
            int i;
            if (compst) {
                for (i = 0; i < compst->nchild; i++) {
                    Node *ck = compst->children[i];
                    if (ck && !ck->is_token && strcmp(ck->name, "DefList") == 0) {
                        deflist = ck; break;
                    }
                }
            }
            while (deflist && deflist->nchild >= 1 && deflist->children[0]) {
                Node *def = deflist->children[0];   /* Def -> Specifier DecList SEMI */
                Node *spec = def->children[0];
                Type base = tr_specifier_to_type(spec);
                Node *declist = def->children[1];
                while (declist && declist->nchild >= 1 && declist->children[0]) {
                    Node *dec = declist->children[0];  /* Dec -> VarDec ... */
                    Type t = tr_vardec_to_type(dec->children[0], base);
                    if (type_has_struct(t)) { cannot_translate = 1; return; }
                    declist = (declist->nchild == 3) ? declist->children[2] : NULL;
                }
                deflist = (deflist->nchild >= 2) ? deflist->children[1] : NULL;
            }
        }
        extdeflist = (extdeflist->nchild >= 2) ? extdeflist->children[1] : NULL;
    }
#endif
}
