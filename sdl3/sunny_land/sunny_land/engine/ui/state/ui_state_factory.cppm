module;

#include <memory>

export module sunny_land.engine.ui.state:ui_state_factory;

export import sunny_land.engine.ui;

export namespace pyc::sunny_land {

class UIHoverState;
class UINormalState;
class UIPressedState;

class UIStateFactory final {
public:
    template <typename T>
    static std::unique_ptr<UIState> create(UIInteractive* owner);
};

}  // namespace pyc::sunny_land
