module;

#include <memory>

export module monster_war.engine.ui.state.ui_state_factory;

export import monster_war.engine.ui;

export namespace pyc::monster_war {

class UIStateFactory final {
public:
    template <typename T>
    static std::unique_ptr<UIState> create(UIInteractive* owner) {
        return std::make_unique<T>(owner);
    }
};

}  // namespace pyc::monster_war
