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
static lua_State* main_L = nullptr;

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
    // Создаем таблицу KEY
    lua_newtable(L);

    // Буквы
    for (char c = 'A'; c <= 'Z'; ++c) {
        char keyName[16];
        snprintf(keyName, sizeof(keyName), "KEY_%c", c);
        lua_pushinteger(L, c);
        lua_setfield(L, -2, keyName);
    }

    // Цифры
    for (char c = '0'; c <= '9'; ++c) {
        char keyName[16];
        snprintf(keyName, sizeof(keyName), "KEY_%c", c);
        lua_pushinteger(L, c);
        lua_setfield(L, -2, keyName);
    }

    // Функциональные клавиши F1-F24
    for (int i = 1; i <= 24; ++i) {
        char keyName[16];
        snprintf(keyName, sizeof(keyName), "KEY_F%d", i);
        lua_pushinteger(L, VK_F1 + (i - 1));
        lua_setfield(L, -2, keyName);
    }

    // Стрелки
    lua_pushinteger(L, VK_UP); lua_setfield(L, -2, "KEY_UP");
    lua_pushinteger(L, VK_DOWN); lua_setfield(L, -2, "KEY_DOWN");
    lua_pushinteger(L, VK_LEFT); lua_setfield(L, -2, "KEY_LEFT");
    lua_pushinteger(L, VK_RIGHT); lua_setfield(L, -2, "KEY_RIGHT");

    // Control, Shift, Alt, Win
    lua_pushinteger(L, VK_LSHIFT); lua_setfield(L, -2, "KEY_LSHIFT");
    lua_pushinteger(L, VK_RSHIFT); lua_setfield(L, -2, "KEY_RSHIFT");
    lua_pushinteger(L, VK_LCONTROL); lua_setfield(L, -2, "KEY_LCTRL");
    lua_pushinteger(L, VK_RCONTROL); lua_setfield(L, -2, "KEY_RCTRL");
    lua_pushinteger(L, VK_LMENU); lua_setfield(L, -2, "KEY_LALT");
    lua_pushinteger(L, VK_RMENU); lua_setfield(L, -2, "KEY_RALT");
    lua_pushinteger(L, VK_LWIN); lua_setfield(L, -2, "KEY_LWIN");
    lua_pushinteger(L, VK_RWIN); lua_setfield(L, -2, "KEY_RWIN");

    // Пробел, таб, энтер, esc, backspace, delete, insert, home, end, page up/down
    lua_pushinteger(L, VK_SPACE); lua_setfield(L, -2, "KEY_SPACE");
    lua_pushinteger(L, VK_TAB); lua_setfield(L, -2, "KEY_TAB");
    lua_pushinteger(L, VK_RETURN); lua_setfield(L, -2, "KEY_ENTER");
    lua_pushinteger(L, VK_ESCAPE); lua_setfield(L, -2, "KEY_ESCAPE");
    lua_pushinteger(L, VK_BACK); lua_setfield(L, -2, "KEY_BACKSPACE");
    lua_pushinteger(L, VK_DELETE); lua_setfield(L, -2, "KEY_DELETE");
    lua_pushinteger(L, VK_INSERT); lua_setfield(L, -2, "KEY_INSERT");
    lua_pushinteger(L, VK_HOME); lua_setfield(L, -2, "KEY_HOME");
    lua_pushinteger(L, VK_END); lua_setfield(L, -2, "KEY_END");
    lua_pushinteger(L, VK_PRIOR); lua_setfield(L, -2, "KEY_PAGEUP");
    lua_pushinteger(L, VK_NEXT); lua_setfield(L, -2, "KEY_PAGEDOWN");

    // Numpad
    lua_pushinteger(L, VK_NUMPAD0); lua_setfield(L, -2, "KEY_NUMPAD0");
    lua_pushinteger(L, VK_NUMPAD1); lua_setfield(L, -2, "KEY_NUMPAD1");
    lua_pushinteger(L, VK_NUMPAD2); lua_setfield(L, -2, "KEY_NUMPAD2");
    lua_pushinteger(L, VK_NUMPAD3); lua_setfield(L, -2, "KEY_NUMPAD3");
    lua_pushinteger(L, VK_NUMPAD4); lua_setfield(L, -2, "KEY_NUMPAD4");
    lua_pushinteger(L, VK_NUMPAD5); lua_setfield(L, -2, "KEY_NUMPAD5");
    lua_pushinteger(L, VK_NUMPAD6); lua_setfield(L, -2, "KEY_NUMPAD6");
    lua_pushinteger(L, VK_NUMPAD7); lua_setfield(L, -2, "KEY_NUMPAD7");
    lua_pushinteger(L, VK_NUMPAD8); lua_setfield(L, -2, "KEY_NUMPAD8");
    lua_pushinteger(L, VK_NUMPAD9); lua_setfield(L, -2, "KEY_NUMPAD9");
    lua_pushinteger(L, VK_DECIMAL); lua_setfield(L, -2, "KEY_NUMPAD_DECIMAL");
    lua_pushinteger(L, VK_DIVIDE); lua_setfield(L, -2, "KEY_NUMPAD_DIVIDE");
    lua_pushinteger(L, VK_MULTIPLY); lua_setfield(L, -2, "KEY_NUMPAD_MULTIPLY");
    lua_pushinteger(L, VK_SUBTRACT); lua_setfield(L, -2, "KEY_NUMPAD_SUBTRACT");
    lua_pushinteger(L, VK_ADD); lua_setfield(L, -2, "KEY_NUMPAD_ADD");
    lua_pushinteger(L, VK_SEPARATOR); lua_setfield(L, -2, "KEY_NUMPAD_SEPARATOR");

    // Клавиши Caps, Scroll, Num Lock
    lua_pushinteger(L, VK_CAPITAL); lua_setfield(L, -2, "KEY_CAPSLOCK");
    lua_pushinteger(L, VK_NUMLOCK); lua_setfield(L, -2, "KEY_NUMLOCK");
    lua_pushinteger(L, VK_SCROLL); lua_setfield(L, -2, "KEY_SCROLLLOCK");

    // PrintScreen, Pause, Menu, Apps
    lua_pushinteger(L, VK_SNAPSHOT); lua_setfield(L, -2, "KEY_PRINTSCREEN");
    lua_pushinteger(L, VK_PAUSE); lua_setfield(L, -2, "KEY_PAUSE");
    lua_pushinteger(L, VK_APPS); lua_setfield(L, -2, "KEY_APPS");
    lua_pushinteger(L, VK_MENU); lua_setfield(L, -2, "KEY_MENU");

    // OEM keys (точка, запятая, тире и др.)
    lua_pushinteger(L, VK_OEM_PERIOD); lua_setfield(L, -2, "KEY_PERIOD");
    lua_pushinteger(L, VK_OEM_COMMA); lua_setfield(L, -2, "KEY_COMMA");
    lua_pushinteger(L, VK_OEM_MINUS); lua_setfield(L, -2, "KEY_MINUS");
    lua_pushinteger(L, VK_OEM_PLUS); lua_setfield(L, -2, "KEY_PLUS");
    lua_pushinteger(L, VK_OEM_1); lua_setfield(L, -2, "KEY_SEMICOLON");
    lua_pushinteger(L, VK_OEM_2); lua_setfield(L, -2, "KEY_SLASH");
    lua_pushinteger(L, VK_OEM_3); lua_setfield(L, -2, "KEY_TILDE");
    lua_pushinteger(L, VK_OEM_4); lua_setfield(L, -2, "KEY_LBRACKET");
    lua_pushinteger(L, VK_OEM_5); lua_setfield(L, -2, "KEY_BACKSLASH");
    lua_pushinteger(L, VK_OEM_6); lua_setfield(L, -2, "KEY_RBRACKET");
    lua_pushinteger(L, VK_OEM_7); lua_setfield(L, -2, "KEY_APOSTROPHE");

    lua_pushinteger(L, VK_LBUTTON); lua_setfield(L, -2, "MB_LBUTTON");
    lua_pushinteger(L, VK_RBUTTON); lua_setfield(L, -2, "MB_RBUTTON");
    lua_pushinteger(L, VK_MBUTTON); lua_setfield(L, -2, "MB_MBUTTON");
    lua_pushinteger(L, VK_XBUTTON1); lua_setfield(L, -2, "MB_XBUTTON1");
    lua_pushinteger(L, VK_XBUTTON2); lua_setfield(L, -2, "MB_XBUTTON2");

    lua_setglobal(L, "keys");
}

