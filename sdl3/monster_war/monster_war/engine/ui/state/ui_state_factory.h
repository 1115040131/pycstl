#pragma once

#include <memory>

#include "monster_war/engine/ui/state/ui_state.h"

namespace pyc::monster_war {

class UIInteractive;

class UIHoverState;
class UINormalState;
class UIPressedState;

class UIStateFactory final {
public:
    template <typename T>
    static std::unique_ptr<UIState> create(UIInteractive* owner);
};

}  // namespace pyc::monster_war