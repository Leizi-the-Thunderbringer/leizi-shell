# Doxygen 文档生成指南

## 安装 Doxygen

### macOS
```bash
brew install doxygen
```

### Ubuntu/Debian
```bash
sudo apt-get install doxygen graphviz
```

### Arch Linux
```bash
sudo pacman -S doxygen graphviz
```

## 生成文档

```bash
# 1. 生成默认配置文件（如果不存在）
doxygen -g Doxyfile

# 2. 编辑关键配置项
# 已在 Doxyfile 中预配置

# 3. 生成 HTML 文档
doxygen Doxyfile

# 4. 查看文档
open html/index.html  # macOS
xdg-open html/index.html  # Linux
```

## 配置说明

主要配置项（在 Doxyfile 中）：

```
PROJECT_NAME           = "Leizi Shell"
PROJECT_NUMBER         = 1.3.0
PROJECT_BRIEF          = "A modern POSIX-compatible shell"
OUTPUT_DIRECTORY       = docs/doxygen
INPUT                  = src/
RECURSIVE              = YES
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
HTML_OUTPUT            = html
HTML_COLORSTYLE_HUE    = 220
HTML_COLORSTYLE_SAT    = 100
HTML_COLORSTYLE_GAMMA  = 80
HAVE_DOT               = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
```

## 文档注释规范

### 类注释

```cpp
/**
 * @brief 命令解析器类
 *
 * 负责解析用户输入的命令字符串，识别命令、参数、
 * 重定向和管道等语法元素。
 *
 * @see CommandParser::parse()
 */
class CommandParser {
    // ...
};
```

### 函数注释

```cpp
/**
 * @brief 解析命令字符串
 *
 * 将输入字符串解析为命令、参数和重定向的结构化表示。
 *
 * @param input 用户输入的命令字符串
 * @return ParsedCommand 解析后的命令结构
 *
 * @throws std::runtime_error 当解析失败时
 *
 * @code
 * CommandParser parser;
 * auto cmd = parser.parse("ls -la | grep test");
 * @endcode
 */
ParsedCommand parse(const std::string& input);
```

### 成员变量注释

```cpp
class VariableManager {
private:
    /** 存储所有变量的映射表 */
    std::unordered_map<std::string, Variable> variables;

    /** 已导出变量的名称集合 */
    std::set<std::string> exportedVars;
};
```

### 枚举注释

```cpp
/**
 * @brief 作业状态枚举
 */
enum class JobStatus {
    RUNNING,  /**< 作业正在运行 */
    STOPPED,  /**< 作业已暂停 */
    DONE      /**< 作业已完成 */
};
```

## 文档组织

Doxygen 会自动生成以下页面：

1. **Main Page** - 项目概述
2. **Modules** - 模块列表
3. **Classes** - 类列表
4. **Files** - 文件列表
5. **Namespaces** - 命名空间
6. **Functions** - 函数列表

## 生成流程图

使用 GraphViz (需要安装 graphviz 包):

```
HAVE_DOT = YES
DOT_PATH = /usr/bin/dot
CLASS_DIAGRAMS = YES
COLLABORATION_DIAGRAMS = YES
CALL_GRAPH = YES
CALLER_GRAPH = YES
```

## 输出格式

### HTML (推荐)
- 默认生成格式
- 交互式浏览
- 搜索功能
- 主题定制

### LaTeX/PDF
```
GENERATE_LATEX = YES
PDF_HYPERLINKS = YES
USE_PDFLATEX = YES
```

### Man Pages
```
GENERATE_MAN = YES
MAN_OUTPUT = man
MAN_EXTENSION = .3
```

## 集成到 CMake

在 CMakeLists.txt 中添加：

```cmake
# 查找 Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
    # 配置 Doxyfile
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                   ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    # 添加文档生成目标
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
```

使用：
```bash
make doc
```

## 最佳实践

1. **保持注释更新**: 代码修改时同步更新注释
2. **使用简洁语言**: 注释应清晰简洁
3. **添加示例代码**: 使用 @code 块展示用法
4. **链接相关函数**: 使用 @see 和 @ref
5. **文档化公共 API**: 至少为 public 成员添加注释
6. **分组相关功能**: 使用 @defgroup 和 @addtogroup

## 示例：完整的类文档

```cpp
/**
 * @file builtin_manager.h
 * @brief 内建命令管理器
 * @author Leizi Team
 * @date 2025-10-03
 */

/**
 * @brief 内建命令管理器
 *
 * 负责注册和执行所有内建命令。使用工厂模式动态创建
 * 命令实例，支持运行时命令注册和查询。
 *
 * @code
 * BuiltinManager manager;
 * if (manager.isBuiltin("cd")) {
 *     manager.execute({"cd", "/tmp"}, context);
 * }
 * @endcode
 *
 * @see BuiltinCommand
 * @see BuiltinContext
 */
class BuiltinManager {
public:
    /**
     * @brief 构造函数
     *
     * 自动注册所有内建命令
     */
    BuiltinManager();

    /**
     * @brief 检查命令是否为内建命令
     *
     * @param name 命令名
     * @return true 如果是内建命令
     * @return false 如果不是内建命令
     */
    bool isBuiltin(const std::string& name) const;

    // ... 其他成员
};
```

---

**注意**: 完整的 Doxygen 配置文件可以通过 `doxygen -g` 生成。
上述配置只是关键选项，实际 Doxyfile 有数百个配置项。
