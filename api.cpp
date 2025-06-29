#include "api.h"
#include <thread>
#include <chrono>
#include <vector>
#include <windows.h>
#include <ctime>

// --- Coroutine manager ---

struct ScriptCoroutine {
    lua_State* thread;
    double resumeTime;
    bool finished = false;
};

std::vector<ScriptCoroutine> g_coroutines;

double CurrentTime() {
    return static_cast<double>(std::clock()) / CLOCKS_PER_SEC;
}

// --- API: MessageBox ---

int lua_MessageBox(lua_State* L) {
    const char* message = luaL_checkstring(L, 1);
    MessageBoxA(0, message, "Lua", 0);
    return 0;
}

// --- API: input.IsKeyDown(key) ---

int lua_input_IsKeyDown(lua_State* L) {
    int key = luaL_checkinteger(L, 1);
    bool isDown = (GetAsyncKeyState(key) & 0x8000) != 0;
    lua_pushboolean(L, isDown);
    return 1;
}

// --- API: wait(seconds) ---

int lua_wait(lua_State* L) {
    double seconds = luaL_checknumber(L, 1);
    if (seconds < 0) seconds = 0;
    lua_pushnumber(L, seconds);
    return lua_yield(L, 1);
}

// --- Register input table and key constants in Lua ---

void RegisterInputTable(lua_State* L) {
    lua_newtable(L); // input table

    lua_pushcfunction(L, lua_input_IsKeyDown);
    lua_setfield(L, -2, "IsKeyDown");

    lua_setglobal(L, "input");
}

