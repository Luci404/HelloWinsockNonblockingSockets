#include "MyClient.h"

#include <iostream>

bool MyClient::ProcessPacket(std::shared_ptr<HWNS::Packet> packet)
{
	switch (packet->GetPacketType())
	{
	case HWNS::PacketType::PT_ChatMessage:
	{
		std::string chatmessage;
		*packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;
	}
	case HWNS::PacketType::PT_IntegerArray:
	{
		uint32_t arraySize = 0;
		*packet >> arraySize;
		std::cout << "Array Size: " << arraySize << std::endl;
		for (uint32_t i = 0; i < arraySize; i++)
		{
			uint32_t element = 0;
			*packet >> element;
			std::cout << "Element[" << i << "] - " << element << std::endl;
		}
		break;
	}
	default:
		std::cout << "Unrecognized packet type: " << packet->GetPacketType() << std::endl;
		return false;
	}
}

void MyClient::OnConnect()
{
	std::cout << "Successfully connected to the server!" << std::endl;

	std::shared_ptr<HWNS::Packet> helloPacket = std::make_shared<HWNS::Packet>(HWNS::PacketType::PT_ChatMessage);
	*helloPacket << std::string("Welcome from the client!");
	m_Connection.OutgoingPacketManager.Append(helloPacket);
}