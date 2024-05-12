#pragma once
#include <cstddef>

namespace globals
{
    inline bool skeletonEsp = false;
    inline float skeletonColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    inline bool healthEsp = false;
    inline float healthColor[] = { 1.0f, 0.f, 0.f, 1.0f };
}

namespace offsets
{
    constexpr std::ptrdiff_t dwLocalPlayer = 0x19176A8;
    constexpr std::ptrdiff_t dwEntityList = 0x18C7F98;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1929430;

    constexpr std::ptrdiff_t m_iHealth = 0x334;
    constexpr std::ptrdiff_t dwPlayerPawn = 0x7E4;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3CB;
    constexpr std::ptrdiff_t m_vec_Origin = 0x80;
}