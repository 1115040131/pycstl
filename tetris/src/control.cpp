#include "tetris/control.h"

#include <termios.h>

#include <iostream>
#include <thread>

#include "tetris/game.h"

namespace pyc {
namespace tetris {

void Control::StartListener() {
    static std::jthread key_event([]() {
        termios original_termios;
        // 获取当前终端属性，并保存以便于稍后恢复
        ::tcgetattr(STDIN_FILENO, &original_termios);
        termios raw_termios = original_termios;
        // 修改终端设置为原始模式
        cfmakeraw(&raw_termios);
        // 应用新的终端设置
        tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios);

        Control::ProcessCommand();

        // 恢复终端设置
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    });
}

void Control::ProcessCommand() {
    while (Game::GetInstance().Running()) {
        switch (std::cin.get()) {
            case kKeyQ:
                CommandQuit();
                break;
            case kKeyW:
                CommandRotate();
                break;
            case kKeyA:
                CommandLeft();
                break;
            case kKeyD:
                CommandRight();
                break;
            case kKeyS:
                CommandDown();
            default:
                break;
        }
    }
}

void Control::CommandQuit() { Game::GetInstance().Quit(); }

void Control::CommandRotate() { Game::GetInstance().Rotate(); }

void Control::CommandLeft() { Game::GetInstance().Left(); }

void Control::CommandRight() { Game::GetInstance().Right(); }

void Control::CommandDown() { Game::GetInstance().Down(); }

}  // namespace tetris
}  // namespace pyc