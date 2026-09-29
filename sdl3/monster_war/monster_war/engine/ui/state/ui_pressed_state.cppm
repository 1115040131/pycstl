module;

export module monster_war.engine.ui.state.ui_pressed_state;

export import monster_war.engine.ui;

export namespace pyc::monster_war {

/**
 * @brief 按下状态
 *
 * 当鼠标按下UI元素时，会切换到该状态。
 */
class UIPressedState final : public UIState {
    friend class UIInteractive;

public:
    UIPressedState(UIInteractive* owner);
    ~UIPressedState();

private:
    void enter() override;

    bool onMouseReleased();
};

}  // namespace pyc::monster_war
