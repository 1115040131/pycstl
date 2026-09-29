// 宏导不出模块，ASSET/ASSET_PATH 只能留在纯头文件里。
#pragma once

#define ASSET_PATH "sdl3/ghost_escape/assets/"
#define ASSET(filename) (ASSET_PATH filename)
