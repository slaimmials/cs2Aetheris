#include "game.h"
#include "lua_additions.hpp"
#include <assert.h>
#include <cctype>
#include <algorithm>

int Player::health() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_iHealth);
}
int Player::team() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_iTeamNum);
}
bool Player::alive() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_lifeState)==256 ? true : false;
}
Vector3 Player::pos() const {
    Vector3 position = ReadVec3(pawnAddr + offsets::C_BasePlayerPawn::m_vOldOrigin);
    return position;
}
Vector3 Player::velocity() const {
    Vector3 velocity = ReadVec3(pawnAddr + offsets::C_BaseEntity::m_vecVelocity);
    return velocity;
}
bool Player::scoped() const {
    return ReadBool(pawnAddr + offsets::C_CSPlayerPawn::m_bIsScoped);
}

std::string Player::name() const {
    std::string name = ReadString(controllerAddr + offsets::CBasePlayerController::m_iszPlayerName, 16);
	if (name.empty()) return "";
    return name;
}

Player Ents::GetLocalPlayer() const {
    uintptr_t localPawn = ReadPointer(clientBase, offsets::dwLocalPlayerPawn);
    if (!localPawn) return Player();
    return Player{ localPawn, 0 };
}

std::vector<Player> Ents::GetPlayers(bool skipLocal) const {
    std::vector<Player> players;
    uintptr_t entityList = ReadPointer(clientBase, offsets::dwEntityList);
    uintptr_t localPawn = ReadPointer(clientBase, offsets::dwLocalPlayerPawn);

    uintptr_t listEntry = ReadPointer(entityList, 0x10);
    for (int i = 0; i < 64; ++i) {
        if (!listEntry) continue;
        uintptr_t controller = ReadPointer(listEntry, i * 0x78);
        if (controller == 0) continue;

        int pawnHandle = ReadInt(controller + offsets::C_CSPlayerController::m_hPlayerPawn);
        if (pawnHandle == 0) continue;

        uintptr_t listEntry2 = ReadPointer(entityList, 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
        if (listEntry2 == 0) continue;

        uintptr_t pawn = ReadPointer(listEntry2, 0x78 * (pawnHandle & 0x1FF));
        if (pawn == localPawn) continue;

        Player player{ pawn, controller };
        players.push_back(player);
    }
    return players;
}

uintptr_t GetClientBase() {
    HMODULE hClient = GetModuleHandleA("client.dll");
    return reinterpret_cast<uintptr_t>(hClient);
}

bool GameWorld::isLoaded() const {
    uintptr_t localPawn = ReadPointer(GetClientBase(), offsets::dwLocalPlayerPawn);
    //for other debug things, you know :)
	//std::string hexAddr = int_to_hex(localPawn);
    //std::transform(hexAddr.begin(), hexAddr.end(), hexAddr.begin(),
    //    [](unsigned char c) { return std::toupper(c); });
    //printf("0x%s", hexAddr.c_str());
	return !localPawn==0;
}

GameWorld world{ GetClientBase() };

static const char* LUA_PLAYER_MT = "MetaPlayer";

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
    if (strcmp(key, "name") == 0) {
        lua_pushstring(L, proxy->player.name().c_str());
        return 1;
    }
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
        printf("Local player is huinya\n");
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

    // color, vectors
    lua_pushcfunction(L, [](lua_State* L) {
        int r = luaL_checknumber(L, 1);
        int g = luaL_checknumber(L, 2);
        int b = luaL_checknumber(L, 3);
        int a = luaL_optnumber(L, 4, 255);
        lua_newtable(L);
        lua_pushnumber(L, r); lua_setfield(L, -2, "r");
        lua_pushnumber(L, g); lua_setfield(L, -2, "g");
        lua_pushnumber(L, b); lua_setfield(L, -2, "b");
        lua_pushnumber(L, a); lua_setfield(L, -2, "a");
        return 1;
        });
    lua_setglobal(L, "Color");

    lua_pushcfunction(L, [](lua_State* L) {
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        lua_newtable(L);
        lua_pushnumber(L, x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, y); lua_setfield(L, -2, "y");
        return 1;
        });
    lua_setglobal(L, "Vector2");

    lua_pushcfunction(L, [](lua_State* L) {
        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        float z = luaL_checknumber(L, 3);
        lua_newtable(L);
        lua_pushnumber(L, x); lua_setfield(L, -2, "x");
        lua_pushnumber(L, y); lua_setfield(L, -2, "y");
        lua_pushnumber(L, z); lua_setfield(L, -2, "z");
        return 1;
        });
    lua_setglobal(L, "Vector3");
}