#include "game.h"

int Player::health() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_iHealth);
}
int Player::team() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_iTeamNum);
}
int Player::lifeState() const {
    return ReadInt(pawnAddr + offsets::C_BaseEntity::m_lifeState);
}
Vector3 Player::pos() const {
    return {
        ReadFloat(pawnAddr + offsets::C_BasePlayerPawn::m_vOldOrigin),
        ReadFloat(pawnAddr + offsets::C_BasePlayerPawn::m_vOldOrigin + 0x4),
        ReadFloat(pawnAddr + offsets::C_BasePlayerPawn::m_vOldOrigin + 0x8)
    };
}
Vector3 Player::velocity() const {
    return {
        ReadFloat(pawnAddr + offsets::C_BaseEntity::m_vecVelocity),
        ReadFloat(pawnAddr + offsets::C_BaseEntity::m_vecVelocity + 0x4),
        ReadFloat(pawnAddr + offsets::C_BaseEntity::m_vecVelocity + 0x8)
    };
}
bool Player::scoped() const {
    return ReadBool(pawnAddr + offsets::C_CSPlayerPawn::m_bIsScoped);
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
        if (!controller) continue;

        int pawnHandle = ReadInt(controller + offsets::C_CSPlayerController::m_hPlayerPawn);
        if (!pawnHandle) continue;

        uintptr_t listEntry2 = ReadPointer(entityList, 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
        uintptr_t pawn = ReadPointer(listEntry2, 0x78 * (pawnHandle & 0x1FF));
        if (!pawn) continue;
        if (skipLocal && pawn == localPawn) continue;

        Player player{ pawn, controller };
        if (player.lifeState() != 0) continue; // 0 = alive
        players.push_back(player);
    }
    return players;
}

uintptr_t GetClientBase() {
    HMODULE hClient = GetModuleHandleA("client.dll");
    return reinterpret_cast<uintptr_t>(hClient);
}

GameWorld world{ GetClientBase() };