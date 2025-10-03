# 🚀 Leizi Shell 详细待办事项清单

## 📋 项目概述
- **项目名**: Leizi Shell
- **版本**: 当前 v1.2.0
- **语言**: C++20
- **构建系统**: CMake
- **目标**: 现代化 POSIX 兼容的 shell，具有 ZSH 风格数组和美观提示符

---

## 🚨 紧急修复 (Priority: CRITICAL)

### ~~TASK-001: 统一项目命名~~ ✅ **已完成**
- **描述**: 将 "leizi" 统一重命名为 "leizi"
- **状态**: ✅ 已完成 - 所有文件中的项目名已统一为 "leizi"
- **影响文件**:
  - `CMakeLists.txt` - 项目名和可执行文件名
  - `README.md` - 所有提及项目名的地方
  - `CONTRIBUTING.md` - 项目名称和命令示例
  - `install.sh` - BINARY_NAME 变量和下载路径
  - `src/main.cpp` - 注释中的项目名
  - `.github/workflows/ci.yml` - 构建产物名称
- **预期工作量**: 1-2 小时
- **验收标准**:
  - [ ] 所有文件中的项目名统一为 "leizi"
  - [ ] 可执行文件名为 "leizi"
  - [ ] README 中的命令示例更新
  - [ ] CI 构建通过

### ~~TASK-002: GitHub 配置验证~~ ✅ **已完成**
- **状态**: GitHub Actions 全绿，v1.0.0 Release 已发布
- **结果**: 项目基础设施完善，可以直接进入功能开发阶段

---

## 🏗️ 核心架构重构 (Priority: HIGH)

### TASK-003: 代码模块化重构
- **描述**: 将 `src/main.cpp` (2000+ 行) 拆分为模块化结构
- **目标结构**:
```
src/
├── core/
│   ├── shell.h/.cpp          # LeiziShell 主类
│   ├── parser.h/.cpp         # CommandParser 类
│   └── executor.h/.cpp       # CommandExecutor 类
├── builtin/
│   ├── builtin.h             # 内建命令基类
│   ├── cd.cpp                # cd 命令实现
│   ├── echo.cpp              # echo 命令实现
│   ├── export.cpp            # export/unset 命令
│   ├── array.cpp             # array 命令实现
│   └── history.cpp           # history 命令实现
├── prompt/
│   ├── prompt.h/.cpp         # PromptGenerator 类
│   └── git.h/.cpp            # GitStatus 类
├── completion/
│   └── completer.h/.cpp      # TabCompleter 类
├── utils/
│   ├── colors.h              # 颜色常量和工具函数
│   ├── variables.h/.cpp      # VariableManager 类
│   └── signal_handler.h/.cpp # 信号处理
└── main.cpp                  # 简化的 main 函数
```

- **具体任务**:
  - [x] 创建 `src/utils/colors.h` - 将 Color 命名空间提取（2025-10-01 完成）
  - [x] 创建 `src/utils/variables.h/.cpp` - 实现改进的变量系统（引入 VariableManager，2025-10-01 完成初版）
  - [x] 创建 `src/prompt/prompt.h/.cpp` - 提取提示符生成逻辑（2025-10-01 完成）
  - [x] 创建 `src/prompt/git.h/.cpp` - 提取 Git 集成功能（2025-10-02 完成）
  - [x] 创建 `src/core/parser.h/.cpp` - 提取命令解析逻辑（2025-10-01 完成）
  - [x] 创建 `src/core/job_control.h/.cpp` - 提取作业控制逻辑（2025-10-02 完成）
  - [x] 创建 `src/utils/signal_handler.h/.cpp` - 提取信号处理逻辑（2025-10-02 完成）
  - [x] 创建 `src/builtin/builtin.h` - 内建命令基类框架（2025-10-02 完成）
  - [x] 创建 `src/builtin/` 目录下的各个命令实现（2025-10-03 完成）✨
    - [x] `src/builtin/cd.cpp` - cd 命令
    - [x] `src/builtin/echo.cpp` - echo 命令
    - [x] `src/builtin/export.cpp` - export/unset 命令
    - [x] `src/builtin/array.cpp` - array 命令
    - [x] `src/builtin/history.cpp` - history 命令
    - [x] `src/builtin/simple.cpp` - pwd/exit/clear 命令
    - [x] `src/builtin/info.cpp` - help/version 命令
    - [x] `src/builtin/builtin_manager.h/.cpp` - 内建命令管理器
  - [x] 更新 `CMakeLists.txt` 支持多文件编译（2025-10-03 完成）
  - [x] 更新 `main.cpp` 使用模块化内建命令系统（2025-10-03 完成）
  - [x] 确保所有模块都有适当的头文件保护和命名空间

