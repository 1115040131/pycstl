#include "tetris/control.h"

#include <termios.h>
#include <unistd.h>

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
        ::cfmakeraw(&raw_termios);
        // 应用新的终端设置
        ::tcsetattr(STDIN_FILENO, TCSANOW, &raw_termios);

        ProcessCommand();

        // 恢复终端设置
        ::tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    });
}

void Control::ProcessCommand() {
    while (Game::GetInstance().Running()) {
        switch (std::cin.get()) {
            case KeyQ:
                Game::GetInstance().Quit();
                break;
            case KeyW:
                Game::GetInstance().RotateR();
                break;
            case KeyA:
                Game::GetInstance().Left();
                break;
            case KeyD:
                Game::GetInstance().Right();
                break;
            case KeyS:
                Game::GetInstance().Down();
                break;
            case KeyZ:
                Game::GetInstance().RotateL();
                break;
            case KeyX:
                Game::GetInstance().Rotate2();
                break;
            case KeyC:
                Game::GetInstance().Hold();
                break;
            case KeyR:
                Game::GetInstance().Reset();
                break;
            case KeyH:
                Game::GetInstance().Help();
                break;
            case KeySpace:
                Game::GetInstance().Drop();
                break;
            default:
                break;
        }
    }
}

}  // namespace tetris
}  // namespace pyc