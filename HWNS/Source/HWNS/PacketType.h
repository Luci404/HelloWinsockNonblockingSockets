#pragma once
#include <stdint.h>

namespace HWNS
{
    enum PacketType : uint16_t
    {
        PT_Invalid,
        PT_ChatMessage,
        PT_IntegerArray
    };
}