void RegisterKeyConstants(lua_State* L) {
    // Буквы
    for (char c = 'A'; c <= 'Z'; ++c) {
        char keyName[8];
        snprintf(keyName, sizeof(keyName), "KEY_%c", c);
        lua_pushinteger(L, c);
        lua_setglobal(L, keyName);
    }

    // Цифры
    for (char c = '0'; c <= '9'; ++c) {
        char keyName[10];
        snprintf(keyName, sizeof(keyName), "KEY_%c", c);
        lua_pushinteger(L, c);
        lua_setglobal(L, keyName);
    }

    // Функциональные клавиши F1-F24
    for (int i = 1; i <= 24; ++i) {
        char keyName[10];
        snprintf(keyName, sizeof(keyName), "KEY_F%d", i);
        lua_pushinteger(L, VK_F1 + (i - 1));
        lua_setglobal(L, keyName);
    }

    // Стрелки
    lua_pushinteger(L, VK_UP); lua_setglobal(L, "KEY_UP");
    lua_pushinteger(L, VK_DOWN); lua_setglobal(L, "KEY_DOWN");
    lua_pushinteger(L, VK_LEFT); lua_setglobal(L, "KEY_LEFT");
    lua_pushinteger(L, VK_RIGHT); lua_setglobal(L, "KEY_RIGHT");

    // Control, Shift, Alt, Win
    lua_pushinteger(L, VK_LSHIFT); lua_setglobal(L, "KEY_LSHIFT");
    lua_pushinteger(L, VK_RSHIFT); lua_setglobal(L, "KEY_RSHIFT");
    lua_pushinteger(L, VK_LCONTROL); lua_setglobal(L, "KEY_LCTRL");
    lua_pushinteger(L, VK_RCONTROL); lua_setglobal(L, "KEY_RCTRL");
    lua_pushinteger(L, VK_LMENU); lua_setglobal(L, "KEY_LALT");
    lua_pushinteger(L, VK_RMENU); lua_setglobal(L, "KEY_RALT");
    lua_pushinteger(L, VK_LWIN); lua_setglobal(L, "KEY_LWIN");
    lua_pushinteger(L, VK_RWIN); lua_setglobal(L, "KEY_RWIN");

    // Пробел, таб, энтер, esc, backspace, delete, insert, home, end, page up/down
    lua_pushinteger(L, VK_SPACE); lua_setglobal(L, "KEY_SPACE");
    lua_pushinteger(L, VK_TAB); lua_setglobal(L, "KEY_TAB");
    lua_pushinteger(L, VK_RETURN); lua_setglobal(L, "KEY_ENTER");
    lua_pushinteger(L, VK_ESCAPE); lua_setglobal(L, "KEY_ESCAPE");
    lua_pushinteger(L, VK_BACK); lua_setglobal(L, "KEY_BACKSPACE");
    lua_pushinteger(L, VK_DELETE); lua_setglobal(L, "KEY_DELETE");
    lua_pushinteger(L, VK_INSERT); lua_setglobal(L, "KEY_INSERT");
    lua_pushinteger(L, VK_HOME); lua_setglobal(L, "KEY_HOME");
    lua_pushinteger(L, VK_END); lua_setglobal(L, "KEY_END");
    lua_pushinteger(L, VK_PRIOR); lua_setglobal(L, "KEY_PAGEUP");
    lua_pushinteger(L, VK_NEXT); lua_setglobal(L, "KEY_PAGEDOWN");

    // Numpad
    lua_pushinteger(L, VK_NUMPAD0); lua_setglobal(L, "KEY_NUMPAD0");
    lua_pushinteger(L, VK_NUMPAD1); lua_setglobal(L, "KEY_NUMPAD1");
    lua_pushinteger(L, VK_NUMPAD2); lua_setglobal(L, "KEY_NUMPAD2");
    lua_pushinteger(L, VK_NUMPAD3); lua_setglobal(L, "KEY_NUMPAD3");
    lua_pushinteger(L, VK_NUMPAD4); lua_setglobal(L, "KEY_NUMPAD4");
    lua_pushinteger(L, VK_NUMPAD5); lua_setglobal(L, "KEY_NUMPAD5");
    lua_pushinteger(L, VK_NUMPAD6); lua_setglobal(L, "KEY_NUMPAD6");
    lua_pushinteger(L, VK_NUMPAD7); lua_setglobal(L, "KEY_NUMPAD7");
    lua_pushinteger(L, VK_NUMPAD8); lua_setglobal(L, "KEY_NUMPAD8");
    lua_pushinteger(L, VK_NUMPAD9); lua_setglobal(L, "KEY_NUMPAD9");
    lua_pushinteger(L, VK_DECIMAL); lua_setglobal(L, "KEY_NUMPAD_DECIMAL");
    lua_pushinteger(L, VK_DIVIDE); lua_setglobal(L, "KEY_NUMPAD_DIVIDE");
    lua_pushinteger(L, VK_MULTIPLY); lua_setglobal(L, "KEY_NUMPAD_MULTIPLY");
    lua_pushinteger(L, VK_SUBTRACT); lua_setglobal(L, "KEY_NUMPAD_SUBTRACT");
    lua_pushinteger(L, VK_ADD); lua_setglobal(L, "KEY_NUMPAD_ADD");
    lua_pushinteger(L, VK_SEPARATOR); lua_setglobal(L, "KEY_NUMPAD_SEPARATOR");

    // Клавиши Caps, Scroll, Num Lock
    lua_pushinteger(L, VK_CAPITAL); lua_setglobal(L, "KEY_CAPSLOCK");
    lua_pushinteger(L, VK_NUMLOCK); lua_setglobal(L, "KEY_NUMLOCK");
    lua_pushinteger(L, VK_SCROLL); lua_setglobal(L, "KEY_SCROLLLOCK");

    // PrintScreen, Pause, Menu, Apps
    lua_pushinteger(L, VK_SNAPSHOT); lua_setglobal(L, "KEY_PRINTSCREEN");
    lua_pushinteger(L, VK_PAUSE); lua_setglobal(L, "KEY_PAUSE");
    lua_pushinteger(L, VK_APPS); lua_setglobal(L, "KEY_APPS");
    lua_pushinteger(L, VK_MENU); lua_setglobal(L, "KEY_MENU");

    // OEM keys (точка, запятая, тире и др.)
    lua_pushinteger(L, VK_OEM_PERIOD); lua_setglobal(L, "KEY_PERIOD");
    lua_pushinteger(L, VK_OEM_COMMA); lua_setglobal(L, "KEY_COMMA");
    lua_pushinteger(L, VK_OEM_MINUS); lua_setglobal(L, "KEY_MINUS");
    lua_pushinteger(L, VK_OEM_PLUS); lua_setglobal(L, "KEY_PLUS");
    lua_pushinteger(L, VK_OEM_1); lua_setglobal(L, "KEY_SEMICOLON");
    lua_pushinteger(L, VK_OEM_2); lua_setglobal(L, "KEY_SLASH");
    lua_pushinteger(L, VK_OEM_3); lua_setglobal(L, "KEY_TILDE");
    lua_pushinteger(L, VK_OEM_4); lua_setglobal(L, "KEY_LBRACKET");
    lua_pushinteger(L, VK_OEM_5); lua_setglobal(L, "KEY_BACKSLASH");
    lua_pushinteger(L, VK_OEM_6); lua_setglobal(L, "KEY_RBRACKET");
    lua_pushinteger(L, VK_OEM_7); lua_setglobal(L, "KEY_APOSTROPHE");

    //Mouse
    lua_pushinteger(L, VK_LBUTTON); lua_setglobal(L, "MOUSE_LBUTTON"); // Левая кнопка мыши
    lua_pushinteger(L, VK_RBUTTON); lua_setglobal(L, "MOUSE_RBUTTON"); // Правая кнопка мыши
    lua_pushinteger(L, VK_MBUTTON); lua_setglobal(L, "MOUSE_MBUTTON"); // Средняя кнопка мыши (колесо)
    lua_pushinteger(L, VK_XBUTTON1); lua_setglobal(L, "MOUSE_XBUTTON1"); // Боковая (X1)
    lua_pushinteger(L, VK_XBUTTON2); lua_setglobal(L, "MOUSE_XBUTTON2"); // Боковая (X2)
}

// =====================
// === GAME BINDINGS ===
// =====================

static const char* LUA_PLAYER_MT = "MetaPlayer";

