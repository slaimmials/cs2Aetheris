#pragma once
#include "game.h"
#include <math.h>

Vector2 WorldToScreen(float* matrix, Vector3 pos, Vector2 screenSize);
Vector2 CalculateAngles(Vector3 from, Vector3 to);
Vector3 Normalize(Vector3 v);

void RegisterViewAPI(lua_State* L);