- **预期工作量**: 2-3 天
- **验收标准**:
  - [x] 编译通过，功能无回归 ✅
  - [x] 每个模块职责单一，接口清晰 ✅
  - [ ] 包含适当的单元测试
  - [x] 代码覆盖率不低于当前水平 ✅

**最新进展** (2025-10-03): 完成了内建命令模块化重构！所有内建命令已成功提取到独立模块，实现了清晰的职责分离和统一的命令管理接口。

### TASK-004: 改进变量系统
- **描述**: 使用现代 C++ 特性改进变量管理
- **参考实现**: 见 artifact `improved_variable_system`
- **具体改进**:
  - [ ] 使用 `std::variant` 替代 union 结构
  - [ ] 实现类型安全的变量访问
  - [ ] 支持特殊变量的动态计算 ($?, $$, $PWD 等)
  - [ ] 添加变量标志系统 (readonly, exported, local)
  - [ ] 实现作用域管理 (为将来的函数支持做准备)

- **API 设计示例**:
```cpp
// 使用方式
VariableManager vm;
vm.set("PATH", std::string("/usr/bin:/bin"));
vm.set("colors", std::vector<std::string>{"red", "green", "blue"});
vm.registerSpecialVar("?", [this]() { return std::to_string(lastExitCode); });

auto expanded = vm.expand("Current directory: $PWD");
```

- **预期工作量**: 1-2 天
- **验收标准**:
  - [ ] 支持字符串、数组、整数、浮点数类型
  - [ ] 变量展开功能完整 ($var, ${var}, ${var:-default})
  - [ ] 特殊变量正确计算
  - [ ] 与现有代码兼容

---

## 🔧 核心 Shell 功能 (Priority: HIGH)

### TASK-005: 实现管道支持
- **描述**: 实现 shell 管道操作符 `|`
- **参考设计**: 见 artifact `pipe_redirection_design`
- **实现要求**:
  - [ ] 解析管道命令: `command1 | command2 | command3`
  - [ ] 创建进程间管道连接
  - [ ] 正确处理文件描述符
  - [ ] 支持复杂管道链
  - [ ] 错误处理和资源清理

- **测试用例**:
```bash
ls -la | grep .cpp          # 基本管道
cat file.txt | sort | uniq  # 多级管道
echo "hello" | cat          # 简单管道
false | echo "test"         # 错误处理
```

- **预期工作量**: 2-3 天
- **验收标准**:
  - [ ] 支持任意长度的管道链
  - [ ] 正确传递退出代码
  - [ ] 无内存泄漏
  - [ ] 通过所有测试用例

### TASK-006: 实现重定向支持
- **描述**: 实现输入输出重定向操作符
- **支持的重定向类型**:
  - [ ] `>` - 输出重定向
  - [ ] `>>` - 输出追加
  - [ ] `<` - 输入重定向
  - [ ] `2>` - 错误重定向
  - [ ] `2>>` - 错误追加
  - [ ] `&>` - 同时重定向输出和错误
  - [ ] `<<` - Here Document (可选)

- **测试用例**:
```bash
echo "test" > file.txt
cat < file.txt
ls nonexistent 2> error.log
echo "append" >> file.txt
```

- **预期工作量**: 1-2 天
- **验收标准**:
  - [ ] 所有重定向类型正常工作
  - [ ] 文件权限正确处理
  - [ ] 错误情况下的回滚机制
  - [ ] 与管道功能兼容

### ~~TASK-007: 作业控制系统~~ ✅ **已完成**
- **描述**: 实现后台作业和作业控制
- **状态**: ✅ 已完成 - v1.1.1中实现了完整的作业控制功能
- **功能实现**:
  - [x] 后台执行: `command &`
  - [x] `jobs` 命令 - 列出活跃作业
  - [x] `fg [job]` 命令 - 前台化作业
  - [x] `bg [job]` 命令 - 后台化作业
  - [x] Ctrl+Z 支持 - 暂停前台进程
  - [x] 作业状态跟踪 (运行中、暂停、完成)
