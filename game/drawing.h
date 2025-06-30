#pragma once

#include <vector>
#include "game.h"
#include "lua.hpp"

struct DrawRect {
    Vector2 p1, p2;
    Color color;
    float rounding;
    bool visible = true;
    bool destroyed = false;
};

struct DrawTextL {
    Vector2 pos;
    std::string text;
    Color color;
    float size;
    bool visible = true;
    bool destroyed = false;
};

extern std::vector<DrawRect*> g_drawRects;
extern std::vector<DrawTextL*> g_drawTexts;

void RegisterDrawingAPI(lua_State* L);
void DrawGui();