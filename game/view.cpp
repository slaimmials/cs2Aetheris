#include "view.h"
#include "game.h"
#include "lua_additions.hpp"
#include <imgui.h>
#include "../memoryModule.h"

Vector2 GetScreenSize() {
	ImVec2 size = ImGui::GetIO().DisplaySize;
	Vector2 screenSize(size.x, size.y);
	return screenSize;
}

Vector2 WorldToScreen(float* matrix, Vector3 pos, Vector2 screenSize) {
    float screenW = matrix[12] * pos.x + matrix[13] * pos.y + matrix[14] * pos.z + matrix[15];

    if (screenW > 0.001f) {
        float screenX = matrix[0] * pos.x + matrix[1] * pos.y + matrix[2] * pos.z + matrix[3];
        float screenY = matrix[4] * pos.x + matrix[5] * pos.y + matrix[6] * pos.z + matrix[7];

        float X = screenSize.x / 2.0f + screenSize.x / 2.0f * screenX / screenW;
        float Y = screenSize.y / 2.0f - screenSize.y / 2.0f * screenY / screenW;

        return Vector2{ X, Y };
    }
    else {
        return Vector2{ -99, -99 };
    }
}

Vector2 CalculateAngles(Vector3 from, Vector3 to)
{
	Vector2 angles;
	float deltaX = to.x - from.x;
	float deltaY = to.y - from.y;
	float deltaZ = to.z - from.z;

	angles.x = atan2(deltaY, sqrt(deltaX * deltaX + deltaZ * deltaZ)) * (180.0f / PI);
	angles.y = atan2(deltaX, deltaZ) * (180.0f / PI);

	return angles;
}

Vector3 Normalize(Vector3 v) {
	float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (length == 0) return Vector3(0, 0, 0);
	return Vector3(v.x / length, v.y / length, v.z / length);
}

void SetViewAngles(Vector3 v) {
	uintptr_t viewanglesAddr = world.ents.clientBase + offsets::dwViewAngles;
	if (viewanglesAddr) {
		WriteVec3(viewanglesAddr, v);
	}
}

static int l_view_WorldToScreen(lua_State* L) {
    Vector3 pos = lua_to_vector3(L, 1);

	uintptr_t matrixAddr = world.ents.clientBase + offsets::dwViewMatrix;
    float* matrix = ReadMatrix(matrixAddr);
    Vector2 screenSize = GetScreenSize();
    Vector2 screenPos = WorldToScreen(matrix, pos, screenSize);

    push_vec2(L, screenPos);
    return 1;
}

static int l_view_CalculateAngles(lua_State* L) {
    Vector3 from = lua_to_vector3(L, 1);
    Vector3 to = lua_to_vector3(L, 2);

    Vector2 angles = CalculateAngles(from, to);

    push_vec2(L, angles);
    return 1;
}

static int l_view_Normalize(lua_State* L) {
    Vector3 v = lua_to_vector3(L, 1);
    Vector3 norm = Normalize(v);
    push_vec3(L, norm);
    return 1;
}

static int l_view_SetViewAngles(lua_State* L) {
    Vector3 v = lua_to_vector3(L, 1);
	SetViewAngles(v);
    return 0;
}

void RegisterViewAPI(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, l_view_WorldToScreen);
    lua_setfield(L, -2, "WorldToScreen");

    lua_pushcfunction(L, l_view_CalculateAngles);
    lua_setfield(L, -2, "CalculateAngles");

    lua_pushcfunction(L, l_view_Normalize);
    lua_setfield(L, -2, "Normalize");

    lua_pushcfunction(L, l_view_SetViewAngles);
    lua_setfield(L, -2, "SetViewAngles");

    lua_setglobal(L, "view");
}