#include "ghost_escape/scene_main.h"

#include <fstream>

#include <fmt/format.h>

#include "ghost_escape/hud_stats.h"
#include "ghost_escape/player.h"
#include "ghost_escape/scene_title.h"
#include "ghost_escape/spawner.h"
#include "sdl3/common/raw/timer.h"
#include "sdl3/common/screen/hud_button.h"
#include "sdl3/common/screen/hud_text.h"
#include "sdl3/common/screen/ui_mouse.h"

namespace pyc {
namespace sdl3 {

void SceneMain::init() {
    Scene::init();
    SDL_HideCursor();

#ifdef DEBUG_MODE
    name_ = "SceneMain";
#endif

    game_.playMusic(ASSET("bgm/OhMyGhost.ogg"));
    game_.setScore(0);

    world_size_ = game_.getScreenSize() * 3.0F;
    camera_position_ = world_size_ / 2.F - game_.getScreenSize() / 2.F;

    auto player = std::make_unique<Player>();
    player->init();
    player->setPosition(world_size_ / 2.F);
    player_ = static_cast<Player*>(addChild(std::move(player)));

    auto spawner = std::make_unique<Spawner>();
    spawner->init();
#ifdef DEBUG_MODE
    spawner->setName("Spawner");
#endif
    spawner->setTarget(player_);
    spawner_ = static_cast<Spawner*>(addChild(std::move(spawner)));

    hud_stats_ = HUDStatus::CreateAndSet(this, player_, glm::vec2(30));
    hud_text_score_ = HUDText::CreateAndSet(this, "Score: 0", glm::vec2(game_.getScreenSize().x - 120.f, 30.f),
                                            glm::vec2(200, 50), ASSET("font/VonwaonBitmap-16px.ttf"), 32,
                                            ASSET("UI/Textfield_01.png"));

    button_pause_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() - glm::vec2(230, 30), ASSET("UI/A_Pause1.png"),
                                ASSET("UI/A_Pause2.png"), ASSET("UI/A_Pause3.png"));
    button_restart_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() - glm::vec2(140, 30), ASSET("UI/A_Restart1.png"),
                                ASSET("UI/A_Restart2.png"), ASSET("UI/A_Restart3.png"));
    button_back_ =
        HUDButton::CreateAndSet(this, game_.getScreenSize() - glm::vec2(50, 30), ASSET("UI/A_Back1.png"),
                                ASSET("UI/A_Back2.png"), ASSET("UI/A_Back3.png"));
    ui_mouse_ = UIMouse::CreateAndSet(this, ASSET("UI/29.png"), ASSET("UI/30.png"), 2.0F);

    end_timer_ = Timer::CreateAndSet(this);
}

void SceneMain::clean() {
    Scene::clean();
    saveData(ASSET("score.dat"));
}

void SceneMain::update(std::chrono::duration<float> delta) {
    Scene::update(delta);
    updateScore();
    checkButtonPause();
    checkButtonRestart();
    checkButtonBack();
    checkEndTimer();
#ifdef DEBUG_MODE
    // fmt::println("children_world: {}, children_scrren: {}, children: {}", children_world_.size(),
    //              children_screen_.size(), children_.size());
    // printChildren();
#endif
}

void SceneMain::render() {
    renderBackground();
    Scene::render();
}

void SceneMain::saveData(std::string_view file_path) const {
    std::ofstream file(file_path.data(), std::ios::binary);
    if (!file.is_open()) {
        fmt::println("file {} open failed", file_path);
        return;
    }

    auto score = game_.getHighScore();
    file.write(reinterpret_cast<const char*>(&score), sizeof(score));
    file.close();
}

void SceneMain::checkButtonPause() {
    if (button_pause_->getIsTrigger()) {
        if (is_pause_) {
            resume();
        } else {
            pause();
        }
    }
}

void SceneMain::checkButtonRestart() {
    if (button_restart_->getIsTrigger()) {
        game_.changeScene(std::make_unique<SceneMain>());
    }
}

void SceneMain::checkButtonBack() {
    if (button_back_->getIsTrigger()) {
        game_.changeScene(std::make_unique<SceneTitle>());
    }
}

void SceneMain::checkEndTimer() {
    if (player_ && !player_->isActive()) {
        end_timer_->start();
    }
    if (end_timer_->isTimeOut()) {
        pause();
        button_restart_->setRenderPosition(game_.getScreenSize() / 2.0f - glm::vec2(200, 0));
        button_restart_->setScale(4.0f);
        button_back_->setRenderPosition(game_.getScreenSize() / 2.0f + glm::vec2(200, 0));
        button_back_->setScale(4.0f);
        button_pause_->setActive(false);
        end_timer_->stop();
    }
}

void SceneMain::updateScore() { hud_text_score_->setText(fmt::format("Score: {}", game_.getScore())); }

void SceneMain::renderBackground() const {
    auto start = -camera_position_;
    auto end = world_size_ - camera_position_;
    game_.drawGrid(start, end, 80.0F, 80.0F, {0.5, 0.5, 0.5, 1});
    game_.drawBoundary(start, end, 5.F, {1, 1, 1, 1});
}

}  // namespace sdl3
}  // namespace pyc