- **完成日期**: 2025-09-26

- **数据结构设计**:
```cpp
struct Job {
    pid_t pid;
    int job_id;
    std::string command;
    JobStatus status;  // RUNNING, STOPPED, DONE
    time_t start_time;
};

class JobManager {
public:
    int startBackground(const Pipeline& pipeline);
    void listJobs() const;
    bool foreground(int job_id);
    bool background(int job_id);
    void checkJobs();  // 检查完成的作业
};
```

- **预期工作量**: 2-3 天
- **验收标准**:
  - [ ] 后台作业正确执行
  - [ ] 作业状态准确跟踪
  - [ ] fg/bg 命令正常工作
  - [ ] 支持 Ctrl+Z 暂停作业

---

## 🎨 用户体验改进 (Priority: MEDIUM)

### ~~TASK-008: 增强自动补全系统~~ ✅ **已完成** (2025-10-03)
- **描述**: 实现智能的自动补全功能
- **状态**: ✅ 已完成 - SmartCompleter系统已建立，支持多种补全提供者
- **改进目标**:
  - [x] 模块化补全架构 (Provider模式)
  - [x] 命令补全 (builtin + PATH)
  - [x] 变量名补全 ($VAR<TAB>)
  - [x] 路径智能补全 (支持 ~ 展开)
  - [x] 历史命令补全
  - [ ] 命令参数补全 (针对常用命令) - 未来扩展
  - [ ] 模糊匹配 - 未来扩展

- **实现架构**:
```cpp
SmartCompleter (主管理器)
├── CommandCompleter (命令补全, 优先级100)
├── VariableCompleter (变量补全, 优先级90)
├── HistoryCompleter (历史补全, 优先级80)
└── FileCompleter (文件补全, 优先级50)
```

- **新增文件**:
  - `src/completion/completer.h` - 补全系统接口和管理器
  - `src/completion/completer.cpp` - 各种补全提供者实现

- **功能特性**:
  - CompletionProvider基类，支持优先级排序
  - CompletionContext上下文分析
  - 支持环境变量补全 ($PATH, $HOME, etc.)
  - 支持特殊变量 ($?, $$, $PWD, $HOME, $USER, $PATH)
  - Tilde (~) 自动展开
  - 目录补全带 / 后缀
  - 历史命令去重

### ~~TASK-009: 配置系统~~ ✅ **已完成** (2025-10-03)
- **描述**: 实现 shell 配置文件支持
- **状态**: ✅ 已完成 - ConfigManager系统建立，支持INI风格配置
- **配置文件位置**: `~/.config/leizi/config`
- **配置项设计**:
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

- **已实现功能**:
  - [x] INI风格配置文件解析
  - [x] Section分组 ([prompt], [completion], [history], [aliases])
  - [x] 类型自动推断 (bool, int, string)
  - [x] 默认配置生成
  - [x] Alias扩展支持
  - [x] 向后兼容 (默认值)
  - [ ] 配置热重载 - 未来扩展

- **新增文件**:
  - `src/config/config.h` - 配置系统接口
  - `src/config/config.cpp` - 配置解析和管理

- **API功能**:
  - ConfigValue: 支持bool/int/string类型
  - loadConfig(): 从文件加载配置
  - generateDefaultConfig(): 生成默认配置
  - getBool/getInt/getString(): 类型安全访问
  - getAlias/getAllAliases(): Alias管理

### ~~TASK-010: 语法高亮~~ ✅ **已完成** (2025-10-03)
- **描述**: 实现实时语法高亮 (Fish Shell风格)
- **状态**: ✅ 已完成 - 实现了完整的语法高亮系统
- **高亮规则**:
  - [x] 有效命令 - 绿色 ✅
  - [x] 无效命令 - 红色 ✅
  - [x] 字符串字面量 - 黄色 ✅
  - [x] 变量 - 蓝色 ✅
  - [x] 管道和重定向符 - 紫色 ✅

- **新增文件**:
  - `src/syntax/highlighter.h` - 语法高亮器接口
  - `src/syntax/highlighter.cpp` - 语法高亮器实现
  - `src/builtin/highlight.cpp` - highlight 演示命令

