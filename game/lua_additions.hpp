#pragma once
#include "game.h"

inline void push_vec3(lua_State* L, const Vector3& v) {
	lua_newtable(L);
	lua_pushnumber(L, v.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, v.y); lua_setfield(L, -2, "y");
	lua_pushnumber(L, v.z); lua_setfield(L, -2, "z");
}

inline void push_vec2(lua_State* L, const Vector2& v) {
	lua_newtable(L);
	lua_pushnumber(L, v.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, v.y); lua_setfield(L, -2, "y");
}

inline void push_color(lua_State* L, const Color& c) {
	lua_newtable(L);
	lua_pushnumber(L, c.r); lua_setfield(L, -2, "r");
	lua_pushnumber(L, c.g); lua_setfield(L, -2, "g");
	lua_pushnumber(L, c.b); lua_setfield(L, -2, "b");
	lua_pushnumber(L, c.a); lua_setfield(L, -2, "a");
}

inline Color lua_to_color(lua_State* L, int index) {
	Color c;
	lua_getfield(L, index, "r"); c.r = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "g"); c.g = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "b"); c.b = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "a"); c.a = luaL_optnumber(L, -1, 255); lua_pop(L, 1);
	return c;
}

inline Vector2 lua_to_vector2(lua_State* L, int index) {
	Vector2 v;
	lua_getfield(L, index, "x"); v.x = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "y"); v.y = luaL_checknumber(L, -1); lua_pop(L, 1);
	return v;
}

inline Vector3 lua_to_vector3(lua_State* L, int index) {
	Vector3 v;
	lua_getfield(L, index, "x"); v.x = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "y"); v.y = luaL_checknumber(L, -1); lua_pop(L, 1);
	lua_getfield(L, index, "z"); v.z = luaL_checknumber(L, -1); lua_pop(L, 1);
	return v;
}