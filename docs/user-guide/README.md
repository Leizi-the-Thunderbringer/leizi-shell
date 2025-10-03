# Leizi Shell 用户指南

## 📖 简介

Leizi Shell 是一个现代化的 POSIX 兼容 Shell，具有以下特点：
- 🚀 快速启动（<50ms）
- 🎨 美观的 Powerlevel10k 风格提示符
- 🔧 完整的作业控制
- 🌈 语法高亮支持
- 📝 ZSH 风格数组
- ⚡ 智能自动补全

## 🚀 快速开始

### 启动 Shell

```bash
# 直接运行
./leizi

# 或设置为默认 Shell
chsh -s $(which leizi)
```

### 基本使用

```bash
# 运行命令
echo "Hello, Leizi!"

# 使用管道
ls -la | grep leizi

# I/O 重定向
echo "test" > file.txt
cat < file.txt

# 后台作业
sleep 30 &
jobs
fg %1
```

## 📋 核心功能

### 1. 变量管理

```bash
# 设置环境变量
export PATH=/usr/local/bin:$PATH

# 使用变量
echo $HOME
echo ${USER}

# 特殊变量
echo $?      # 上一命令退出码
echo $$      # 当前 Shell PID
echo $PWD    # 当前目录
```

### 2. ZSH 风格数组

```bash
# 创建数组
array colors=(red green blue)

# 显示数组
array colors

# 数组在命令中展开
echo colors: ${colors[@]}
```

### 3. 作业控制

```bash
# 后台执行
sleep 100 &

# 查看作业列表
jobs

# 前台化作业
fg %1

# 后台化作业（暂停后）
# Ctrl+Z 暂停当前作业
bg %1

# 作业状态
# [1]  Running    sleep 100 &
# [2]+ Stopped    vim file.txt
```

### 4. 命令历史

```bash
# 查看历史
history

# 历史文件位置
~/.leizi_history
```

### 5. 语法高亮

```bash
# 使用 highlight 命令演示
highlight echo hello world
highlight ls -la | grep test > file.txt
highlight export PATH=/usr/bin:$PATH
```

颜色规则：
- 🟢 有效命令 → 绿色
- 🔴 无效命令 → 红色
- 🟡 字符串 → 黄色
- 🔵 变量 → 蓝色
- 🟣 操作符 → 紫色

### 6. 智能补全

按 Tab 键触发自动补全：
- 命令补全（builtin + PATH）
- 文件路径补全（支持 ~ 展开）
- 变量名补全
- 历史命令补全

## ⚙️ 配置

### 配置文件位置

`~/.config/leizi/config`

### 配置示例

```ini
[prompt]
show_git = true
show_time = true
show_user = true
colors = true
symbol = "❯"

[completion]
case_sensitive = false
show_hidden = false

[history]
size = 10000
ignore_duplicates = true
ignore_space = true

[aliases]
ll = "ls -la"
la = "ls -A"
l = "ls -CF"
```

### 配置项说明

#### [prompt] 提示符设置
- `show_git`: 显示 Git 分支和状态
- `show_time`: 显示时间戳
- `show_user`: 显示用户名@主机名
- `colors`: 启用彩色提示符
- `symbol`: 提示符符号

#### [completion] 补全设置
- `case_sensitive`: 大小写敏感
- `show_hidden`: 显示隐藏文件

#### [history] 历史设置
- `size`: 历史记录最大条数
- `ignore_duplicates`: 忽略重复命令
- `ignore_space`: 忽略以空格开头的命令

#### [aliases] 别名设置
- 格式: `alias = "command"`
- 示例: `ll = "ls -la"`

## 🔨 内建命令

| 命令 | 说明 | 示例 |
|------|------|------|
| `cd` | 切换目录 | `cd /usr/bin` |
| `pwd` | 打印当前目录 | `pwd` |
| `echo` | 输出文本 | `echo "hello"` |
| `export` | 设置环境变量 | `export PATH=/usr/bin` |
| `unset` | 删除变量 | `unset MYVAR` |
| `env` | 显示所有环境变量 | `env` |
| `array` | 管理数组 | `array list=(a b c)` |
| `history` | 显示命令历史 | `history` |
| `jobs` | 列出后台作业 | `jobs` |
| `fg` | 前台化作业 | `fg %1` |
| `bg` | 后台化作业 | `bg %1` |
| `highlight` | 演示语法高亮 | `highlight ls -la` |
| `help` | 显示帮助信息 | `help` |
| `version` | 显示版本信息 | `version` |
| `exit` | 退出 Shell | `exit` |
| `clear` | 清屏 | `clear` |
| `exec` | 执行命令替换当前进程 | `exec bash` |

## 🎯 高级特性

### 管道和重定向

```bash
# 基本管道
ls | grep leizi

# 多级管道
cat file.txt | grep pattern | wc -l

# 输出重定向
echo "test" > output.txt     # 覆盖
echo "more" >> output.txt    # 追加

# 输入重定向
cat < input.txt

# 错误重定向
command 2> error.log         # stderr
command 2>> error.log        # stderr 追加
command &> all.log           # stdout + stderr
```

### Git 集成

提示符自动显示 Git 状态：

```
user@host ~/project (main) ✓ [0] 14:30:25 ❯
                    ^^^^^^  ^  ^^^ ^^^^^^^^^
                    分支名  状态 退出码 时间
```

Git 状态符号：
- ✓ 无修改
- ●N 已修改文件数
- +N 新增文件数
- -N 删除文件数
- ?N 未跟踪文件数

## 🐛 故障排除

### Shell 启动慢

检查 Git 状态查询（已优化缓存）：
```bash
# Git 分支缓存 10 秒
# Git 状态缓存 2 秒
```

### 命令未找到

1. 检查 PATH:
   ```bash
   echo $PATH
   ```

2. 刷新 PATH 缓存:
   ```bash
   export PATH=$PATH  # 触发缓存刷新
   ```

### 配置不生效

1. 检查配置文件语法
2. 重启 Shell
3. 查看默认配置:
   ```bash
   cat ~/.config/leizi/config
   ```

## 📚 更多资源

- [GitHub 仓库](https://github.com/Zixiao-System/leizi-shell)
- [问题反馈](https://github.com/Zixiao-System/leizi-shell/issues)
- [贡献指南](../CONTRIBUTING.md)
- [变更日志](../CHANGELOG.md)

## 🆘 获取帮助

```bash
# Shell 内帮助
help
version

# 特定命令帮助
help <command>
```

---

**版本**: 1.3.0
**最后更新**: 2025-10-03
**许可证**: GPL-3.0
