# PL/0 编译器移植与修复

> 配套代码：`C/src/PL/PL.cpp` + `C/src/PL/common.h`（编译器）、`C/src/interpret/interpret.cpp` + `C/src/interpret/common.h`（解释器）、`C/include/clist.h` + `C/include/conio.h`（本地适配 shim）。
> 完成日期：2026-06-28。本文档配合代码使用，支撑评优答辩时口述原理。

## 一、为什么（Why）

### 这套源码从哪来，为什么要"移植 + 修复"

课程给定的 PL/0 编译器源码是一份 **2003—2004 年用 Visual C++ 6.0 写的老代码**（看 zip 里文件的日期就知道）。它当年跑在 Windows + MFC 的环境下，能正常编译运行。但我们的目标是 **Ubuntu 20.04 + GCC 7.5.0**（同伴的真机环境，也就是协作的目标版本），这套源码直接搬过来是编不过的，原因有两层：

1. **平台依赖**：它依赖了 Windows/MFC 特有的东西（MFC 的 `CList` 模板类、`<conio.h>` 的 `getch`、VC 的 `iscsymf`），这些 Linux 下都没有。
2. **老式 C++ 写法**：用了上世纪的"隐式 int"、`void main`、`stricmp` 等不被现代 GCC 接受的写法。

所以"移植"= 把平台依赖换掉，"修复"= 把过时写法改成标准 C++，让它能在 GCC 下编译通过并正确运行。

### 为什么要费劲修它，而不是重写

PL/0 编译器是**上机检查（50 分，最高占比）的现场编译运行对象**，是核心交付物。它是课程给的起点源码，重写既不现实（8 天时间还要做 C-- 五阶段），也偏离任务（任务是"调试原生 PL/0 编译器"）。修活它、理解它、能在现场演示它，就是这门课的得分主力。

### 协作约束带来的额外要求

作者在 WSL2（GCC 13 / Bison 3.8）开发，同伴在真机 Ubuntu 20.04（GCC 7.5 / Flex 2.6.4 / Bison 3.5.1）。**同伴的低版本才是目标版本**，所以所有修改都必须保证在 GCC 7.5 上能编能跑，不能用高版本特性。本文提到的每一处改动都遵循这个约束（只用 C++14 保守档，不用 VLA、不用新语法）。

## 二、是什么（What）

### 先看懂：这套 PL 编译器是个什么东西

它不是教科书里最简版 PL/0，而是一个**带类型的、Pascal 风格的扩展 PL/0**（以下简称 PL 语言）。一条完整的 PL 程序长这样（`test.pl`）：

```
program demo;
  var i, n, sum : integer;
  procedure addto;
    var k : integer;
  begin
    while i <= n do
    begin
      sum := sum + i;
      i := i + 1
    end
  end;
begin
  call read(n);
  i := 1;
  sum := 0;
  call addto;
  call write(sum)
end.
```

它比经典 PL/0 多了：**类型系统**（`integer`/`char`/`boolean`/`array`）、**过程**（`procedure`）、**数组**。但核心还是老三样：递归下降语法分析、符号表、语法制导翻译。

### 它的整体结构（两遍 + 一个解释器）

整个工程分两个独立的可执行文件，构成一条流水线：

```
test.pl（源代码）
   │  PL.exe（编译器，两遍扫描）
   ▼
test.pld（抽象机中间代码，二进制）
   │  INTERPRET.exe（栈式抽象机解释器）
   ▼
运行结果
```

- **PL.exe** 自己又分两遍：第一遍 `getSymbols` 做词法分析，把源码切成一串符号（token）串成链表；第二遍 `BLOCK` 等函数做语法分析 + 语义分析 + 生成中间代码，期间维护**符号表**（`NAMETAB`）、**块表**（`BTAB`）、**数组表**（`ATAB`）。
- **INTERPRET.exe** 读 `.pld` 二进制，在一个栈式抽象机上逐条解释执行（`S[]` 是运行栈，`DISPLAY[]` 是层显式访问表，`pc` 是程序计数器）。

