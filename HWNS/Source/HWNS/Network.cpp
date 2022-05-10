#include "Network.h"

#include <iostream>

bool HWNS::Network::Initialize()
{
    WSADATA wsadata;

    int wsastartupResult = WSAStartup(MAKEWORD(2, 2), &wsadata);

    if (wsastartupResult != 0)
    {
        std::cout << "Error: Failed to initialize winsock API." << std::endl;
        return false;
    }

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
    {
        std::cout << "Error: Failed to find a usable version of the winsock API." << std::endl;
        return false;
    }

	return true;
}

void HWNS::Network::Shutdown()
{
	WSACleanup();
}
