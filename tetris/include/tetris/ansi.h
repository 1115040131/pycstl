#pragma once

#include <string>
#include <string_view>

#include <fmt/core.h>

namespace pyc {
namespace tetris {

// 定义ANSI转义序列的引导符
#define CSI "\033["
#define SGR(params) CSI params "m"

// 常量定义
inline constexpr std::string_view kClear = CSI "2J";              // 清屏
inline constexpr std::string_view kCursorInvisible = CSI "?25l";  // 隐藏光标
inline constexpr std::string_view kCursorVisible = CSI "?25h";    // 显示光标
inline constexpr std::string_view kReset = SGR("0");              // 清除格式化样式

// 格式化样式
inline constexpr std::string_view kBold = SGR("1");
inline constexpr std::string_view kFaint = SGR("2");
inline constexpr std::string_view kItalic = SGR("3");
inline constexpr std::string_view kUnderline = SGR("4");
inline constexpr std::string_view kBlinkSlow = SGR("5");
inline constexpr std::string_view kBlinkRapid = SGR("6");
inline constexpr std::string_view kReverse = SGR("7");
inline constexpr std::string_view kConceal = SGR("8");
inline constexpr std::string_view kCrossedOut = SGR("9");

// 取消格式化样式
inline constexpr std::string_view kNoBoldNoFaint = SGR("22");
inline constexpr std::string_view kNoItalic = SGR("23");
inline constexpr std::string_view kNoUnderline = SGR("24");
inline constexpr std::string_view kNoBlink = SGR("25");
inline constexpr std::string_view kNoReverse = SGR("27");
inline constexpr std::string_view kReveal = SGR("28");
inline constexpr std::string_view kNoCrossedOut = SGR("29");

// 前景色
inline constexpr std::string_view kFgBlack = SGR("30");
inline constexpr std::string_view kFgRed = SGR("31");
inline constexpr std::string_view kFgGreen = SGR("32");
inline constexpr std::string_view kFgYellow = SGR("33");
inline constexpr std::string_view kFgBlue = SGR("34");
inline constexpr std::string_view kFgMagenta = SGR("35");
inline constexpr std::string_view kFgCyan = SGR("36");
inline constexpr std::string_view kFgWhite = SGR("37");
inline constexpr std::string_view kFgDefault = SGR("39");

// 背景色
inline constexpr std::string_view kBlackBk = SGR("40");
inline constexpr std::string_view kRedBk = SGR("41");
inline constexpr std::string_view kGreenBk = SGR("42");
inline constexpr std::string_view kYellowBk = SGR("43");
inline constexpr std::string_view kBlueBk = SGR("44");
inline constexpr std::string_view kMagentaBk = SGR("45");
inline constexpr std::string_view kCyanBk = SGR("46");
inline constexpr std::string_view kWhiteBk = SGR("47");
inline constexpr std::string_view kDefaultBk = SGR("49");

// 256 颜色 id
enum class ColorId {
    kBlack = 0,
    kRed,
    kGreen,
    kYellow,
    kBlue,
    kMagenta,
    kCyan,
    kWhite,
    kBrightBlack,
    kBrightRed,
    kBrightGreen,
    kBrightYellow,
    kBrightBlue,
    kBrightMagenta,
    kBrightCyan,
    kBrightWhite,

    kOrange = 214,
};

/// @brief 移动光标
inline std::string MoveTo(int line, int column) { return fmt::format(CSI "{};{}H", line, column); }

/// @brief 设置前景色
inline std::string SetColor(ColorId color_id) { return fmt::format(CSI "38;5;{}m", static_cast<int>(color_id)); }

/// @brief 设置背景色
inline std::string SetBackgroundColor(ColorId color_id) {
    return fmt::format(CSI "48;5;{}m", static_cast<int>(color_id));
}

}  // namespace tetris
}  // namespace pyc