### 关键数据结构（修复时要懂这些才能动手）

- **`NAMETAB[]` 名字表**：每登记一个标识符（变量/常量/类型/过程）就占一项，记录它的名字、种类（`kind`）、类型、所在层（`level`）、地址。表项之间用 `link` 字段串成链，同一层内的标识符通过 `link` 串起来，方便查重和按作用域查找。
- **`BTAB[]` 块表**：每个程序/过程块占一项，记录这一层的局部信息（`last` 指向该层最后一个登记的标识符）。
- **`DISPLAY[]` 层显示表**：`DISPLAY[level]` 指向第 `level` 层的块表项，是运行时按层访问变量的关键。
- **`CODE[]` 指令数组**：生成的中间代码一条条放这里，最后写入 `.pld`。
- **`SYMLIST` 符号集合**：编译器里大量用到"符号集合"（比如"哪些符号能开始一个语句"= `STATBEGSYS`），它继承自 `CList<SYMBOL,SYMBOL>`——**这就是 MFC 依赖的源头**。

### 用到的核心概念（用户可能不懂的，先解释）

- **符号表（Symbol Table）**：编译时用来记录"这个名字是什么、什么类型、在哪个地址"的字典。查变量时先查它。
- **递归下降（Recursive Descent）**：语法分析方法。每条语法规则对应一个函数，函数互相调用（`BLOCK` → `STATEMENT` → `ASSIGNMENT`/`IFSTATEMENT`/...），调用层次正好对应语法嵌套层次，所以叫"递归""下降"。
- **语法制导翻译（Syntax-Directed Translation）**：一边做语法分析，一边在合适的时机"吐出"中间代码。比如分析到 `c := a + b`，分析完右边的表达式就生成一条 `STO`（存入）指令。
- **栈式抽象机（Stack Machine）**：解释器执行的假想机器。它没有寄存器，所有运算都通过栈完成——比如加法是把两个数压栈、弹出、相加、结果压回去。PL 的中间代码（`LIT`/`LOD`/`STO`/`ADD`/`JMP`...）就是这套抽象机的指令集。

## 三、怎么做（How）：逐处修复

下面按修复顺序讲，每一处都说明**症状**（为什么编不过/跑不对）、**根因**、**怎么改**。

### 修复 1：MFC `CList` → 自写 `clist.h`（最大一处）

**症状**：编译报 `CList was not declared`、`CList does not name a type`。因为 `common.h` 里写了：

```cpp
class SYMLIST:public CList<SYMBOL,SYMBOL>   // 继承 MFC 的 CList
{ };
```

而 Linux 下根本没有 MFC。

**根因**：`CList` 是 MFC（Microsoft Foundation Classes）的模板链表类。原始代码依赖 MFC。

**怎么改**：先精确统计源码**到底用了 `CList` 的哪些方法**（不能瞎写一个完整的，要按需写最小实现，也方便理解）。我在 `PL.cpp` 全文 grep 后确认只用到这些：

| 方法 | 用在哪 | 干什么 |
|------|--------|--------|
| `AddHead(T)` | 初始化 5 个符号集合 | 头插一个元素 |
| `AddTail(T)` | `listAddSym`/`COPYLIST` | 尾插一个元素 |
| `AddTail(CList*)` | `listsAdd` | 把另一个链表整体追加到尾部 |
| `GetHeadPosition()` | `SYMINLIST`/`COPYLIST` | 取链表头的游标 |
| `GetNext(POSITION&)` | 同上 | 取下一个元素，游标后移 |
| `POSITION`（类型） | 上面两个的参数 | 遍历游标 |

于是写了 `C/include/clist.h`，一个约 90 行的模板类，用内部节点指针实现双向链表，只覆盖上面这些方法。`POSITION` 直接 typedef 成节点指针，遍历到末尾自然变 `NULL`，和 MFC 行为一致。

