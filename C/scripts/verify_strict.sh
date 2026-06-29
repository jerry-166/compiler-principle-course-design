#!/bin/bash
# verify_strict.sh - 严格逐字节 diff（不剥 CRLF）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$ROOT" || exit 1
INPUTS="$ROOT/Tests/Tests1/inputs"
EXPECTS="$ROOT/Tests/Tests1/expects"
PARSER="$ROOT/C/bin/parser"

pass=0; fail=0; missing=0
for f in $(ls "$INPUTS" | grep '\.cmm$' | sort); do
  base=$(basename "$f" .cmm)
  if [ ! -f "$EXPECTS/$base.exp" ]; then
    echo "MISSING $base"; missing=$((missing+1)); continue
  fi
  "$PARSER" "$INPUTS/$f" > /tmp/strict_$base.txt 2>&1
  if diff -q "$EXPECTS/$base.exp" /tmp/strict_$base.txt > /dev/null 2>&1; then
    pass=$((pass+1))
  else
    echo "STRICT-FAIL $base"
    fail=$((fail+1))
  fi
done
echo "---"
echo "STRICT PASS=$pass  STRICT-FAIL=$fail  MISSING=$missing"
