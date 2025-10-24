#pragma once

#include <memory>

#include "sunny_land/engine/ui/state/ui_state.h"

namespace pyc::sunny_land {

class UIInteractive;

class UIHoverState;
class UINormalState;
class UIPressedState;

class UIStateFactory final {
public:
    template <typename T>
    static std::unique_ptr<UIState> create(UIInteractive* owner);
};

}  // namespace pyc::sunny_land