module;

#include <chrono>
#include <cmath>
#include <fstream>
#include <ios>
#include <memory>
#include <string_view>

#include <SDL3/SDL.h>
#include <fmt/format.h>
#include <glm/vec2.hpp>

module ghost_escape.scene_title;

import ghost_escape.core;
import ghost_escape.scene_main;
import ghost_escape.screen.hud_button;
import ghost_escape.screen.hud_text;
import ghost_escape.screen.ui_mouse;

namespace pyc::sdl3 {

void SceneTitle::init() {
    Scene::init();
    loadData(Asset("score.dat"));
    SDL_HideCursor();

#ifdef DEBUG_MODE
    name_ = "SceneTitle";
#endif

    game_.playMusic(Asset("bgm/Spooky music.mp3"));

    HUDText::CreateAndSet(this, "幽灵逃生", game_.getScreenSize() / 2.0f - glm::vec2(0, 100),
                          glm::vec2(game_.getScreenSize().x / 2.0f, game_.getScreenSize().y / 3.0f),
                          Asset("font/VonwaonBitmap-16px.ttf"), 64, Asset("UI/Textfield_01.png"));
    HUDText::CreateAndSet(this, fmt::format("最高分: {}", game_.getHighScore()),
                          game_.getScreenSize() / 2.0f + glm::vec2(0, 100), glm::vec2(200, 50),
                          Asset("font/VonwaonBitmap-16px.ttf"), 32, Asset("UI/Textfield_01.png"));

    button_start_ = HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(-200, 200),
                                            Asset("UI/A_Start1.png"), Asset("UI/A_Start2.png"),
                                            Asset("UI/A_Start3.png"), 2.0f);
    button_credits_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(0, 200), Asset("UI/A_Credits1.png"),
                                Asset("UI/A_Credits2.png"), Asset("UI/A_Credits3.png"), 2.0f);
    button_quit_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() / 2.0f + glm::vec2(200), Asset("UI/A_Quit1.png"),
                                Asset("UI/A_Quit2.png"), Asset("UI/A_Quit3.png"), 2.0f);

    auto text = game_.loadTextFile(Asset("credits.txt"));
    credits_text_ = HUDText::CreateAndSet(this, text, game_.getScreenSize() / 2.0f, glm::vec2(500),
                                          Asset("font/VonwaonBitmap-16px.ttf"), 16, Asset("UI/Textfield_01.png"));
    credits_text_->setSizeByText();
    credits_text_->setActive(false);

    ui_mouse_ = UIMouse::CreateAndSet(this, Asset("UI/pointer_c_shaded.png"), Asset("UI/pointer_c_shaded.png"),
                                      1.0F, Anchor::kTopLeft);
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
        ui_mouse_->update(delta);
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

void SceneTitle::loadData(std::string_view file_path) const {
    int score = 0;

    std::ifstream file(file_path.data(), std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&score), sizeof(score));
        file.close();
    }

    game_.setHighScore(score);
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

}  // namespace pyc::sdl3
