#pragma once

#include <chrono>
#include <list>
#include <optional>

#include "common/singleton.h"
#include "tetris/piece.h"
#include "tetris/play_field.h"

namespace pyc {
namespace tetris {

using namespace std::chrono_literals;

class Game : public Singleton<Game> {
    friend class Singleton<Game>;
    friend class Control;

public:
    void Init();

    void Process(std::chrono::nanoseconds delta);

    bool Running() const { return running_; }

    bool Ending() const { return ending_; }

    bool Helping() const { return helping_; }

    std::size_t FPS() const { return fps_; }

    int Level() const { return level_; }

    int Score() const { return score_; }

    int Lines() const { return lines_; }

    const Matrix& GetPlayField() const { return play_field_; }

    const Matrix& GetFrame() const { return frame_; }

    const Matrix& GetPreviewField() const { return preview_field_; }

    const Matrix& GetHoldField() const { return hold_field_; }

private:
    /// @brief 随机生成一个块
    Piece GeneratePiece();

    /// @brief 获取一个 piece
    void PickPiece();

    /// @brief 载入预设地图
    void Load();

    void UpdateFps(std::chrono::nanoseconds delta);

    void UpdateLogic(std::chrono::nanoseconds delta);

    void UpdateRender();

    /// @brief 渲染
    void Render();

    void Quit() { running_ = false; }

    void Rotate2() {
        if (!locking_) {
            piece_.Rotate2();
        }
    }

    void RotateR() {
        if (!locking_) {
            piece_.RotateR();
        }
    }

    void RotateL() {
        if (!locking_) {
            piece_.RotateL();
        }
    }

    void Left() {
        if (!locking_) {
            piece_.Left();
        }
    }

    void Right() {
        if (!locking_) {
            piece_.Right();
        }
    }

    void Down() {
        if (!locking_) {
            piece_.Down();
        }
    }

    /// @brief 锁定
    void Lock();

    /// @brief 消行
    void Clear();

    /// @brief 直落
    void Drop();

    /// @brief 暂存
    void Hold();

    /// @brief 升级
    void Levelup();

    /// @brief 重置
    void Reset();

    /// @brief 展示 Help
    void Help();

private:
    static constexpr int kPieceInitX = 4;
    static constexpr int kPieceInitY = 20;
    static constexpr int kPieceInitIndex = 0;

    bool running_{false};
    bool locking_{false};
    bool holding_{false};  // 是否使用过暂存
    bool ending_{false};
    bool helping_{false};

    std::size_t fps_ = 0;
    /*
    Single 100 x level
    Double 300 x level
    Triple 500 x level
    Tetris 800 x level
    Soft Drop 1
    Hard Drop 2
    */
    int level_ = 1;
    int score_ = 0;
    int lines_ = 0;
    std::chrono::nanoseconds down_duration_{800ms};

    Matrix play_field_ = std::vector<std::vector<int>>(kPlayFieldRow, std::vector<int>(kPlayFieldCol));
    Piece piece_{};

    std::vector<TetrominoSet> bags_;
    std::list<Piece> preview_;  // 预览队列

    std::optional<Piece> hold_piece_;  // 暂存块

    // 渲染用
    Matrix frame_;
    Matrix preview_field_;
    Matrix hold_field_;
};

}  // namespace tetris
}  // namespace pyc
