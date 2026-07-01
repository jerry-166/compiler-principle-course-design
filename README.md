# 编译原理课程设计

C-- 编译器（分阶段开发，实践 1.1~5）+ PL/0 编译器（核心交付物，上机检查对象）

- 授课：朱雪峰 | 2026.6.28–7.5
- 环境：Ubuntu 20.04 / GCC 7.5.0 / Flex 2.6.4 / Bison 3.5.1
- 目标 MIPS 仿真器：QtSpim 9.1.9

## 快速开始

```bash
# 编译 PL/0 编译器
g++ -o C/bin/PL.exe C/src/PL/PL.cpp -I C/include -lm

# 编译 PL/0 解释器
g++ -o C/bin/INTERPRET.exe C/src/interpret/interpret.cpp -I C/include -lm

# 运行 PL/0 全链路：源码 → 编译 → 中间代码 → 解释执行 → 输出
./C/bin/PL.exe test.pl && ./C/bin/INTERPRET.exe test.pld

# 编译 C-- 词法分析器（Flex 生成 .c → gcc 编译）
flex -o C/src/lex/lex.yy.c C/src/lex/lexer.l
gcc -o C/bin/scanner C/src/lex/lex.yy.c -lfl

# 编译 C-- 语法分析器（Bison 生成 .c/.h → gcc 编译）
bison -d -o C/src/syntax/parser.tab.c C/src/syntax/parser.y
gcc -o C/bin/parser C/src/syntax/parser.tab.c C/src/syntax/tree.c -lfl
```

## 目录结构

```
C/
├── src/
│   ├── PL/                      # PL/0 编译器源码
│   │   ├── PL.cpp               # [PL/0] 递归下降编译器主程序
│   │   └── common.h             # [PL/0] 符号表、CList 适配
│   ├── interpret/              # PL/0 抽象机解释器
│   │   ├── interpret.cpp        # [PL/0] 解释执行引擎
│   │   └── common.h            # [PL/0] 共用定义
│   ├── lex/                     # C-- 词法分析器
│   │   ├── lexer.l             # [1.1] Flex 词法规则源码
│   │   └── lex.yy.c            # [1.1] Flex 自动生成
│   └── syntax/                  # C-- 语法分析器
│       ├── parser.y            # [1.2] Bison 语法规则
│       ├── parser.tab.c/.h     # [1.2] Bison 自动生成
│       ├── node.h              # [1.2] 语法树节点定义
│       ├── tree.c              # [1.2] 语法树构建函数
│       └── main_tree.c         # [1.2] parser 入口（建树+打印）
│   └── sem/                     # C-- 语义分析器
│       ├── type.h / type.c     # [2]   类型系统（BASIC/ARRAY/STRUCTURE）
│       ├── symtab.h / symtab.c # [2]   符号表（pjw 哈希 + 函数式作用域栈）
│       ├── sem.h / sem.c       # [2]   语义分析主体（visit/check_exp/错误输出）
│       └── main_sem.c          # [2]   cmmc / cmmc_full 入口
├── include/                     # 本地适配头
│   ├── conio.h                 # Linux 替代 DOS conio.h
│   └── clist.h                 # 纯 C++ CList shim（替代 MFC）
└── bin/                         # 编译产物
    ├── PL.exe                  # [PL/0] PL/0 编译器
    ├── INTERPRET.exe           # [PL/0] 解释器
    ├── scanner                 # [1.1] C-- 词法分析器
    ├── parser                  # [1.2] C-- 语法分析器
    ├── cmmc                    # [2]   C-- 语义分析器（必做，无宏）
    └── cmmc_full               # [2]   C-- 语义分析器（选做三宏全开）

docs/                            # 配套文档（按阶段分文件夹）
├── PL0编译器移植与修复.md       # [PL/0] PL/0 修活全过程
├── C--语言分析.md              # [参考] C-- 语言特性分析
├── C--词法语法分析.md          # [参考] 1.1+1.2 学习文档
├── 1.1-词法分析/               # → 实践 1.1 词法分析文档
├── 1.2-语法分析/               # → 实践 1.2 语法分析文档
├── 阶段交付总览.md              # 按实践阶段的文件级归属表
└── 变更日志.md                  # 按时间倒序记录增量变更

Tests/                           # C-- 测试集
├── Tests1/  (inputs/ + expects/) # 词法 + 语法分析测试（23 个用例）
├── Tests2/  (inputs/ + expects/) # 语义分析测试（30 个用例，A/B/C/D/E 五组）
└── Tests3/  (inputs/)           # 中间代码生成测试（待 expects）
```

**标签说明**：`[PL/0]` = PL/0 主线，`[1.1]` = 实践 1.1 词法分析，`[1.2]` = 实践 1.2 语法分析，`[2]` = 实践 2 语义分析，`[参考]` = 跨阶段参考文档。

## 阶段交付清单

| 标签 | 阶段 | 代码产物 | 文档产物 | 状态 |
|------|------|---------|---------|------|
| [PL/0] | PL/0 编译器 | PL.cpp, interpret.cpp | PL0编译器移植与修复.md | ✅ 完成 |
| [1.1] | 词法分析 | lexer.l → scanner | 1.1-词法分析/ | ✅ 完成 |
| [1.2] | 语法分析 | parser.y → parser | 1.2-语法分析/ | ✅ 完成 |
| [2] | 语义分析 | sem/*.c → cmmc, cmmc_full | 2.0-语义分析/ | ✅ 完成（Tests2 30/30）|
| [3] | 中间代码生成 | （待开发） | — | ⬜ 待开始 |
| [4] | 目标代码生成 | （待开发） | — | ⬜ 待开始 |
| [5] | 代码优化 | （待开发） | — | ⬜ 待开始 |

> 完整的文件级归属表见 [阶段交付总览](docs/阶段交付总览.md)，增量变更记录见 [变更日志](docs/变更日志.md)。

## 配套文档索引

| 文档 | 用途 |
|------|------|
| [CLAUDE.md](CLAUDE.md) | AI 协作规则（开发约束、目录规范、兼容性要求） |
| [CODEBUDDY.md](CODEBUDDY.md) | 课程详细信息（考核评分、时间线、完整文档索引） |
| [TODO.md](TODO.md) | 遗留问题备忘（PL/0 修复清单） |
| [PL0编译器移植与修复](docs/PL0编译器移植与修复.md) | PL/0 编译器修活全过程（含协作坑记录） |
| [C--语言分析](docs/C--语言分析.md) | C-- 语言特性分析（类型、运算符、结构体、函数等） |
| [C--词法语法分析](docs/C--词法语法分析.md) | 1.1+1.2 综合学习文档（Flex/Bison 原理 + C-- 文法） |
| [1.1 词法分析文档](docs/1.1-词法分析/) | 词法分析原理 + 测试指南 + 测试记录 |
| [1.2 语法分析文档](docs/1.2-语法分析/) | 语法分析原理 + 文法规则 + 语法树构建 |
| [2.0 语义分析文档](docs/2.0-语义分析/) | 语义分析原理 + 宏开关机制 + 测试记录 |