- **功能特性**:
  - 命令有效性检查（PATH + builtin）
  - 变量识别（$VAR 和 ${VAR}）
  - 字符串边界检测（单引号和双引号）
  - 操作符识别（|, >, <, &, ;）
  - 多字符操作符支持（>>, 2>, &>）
  - PATH 命令缓存优化
  - 文件系统遍历优化（filesystem库）

- **使用示例**:
  ```bash
  # 使用 highlight 命令演示语法高亮
  highlight echo hello world
  highlight ls -la | grep test > file.txt
  highlight export PATH=/usr/bin:$PATH
  ```

- **技术挑战**:
  - [x] 实时解析和着色 ✅
  - [ ] Readline 深度集成（限于系统版本）- 未来扩展
  - [x] 性能优化（PATH 缓存）✅

- **预期工作量**: 2-3 天
- **验收标准**:
  - [x] 语法高亮准确 ✅
  - [x] 命令有效性检查 ✅
  - [x] 支持所有主要语法元素 ✅
  - [ ] Readline集成 - 受限于系统版本

**完成日期**: 2025-10-03

---

## 🧪 测试和质量保证 (Priority: MEDIUM)

### ~~TASK-011: 完善测试套件~~ ✅ **已完成**
- **描述**: 建立全面的测试体系
- **状态**: ✅ 已完成 - Catch2 测试框架已建立，单元测试和集成测试运行正常
- **测试体系设计**:
```
tests/
├── catch.hpp                # Catch2 v2.13.10 单头文件
├── unit/                    # 单元测试
│   ├── test_parser.cpp     # 命令解析测试 ✅
│   ├── test_variables.cpp  # 变量系统测试 ✅
│   └── test_builtin.cpp    # 内建命令测试 ✅
├── integration/             # 集成测试
│   ├── test_main.cpp       # Catch2 主入口
│   ├── test_pipeline.cpp   # 管道测试 ✅
│   ├── test_redirection.cpp # 重定向测试 ✅
│   └── test_jobs.cpp       # 作业控制测试 ✅
```

- **已完成任务**:
  - [x] 设置 Catch2 测试框架和 CMake 集成
  - [x] 编写单元测试 (parser, variables, builtin)
  - [x] 编写集成测试 (pipeline, redirection, jobs)
  - [x] CI/CD 准备 (CMake 已配置，enable_testing)
  - [ ] 功能测试 (BATS) - 未来扩展
  - [ ] 性能基准测试 - 未来扩展

- **测试运行结果**:
  - ✅ UnitTests: PASSED (1.00s)
  - ✅ IntegrationTests: PASSED (2.27s)
  - ✅ 总体: 100% 测试通过 (2/2)

### TASK-012: 内存安全和性能优化 ✅ **已完成** (2025-10-03)
- **描述**: 提升代码质量和性能
- **状态**: ✅ 已完成 - 添加了完整的Sanitizer支持、性能基准测试和优化
- **具体任务**:
  - [x] 添加可选的 AddressSanitizer 和 UBSan 支持
  - [x] 添加 ThreadSanitizer 支持
  - [x] 添加 gprof 性能分析支持
  - [x] Git 状态缓存优化（分支缓存10秒，状态缓存2秒）
  - [x] 创建性能基准测试脚本 (scripts/benchmark.sh)
  - [x] 创建 Valgrind 内存检测脚本 (scripts/valgrind-check.sh)
  - [x] 性能测试结果：启动时间 2ms (<50ms 目标) ✅
  - [x] 性能测试结果：提示符生成 2ms (<100ms 目标) ✅

- **新增文件**:
  - `scripts/benchmark.sh` - 性能基准测试脚本
  - `scripts/valgrind-check.sh` - Valgrind 内存泄漏检测脚本

- **CMake 选项**:
  ```bash
  # 启用 AddressSanitizer 和 UBSan
  cmake -DENABLE_SANITIZERS=ON ..

  # 启用 ThreadSanitizer
  cmake -DENABLE_TSAN=ON ..

  # 启用性能分析
  cmake -DENABLE_PROFILING=ON ..
  ```

