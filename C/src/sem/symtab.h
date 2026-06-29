/*
 * symtab.h - 符号表（实践 2 语义分析）
 * 函数式作用域栈 + pjw 哈希（Project_2.pdf 3.2.2/3.2.3）。
 */
#ifndef SYMTAB_H
#define SYMTAB_H

#include "type.h"

enum { SYM_VAR, SYM_FUNC, SYM_STRUCT };

typedef struct Symbol_* Symbol;
struct Symbol_ {
    char *name;
    int   kind;            /* SYM_VAR / SYM_FUNC / SYM_STRUCT */
    Type  type;            /* SYM_VAR/SYM_STRUCT: 变量或结构体类型；
                              SYM_FUNC: 返回类型 */
    FieldList params;      /* SYM_FUNC: 形参链表（每个 field 的 type 是形参类型） */
    int   nparam;          /* SYM_FUNC: 形参个数 */
    int   declared_only;   /* SYM_FUNC: 1=仅声明未定义（要求3.1 用） */
    int   defined;         /* SYM_FUNC: 1=已定义 */
    int   lineno;          /* 声明/定义出现的行号（错误18/19 等用）*/
    int   depth;           /* 所在作用域深度（0=全局） */
    Symbol next_hash;      /* 同哈希桶拉链 */
    Symbol next_scope;     /* 同一作用域链表（弹栈时用） */
};

void symtab_init(void);

/* 返回当前作用域深度（cur_depth）。错误3 区分 A-3/D-2 文字用。*/
int symtab_cur_depth(void);

/* 作用域栈：进入/离开语句块（CompSt）。
   ENABLE_SCOPE 关闭时：不真正压栈，所有符号都在 depth 0（全局），
   这样嵌套重定义会被查重逻辑报错（D-2 行为）。*/
void symtab_enter_scope(void);
void symtab_leave_scope(void);

/* 插入符号。返回 1=成功，0=名字在当前作用域已存在。
   查重只在"当前作用域"内查同名。params 所有权归符号表。*/
int symtab_insert(Symbol sym);

/* 查找：返回名字最近的符号（作用域从内到外）。
   kind_mask 指定种类（SYM_VAR 等）；-1 表示任意种类。找不到返回 NULL。*/
Symbol symtab_lookup(const char *name, int kind_mask);

/* 全局扫描：对每个 SYM_FUNC 且 declared_only && !defined 的符号调用 cb（错误18用）。*/
void symtab_for_undeclared(void (*cb)(Symbol, void*), void *arg);

#endif /* SYMTAB_H */
