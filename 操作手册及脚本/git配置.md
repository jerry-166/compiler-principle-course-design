# Git 配置与 SSH 连接操作手册

> 适用环境：WSL2 / Ubuntu / Git Bash
> 最后验证：2026-06-28

## 目录

1. [前置检查](#1-前置检查)
2. [配置用户信息](#2-配置用户信息)
3. [生成 SSH 密钥并关联 GitHub](#3-生成-ssh-密钥并关联-github)
4. [推送代码到 GitHub](#4-推送代码到-github)
5. [HTTPS 备用方案（PAT）](#5-https-备用方案pat)
6. [国内网络问题处理](#6-国内网络问题处理)
7. [附：自动化脚本](#7-附自动化脚本)

---

## 1. 前置检查

git 命令必须在 git 仓库目录下执行，否则报 `fatal: not a git repository`：

```bash
pwd                              # 查看当前目录
git rev-parse --show-toplevel    # 确认是 git 仓库，会显示仓库根路径
```

## 2. 配置用户信息

首次使用 Git 必须设置用户名和邮箱（提交时会记录在 commit 中）：

```bash
git config --global user.name "你的用户名"    # 例: jerry
git config --global user.email "你的邮箱"     # 例: 2132049351@qq.com

# 验证
git config --list
```

- `--global`：全局生效，所有仓库共用
- 不加 `--global`：只对当前仓库生效

## 3. 生成 SSH 密钥并关联 GitHub

### 3.1 生成密钥对

```bash
ssh-keygen -t ed25519 -C "你的邮箱"    # 例: 2132049351@qq.com
# 一路回车，使用默认路径 ~/.ssh/id_ed25519
```

### 3.2 添加公钥到 GitHub

```bash
cat ~/.ssh/id_ed25519.pub
# 复制输出的全部内容
```

浏览器打开 GitHub → Settings → SSH and GPG keys → New SSH key，粘贴保存。

### 3.3 测试连接（首次需确认指纹）

```bash
ssh -T git@github.com
# 首次连接会提示：
# The authenticity of host 'github.com (20.205.243.166)' can't be established.
# ED25519 key fingerprint is SHA256:+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU.
# Are you sure you want to continue connecting (yes/no/[fingerprint])?
# 输入 yes 回车（仅首次需要）
```

看到 `Hi 用户名! You've successfully authenticated` 即成功。

### 3.4 切换远程地址为 SSH 格式

```bash
# HTTPS → SSH
git remote set-url origin git@github.com:jerry-166/compiler-principle-course-design.git

# ⚠️ 注意：命令必须写在一行，不能换行！
# ❌ 错误写法（换行会报错）：
#   git remote set-url origin
#   git@github.com:xxx/xxx.git

# 验证远程地址
git remote -v
```

## 4. 推送代码到 GitHub

```bash
git add .
git commit -m "提交说明"
git push origin main
```

## 5. HTTPS 备用方案（PAT）

如果 SSH 不适用，可以用 HTTPS + Personal Access Token：

1. GitHub → Settings → Developer settings → Personal access tokens → Tokens (classic)
2. Generate new token，勾选 `repo`（全部仓库权限）
3. 生成的 token 复制保存好（只显示一次）
4. 推送时用户名填 GitHub 用户名，密码填 token（粘贴即可，界面不显示字符）

```bash
git remote set-url origin https://github.com/用户名/仓库名.git
git push origin main
# 用户名: 你的GitHub用户名
# 密码:   pat_xxxxx (粘贴)
```

记住 token 免重复输入：

```bash
git config --global credential.helper store
```

## 6. 国内网络问题处理

推送时遇到 `GnuTLS recv error (-110)`，表示 TLS 连接中断，通常是网络不稳定：

```bash
# 有代理时设置（假设代理在 7890 端口）
git config --global http.proxy http://127.0.0.1:7890
git config --global https.proxy http://127.0.0.1:7890

# 推送
git push origin main

# 推送完成后取消代理（避免影响其他操作）
git config --global --unset http.proxy
git config --global --unset https.proxy
```

## 7. 附：自动化脚本

同级目录下的 `setup_git.sh` 可一键完成上述配置（需手动在 GitHub 添加公钥）。