- **工具和检查**:
  ```bash
  # 运行性能基准测试
  ./scripts/benchmark.sh

  # 运行 Valgrind 内存检测
  ./scripts/valgrind-check.sh
  ```

- **预期工作量**: 1-2 天
- **验收标准**:
  - [x] 可选的 Sanitizer 支持已添加 ✅
  - [x] 性能测试基础设施完善 ✅
  - [x] 启动时间 < 50ms (实测: 2ms) ✅
  - [x] 提示符生成时间 < 100ms (实测: 2ms) ✅
  - [x] Git 状态缓存机制实现 ✅

---

## 📚 文档和部署 (Priority: LOW)

### ~~TASK-013: 完善文档~~ ✅ **已完成** (2025-10-03)
- **描述**: 建立完整的文档体系
- **状态**: ✅ 已完成 - 核心文档已创建
- **文档结构**:
```
docs/
├── user-guide/
│   └── README.md         # 用户指南 ✅
├── api/
│   └── README.md         # API 文档 ✅
├── DOXYGEN.md            # Doxygen 配置指南 ✅
└── examples/             # 未来扩展
```

- **已完成任务**:
  - [x] 用户指南（快速开始、核心功能、配置、故障排除）✅
  - [x] API 文档（核心模块、数据结构、扩展开发）✅
  - [x] Doxygen 配置说明和注释规范 ✅
  - [ ] 示例和教程 - 未来扩展
  - [ ] 多语言支持 - 未来扩展

- **文档内容**:
  - **用户指南**: 完整的使用说明（17个核心功能）
  - **API文档**: 7大核心模块详细说明
  - **Doxygen指南**: 文档生成和注释规范

- **预期工作量**: 2-3 天
- **验收标准**:
  - [x] 用户指南完整 ✅
  - [x] API 文档清晰 ✅
  - [x] Doxygen 配置完成 ✅

**完成日期**: 2025-10-03

### TASK-014: 包管理和分发
- **描述**: 改进软件分发和包管理
- **目标平台**:
  - [ ] Ubuntu/Debian - 创建 .deb 包
  - [ ] CentOS/RHEL - 创建 .rpm 包
  - [ ] macOS - Homebrew formula
  - [ ] Arch Linux - AUR 包
  - [ ] Docker 镜像

- **具体任务**:

#### TASK-014a: Homebrew Formula (macOS)
```ruby
# Formula/leizi.rb
class Leizi < Formula
  desc "Modern POSIX-compatible shell with ZSH-style arrays and beautiful prompts"
  homepage "https://github.com/Zixiao-System/leizi-shell"
  url "https://github.com/Zixiao-System/leizi-shell/archive/v1.0.1.tar.gz"
  sha256 "..."
  license "GPL-3.0"
  
  depends_on "cmake" => :build
  depends_on "readline"
  
  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end
  
  test do
    assert_match "Leizi Shell", shell_output("#{bin}/leizi --version")
  end
end
```
- [ ] 创建 Formula 文件
- [ ] 测试本地安装: `brew install --build-from-source ./Formula/leizi.rb`
- [ ] 提交到 homebrew-core 或创建自有 tap

#### TASK-014b: APT Repository (Ubuntu/Debian)
```bash
# 创建 debian/ 目录结构
debian/
├── control          # 包依赖和描述
├── changelog        # Debian 格式的变更日志
├── copyright        # 版权信息
├── rules           # 构建规则
├── install         # 安装文件列表
└── compat          # debhelper 兼容性级别
```

**debian/control 内容**:
```
Source: leizi-shell
Section: shells
Priority: optional
Maintainer: Leizi Team <maintainer@example.com>
Build-Depends: debhelper-compat (= 12), cmake, libreadline-dev
Standards-Version: 4.5.0
Homepage: https://github.com/Zixiao-System/leizi-shell

Package: leizi-shell
Architecture: any
Depends: ${shlibs:Depends}, ${misc:Depends}, libreadline8
Description: Modern POSIX-compatible shell with beautiful prompts
 Leizi Shell is a modern shell implementation featuring:
  * Beautiful Powerlevel10k-inspired prompts
  * Git integration with real-time status
  * ZSH-style array support
  * Smart tab completion
  * POSIX compatibility
```

