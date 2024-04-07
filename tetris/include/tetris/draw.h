#pragma once

#include <string_view>

namespace pyc {
namespace tetris {

enum class WindowStyle { kStyle1, kStyle2, kStyle3, kStyle4 };

template<WindowStyle style = WindowStyle::kStyle1>
void Window(int top, int left, int width, int height, std::string_view title);

}  // namespace tetris
}  // namespace pyc
