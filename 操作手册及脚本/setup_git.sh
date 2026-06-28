#!/bin/bash
# Git + SSH 一键配置脚本
# 用法: chmod +x setup_git.sh && ./setup_git.sh
# 注意：公钥需要手动添加到 GitHub

set -e

GIT_NAME="jerry"
GIT_EMAIL="2132049351@qq.com"
GITHUB_USER="jerry-166"
REPO_SSH="git@github.com:jerry-166/compiler-principle-course-design.git"

echo "=========================================="
echo "  Git + SSH 配置脚本"
echo "=========================================="

# 1. 配置用户信息
echo ""
echo "[1/5] 配置 Git 用户信息..."
git config --global user.name "$GIT_NAME"
git config --global user.email "$GIT_EMAIL"
echo "  ✓ user.name = $GIT_NAME"
echo "  ✓ user.email = $GIT_EMAIL"

# 2. 生成 SSH 密钥
echo ""
echo "[2/5] 生成 SSH 密钥..."
if [ -f ~/.ssh/id_ed25519 ]; then
    echo "  ~/.ssh/id_ed25519 已存在，跳过生成"
else
    ssh-keygen -t ed25519 -C "$GIT_EMAIL" -N "" -f ~/.ssh/id_ed25519
    echo "  ✓ SSH 密钥已生成"
fi

# 3. 显示公钥
echo ""
echo "[3/5] 你的公钥如下，请添加到 GitHub："
echo "  GitHub → Settings → SSH and GPG keys → New SSH key"
echo ""
cat ~/.ssh/id_ed25519.pub
echo ""
read -p "  添加完成后按回车继续..."

# 4. 测试 SSH 连接
echo ""
echo "[4/5] 测试 GitHub SSH 连接..."
echo "  首次连接需输入 yes 确认指纹..."
ssh -T git@github.com || true
echo "  ✓ SSH 连接测试完成"

# 5. 切换 remote 为 SSH 格式
echo ""
echo "[5/5] 切换远程地址为 SSH..."
cd "$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "  ✗ 请在 git 仓库目录下运行此脚本"
    exit 1
}
CURRENT_URL=$(git remote get-url origin 2>/dev/null || echo "")
if [ "$CURRENT_URL" = "$REPO_SSH" ]; then
    echo "  已经是 SSH 地址，无需修改"
else
    git remote set-url origin "$REPO_SSH"
    echo "  ✓ 远程地址已切换为 SSH"
fi

echo ""
echo "=========================================="
echo "  配置完成！运行 git push origin main 推送"
echo "=========================================="