// --- Register all API functions ---

void RegisterFunctions(lua_State* L) {
    lua_register(L, "wait", lua_wait);
    lua_register(L, "MessageBox", lua_MessageBox);
    RegisterInputTable(L);
    RegisterKeyConstants(L);
    RegisterWorldAPI(L);
    RegisterDrawingAPI(L);
    RegisterViewAPI(L);
    register_lua_hooks(L);
}

// --- Coroutine launcher ---

void Lua::Init() {
    main_L = luaL_newstate();
    luaL_openlibs(main_L);
    RegisterFunctions(main_L);
    hooks_set_lua_state(main_L);
}

void Lua::Execute(const char* code) {
    if (!main_L) Lua::Init();
    lua_State* thread = lua_newthread(main_L);
    if (luaL_loadstring(thread, code) == LUA_OK) {
        g_coroutines.push_back({ thread, 0.0, false });
    }
    else {
        const char* error = lua_tostring(thread, -1);
        if (error) printf("%s\n", error);
    }
}

void Lua::UpdateLuaCoroutines() {
    double now = CurrentTime();
    for (auto& co : g_coroutines) {
        if (co.finished) continue;
        if (co.resumeTime > now) {
            continue;
        }
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
    return world.isLoaded();
}

lua_State* GetMainLuaState() {
    return main_L;
}