// --- Push Vector3 as Lua table ---
void push_vec3(lua_State* L, const Vector3& v) {
    lua_newtable(L);
    lua_pushnumber(L, v.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, v.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, v.z); lua_setfield(L, -2, "z");
}

// --- Lua Player Proxy Userdata ---
struct LuaPlayerProxy {
    Player player;
};

// --- __index for Player proxy ---
int lua_player_index(lua_State* L) {
    LuaPlayerProxy* proxy = (LuaPlayerProxy*)luaL_checkudata(L, 1, LUA_PLAYER_MT);
    const char* key = luaL_checkstring(L, 2);

    // Добавляем поля
    if (strcmp(key, "health") == 0) {
        lua_pushinteger(L, proxy->player.health());
        return 1;
    }
    if (strcmp(key, "team") == 0) {
        lua_pushinteger(L, proxy->player.team());
        return 1;
    }
    if (strcmp(key, "alive") == 0) {
        lua_pushboolean(L, proxy->player.alive());
        return 1;
    }
    if (strcmp(key, "pos") == 0) {
        push_vec3(L, proxy->player.pos());
        return 1;
    }
    if (strcmp(key, "velocity") == 0) {
        push_vec3(L, proxy->player.velocity());
        return 1;
    }
    if (strcmp(key, "scoped") == 0) {
        lua_pushboolean(L, proxy->player.scoped());
        return 1;
    }
    // ... другие поля если необходимо

    lua_pushnil(L);
    return 1;
}

// --- Push Player as Lua userdata ---
void push_player(lua_State* L, const Player& player) {
    LuaPlayerProxy* proxy = (LuaPlayerProxy*)lua_newuserdata(L, sizeof(LuaPlayerProxy));
    new(proxy) LuaPlayerProxy{ player };
    luaL_getmetatable(L, LUA_PLAYER_MT);
    lua_setmetatable(L, -2);
}

// --- Lua: world.ents.LocalPlayer() ---
int lua_ents_LocalPlayer(lua_State* L) {
    Player local = world.ents.GetLocalPlayer();
    if (local.pawnAddr) {
        push_player(L, local);
        return 1;
    }
    else {
        printf("Local player huynya\n");
    }
    lua_pushnil(L);
    return 1;
}

// --- Lua: world.ents.GetPlayers() ---
int lua_ents_GetPlayers(lua_State* L) {
    std::vector<Player> players = world.ents.GetPlayers();
    lua_newtable(L);
    int idx = 1;
    for (const auto& p : players) {
        if (p.pawnAddr) {
            push_player(L, p);
            lua_rawseti(L, -2, idx++);
        }
    }
    return 1;
}

// --- Register world/ents in Lua ---
void RegisterWorldAPI(lua_State* L) {
    // Player metatable
    luaL_newmetatable(L, LUA_PLAYER_MT);
    lua_pushcfunction(L, lua_player_index); lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    // ents table
    lua_newtable(L);
    lua_pushcfunction(L, lua_ents_LocalPlayer); lua_setfield(L, -2, "LocalPlayer");
    lua_pushcfunction(L, lua_ents_GetPlayers); lua_setfield(L, -2, "GetPlayers");

    // world table
    lua_newtable(L);
    lua_pushvalue(L, -2); lua_setfield(L, -2, "ents");
    lua_setglobal(L, "world");
    lua_pop(L, 1); // ents
}

// --- Register all API functions ---

void RegisterFunctions(lua_State* L) {
    lua_register(L, "wait", lua_wait);
    lua_register(L, "MessageBox", lua_MessageBox);
    RegisterInputTable(L);
    RegisterKeyConstants(L);
    RegisterWorldAPI(L);
}

// --- Coroutine launcher ---

void Lua::Execute(const char* code) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    RegisterFunctions(L);

    lua_State* thread = lua_newthread(L);
    if (luaL_loadstring(thread, code) == LUA_OK) {
        g_coroutines.push_back({ thread, 0.0, false });
    }
    else {
        const char* error = lua_tostring(thread, -1);
        if (error) printf("%s\n", error);
        lua_close(L);
    }
}

// --- Coroutine updater ----

void Lua::UpdateLuaCoroutines() {
    double now = CurrentTime();
    for (auto& co : g_coroutines) {
        if (co.finished) continue;
        if (co.resumeTime > now) continue;
        int nres = 0;
        int result = lua_resume(co.thread, nullptr, 0, &nres);
        if (result == LUA_YIELD) {
            double waitSec = lua_tonumber(co.thread, -1);
            lua_pop(co.thread, nres);
            co.resumeTime = now + waitSec;
        }
        else if (result == LUA_OK) {
            co.finished = true;
        }
        else {
            const char* error = lua_tostring(co.thread, -1);
            if (error) printf("%s\n", error);
            co.finished = true;
        }
    }
}

bool Lua::isGameReady() {
	uintptr_t entityList = ReadPointer(world.ents.clientBase, offsets::dwEntityList);
	if (entityList != 0) return true;
    
    return false;
}