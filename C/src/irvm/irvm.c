/*
 * irvm.c - IR 虚拟机（线性三地址码解释器）
 *
 * 用途：自动验证工具（非交付物）。读一个 .ir 文本文件 + stdin，
 *      执行 IR 语句，把所有 WRITE 的值逐行打到 stdout。
 *
 * 支持的 17 种语句（PDF 表 4.6）：
 *   LABEL name :            标号定义
 *   FUNCTION name :         函数入口
 *   x := #k                 赋值常量
 *   x := y op z             二元运算 (op = + - * /)
 *   x := &y                 取地址（y 为变量名，返回其 mem_addr）
 *   x := *y                 读内存（y 的值是地址，返回 mem[该地址]）
 *   *x := y                 写内存（把 y 写到 mem[x 的值]）
 *   GOTO label              无条件跳转
 *   IF x relop y GOTO label 条件跳转
 *   RETURN x                函数返回
 *   DEC v size              给 v 分配 size 字节内存
 *   ARG x                   压参数
 *   x := CALL f             函数调用
 *   PARAM v                 形参声明
 *   READ x                  从 stdin 读整数到 x
 *   WRITE x                 把 x 打到 stdout
 *
 * 变量作用域：使用 scope 栈。每个函数调用创建一个新的 scope，
 * 变量查找从当前 scope 向上搜索直到找到。CALL 创建 scope，RETURN 弹出。
 * 这支持递归且不复制整个哈希表。
 *
 * ARG/PARAM 语义：ARG 按压栈顺序存入 arg_stack。CALL f 时：
 *   arg_base = arg_top - f.param_count。
 *   PARAM 第 i 个形参取 arg_stack[arg_base + i]。
 *   即：最先 ARG 的 = PARAM[0]（第一个形参）。
 *
 * 兼容性：C99，目标 GCC 7.5.0。
 *         禁用 VLA、_Generic、typeof。自包含（不依赖 POSIX strdup）。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* 常量                                                                */
/* ------------------------------------------------------------------ */

#define HASH_SIZE   0x3fff   /* pjw 哈希桶数（16383）                     */
#define MAX_LINES   200000   /* 程序最大行数                              */
#define MAX_LINE    256      /* 单行最大长度                              */
#define MEM_SIZE    (1<<20)  /* 模拟内存大小（1M long）                   */
#define STACK_DEPTH 4096     /* 调用栈/参数栈深度                         */
#define MAX_PARAMS  64       /* 每个函数最大形参个数                      */

/* ------------------------------------------------------------------ */
/* Scope 变量表（支持函数调用的局部作用域）                              */
/*                                                                      */
/*   每个 scope 是一个链式哈希表，有一个 parent 指针指向调用者的 scope。*/
/*   var_get 查找当前 scope，找不到则向上找 parent（直到全局 scope）。   */
/*   var_set 在当前 scope 创建或覆盖。                                 */
/*   这样递归调用时，内层 scope 有自己的变量，不会破坏外层。             */
/* ------------------------------------------------------------------ */

typedef struct ScopeVar {
    char            *name;
    long             val;
    int              mem_addr;       /* -1 = 未分配                    */
    struct ScopeVar *next;           /* 哈希链                          */
} ScopeVar;

typedef struct Scope {
    ScopeVar      *vars[HASH_SIZE];  /* 本层变量哈希表                  */
    struct Scope  *parent;          /* 调用者的 scope（NULL = 全局）    */
} Scope;

static Scope global_scope;          /* 全局 scope（程序启动时）        */
static Scope *current_scope = &global_scope;  /* 当前活动 scope      */

/* pjw 散列 */
static unsigned int hash_pjw(const char *s)
{
    unsigned int h = 0, g;
    for (; *s; ++s) {
        h = (h << 4) + (unsigned char)(*s);
        if ((g = (h & 0xf0000000)) != 0) {
            h ^= (g >> 24);
            h ^= g;
        }
    }
    return h % HASH_SIZE;
}

static char *xstrdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char  *p = (char *)malloc(n);
    if (!p) {
        fprintf(stderr, "irvm: out of memory\n");
        exit(1);
    }
    memcpy(p, s, n);
    return p;
}

