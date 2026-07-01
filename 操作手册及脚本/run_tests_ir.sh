#!/bin/bash
# 实践3 中间代码生成批量测试脚本
# 必做(cmmc_ir)测 A/B/C，选做(cmmc_ir_full)测 E1/E2。
# 验证链：cmmc_ir 生成 .ir -> irvm 执行(读 stdin) -> 收集 WRITE 输出 -> 与期望 write 序列对比。
# 用法：bash 操作手册及脚本/run_tests_ir.sh [必做|选做|all]  （默认必做）
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TESTS="$ROOT/Tests/Tests3/inputs"
BIN="$ROOT/C/bin"
TMP="/tmp/ir_tests"
mkdir -p "$TMP"

MODE="${1:-必做}"

# 选做实现开关：Task 9/10 完成后把下面这行改成 "yes"。
# "no" 时 cmmc_ir_full 视为未实现，选做/all 模式会跳过 E1/E2 并提示。
OPTIONAL_READY="no"

# 颜色
G=$'\033[32m'
R=$'\033[31m'
N=$'\033[0m'

# ---------- 测试数据：name|reads|expect ----------
# reads 用逗号分隔，Empty 表示无输入；expect 是 write 序列，逗号分隔
CASES_REQUIRED=(
  "A-1|Empty|120,325"
  "A-2|40,30,20|7,2000,3000"
  "A-2|10,50,8|6,2000,3000"
  "A-2|60,50,5|7,1000,3000"
  "A-3|2|3,7,28"
  "A-3|4|3,19,28"
  "A-3|0|3,3,28"
  "A-4|3|24"
  "A-4|2|3"
  "A-5|6|8"
  "A-5|5|5"
  "A-5|0|0"
  "B-1|3,8,2,8,1,0|1,8,17"
  "B-1|5,4,3,9,9,2|3,9,19"
  "B-1|1,2,3,4,5,0|4,5,11"
  "B-2|5|4"
  "B-2|20|1"
  "B-3|1|7,7"
  "B-3|3|693,11"
  "B-3|5|135135,15"
  "C-1|48,18|6"
  "C-1|100,35|5"
  "C-1|17,13|1"
  "C-2|12345|15"
  "C-2|909|18"
  "C-2|7|7"
  "C-2|0|0"
)

CASES_OPTIONAL=(
  "E1-1|Empty|42,13"
  "E1-2|4,5|16"
  "E1-2|10,3|20"
  "E1-3|5,4,3|13"
  "E1-3|1,2,15|1"
  "E1-3|0,0,-1|0"
  "E2-1|Empty|31,6"
  "E2-2|Empty|12,21"
  "E2-3|9|1,23"
  "E2-3|0|1,32"
)

# ---------- 编译 ----------
compile_required() {
  echo "=== 编译 cmmc_ir (必做) ==="
  gcc -std=c99 -Wall -o "$BIN/cmmc_ir" \
      "$ROOT/C/src/syntax/parser.tab.c" "$ROOT/C/src/syntax/tree.c" \
      "$ROOT/C/src/lex/lex.yy.c" \
      "$ROOT/C/src/sem/type.c" \
      "$ROOT/C/src/ir/ir.c" "$ROOT/C/src/ir/operand.c" \
      "$ROOT/C/src/ir/translate.c" "$ROOT/C/src/ir/translate_exp.c" "$ROOT/C/src/ir/translate_addr.c" \
      "$ROOT/C/src/ir/main_ir.c" \
      -I "$ROOT/C/src/syntax" -I "$ROOT/C/src/sem" -I "$ROOT/C/src/ir" -lm \
      || { echo "cmmc_ir 编译失败"; exit 1; }
  echo "=== 编译 irvm ==="
  gcc -std=c99 -Wall -o "$BIN/irvm" "$ROOT/C/src/irvm/irvm.c" -lm \
      || { echo "irvm 编译失败"; exit 1; }
  echo
}

compile_optional() {
  echo "=== 编译 cmmc_ir_full (选做) ==="
  gcc -std=c99 -Wall -o "$BIN/cmmc_ir_full" \
      "$ROOT/C/src/syntax/parser.tab.c" "$ROOT/C/src/syntax/tree.c" \
      "$ROOT/C/src/lex/lex.yy.c" \
      "$ROOT/C/src/sem/type.c" \
      "$ROOT/C/src/ir/ir.c" "$ROOT/C/src/ir/operand.c" \
      "$ROOT/C/src/ir/translate.c" "$ROOT/C/src/ir/translate_exp.c" "$ROOT/C/src/ir/translate_addr.c" \
      "$ROOT/C/src/ir/main_ir.c" \
      -DENABLE_STRUCT -DENABLE_HIGH_DIM_ARRAY \
      -I "$ROOT/C/src/syntax" -I "$ROOT/C/src/sem" -I "$ROOT/C/src/ir" -lm \
      || { echo "cmmc_ir_full 编译失败（选做宏可能未实现）"; return 1; }
  echo
}

