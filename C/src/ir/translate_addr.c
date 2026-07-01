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
