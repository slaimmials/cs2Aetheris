#include "drawing.h"
#include "lua_additions.hpp"
#include <lua.hpp>
#include "imgui.h"

std::vector<DrawRect*> g_drawRects;
std::vector<DrawTextL*> g_drawTexts;

static int lua_draw_rect(lua_State* L) {
    Vector2 p1 = lua_to_vector2(L, 1);
    Vector2 p2 = lua_to_vector2(L, 2);

    DrawRect* rect = new DrawRect{ p1, p2, Color(255,255,255,255), 0.f, true, false };
    g_drawRects.push_back(rect);

    // Userdata для Lua
    DrawRect** udata = (DrawRect**)lua_newuserdata(L, sizeof(DrawRect*));
    *udata = rect;
    luaL_getmetatable(L, "MetaDrawRect");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_drawrect_index(lua_State* L) {
    DrawRect* rect = *(DrawRect**)luaL_checkudata(L, 1, "MetaDrawRect");
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "color") == 0) push_color(L, rect->color);
    else if (strcmp(key, "point1") == 0) push_vec2(L, rect->p1);
    else if (strcmp(key, "point2") == 0) push_vec2(L, rect->p2);
    else if (strcmp(key, "rounding") == 0) lua_pushnumber(L, rect->rounding);
    else if (strcmp(key, "visible") == 0) lua_pushboolean(L, rect->visible);
    else if (strcmp(key, "destroy") == 0) {
        lua_pushcfunction(L, [](lua_State* L) {
            DrawRect* rect = *(DrawRect**)luaL_checkudata(L, 1, "MetaDrawRect");
            rect->destroyed = true;
            return 0;
            });
    }
    else lua_pushnil(L);
    return 1;
}

static int lua_drawrect_newindex(lua_State* L) {
    DrawRect* rect = *(DrawRect**)luaL_checkudata(L, 1, "MetaDrawRect");
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "color") == 0) rect->color = lua_to_color(L, 3);
    else if (strcmp(key, "point1") == 0) rect->p1 = lua_to_vector2(L, 3);
    else if (strcmp(key, "point2") == 0) rect->p2 = lua_to_vector2(L, 3);
    else if (strcmp(key, "rounding") == 0) rect->rounding = luaL_checknumber(L, 3);
    else if (strcmp(key, "visible") == 0) rect->visible = lua_toboolean(L, 3);
    return 0;
}

static void RegisterDrawRectMeta(lua_State* L) {
    luaL_newmetatable(L, "MetaDrawRect");
    lua_pushcfunction(L, lua_drawrect_index); lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua_drawrect_newindex); lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

//TEXT
static int lua_draw_text(lua_State* L) {
    Vector2 pos = lua_to_vector2(L, 1);
    const char* txt = luaL_checkstring(L, 2);
    float size = luaL_optnumber(L, 3, 16.0f);

    DrawTextL* text = new DrawTextL{ pos, txt, Color(255,255,255,255), size, true, false };
    g_drawTexts.push_back(text);

    DrawTextL** udata = (DrawTextL**)lua_newuserdata(L, sizeof(DrawTextL*));
    *udata = text;
    luaL_getmetatable(L, "MetaDrawTextL");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_drawtextl_index(lua_State* L) {
    DrawTextL* t = *(DrawTextL**)luaL_checkudata(L, 1, "MetaDrawTextL");
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "color") == 0) push_color(L, t->color);
    else if (strcmp(key, "pos") == 0) push_vec2(L, t->pos);
    else if (strcmp(key, "text") == 0) lua_pushstring(L, t->text.c_str());
    else if (strcmp(key, "size") == 0) lua_pushnumber(L, t->size);
    else if (strcmp(key, "visible") == 0) lua_pushboolean(L, t->visible);
    else if (strcmp(key, "destroy") == 0) {
        lua_pushcfunction(L, [](lua_State* L) {
            DrawTextL* t = *(DrawTextL**)luaL_checkudata(L, 1, "MetaDrawTextL");
            t->destroyed = true;
            return 0;
            });
    }
    else lua_pushnil(L);
    return 1;
}

static int lua_drawtextl_newindex(lua_State* L) {
    DrawTextL* t = *(DrawTextL**)luaL_checkudata(L, 1, "MetaDrawTextL");
    const char* key = luaL_checkstring(L, 2);
    if (strcmp(key, "color") == 0) t->color = lua_to_color(L, 3);
    else if (strcmp(key, "pos") == 0) t->pos = lua_to_vector2(L, 3);
    else if (strcmp(key, "text") == 0) t->text = luaL_checkstring(L, 3);
    else if (strcmp(key, "size") == 0) t->size = luaL_checknumber(L, 3);
    else if (strcmp(key, "visible") == 0) t->visible = lua_toboolean(L, 3);
    return 0;
}

static void RegisterDrawTextLMeta(lua_State* L) {
    luaL_newmetatable(L, "MetaDrawTextL");
    lua_pushcfunction(L, lua_drawtextl_index); lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lua_drawtextl_newindex); lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
}

//BASE

void RegisterDrawingAPI(lua_State* L) {
    RegisterDrawRectMeta(L);
    RegisterDrawTextLMeta(L);
    lua_newtable(L);
    lua_pushcfunction(L, lua_draw_rect); lua_setfield(L, -2, "rect");
    lua_pushcfunction(L, lua_draw_text); lua_setfield(L, -2, "text");
    lua_setglobal(L, "draw");
}

void DrawGui() {
    for (auto it = g_drawRects.begin(); it != g_drawRects.end(); ) {
        DrawRect* rect = *it;
        if (rect->destroyed) {
            delete rect;
            it = g_drawRects.erase(it);
            continue;
        }
        if (rect->visible) {
            ImGui::GetOverlayDrawList()->AddRectFilled(
                ImVec2(rect->p1.x, rect->p1.y),
                ImVec2(rect->p2.x, rect->p2.y),
                ImColor(rect->color.r, rect->color.g, rect->color.b, rect->color.a),
                rect->rounding
            );
        }
        ++it;
    }
    for (auto it = g_drawTexts.begin(); it != g_drawTexts.end(); ) {
        DrawTextL* t = *it;
        if (t->destroyed) {
            delete t;
            it = g_drawTexts.erase(it);
            continue;
        }
        if (t->visible) {
            ImGui::GetOverlayDrawList()->AddText(
                nullptr, t->size,
                ImVec2(t->pos.x, t->pos.y),
                ImColor(t->color.r, t->color.g, t->color.b, t->color.a),
                t->text.c_str()
            );
        }
        ++it;
    }
}