**一个隐藏的坑**：`PL.cpp` 里写的是非限定名 `POSITION pos`，但 MFC 里 `POSITION` 是个**全局** typedef，而我的 `POSITION` 是 `CList` 的**嵌套**类型。解决办法是在 `common.h` 的 `SYMLIST` 定义之后加一行，把嵌套类型提升为全局名：

```cpp
class SYMLIST:public CList<SYMBOL,SYMBOL> { };
typedef CList<SYMBOL,SYMBOL>::POSITION POSITION;   // 让 PL.cpp 里的 POSITION pos 能用
```

### 修复 2：`stricmp` → `strcasecmp`（3 处）

**症状**：`stricmp was not declared`。

**根因**：`stricmp`（不分大小写比较字符串）是 VC/Windows 的名字，POSIX/Linux 下叫 `strcasecmp`，在 `<string.h>` 里。

**怎么改**：`PL.cpp` 第 189、227、1605 三处（标识符查重、保留字匹配），全局替换 `stricmp` → `strcasecmp`。

### 修复 3：`iscsymf`/`iscsym` → 自写包装（词法分析专用）

**症状**：`iscsymf was not declared`、`iscsym was not declared`。

**根因**：这俩是 Microsoft VC 给 `<ctype.h>` 加的扩展函数，专门判断"是不是合法的 C 标识符字符"：
- `iscsymf(c)`：标识符**首**字符（字母或下划线）
- `iscsym(c)`：标识符**后续**字符（字母、数字、下划线）

Linux 的 `<ctype.h>` 没有这俩。

**怎么改**：在 `PL.cpp` 顶部（`#include "common.h"` 之后）加两个 inline 包装，用标准 ctype 等价实现：

```cpp
static inline int iscsymf(int c) { return (isalpha(c) || c == '_'); }
static inline int iscsym(int c)  { return (isalnum(c) || c == '_'); }
```

调用处（`getSymbols` 里识别标识符）一行都不用改。

### 修复 4：解释器的"隐式 int" 和 `void main`

**症状**：`interpret/common.h` 的 `const STACKSIZE=2047` 编不过；`interpret.cpp` 的 `void main` 警告。

**根因**：
- `const STACKSIZE=2047` 是上世纪 C 的"隐式 int"写法（省略了类型，默认 int），现代 C++ 标准已禁止，GCC 报错。
- `void main(...)` 不符合 C++ 标准（标准要求 `main` 返回 `int`）。

**怎么改**：
- `const STACKSIZE` → `const int STACKSIZE`
- `void main(int arg,char ** argv)` → `int main(int arg,char ** argv)`（C++ 的 `main` 末尾可以不写 `return 0`，标准保证隐式返回 0）。

### 修复 5（最隐蔽、最关键）：换行符导致的行号全错

这一处不在原 TODO 计划里，是**跑起来后才发现的运行期 bug**，但影响很大，值得单独讲。

**症状**：编译器跑起来了、能生成代码，但报错时**所有错误的行号都显示成 Line 1**，而真正的源码内容明明在第 1~16 行。更糟的是，因为行号不对，错误恢复的逻辑也跟着乱。

**根因**：看 `PL.cpp` 词法分析 `getSymbols` 里对换行符的处理（原始代码）：

```cpp
case '\n':
    break;          // 换行符：直接忽略，行号不增
case '\r':
    lineNumber++;   // 只有回车符才让行号 +1
    break;
```

这是**老 VC 代码假设源码是 Windows 的 CRLF 换行**（`\r\n`）——靠 `\r` 累加行号，`\n` 跳过。但 CLAUDE.md 的协作约束要求**源码一律 LF 换行**（`.gitattributes` 强制 `* text=auto`，CRLF 的脚本在 Linux 会出各种问题）。LF 换行的文件**只有 `\n`、没有 `\r`**，所以 `lineNumber` 永远停在初始值 1。

