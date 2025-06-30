#pragma once
#include <cstdint>
#include <vector>
#include <Windows.h>
#include "engine.h"
#include "lua.hpp"
#include "../memoryModule.h"
#include "game_types.h"

extern GameWorld world;

void RegisterWorldAPI(lua_State* L);