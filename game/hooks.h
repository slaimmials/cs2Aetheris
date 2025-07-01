#pragma once

#include <string>
#include <lua.hpp>

// ���������������� lua_State* ��� �����
void hooks_set_lua_state(lua_State* L);

// �������� ��� �� Lua: hook.Add("event", "id", func)
int lua_hook_add(lua_State* L);
// ������� ���: hook.Remove("event", "id")
int lua_hook_remove(lua_State* L);

// ��������� ��� �� C++: hook_Run("event", nargs = 0)
// ����� ������� ����� �������� ������ ��������� �� ���� main_L
int hook_Run(const char* event, int nargs = 0);

// ����������� ����� � Lua
void register_lua_hooks(lua_State* L);