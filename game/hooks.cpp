#include "hooks.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdio>

static lua_State* g_L = nullptr;

struct LuaHook {
    int ref;
};

using HookMap = std::unordered_map<std::string, std::unordered_map<std::string, LuaHook>>;
static HookMap hooks;

void hooks_set_lua_state(lua_State* L) {
    g_L = L;
}

// hook.Add(event, id, func)
int lua_hook_add(lua_State* L) {
    const char* event = luaL_checkstring(L, 1);
    const char* id = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    lua_pushvalue(L, 3);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto& eventHooks = hooks[event];
    auto it = eventHooks.find(id);
    if (it != eventHooks.end())
        luaL_unref(L, LUA_REGISTRYINDEX, it->second.ref);

    eventHooks[id] = { ref };
    return 0;
}

// hook.Remove(event, id)
int lua_hook_remove(lua_State* L) {
    const char* event = luaL_checkstring(L, 1);
    const char* id = luaL_checkstring(L, 2);

    auto eventIt = hooks.find(event);
    if (eventIt != hooks.end()) {
        auto& eventHooks = eventIt->second;
        auto idIt = eventHooks.find(id);
        if (idIt != eventHooks.end()) {
            luaL_unref(L, LUA_REGISTRYINDEX, idIt->second.ref);
            eventHooks.erase(idIt);
        }
    }
    return 0;
}

// hook.Run(event, ...args)
// Аргументы должны быть заранее на стеке g_L
int hook_Run(const char* event, int nargs) {
    if (!g_L) return 0;

    auto eventIt = hooks.find(event);
    if (eventIt == hooks.end())
        return 0;

    std::vector<int> argRefs;
    for (int i = 0; i < nargs; ++i) {
        lua_pushvalue(g_L, -(nargs));
        argRefs.push_back(luaL_ref(g_L, LUA_REGISTRYINDEX));
    }

    for (const auto& kv : eventIt->second) {
        lua_rawgeti(g_L, LUA_REGISTRYINDEX, kv.second.ref);
        for (int i = 0; i < nargs; ++i)
            lua_rawgeti(g_L, LUA_REGISTRYINDEX, argRefs[i]);

        if (lua_pcall(g_L, nargs, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(g_L, -1);
            printf("Lua hook error: %s\n", err ? err : "(unknown)");
            lua_pop(g_L, 1);
        }
    }
    for (int ref : argRefs)
        luaL_unref(g_L, LUA_REGISTRYINDEX, ref);

    return 0;
}

// Lua binding: hook.Run(event, ...)
int lua_hook_run(lua_State* L) {
    const char* event = luaL_checkstring(L, 1);
    int nargs = lua_gettop(L) - 1;
    for (int i = 0; i < nargs; ++i)
        lua_pushvalue(L, i + 2);
    int res = hook_Run(event, nargs);
    return res;
}

void register_lua_hooks(lua_State* L) {
    hooks_set_lua_state(L);

    lua_getglobal(L, "hook");
    if (lua_isnil(L, -1))
        lua_newtable(L);

    lua_pushcfunction(L, lua_hook_add);
    lua_setfield(L, -2, "Add");
    lua_pushcfunction(L, lua_hook_remove);
    lua_setfield(L, -2, "Remove");
    lua_pushcfunction(L, lua_hook_run);
    lua_setfield(L, -2, "Run");
    lua_setglobal(L, "hook");
}