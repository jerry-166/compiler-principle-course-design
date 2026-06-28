# 配置 Git 并推送到 GitHub 的完整步骤

1. 首次安装 WSL 后配置 Git
   git config --global user.name "你的名字"
   git config --global user.email "你的邮箱@example.com"
   
   # 验证
   
   git config --list

2. 推送代码到 GitHub
   
   # 先在 GitHub 上建好仓库（不要勾选 README）
   
   # 然后在本地项目里：
   
   git remote add origin https://github.com/用户名/仓库名.git
   
   # 或修改已有远程地址
   
   git remote set-url origin https://github.com/用户名/仓库名.git
   
   # 暂存、提交、推送
   
   git add .
   git commit -m "提交说明"
   git push origin main

3. 解决 Git 认证问题
   推荐用 Personal Access Token（HTTPS 方式）：
- GitHub → Settings → Developer settings → Personal access tokens → Tokens
  (classic) → Generate new token

- 勾选 repo 权限，生成一串 token

- git push 时用户名填 GitHub 用户名，密码填 token
  更稳定：换成 SSH 方式：
  ssh-keygen -t ed25519 -C "你的邮箱"       # 生成密钥
  cat ~/.ssh/id_ed25519.pub                 # 查看公钥
  
  # 把公钥添加到 GitHub Settings → SSH and GPG keys
  
  git remote set-url origin git@github.com:用户名/仓库名.git
  git push origin main
  Git 记住密码不用每次输入：
  git config --global credential.helper store
4. 国内网络问题
   如果 git push 出现 TLS connection 报错：
   
   # 有代理就设代理
   
   git config --global http.proxy http://127.0.0.1:7890
   git config --global https.proxy http://127.0.0.1:7890
   
   # 用完取消
   
   git config --global --unset http.proxy
   git config --global --unset https.proxy
