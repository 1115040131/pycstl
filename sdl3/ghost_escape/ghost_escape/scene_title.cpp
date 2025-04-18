#include "ghost_escape/scene_title.h"

#include <fmt/format.h>

#include "ghost_escape/scene_main.h"

namespace pyc {
namespace sdl3 {

void SceneTitle::init() {
    Scene::init();
    SDL_ShowCursor();

#ifdef DEBUG_MODE
    name_ = "SceneTitle";
#endif

    game_.playMusic(ASSET("bgm/Spooky music.mp3"));

    HUDText::CreateAndSet(this, "幽灵逃生", game_.getScreenSize() / 2.0f - glm::vec2(0, 100),
                          glm::vec2(game_.getScreenSize().x / 2.0f, game_.getScreenSize().y / 3.0f),
                          ASSET("font/VonwaonBitmap-16px.ttf"), 64, ASSET("UI/Textfield_01.png"));
    HUDText::CreateAndSet(this, fmt::format("最高分: {}", game_.getHighScore()),
                          game_.getScreenSize() / 2.0f + glm::vec2(0, 100), glm::vec2(200, 50),
                          ASSET("font/VonwaonBitmap-16px.ttf"), 32, ASSET("UI/Textfield_01.png"));

    button_start_ = HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(-200, 200),
                                            ASSET("UI/A_Start1.png"), ASSET("UI/A_Start2.png"),
                                            ASSET("UI/A_Start3.png"), 2.0f);
    button_credits_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(0, 200), ASSET("UI/A_Credits1.png"),
                                ASSET("UI/A_Credits2.png"), ASSET("UI/A_Credits3.png"), 2.0f);
    button_quit_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(200), ASSET("UI/A_Quit1.png"),
                                ASSET("UI/A_Quit2.png"), ASSET("UI/A_Quit3.png"), 2.0f);

    auto text = game_.loadTextFile(ASSET("credits.txt"));
    credits_text_ = HUDText::CreateAndSet(this, text, game_.getScreenSize() / 2.0f, glm::vec2(500),
                                          ASSET("font/VonwaonBitmap-16px.ttf"), 16, ASSET("UI/Textfield_01.png"));
    credits_text_->setSizeByText();
    credits_text_->setActive(false);
}

bool SceneTitle::handleEvents(const SDL_Event& event) {
    if (credits_text_->isActive()) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            credits_text_->setActive(false);
            return true;
        }
    }
    return Scene::handleEvents(event);
}

void SceneTitle::update(std::chrono::duration<float> delta) {
    updateColor(delta);

    if (credits_text_->isActive()) {
        return;
    }
    Scene::update(delta);
    checkButtonStart();
    checkButtonCredits();
    checkButtonQuit();

#ifdef DEBUG_MODE
    // fmt::println("children_world: {}, children_scrren: {}, children: {}", children_world_.size(),
    //              children_screen_.size(), children_.size());
    // printChildren();
#endif
}

void SceneTitle::render() {
    renderBackground();
    Scene::render();
}

void SceneTitle::updateColor(std::chrono::duration<float> delta) {
    timer_ += delta;
    boundary_color_.r = 0.5f + 0.5 * std::sin(timer_.count() * 0.9f);
    boundary_color_.g = 0.5f + 0.5 * std::sin(timer_.count() * 0.8f);
    boundary_color_.b = 0.5f + 0.5 * std::sin(timer_.count() * 0.7f);
}

void SceneTitle::checkButtonStart() {
    if (button_start_->getIsTrigger()) {
        game_.changeScene(std::make_unique<SceneMain>());
    }
}

void SceneTitle::checkButtonCredits() {
    if (button_credits_->getIsTrigger()) {
        credits_text_->setActive(!credits_text_->isActive());
    }
}

void SceneTitle::checkButtonQuit() {
    if (button_quit_->getIsTrigger()) {
        game_.quit();
    }
}

void SceneTitle::renderBackground() const {
    game_.drawBoundary(glm::vec2(30), game_.getScreenSize() - glm::vec2(30), 10.0f, boundary_color_);
}

}  // namespace sdl3
}  // namespace pyc