#pragma once
#include <string>

namespace HWNS
{
    class PacketException
    {
    public:
        PacketException(std::string exception)
            : m_Exception(exception)
        {
        }

        const char* What()
        {
            return m_Exception.c_str();
        }

        std::string ToString()
        {
            return m_Exception;
        }

    private:
        std::string m_Exception;
    };
}