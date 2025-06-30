#pragma once
#include <Windows.h>
#include "game/game.h"
#include "game/drawing.h"
#include "game/math2.h"
#include "game/lua_additions.hpp"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace Lua {
	extern void Execute(const char* code);
	extern void UpdateLuaCoroutines();
	extern bool isGameReady();
}