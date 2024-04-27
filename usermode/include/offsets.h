#pragma once
#include <cstddef>

namespace offsets
{
    constexpr std::ptrdiff_t dwLocalPlayer = 0x1915C08;
    constexpr std::ptrdiff_t dwEntityList = 0x18C6268;
    constexpr std::ptrdiff_t dwViewMatrix = 0x19278A0;

    constexpr std::ptrdiff_t m_iHealth = 0x334;
    constexpr std::ptrdiff_t dwPlayerPawn = 0x7E4;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3CB;
    constexpr std::ptrdiff_t m_vec_Origin = 0x80;
}