/*
 * scope_var_set(scope, name, val)：在指定 scope 中设置变量值。
 *   如果变量不存在则创建，已存在则更新。
 */
static void scope_var_set(Scope *sc, const char *name, long val)
{
    unsigned int h = hash_pjw(name);
    ScopeVar *v;
    for (v = sc->vars[h]; v; v = v->next) {
        if (strcmp(v->name, name) == 0) {
            v->val = val;
            return;
        }
    }
    v = (ScopeVar *)malloc(sizeof(ScopeVar));
    if (!v) { fprintf(stderr, "irvm: oom\n"); exit(1); }
    v->name     = xstrdup(name);
    v->val      = val;
    v->mem_addr = -1;
    v->next     = sc->vars[h];
    sc->vars[h] = v;
}

/*
 * scope_var_find(scope, name)：在指定 scope 中查找变量，找不到返回 NULL。
 */
static ScopeVar *scope_var_find(Scope *sc, const char *name)
{
    unsigned int h = hash_pjw(name);
    ScopeVar *v;
    for (v = sc->vars[h]; v; v = v->next) {
        if (strcmp(v->name, name) == 0)
            return v;
    }
    return NULL;
}

/*
 * var_get(name)：在当前 scope 中查找变量，找不到则向上搜索到全局 scope。
 * 如果所有 scope 都没有，在当前 scope 创建一个（val=0, mem_addr=-1）。
 */
static ScopeVar *var_get(const char *name)
{
    Scope *sc = current_scope;
    while (sc) {
        ScopeVar *v = scope_var_find(sc, name);
        if (v) return v;
        sc = sc->parent;
    }
    /* 没找到，在当前 scope 创建 */
    scope_var_set(current_scope, name, 0);
    return scope_var_find(current_scope, name);
}

/*
 * var_set(name, val)：在当前 scope 设置变量值（如果变量在父 scope
 * 已存在，也在当前 scope 创建一个同名遮蔽变量）。
 */
static void var_set(const char *name, long val)
{
    ScopeVar *v = scope_var_find(current_scope, name);
    if (v) {
        v->val = val;
    } else {
        scope_var_set(current_scope, name, val);
    }
}

/*
 * new_scope(parent)：分配一个新的 scope，parent 指向调用者的 scope。
 */
static Scope *new_scope(Scope *parent)
{
    Scope *sc = (Scope *)calloc(1, sizeof(Scope));
    int i;
    if (!sc) { fprintf(stderr, "irvm: oom\n"); exit(1); }
    sc->parent = parent;
    /* calloc 已将 vars[] 清零 */
    (void)i;
    return sc;
}

/* ------------------------------------------------------------------ */
/* 标号表：name -> 行号（全局，不随调用切换）                           */
/* ------------------------------------------------------------------ */

typedef struct LabelEntry {
    char              *name;
    int                line;
    struct LabelEntry *next;
} LabelEntry;

static LabelEntry *label_table[HASH_SIZE];

static void label_set(const char *name, int line)
{
    unsigned int h = hash_pjw(name);
    LabelEntry  *e;
    for (e = label_table[h]; e; e = e->next) {
        if (strcmp(e->name, name) == 0) {
            e->line = line;
            return;
        }
    }
    e = (LabelEntry *)malloc(sizeof(LabelEntry));
    if (!e) { fprintf(stderr, "irvm: oom\n"); exit(1); }
    e->name = xstrdup(name);
    e->line = line;
    e->next = label_table[h];
    label_table[h] = e;
}

