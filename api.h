#pragma once
#include <Windows.h>
#include "lua.hpp"
#include "game/game.h"
#include "game/drawing.h"
#include "game/view.h"
#include "game/lua_additions.hpp"
#include "game/hooks.h"

namespace Lua {
    void Init();
    void Execute(const char* code);
    void UpdateLuaCoroutines();
    bool isGameReady();
}