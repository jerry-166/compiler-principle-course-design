#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

#define HASH_SIZE 0x3fff
#define MAX_DEPTH 256

static Symbol buckets[HASH_SIZE];
static Symbol scope_stack[MAX_DEPTH];   /* 每层作用域的符号链表头 */
static int    cur_depth = -1;           /* -1=未初始化 */

/* pjw 哈希（Project_2.pdf 3.2.3）*/
static unsigned int hash_pjw(const char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + (unsigned char)*name;
        if ((i = val & ~0x3fffU)) val = (val ^ (i >> 12)) & 0x3fffU;
    }
    return val;
}

void symtab_init(void)
{
    memset(buckets, 0, sizeof(buckets));
    memset(scope_stack, 0, sizeof(scope_stack));
    cur_depth = 0;
}

int symtab_cur_depth(void)
{
    return (cur_depth >= 0) ? cur_depth : 0;
}

void symtab_enter_scope(void)
{
#ifdef ENABLE_SCOPE
    if (cur_depth + 1 < MAX_DEPTH) {
        cur_depth++;
        scope_stack[cur_depth] = NULL;
    }
#endif
}

void symtab_leave_scope(void)
{
#ifdef ENABLE_SCOPE
    if (cur_depth <= 0) return;
    Symbol s = scope_stack[cur_depth];
    while (s) {
        unsigned int h = hash_pjw(s->name);
        Symbol *pp = &buckets[h];
        while (*pp && *pp != s) pp = &(*pp)->next_hash;
        if (*pp) *pp = s->next_hash;
        Symbol next = s->next_scope;
        /* 不 free name/type/params，避免悬空（测试规模下可接受） */
        free(s);
        s = next;
    }
    scope_stack[cur_depth] = NULL;
    cur_depth--;
#endif
}

static Symbol find_in_current_scope(const char *name)
{
    Symbol s = (cur_depth >= 0) ? scope_stack[cur_depth] : NULL;
    while (s) {
        if (strcmp(s->name, name) == 0) return s;
        s = s->next_scope;
    }
    return NULL;
}

int symtab_insert(Symbol sym)
{
    if (find_in_current_scope(sym->name)) return 0;
    sym->depth = (cur_depth >= 0) ? cur_depth : 0;
    unsigned int h = hash_pjw(sym->name);
    sym->next_hash = buckets[h];
    buckets[h] = sym;
    if (cur_depth >= 0) {
        sym->next_scope = scope_stack[cur_depth];
        scope_stack[cur_depth] = sym;
    } else {
        sym->next_scope = NULL;
    }
    return 1;
}

Symbol symtab_lookup(const char *name, int kind_mask)
{
    int d;
    for (d = cur_depth; d >= 0; d--) {
        Symbol s = scope_stack[d];
        while (s) {
            if (strcmp(s->name, name) == 0 &&
                (kind_mask < 0 || s->kind == kind_mask)) {
                return s;
            }
            s = s->next_scope;
        }
    }
    return NULL;
}

void symtab_for_undeclared(void (*cb)(Symbol, void*), void *arg)
{
    int d;
    for (d = cur_depth; d >= 0; d--) {
        Symbol s = scope_stack[d];
        while (s) {
            if (s->kind == SYM_FUNC && s->declared_only && !s->defined) {
                cb(s, arg);
            }
            s = s->next_scope;
        }
    }
}
