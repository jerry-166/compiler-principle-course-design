#!/bin/bash
# batch_test_lex.sh — 批量运行词法分析器，收集输入+输出到 docs/词法分析测试记录.md
# 用法：bash batch_test_lex.sh
# 说明：scanner 是 Linux ELF，需通过 WSL 运行

SCANNER="./C/bin/scanner"
TESTS_DIR="Tests/Tests1"
OUTPUT_FILE="docs/词法分析测试记录.md"

# 检测是否在 WSL 内
in_wsl=false
grep -qi microsoft /proc/version 2>/dev/null && in_wsl=true

if [ ! -f "$SCANNER" ]; then
    echo "错误: $SCANNER 不存在，请先编译词法分析器"
    echo "  flex -o C/src/lex/lex.yy.c C/src/lex/lexer.l"
    echo "  gcc C/src/lex/lex.yy.c -o C/bin/scanner"
    exit 1
fi

# 运行 scanner 的命令（WSL 内直接执行，Windows 上通过 wsl.exe）
if $in_wsl; then
    run_scanner() { "$SCANNER" "$1" 2>/dev/null; }
else
    run_scanner() { wsl.exe -d Ubuntu -- bash -c "cd /mnt/d/workspace/bianyiyuanli && $SCANNER '$(wslpath -w "$1" 2>/dev/null || echo "/mnt/d/workspace/bianyiyuanli/$1")'" 2>/dev/null; }
fi

echo "正在批量测试 Tests1 下所有输入文件..."

# 临时目录收集输出
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

INPUTS="$TESTS_DIR/inputs"
EXPECTS="$TESTS_DIR/expects"

cat > "$OUTPUT_FILE" << 'HEADER'
# 词法分析测试记录

> 词法分析器：`C/src/lex/lexer.l`
> 可执行文件：`C/bin/scanner`
> 测试集：`Tests/Tests1/`（A-1~A-10, B-1~B-2, C-1~C-2, D-1~D-3, E1-1~E1-2, E2-1~E2-2, E3-1~E3-2）
> 生成时间：自动生成，重新运行 `bash batch_test_lex.sh` 可更新

---

HEADER

count=0
total=$(find "$INPUTS" -name "*.cmm" | wc -l)

# 遍历所有输入文件，按文件名自然排序
for input_file in $(find "$INPUTS" -name "*.cmm" | sort); do
    count=$((count + 1))
    basename=$(basename "$input_file" .cmm)
    relpath="Tests/Tests1/inputs/${basename}.cmm"

    # 对应的期望输出
    expect_file="$EXPECTS/${basename}.exp"
    has_expect=false
    [ -f "$expect_file" ] && has_expect=true

    echo "  [$count/$total] $basename ..."

    # 运行词法分析器，保存输出
    scanner_output="$TMPDIR/${basename}.out"
    if $in_wsl; then
        "$SCANNER" "$input_file" > "$scanner_output" 2>/dev/null
    else
        wsl.exe -d Ubuntu -- bash -c "cd /mnt/d/workspace/bianyiyuanli && $SCANNER Tests/Tests1/inputs/${basename}.cmm" > "$scanner_output" 2>/dev/null
    fi

    # 写入文档
    echo "## ${basename}" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    echo "**输入文件**：\`${relpath}\`" >> "$OUTPUT_FILE"
    if $has_expect; then
        echo "**期望输出**：\`Tests/Tests1/expects/${basename}.exp\`（语法树/错误，供后续阶段参考）" >> "$OUTPUT_FILE"
    else
        echo "**期望输出**：无" >> "$OUTPUT_FILE"
    fi
    echo "" >> "$OUTPUT_FILE"

    echo "### 输入代码" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    echo '```cmm' >> "$OUTPUT_FILE"
    cat "$input_file" >> "$OUTPUT_FILE"
    echo '```' >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"

    echo "### 词法分析器输出" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
    echo '```' >> "$OUTPUT_FILE"
    cat "$scanner_output" >> "$OUTPUT_FILE"
    echo '```' >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"

    # 分隔线
    echo "---" >> "$OUTPUT_FILE"
    echo "" >> "$OUTPUT_FILE"
done

echo "完成！测试记录已写入 $OUTPUT_FILE"