# ---------- 单组测试 ----------
# 参数：exe  name  reads  expect
run_one() {
  local exe="$1" name="$2" reads="$3" expect="$4"
  local src="$TESTS/$name.cmm"
  local irfile="$TMP/$name.ir"
  local infile="$TMP/$name.in"
  [ -e "$src" ] || { echo "  ${R}FAIL${N} $name  [源文件缺失]"; return 1; }

  # 生成 IR
  if ! "$BIN/$exe" "$src" "$irfile" >/dev/null 2>&1; then
    echo "  ${R}FAIL${N} $name  [IR 生成失败/Cannot translate]"
    return 1
  fi

  # 准备 stdin
  if [ "$reads" = "Empty" ]; then
    : > "$infile"
  else
    echo "$reads" | tr ',' '\n' > "$infile"
  fi

  # 跑 irvm，把 WRITE 输出多行转逗号串
  local got
  got=$("$BIN/irvm" "$irfile" < "$infile" 2>/dev/null | tr -d '\r' | paste -sd, -)

  if [ "$got" = "$expect" ]; then
    echo "  ${G}PASS${N} $name  [reads=$reads]  -> $got"
    return 0
  else
    echo "  ${R}FAIL${N} $name  [reads=$reads]"
    echo "       expected: $expect"
    echo "       got:      $got"
    return 1
  fi
}

# ---------- 跑一组数据 ----------
# 参数：exe  CASES 数组名  group 名
run_group() {
  local exe="$1"; local arr_name="$2"; local label="$3"
  local pass=0 fail=0 total=0
  # 间接展开数组
  local arr_ref="$arr_name[@]"
  local entries=("${!arr_ref}")
  echo "=== $label ($exe) ==="
  for entry in "${entries[@]}"; do
    total=$((total+1))
    local name rest reads expect
    name=${entry%%|*}
    rest=${entry#*|}
    reads=${rest%%|*}
    expect=${rest##*|}
    if run_one "$exe" "$name" "$reads" "$expect"; then
      pass=$((pass+1))
    else
      fail=$((fail+1))
    fi
  done
  echo "  -> $label: ${G}$pass${N} pass, ${R}$fail${N} fail (共 $total 组)"
  echo
  TOTAL_PASS=$((TOTAL_PASS + pass))
  TOTAL_FAIL=$((TOTAL_FAIL + fail))
  TOTAL_COUNT=$((TOTAL_COUNT + total))
}

TOTAL_PASS=0; TOTAL_FAIL=0; TOTAL_COUNT=0

case "$MODE" in
  必做|required)
    compile_required
    run_group cmmc_ir CASES_REQUIRED "必做"
    echo "==== 必做: ${G}$TOTAL_PASS${N} pass, ${R}$TOTAL_FAIL${N} fail (共 $TOTAL_COUNT 组) ===="
    ;;
  选做|optional)
    if [ "$OPTIONAL_READY" != "yes" ]; then
      echo "${R}选做（结构体/高维数组/数组参数）尚未实现（Task 9/10），跳过 E1/E2。${N}"
      echo "完成后请将脚本顶部 OPTIONAL_READY 改为 yes 再跑。"
      exit 0
    fi
    compile_optional
    run_group cmmc_ir_full CASES_OPTIONAL "选做"
    echo "==== 选做: ${G}$TOTAL_PASS${N} pass, ${R}$TOTAL_FAIL${N} fail (共 $TOTAL_COUNT 组) ===="
    ;;
  all)
    compile_required
    run_group cmmc_ir CASES_REQUIRED "必做"
    echo
    if [ "$OPTIONAL_READY" = "yes" ]; then
      compile_optional
      run_group cmmc_ir_full CASES_OPTIONAL "选做"
    else
      echo "${R}选做尚未实现（Task 9/10），跳过 E1/E2。完成后将 OPTIONAL_READY 改为 yes。${N}"
    fi
    echo "==== 总计: ${G}$TOTAL_PASS${N} pass, ${R}$TOTAL_FAIL${N} fail (共 $TOTAL_COUNT 组) ===="
    ;;
  *)
    echo "用法：bash 操作手册及脚本/run_tests_ir.sh [必做|选做|all]"
    exit 1
    ;;
esac
