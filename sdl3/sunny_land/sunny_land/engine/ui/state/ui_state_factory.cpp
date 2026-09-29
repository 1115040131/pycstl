module;

#include <memory>

module sunny_land.engine.ui.state;

import sunny_land.engine.ui;

namespace pyc::sunny_land {

template <typename T>
std::unique_ptr<UIState> UIStateFactory::create(UIInteractive* owner) {
    return std::make_unique<T>(owner);
}

template std::unique_ptr<UIState> UIStateFactory::create<UIHoverState>(UIInteractive* owner);
template std::unique_ptr<UIState> UIStateFactory::create<UINormalState>(UIInteractive* owner);
template std::unique_ptr<UIState> UIStateFactory::create<UIPressedState>(UIInteractive* owner);

}  // namespace pyc::sunny_land
