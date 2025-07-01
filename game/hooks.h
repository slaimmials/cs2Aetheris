#pragma once

#include <string>
#include <lua.hpp>

// Зарегистрировать lua_State* для хуков
void hooks_set_lua_state(lua_State* L);

// Добавить хук из Lua: hook.Add("event", "id", func)
int lua_hook_add(lua_State* L);
// Удалить хук: hook.Remove("event", "id")
int lua_hook_remove(lua_State* L);

// Запустить хук из C++: hook_Run("event", nargs = 0)
// Перед вызовом нужно положить нужные аргументы на стек main_L
int hook_Run(const char* event, int nargs = 0);

// Регистрация хуков в Lua
void register_lua_hooks(lua_State* L);