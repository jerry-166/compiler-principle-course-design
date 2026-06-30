#!/bin/bash
# 实践2 语义分析批量测试脚本
# 必做(cmmc)测 A/B/C/D，选做(cmmc_full)测 E。
# 用法：bash 操作手册及脚本/run_tests_sem.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TESTS="$ROOT/Tests/Tests2"
BIN="$ROOT/C/bin"

# 编译两个可执行
echo "=== 编译 cmmc (必做) ==="
gcc -std=c99 -Wall -o "$BIN/cmmc" \
    "$ROOT/C/src/syntax/parser.tab.c" "$ROOT/C/src/syntax/tree.c" \
    "$ROOT/C/src/lex/lex.yy.c" \
    "$ROOT/C/src/sem/type.c" "$ROOT/C/src/sem/symtab.c" \
    "$ROOT/C/src/sem/sem.c" "$ROOT/C/src/sem/main_sem.c" \
    -I "$ROOT/C/src/syntax" -I "$ROOT/C/src/sem" -lm \
    || { echo "cmmc 编译失败"; exit 1; }

echo "=== 编译 cmmc_full (选做，三宏全开) ==="
gcc -std=c99 -Wall -o "$BIN/cmmc_full" \
    "$ROOT/C/src/syntax/parser.tab.c" "$ROOT/C/src/syntax/tree.c" \
    "$ROOT/C/src/lex/lex.yy.c" \
    "$ROOT/C/src/sem/type.c" "$ROOT/C/src/sem/symtab.c" \
    "$ROOT/C/src/sem/sem.c" "$ROOT/C/src/sem/main_sem.c" \
    -DENABLE_FUNC_DECL -DENABLE_SCOPE -DENABLE_STRUCT_EQ \
    -I "$ROOT/C/src/syntax" -I "$ROOT/C/src/sem" -lm \
    || { echo "cmmc_full 编译失败"; exit 1; }
echo

run_group() {
  local exe="$1"; shift
  local group="$1"; shift
  local pass=0 fail=0 total=0
  echo "=== $exe @ $group 系列 ==="
  for f in "$TESTS/inputs/$group"-*.cmm; do
    [ -e "$f" ] || continue
    total=$((total+1))
    base=$(basename "$f" .cmm)
    expfile="$TESTS/expects/$base.exp"
    [ -e "$expfile" ] || { echo "  SKIP $base (无 .exp)"; continue; }
    # 行尾归一化（.exp 可能是 CRLF，输出是 LF）
    got=$("$BIN/$exe" "$f" 2>&1 | tr -d '\r')
    exp=$(cat "$expfile" | tr -d '\r')
    if [ "$got" = "$exp" ]; then
      echo "  PASS $base"; pass=$((pass+1))
    else
      echo "  FAIL $base"
      echo "    expected: $(echo "$exp" | head -1)"
      echo "    got:      $(echo "$got" | head -1)"
      fail=$((fail+1))
    fi
  done
  echo "  -> $group: $pass/$total passed"
  echo
  TOTAL_PASS=$((TOTAL_PASS + pass))
  TOTAL_FAIL=$((TOTAL_FAIL + fail))
}

TOTAL_PASS=0; TOTAL_FAIL=0
run_group cmmc       A
run_group cmmc       B
run_group cmmc       C
run_group cmmc       D
run_group cmmc_full  E
echo "================================"
echo "总计：$TOTAL_PASS passed, $TOTAL_FAIL failed"
