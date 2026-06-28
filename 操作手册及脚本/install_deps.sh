#!/bin/bash
# 编译原理课程设计 - 开发环境依赖安装脚本
# 用法: chmod +x install_deps.sh && ./install_deps.sh

set -e

echo "=========================================="
echo "  编译原理课程设计 - 环境依赖安装"
echo "=========================================="

# 目标版本（课程要求）
TARGET_GCC="7.5.0"
TARGET_FLEX="2.6.4"
TARGET_BISON="3.5.1"
TARGET_SPIM="9.1.9"

echo ""
echo ">>> 更新软件源..."
sudo apt update

echo ""
echo ">>> 安装基础编译工具 (build-essential, gcc, g++, make)..."
sudo apt install -y build-essential

echo ""
echo ">>> 安装词法分析工具 flex..."
sudo apt install -y flex

echo ""
echo ">>> 安装语法分析工具 bison..."
sudo apt install -y bison

echo ""
echo ">>> 安装 MIPS 仿真器 spim..."
sudo apt install -y spim

echo ""
echo ">>> 安装其他辅助工具 (unzip, wget)..."
sudo apt install -y unzip wget

# 解压 irsim（课程配套虚拟机）
IRSIM_ZIP="资料/现课程要求(AICanUse)/irsim.zip"
if [ -f "$IRSIM_ZIP" ]; then
    echo ""
    echo ">>> 解压 irsim..."
    unzip -o "$IRSIM_ZIP" -d ~/irsim 2>/dev/null
    echo "    irsim 已解压到 ~/irsim/irsim/"
else
    echo ""
    echo "    [!] 未找到 $IRSIM_ZIP，跳过 irsim 解压"
fi

echo ""
echo "=========================================="
echo "  已安装工具版本"
echo "=========================================="

check_version() {
    local name=$1
    local cmd=$2
    local target=$3
    local version=$($cmd 2>/dev/null | head -1)
    echo ""
    echo "  $name:"
  echo "    已安装: $version"
    echo "    课程要求: $target"
    if echo "$version" | grep -q "$target"; then
        echo "    ✅ 版本匹配"
    else
        echo "    ⚠️  版本不同（不影响开发，避免使用新语法特性即可）"
    fi
}

check_version "GCC"        "gcc --version"        "$TARGET_GCC"
check_version "G++"        "g++ --version"        "$TARGET_GCC"
check_version "Flex"       "flex --version"       "$TARGET_FLEX"
check_version "Bison"      "bison --version"      "$TARGET_BISON"
check_version "Make"       "make --version"       "-"
check_version "SPIM"       "spim -version"        "$TARGET_SPIM"

echo ""
echo "=========================================="
echo "  安装完成!"
echo "=========================================="
echo ""
echo "下一步: 解压 PL/0 编译器源码并开始开发"
echo "  unzip \"资料/现课程要求(AICanUse)/PL编译程序——C语言代码.zip\" -d C/"
echo ""

