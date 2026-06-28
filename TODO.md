# 遗留问题备忘（PL/0 修复）

> 这份文件**只记录挂起、没做完、需要以后回来收尾的遗留问题**，作为提醒。
> 当前正在进行的任务（如 C-- 词法分析）不写在这里。
> 最后更新：2026-06-28

---

## ✅ 已完成：PL/0 编译器修活（2026-06-28）

PL/0 编译器已修活并跑通全链路 `test.pl → PL.exe → test.pld → INTERPRET.exe → 输出`。
编译零错误，解释执行结果正确（已验证加法、while 循环、procedure 调用、call read/write）。
详细修复过程见 `docs/PL0编译器移植与修复.md`。

实际修复内容（相比原 TODO 计划有补充）：

1. ✅ 写了纯 C++ 的 CList shim（`C/include/clist.h`），不止 AddHead，
   还实现了源码用到的 AddTail/GetHeadPosition/GetNext/POSITION（SYMINLIST、COPYLIST、listsAdd 都用到）。
2. ✅ PL.cpp 3 处 `stricmp` → `strcasecmp`。
3. ✅ interpret：`const STACKSIZE` → `const int`；`void main` → `int main`。
4. ✅ `common.h` 接入 `clist.h`，并把 CList 嵌套 `POSITION` 提升为全局 `POSITION`（PL.cpp 用的是非限定名）。
5. ✅ **额外修复**：PL.cpp 用了 Microsoft VC 的 `iscsymf`/`iscsym`（Linux 无），加了两个等价 inline 包装。
6. ✅ **额外修复（关键）**：词法分析行号只在 `\r` 递增、`\n` 跳过——这是老 VC 假设 CRLF 换行。
   Linux 下 `.pl` 是 LF 换行，导致所有符号的 lineNumber 永远=1。改为 `\n` 递增、`\r` 忽略，兼容 LF/CRLF。
7. ✅ 编译验证 + 跑通链路。

### PL 语言语法要点（调试中探明，写测试程序时必须遵守）

- 变量声明**必须带类型**：`var a, b : integer;`（不是 Pascal 的 `var a,b;`）。
  预定义类型：`integer` / `char` / `boolean`。数组用 `type ... = array[...] of ...`。
- **`read`/`write` 不是保留字**，是预登记的标准过程，必须用 `call read(x)` / `call write(x)`，
  直接写 `write(c)` 会被当成赋值语句左值报 "error 16 应该是变量"。
- 标准保留字：`and begin const else if not or program type while array call do end mod of procedure then var`。

---

**接续方法**：回来修 PL/0 时，先读本文件的"已探明的事实"，再读 `CLAUDE.md` 的协作约束，然后按"要做的事"清单执行即可。

### 要做的事

1. 写纯 C/C++ 的 CList shim（只需支持 `AddHead`，放 `C/include/`）
2. 改 PL.cpp 里 3 处 `stricmp` → `strcasecmp`
3. interpret.cpp：`void main` → `int main`（return 0）；`interpret/common.h` 隐式 int → `const int`
4. 编译验证：`g++ -o bin/PL.exe src/PL/PL.cpp -I include -lm`，同理编译 INTERPRET
5. 跑通 `test.pl → test.pld → INTERPRET` 链路

### 已探明的事实（照此做，不用重新查）

- **源码是 C++（.cpp）**，用 `g++` 编译，不是 .c/gcc。
- **目录是小写 `interpret/`**（zip 解压出来的），主准则文档写的是大写 `INTERPRET/`。⚠️ 协作时这个大小写差异要统一（见下方坑）。
- **MFC 依赖范围极小**：
  - `common.h:70` `class SYMLIST:public CList<SYMBOL,SYMBOL>` 是唯一继承点
  - PL.cpp 全文**只用了 `AddHead` 这一个 CList 方法**（用 20 次）
  - 所以不用实现完整 MFC CList，写个支持 AddHead 的最小链表容器即可
- **`PL/common.h` 冗余的 `<iostream.h>` `<Afxtempl.h>` 已删**（确认过 PL.cpp 没真用 cout/其它 CList 接口）。
- **interpret.cpp 第 123 行的 `getch()`** 已用 `C/include/conio.h` 的 termios shim 解决。
- **`interpret/common.h` 第 8 行 `const STACKSIZE=2047`** 是老式隐式 int，GCC 报错，要改成 `const int`。
- **`interpret.cpp` 的 `void main(...)`** 不标准，要改成 `int main(...)` 并 return 0。
- **CRLF 已清理**：4 个源码文件原本是 CRLF，已转 LF。
- **原始注释是 GBK 编码**，在 UTF-8 终端里是乱码（如 `common.h` 里 `SYMBOL_ITEM` 后面的注释），读代码理解意图时可能要转码。

### 相关坑（协作时注意）

- `interpret`（源码小写）vs `INTERPRET`（主准则大写）：真机 Linux 大小写敏感，要统一，否则同伴那边报 `No such file`。
- 本地适配补丁（`C/include/conio.h`、将来的 CList shim）必须 commit，同伴 pull 后即用。

---

**接续方法**：回来修 PL/0 时，先读本文件的"已探明的事实"，再读 `CLAUDE.md` 的协作约束，然后按"要做的事"清单执行即可。