**怎么改**：反过来——让 `\n` 递增行号，`\r` 忽略。这样既兼容 LF（只有 `\n`，加一次），也兼容 CRLF（`\r` 忽略、`\n` 加一次，不会重复加）：

```cpp
case '\r':
    break;          // CRLF 的前半，忽略，行号交给 \n 统一累加
case '\n':
    lineNumber++;   // 统一在换行处累加，兼容 LF/CRLF
    break;
```

改完后行号立刻全部正确。

### 调试中探明的 PL 语法要点（写测试程序必须知道）

修活编译器后，写测试程序 `test.pl` 时又踩了两个语法坑——它们不是 bug，是 PL 语言本身的语法要求，但和教科书 PL/0 不一样，必须知道：

1. **变量声明必须带类型**：要写 `var a, b : integer;`，不能写 Pascal 风格的 `var a, b;`。否则报 `error 0 应该是':'`。预定义类型有 `integer`/`char`/`boolean`。

2. **`read`/`write` 不是保留字，必须用 `call` 调用**：这是最坑的一点。看 `GetReserveWord` 里的保留字表：

   ```
   and begin const else if not or program type while
   array call do end mod of procedure then var
   ```

   里面**没有 `read` 和 `write`**！它们是被 `ENTERPREID` 预登记进符号表的**标准过程**（`kind=PROCEDURE`）。所以：
   - ❌ `write(c)` → 词法把 `write` 当普通 IDENT → 语句分析走 `ASSIGNMENT` → 查表发现 `write` 的 `kind` 是 PROCEDURE 不是 VARIABLE → 报 `error 16 应该是变量` + `error 7 应该是 ':='`。
   - ✅ `call write(c)` → 走 `CALL` 分支 → 正确识别为标准过程调用 → 生成 `WRITE` 指令。

   记住：**所有 read/write 都要写成 `call read(...)` / `call write(...)`**。

## 四、编译运行（验证链路）

```bash
# 在 WSL2 / Ubuntu 20.04 下，仓库根目录执行
g++ -o C/bin/PL.exe        C/src/PL/PL.cpp        -I C/include -lm
g++ -o C/bin/INTERPRET.exe C/src/interpret/interpret.cpp -I C/include -lm

# 编译 test.pl → 生成 test.pld（中间代码）+ test.lst（可读列表）+ test.lab
./C/bin/PL.exe test.pl

# 解释执行（输入从 stdin，每个 read 一个整数）
echo "5" | ./C/bin/INTERPRET.exe test.pld
# 输出：Your Output:15   （1+2+3+4+5）
```

已验证通过的功能：加法运算、`while` 循环、`procedure` 调用（含嵌套作用域）、`call read/write` 标准输入输出。错误处理（`error(n)` + 行号 + 同步点恢复）也正常工作。

## 五、文件清单（本次新增/修改）

| 文件 | 状态 | 说明 |
|------|------|------|
| `C/include/clist.h` | **新增** | MFC `CList` 的最小替代（AddHead/AddTail/GetHeadPosition/GetNext/POSITION） |
| `C/include/conio.h` | 已存在 | `getch` 的 termios shim（之前已做） |
| `C/src/PL/common.h` | 改 | include `clist.h`；提升 `POSITION` 为全局类型 |
| `C/src/PL/PL.cpp` | 改 | `stricmp`→`strcasecmp`；加 `iscsymf`/`iscsym` 包装；修 `\n` 行号 bug |
| `C/src/interpret/common.h` | 改 | `const STACKSIZE`→`const int` |
| `C/src/interpret/interpret.cpp` | 改 | `void main`→`int main` |
| `test.pl` | **新增** | 累加求和示例（含 procedure + while），用于上机演示 |

> 注意协作：`clist.h` 和修改后的源码都已 commit，同伴 pull 后在真机 Ubuntu 20.04 上直接 `g++` 即可编译，无需额外安装 MFC 或改代码。
