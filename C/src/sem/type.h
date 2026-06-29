/*
 * type.h - C-- 类型系统（实践 2 语义分析）
 * 对应 Project_2.pdf 3.2.4。类型用链表表示：数组逐维、结构体逐域。
 */
#ifndef TYPE_H
#define TYPE_H

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_ {
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union {
        int basic;                        /* 0=int, 1=float */
        struct { Type elem; int size; } array;
        FieldList structure;
    } u;
};

struct FieldList_ {
    char *name;      /* 域名 */
    Type  type;      /* 域类型 */
    FieldList tail;
};

/* 构造函数 */
Type  new_basic(int is_float);              /* is_float=0→int, 1→float */
Type  new_array(Type elem, int size);       /* 新增一维：elem 是内层类型 */
Type  new_structure(FieldList fields);      /* fields 可为 NULL（空结构体） */
FieldList new_field(const char *name, Type type, FieldList tail);

/* 类型等价。默认名等价；ENABLE_STRUCT_EQ 时结构体按结构比较（不展开数组）。
   返回 1=相等，0=不等。任一为 NULL 返回 0。 */
int   type_equal(Type a, Type b);

/* 调试用：判断两个数组类型"基类型+维数"是否相同（用于数组类型等价，
   size 不参与比较，PDF：int a[10][2] 与 int b[5][3] 同型）。*/
int   array_compatible(Type a, Type b);

#endif /* TYPE_H */
