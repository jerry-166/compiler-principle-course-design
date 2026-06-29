#include <stdlib.h>
#include <string.h>
#include "type.h"
Type new_basic(int is_float)
{
    Type t = (Type)malloc(sizeof(struct Type_));
    t->kind = BASIC;
    t->u.basic = is_float ? 1 : 0;
    return t;
}

Type new_array(Type elem, int size)
{
    Type t = (Type)malloc(sizeof(struct Type_));
    t->kind = ARRAY;
    t->u.array.elem = elem;
    t->u.array.size = size;
    return t;
}

Type new_structure(FieldList fields)
{
    Type t = (Type)malloc(sizeof(struct Type_));
    t->kind = STRUCTURE;
    t->u.structure = fields;
    return t;
}

FieldList new_field(const char *name, Type type, FieldList tail)
{
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    size_t len = strlen(name);
    f->name = (char *)malloc(len + 1);
    memcpy(f->name, name, len + 1);
    f->type = type;
    f->tail = tail;
    return f;
}

/* 数组兼容：基类型相同 + 维数相同（size 不比）。
   a=int[10][2], b=int[5][3] → 都 2 维、最内层 int → 兼容。*/
int array_compatible(Type a, Type b)
{
    if (!a || !b) return 0;
    if (a->kind != ARRAY || b->kind != ARRAY) return 0;
    /* 递归到最内层，比较维数和元素类型 */
    Type pa = a, pb = b;
    while (pa && pa->kind == ARRAY && pb && pb->kind == ARRAY) {
        pa = pa->u.array.elem;
        pb = pb->u.array.elem;
    }
    if (pa->kind == ARRAY || pb->kind == ARRAY) return 0; /* 维数不同 */
    return type_equal(pa, pb);
}

int type_equal(Type a, Type b)
{
    if (!a || !b) return 0;
    if (a->kind != b->kind) return 0;
    switch (a->kind) {
    case BASIC:
        return a->u.basic == b->u.basic;
    case ARRAY:
        return array_compatible(a, b);
    case STRUCTURE:
#ifdef ENABLE_STRUCT_EQ
    {
        /* 结构等价：逐域比 name + type，长度须相同，不展开数组 */
        FieldList fa = a->u.structure, fb = b->u.structure;
        while (fa && fb) {
            if (!type_equal(fa->type, fb->type)) return 0;
            fa = fa->tail;
            fb = fb->tail;
        }
        return (fa == NULL && fb == NULL);
    }
#else
        /* 名等价：每个结构体定义构造唯一 STRUCTURE Type 实例，
           同名结构体的变量共享同一个 Type 指针，比较 a==b 即名等价。*/
        return (a == b);
#endif
    }
    return 0;
}
