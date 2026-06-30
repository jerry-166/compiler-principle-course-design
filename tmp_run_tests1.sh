#!/bin/bash
# Tests1 回归：只比较有 .exp 的用例，规范化行尾
cd /mnt/d/workspace/bianyiyuanli || exit 1
pass=0
fail=0
failed_list=""
for f in Tests/Tests1/inputs/*.cmm; do
  base=$(basename "$f" .cmm)
  expfile="Tests/Tests1/expects/$base.exp"
  [ -e "$expfile" ] || continue   # 跳过无 .exp 的（选做用例）
  got=$(./C/bin/parser "$f" 2>&1 | tr -d '\r')
  exp=$(cat "$expfile" | tr -d '\r')
  if [ "$got" = "$exp" ]; then
    pass=$((pass+1))
  else
    fail=$((fail+1))
    failed_list="$failed_list $base"
  fi
done
echo "Tests1: $pass passed, $fail failed (of $(ls Tests/Tests1/expects/*.exp | wc -l) comparable)"
[ -n "$failed_list" ] && echo "FAILED:$failed_list"