static int label_get(const char *name)
{
    unsigned int h = hash_pjw(name);
    LabelEntry  *e;
    for (e = label_table[h]; e; e = e->next) {
        if (strcmp(e->name, name) == 0)
            return e->line;
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/* 函数表：name -> 入口行号 + 形参个数 + 形参名列表                    */
/* ------------------------------------------------------------------ */

typedef struct FuncEntry {
    char             *name;
    int               entry;
    int               param_count;
    char             *param_names[MAX_PARAMS];
    struct FuncEntry *next;
} FuncEntry;

static FuncEntry *func_table[HASH_SIZE];

static void func_set(const char *name, int entry,
                      int param_count, char *params[])
{
    unsigned int h = hash_pjw(name);
    FuncEntry   *f;
    int i;
    for (f = func_table[h]; f; f = f->next) {
        if (strcmp(f->name, name) == 0) {
            f->entry       = entry;
            f->param_count = param_count;
            for (i = 0; i < param_count && i < MAX_PARAMS; i++) {
                f->param_names[i] = params[i];
            }
            return;
        }
    }
    f = (FuncEntry *)malloc(sizeof(FuncEntry));
    if (!f) { fprintf(stderr, "irvm: oom\n"); exit(1); }
    f->name        = xstrdup(name);
    f->entry       = entry;
    f->param_count = param_count;
    for (i = 0; i < param_count && i < MAX_PARAMS; i++) {
        f->param_names[i] = params[i];
    }
    f->next = func_table[h];
    func_table[h] = f;
}

static FuncEntry *func_get(const char *name)
{
    unsigned int h = hash_pjw(name);
    FuncEntry   *f;
    for (f = func_table[h]; f; f = f->next) {
        if (strcmp(f->name, name) == 0)
            return f;
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* 程序存储 + 模拟内存                                                  */
/* ------------------------------------------------------------------ */

static char prog[MAX_LINES][MAX_LINE];
static int  prog_len = 0;
static long mem[MEM_SIZE];
static int  mem_top = 1;     /* 0 保留，避免与"未分配"混淆 */

static const char *DELIM = " \t\r\n";

/* ------------------------------------------------------------------ */
/* 操作数求值 / 左值写入                                                */
/* ------------------------------------------------------------------ */

static long eval_operand(const char *tok)
{
    if (tok[0] == '#') {
        return strtol(tok + 1, NULL, 10);
    }
    if (tok[0] == '&') {
        return (long)var_get(tok + 1)->mem_addr;
    }
    if (tok[0] == '*') {
        ScopeVar *v = var_get(tok + 1);
        if (v->val < 0 || v->val >= (long)MEM_SIZE) {
            fprintf(stderr, "irvm: memory read out of bounds: %ld\n", v->val);
            exit(1);
        }
        return mem[v->val];
    }
    return var_get(tok)->val;
}

static void store_operand(const char *tok, long val)
{
    if (tok[0] == '*') {
        ScopeVar *v = var_get(tok + 1);
        if (v->val < 0 || v->val >= (long)MEM_SIZE) {
            fprintf(stderr, "irvm: memory write out of bounds: %ld\n", v->val);
            exit(1);
        }
        mem[v->val] = val;
        return;
    }
    var_set(tok, val);
}

/* ------------------------------------------------------------------ */
/* relop 求值                                                          */
/* ------------------------------------------------------------------ */

static int eval_relop(long a, const char *op, long b)
{
    if      (strcmp(op, "==") == 0) return a == b;
    else if (strcmp(op, "!=") == 0) return a != b;
    else if (strcmp(op, "<")  == 0) return a <  b;
    else if (strcmp(op, ">")  == 0) return a >  b;
    else if (strcmp(op, "<=") == 0) return a <= b;
    else if (strcmp(op, ">=") == 0) return a >= b;
    fprintf(stderr, "irvm: unknown relop '%s'\n", op);
    exit(1);
    return 0;
}

/* ------------------------------------------------------------------ */
/* 第一遍扫描                                                          */
/* ------------------------------------------------------------------ */

static void load_program(const char *path)
{
    FILE *fp = fopen(path, "r");
    int i;
    if (!fp) {
        fprintf(stderr, "irvm: cannot open '%s'\n", path);
        exit(1);
    }
    while (prog_len < MAX_LINES &&
           fgets(prog[prog_len], MAX_LINE, fp)) {
        prog_len++;
    }
    fclose(fp);

    for (i = 0; i < prog_len; i++) {
        char buf[MAX_LINE];
        char *t1, *t2, *t3;
        strcpy(buf, prog[i]);
        t1 = strtok(buf, DELIM);
        if (!t1) continue;
        t2 = strtok(NULL, DELIM);
        t3 = strtok(NULL, DELIM);

        if (strcmp(t1, "LABEL") == 0 && t2 && t3 &&
            strcmp(t3, ":") == 0) {
            label_set(t2, i + 1);
        } else if (strcmp(t1, "FUNCTION") == 0 && t2 && t3 &&
                   strcmp(t3, ":") == 0) {
            int param_count = 0;
            char *params[MAX_PARAMS];
            int j;
            for (j = i + 1; j < prog_len; j++) {
                char b2[MAX_LINE];
                char *u1, *u2;
                strcpy(b2, prog[j]);
                u1 = strtok(b2, DELIM);
                if (!u1) continue;
                u2 = strtok(NULL, DELIM);
                if (strcmp(u1, "PARAM") == 0 && u2) {
                    if (param_count < MAX_PARAMS) {
                        params[param_count] = xstrdup(u2);
                    }
                    param_count++;
                } else {
                    break;
                }
            }
            func_set(t2, i, param_count, params);
        }
    }
}

/* ------------------------------------------------------------------ */
/* 调用帧 + 执行循环                                                   */
/* ------------------------------------------------------------------ */

typedef struct {
    int     ret_pc;
    int     arg_base;
    int     arg_top_saved;
    int     param_taken;
    int     has_ret_lvalue;
    char    ret_lvalue[MAX_LINE];
    Scope  *saved_scope;      /* 调用者的 scope（RETURN 时恢复）      */
} CallFrame;

static long arg_stack[STACK_DEPTH];
static int  arg_top = 0;

static CallFrame call_stack[STACK_DEPTH];
static int  call_top = 0;

static void run(void)
{
    int pc = 0;

    {
        FuncEntry *main_fn = func_get("main");
        if (main_fn) {
            pc = main_fn->entry + 1;
        } else {
            pc = 0;
        }
    }

    while (pc >= 0 && pc < prog_len) {
        char  buf[MAX_LINE];
        char *tok;
        strcpy(buf, prog[pc]);
        tok = strtok(buf, DELIM);
        if (!tok) { pc++; continue; }

        if (strcmp(tok, "LABEL") == 0 || strcmp(tok, "FUNCTION") == 0) {
            pc++;
            continue;
        }

        if (strcmp(tok, "WRITE") == 0) {
            char *a = strtok(NULL, DELIM);
            printf("%ld\n", eval_operand(a));
            fflush(stdout);
            pc++;
            continue;
        }

        if (strcmp(tok, "READ") == 0) {
            char *a = strtok(NULL, DELIM);
            long x;
            if (scanf("%ld", &x) != 1) {
                fprintf(stderr, "irvm: READ: no more input\n");
                x = 0;
            }
            store_operand(a, x);
            pc++;
            continue;
        }

        if (strcmp(tok, "GOTO") == 0) {
            char *lab = strtok(NULL, DELIM);
            int target = label_get(lab);
            if (target < 0) {
                fprintf(stderr, "irvm: unknown label '%s'\n", lab);
                exit(1);
            }
            pc = target;
            continue;
        }

        if (strcmp(tok, "IF") == 0) {
            char *a   = strtok(NULL, DELIM);
            char *op  = strtok(NULL, DELIM);
            char *b   = strtok(NULL, DELIM);
            char *kw  = strtok(NULL, DELIM);
            char *lab = strtok(NULL, DELIM);
            int  target;
            (void)kw;
            target = label_get(lab);
            if (target < 0) {
                fprintf(stderr, "irvm: unknown label '%s'\n", lab);
                exit(1);
            }
            if (eval_relop(eval_operand(a), op, eval_operand(b)))
                pc = target;
            else
                pc++;
            continue;
        }

        if (strcmp(tok, "RETURN") == 0) {
            char *a = strtok(NULL, DELIM);
            long rv = a ? eval_operand(a) : 0;
            if (call_top > 0) {
                CallFrame fr = call_stack[--call_top];
                current_scope = fr.saved_scope;
                if (fr.has_ret_lvalue) {
                    var_set(fr.ret_lvalue, rv);
                }
                arg_top = fr.arg_top_saved;
                pc = fr.ret_pc;
            } else {
                return;
            }
            continue;
        }

        if (strcmp(tok, "ARG") == 0) {
            char *a = strtok(NULL, DELIM);
            if (arg_top >= STACK_DEPTH) {
                fprintf(stderr, "irvm: arg stack overflow\n");
                exit(1);
            }
            arg_stack[arg_top++] = eval_operand(a);
            pc++;
            continue;
        }

        if (strcmp(tok, "PARAM") == 0) {
            char *vname = strtok(NULL, DELIM);
            if (call_top > 0) {
                CallFrame *fr = &call_stack[call_top - 1];
                int idx = fr->arg_base + fr->param_taken;
                var_set(vname, arg_stack[idx]);
                fr->param_taken++;
            }
            pc++;
            continue;
        }

        if (strcmp(tok, "DEC") == 0) {
            char *vname = strtok(NULL, DELIM);
            char *sz    = strtok(NULL, DELIM);
            int  bytes  = sz ? (int)strtol(sz, NULL, 10) : 0;
            int  slots  = bytes / (int)sizeof(long);
            ScopeVar *v;
            if (slots < 1) slots = 1;
            if (mem_top + slots > MEM_SIZE) {
                fprintf(stderr, "irvm: memory exhausted in DEC\n");
                exit(1);
            }
            v = var_get(vname);
            v->mem_addr = mem_top;
            mem_top += slots;
            pc++;
            continue;
        }

        /* 赋值语句：lhs := ... */
        {
            char *lhs  = tok;
            char *asgn;
            char *rhs1;

            asgn = strtok(NULL, DELIM);
            if (!asgn || strcmp(asgn, ":=") != 0) {
                fprintf(stderr, "irvm: cannot parse line %d: %s\n",
                        pc + 1, prog[pc]);
                pc++;
                continue;
            }
            rhs1 = strtok(NULL, DELIM);

            if (strcmp(rhs1, "CALL") == 0) {
                char *fname = strtok(NULL, DELIM);
                FuncEntry *fn = func_get(fname);
                if (!fn) {
                    fprintf(stderr, "irvm: unknown function '%s'\n", fname);
                    exit(1);
                }
                if (call_top >= STACK_DEPTH) {
                    fprintf(stderr, "irvm: call stack overflow\n");
                    exit(1);
                }
                {
                    CallFrame *fr = &call_stack[call_top++];
                    fr->ret_pc        = pc + 1;
                    fr->arg_top_saved = arg_top;
                    fr->arg_base      = arg_top - fn->param_count;
                    fr->param_taken   = 0;
                    fr->saved_scope   = current_scope;
                    if (lhs[0] == '*') {
                        fr->has_ret_lvalue = 0;
                    } else {
                        fr->has_ret_lvalue = 1;
                        strncpy(fr->ret_lvalue, lhs, MAX_LINE - 1);
                        fr->ret_lvalue[MAX_LINE - 1] = '\0';
                    }
                }
                /* 创建新的 scope 给被调函数 */
                current_scope = new_scope(current_scope);
                pc = fn->entry + 1;
                continue;
            }

            {
                char *rhs2 = strtok(NULL, DELIM);
                if (rhs2 == NULL) {
                    store_operand(lhs, eval_operand(rhs1));
                } else {
                    char *op   = rhs2;
                    char *rhs3 = strtok(NULL, DELIM);
                    long a = eval_operand(rhs1);
                    long b = eval_operand(rhs3);
                    long r;
                    switch (op[0]) {
                        case '+': r = a + b; break;
                        case '-': r = a - b; break;
                        case '*': r = a * b; break;
                        case '/':
                            if (b == 0) {
                                fprintf(stderr, "irvm: division by zero\n");
                                exit(1);
                            }
                            r = a / b;
                            break;
                        default:
                            fprintf(stderr, "irvm: unknown op '%s'\n", op);
                            exit(1);
                    }
                    store_operand(lhs, r);
                }
                pc++;
                continue;
            }
        }
    } /* while */
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.ir>  (stdin supplies READ)\n",
                argv[0]);
        return 1;
    }
    load_program(argv[1]);
    run();
    return 0;
}
