#pragma once
#include <cstdint>
#include <vector>
#include <Windows.h>
#include "engine.h"
#include "lua.hpp"
#include "../memoryModule.h"

// Векторные структуры
struct Vector2 {
    float x = 0, y = 0;
    Vector2() = default;
    Vector2(float x, float y) : x(x), y(y) {}
};

struct Vector3 {
    float x = 0, y = 0, z = 0;
    Vector3() = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

// Цветовая структура
struct Color {
	uint8_t r = 255, g = 255, b = 255, a = 255;
	Color() = default;
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
};

// Игрок-прокси
struct Player {
    uintptr_t pawnAddr = 0;
    uintptr_t controllerAddr = 0;

    Player() = default;
    Player(uintptr_t pawn, uintptr_t ctrl) : pawnAddr(pawn), controllerAddr(ctrl) {}

    bool isValid() const { return pawnAddr != 0; }

    int health() const;
    int team() const;
    bool alive() const;
    Vector3 pos() const;
    Vector3 velocity() const;
    bool scoped() const;
    // Добавляй другие методы по желанию
};

// Коллекция сущностей
struct Ents {
    uintptr_t clientBase = 0;
    Ents(uintptr_t base) : clientBase(base) {}

    Player GetLocalPlayer() const;
    std::vector<Player> GetPlayers(bool skipLocal = true) const;
};

// Глобальный мир
struct GameWorld {
    Ents ents;
	bool isLoaded() const;
    GameWorld(uintptr_t base) : ents(base) {}
};

extern GameWorld world;

void RegisterWorldAPI(lua_State* L);