- [ ] 创建完整的 debian/ 目录
- [ ] 配置 GitHub Actions 自动构建 .deb
- [ ] 创建 PPA 或 APT 仓库
- [ ] 测试安装: `sudo dpkg -i leizi-shell_1.0.1_amd64.deb`

#### TASK-014c: AUR Package (Arch Linux)
**PKGBUILD 文件**:
```bash
# Maintainer: Leizi Team <maintainer@example.com>
pkgname=leizi-shell
pkgver=1.0.1
pkgrel=1
pkgdesc="Modern POSIX-compatible shell with ZSH-style arrays and beautiful prompts"
arch=('x86_64')
url="https://github.com/Zixiao-System/leizi-shell"
license=('GPL3')
depends=('readline')
makedepends=('cmake' 'git')
source=("$pkgname-$pkgver.tar.gz::https://github.com/Zixiao-System/leizi-shell/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$pkgname-$pkgver"
    cmake -B build -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "$pkgname-$pkgver"
    DESTDIR="$pkgdir" cmake --install build
    
    # Install shell to /etc/shells
    install -Dm644 /dev/stdin "$pkgdir/usr/share/libalpm/hooks/leizi-shell.hook" <<EOF
[Trigger]
Operation = Install
Operation = Upgrade
Operation = Remove
Type = Path
Target = usr/bin/leizi

[Action]
Description = Updating shell database...
When = PostTransaction
Exec = /usr/bin/sh -c 'grep -qxF "/usr/bin/leizi" /etc/shells || echo "/usr/bin/leizi" >> /etc/shells'
EOF
}

check() {
    cd "$pkgname-$pkgver"
    # Run basic tests
    echo "version" | timeout 5 ./build/leizi || true
}
```

**AUR 提交步骤**:
- [ ] 创建 PKGBUILD 和 .SRCINFO
- [ ] 本地测试: `makepkg -si`
- [ ] 提交到 AUR: `git push aur@aur.archlinux.org:leizi-shell.git`

#### TASK-014d: Docker 镜像
**Dockerfile**:
```dockerfile
FROM ubuntu:22.04 as builder

RUN apt-get update && apt-get install -y \
    build-essential cmake libreadline-dev git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    libreadline8 git \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -m -s /bin/bash leizi

COPY --from=builder /src/build/leizi /usr/local/bin/
RUN echo "/usr/local/bin/leizi" >> /etc/shells

USER leizi
WORKDIR /home/leizi
ENTRYPOINT ["/usr/local/bin/leizi"]
```

- [ ] 创建多阶段 Dockerfile
- [ ] GitHub Actions 自动构建并推送到 Docker Hub
- [ ] 支持多架构 (amd64, arm64)
- [ ] 测试: `docker run -it leizi/leizi-shell`


- **自动化发布**:
  - [ ] GitHub Actions 自动构建多平台二进制 ✅ (已有)
  - [ ] 自动创建 GitHub Release ✅ (已有)
  - [ ] Homebrew tap 自动更新
  - [ ] APT 仓库自动更新
  - [ ] AUR 包自动更新
  - [ ] Docker 镜像自动构建和推送

- **预期工作量**: 3-4 天 (每个平台约 1 天)

### TASK-018: GitHub Actions 包管理集成
- **描述**: 在现有 CI/CD 基础上添加包管理器自动发布
- **当前状态**: ✅ GitHub Actions 已完善，Release 自动化已就绪
- **扩展目标**: 在每次 Release 时自动更新各包管理器

#### 具体实现:
```yaml
# .github/workflows/package-release.yml
name: Package Release

on:
  release:
    types: [published]

jobs:
  update-homebrew:
    runs-on: macos-latest
    steps:
      - name: Update Homebrew Formula
        env:
          HOMEBREW_GITHUB_API_TOKEN: ${{ secrets.BREW_TOKEN }}
        run: |
          # 自动更新 Formula 中的版本和校验和
          # 提交 PR 到 homebrew-core 或自有 tap
  
  update-aur:
    runs-on: ubuntu-latest
    steps:
      - name: Update AUR Package
        env:
          AUR_SSH_KEY: ${{ secrets.AUR_SSH_KEY }}
        run: |
          # 更新 PKGBUILD 中的版本
          # 生成新的 .SRCINFO
          # 推送到 AUR 仓库
  
  build-deb:
    runs-on: ubuntu-latest
    steps:
      - name: Build and Upload DEB
        run: |
          # 构建 .deb 包
          # 上传到 APT 仓库或 GitHub Release
  
  build-docker:
    runs-on: ubuntu-latest
    steps:
      - name: Build and Push Docker Image
        uses: docker/build-push-action@v4
        with:
          platforms: linux/amd64,linux/arm64
          push: true
          tags: |
            leizi/leizi-shell:latest
            leizi/leizi-shell:${{ github.event.release.tag_name }}
```

