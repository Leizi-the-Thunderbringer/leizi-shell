#pragma once

#include <string>

// 统一管理 ANSI 颜色转义序列，便于在不同模块复用。
namespace Color {
inline const std::string RESET = "\033[0m";
inline const std::string BOLD = "\033[1m";
inline const std::string DIM = "\033[2m";
inline const std::string ITALIC = "\033[3m";
inline const std::string UNDERLINE = "\033[4m";

// 前景色
inline const std::string BLACK = "\033[30m";
inline const std::string RED = "\033[31m";
inline const std::string GREEN = "\033[32m";
inline const std::string YELLOW = "\033[33m";
inline const std::string BLUE = "\033[34m";
inline const std::string MAGENTA = "\033[35m";
inline const std::string CYAN = "\033[36m";
inline const std::string WHITE = "\033[37m";

// 亮色前景
inline const std::string BRIGHT_BLACK = "\033[90m";
inline const std::string BRIGHT_RED = "\033[91m";
inline const std::string BRIGHT_GREEN = "\033[92m";
inline const std::string BRIGHT_YELLOW = "\033[93m";
inline const std::string BRIGHT_BLUE = "\033[94m";
inline const std::string BRIGHT_MAGENTA = "\033[95m";
inline const std::string BRIGHT_CYAN = "\033[96m";
inline const std::string BRIGHT_WHITE = "\033[97m";

// 背景色
inline const std::string BG_BLACK = "\033[40m";
inline const std::string BG_RED = "\033[41m";
inline const std::string BG_GREEN = "\033[42m";
inline const std::string BG_YELLOW = "\033[43m";
inline const std::string BG_BLUE = "\033[44m";
inline const std::string BG_MAGENTA = "\033[45m";
inline const std::string BG_CYAN = "\033[46m";
inline const std::string BG_WHITE = "\033[47m";
} // namespace Color

