#include "Server.h"

#include "HWNS/HWNS.h"

#include <iostream>

/*bool ProcessPacket(HWNS::Packet& packet)
{
	switch (packet.GetPacketType())
	{
	case HWNS::PacketType::PT_ChatMessage:
	{
		std::string message;
		packet >> message;
		std::cout << "Chat Message: " << message << std::endl;
		break;
	}
	case HWNS::PacketType::PT_IntegerArray:
	{
		uint32_t arraySize = 0;
		packet >> arraySize;
		std::cout << "Array Size: " << arraySize << std::endl;
		for (uint32_t i = 0; i < arraySize; i++)
		{
			uint32_t element = 0;
			packet >> element;
			std::cout << "Element[" << i << "] - " << element << std::endl;
		}
		break;
	}
	default:
	{
		break;
	}
	}

	return true;
}*/

int main()
{
	Server server;
	if (server.Initialize(HWNS::IPEndpoint("127.0.0.1", 4791)))
	{
		while (true)
		{
			server.Frame();
		}
	}

	HWNS::Network::Shutdown();
	system("pause");
	return 0;
}