---

## 🚀 高级功能 (Priority: LOW, 未来版本)

### TASK-015: 脚本语言功能
- **描述**: 支持 shell 脚本的高级特性
- **功能清单**:
  - [ ] 函数定义和调用
  - [ ] 条件语句 (`if`, `case`)
  - [ ] 循环 (`for`, `while`, `until`)
  - [ ] 算术表达式 (`$(( ))`)
  - [ ] 命令替换 (`$(command)`, `` `command` ``)

### TASK-016: 插件系统
- **描述**: 支持动态加载插件扩展功能
- **设计思路**:
  - 插件接口规范
  - 动态库加载机制
  - 插件管理命令
  - 插件仓库和安装

### TASK-017: 网络和远程功能
- **描述**: 现代化的网络集成功能
- **功能构想**:
  - SSH 集成
  - 远程命令执行
  - 云服务集成 (AWS CLI, Azure CLI 等)
  - 容器支持 (Docker, Kubernetes)

---

## 📊 版本规划

### v1.1.0 (已发布)
- ✅ TASK-005 管道支持实现
- ✅ TASK-006 I/O重定向实现
- ✅ 项目重命名为Leizi

### v1.1.1 (当前开发)
- ✅ TASK-007 作业控制系统（已完成）
- ⏳ TASK-003 代码模块化重构（进行中）

### v1.2.0 (预计 1-2 个月)
- TASK-008 到 TASK-010 (用户体验)
- 完整的作业控制
- 配置系统和语法高亮

### v1.3.0 (预计 2-3 个月)
- TASK-011 到 TASK-014 (质量和分发)
- 完善的测试套件
- 多平台包分发

### v2.0.0 (未来)
- TASK-015 到 TASK-017 (高级功能)
- 脚本语言支持
- 插件系统

---

## 🔧 开发环境设置

### 依赖要求
- **必需**:
  - C++20 编译器 (GCC 10+, Clang 12+, MSVC 19.29+)
  - CMake 3.16+
  - Git

- **可选**:
  - libreadline-dev (用于更好的命令行编辑)
  - pkg-config
  - Doxygen (用于文档生成)
  - BATS (用于功能测试)

### 开发工作流
1. **分支策略**: 使用 Git Flow
  - `main` - 稳定版本
  - `develop` - 开发版本
  - `feature/*` - 功能分支
  - `hotfix/*` - 热修复分支

2. **代码审查**: 所有 PR 需要至少一人审查

3. **CI/CD**: GitHub Actions 自动化测试和构建

---

## 📞 支持和协作

### AI 工具集成提示
对于 GitHub Copilot、Claude Code 等 AI 工具，在处理此项目时请注意：

1. **项目结构**: 这是一个单体 C++ 项目，主要逻辑在 `src/main.cpp`
2. **编码风格**: 使用现代 C++20 特性，RAII 模式，避免裸指针
3. **命名约定**: camelCase 函数，PascalCase 类，snake_case 变量
4. **错误处理**: 优先使用异常而非错误码
5. **测试**: 每个新功能都应该有对应的单元测试

### 人员协作指南
- **任务认领**: 在 GitHub Issues 中认领任务
- **进度更新**: 定期更新任务进度
- **代码审查**: 遵循代码审查清单
- **文档**: 及时更新相关文档

---

## ✅ 检查清单模板

每完成一个任务，请检查以下内容：

- [ ] 功能实现完整且正确
- [ ] 代码通过所有现有测试
- [ ] 添加了适当的单元测试
- [ ] 代码符合项目编码规范
- [ ] 更新了相关文档
- [ ] 无内存泄漏和安全问题
- [ ] 性能无明显回归
- [ ] 通过代码审查

---

**最后更新**: 2025年9月17日
**维护者**: Zixiao System Team
**版本**: v1.0 (待办清单)
