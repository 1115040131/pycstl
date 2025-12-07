#include "monster_war/engine/ui/state/ui_state_factory.h"

#include "monster_war/engine/ui/state/ui_hover_state.h"
#include "monster_war/engine/ui/state/ui_normal_state.h"
#include "monster_war/engine/ui/state/ui_pressed_state.h"

namespace pyc::monster_war {

template <typename T>
std::unique_ptr<UIState> UIStateFactory::create(UIInteractive* owner) {
    return std::make_unique<T>(owner);
}

template std::unique_ptr<UIState> UIStateFactory::create<UIHoverState>(UIInteractive* owner);
template std::unique_ptr<UIState> UIStateFactory::create<UINormalState>(UIInteractive* owner);
template std::unique_ptr<UIState> UIStateFactory::create<UIPressedState>(UIInteractive* owner);

}  // namespace pyc::monster_war