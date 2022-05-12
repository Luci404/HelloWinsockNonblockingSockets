#include "MyServer.h"

#include <iostream>

void MyServer::OnConnect(HWNS::TCPConnection& newConnection)
{
	std::cout << newConnection.ToString() << " - New connection accepted." << std::endl;

	// Send welcome message
	std::shared_ptr<HWNS::Packet> welcomeMessagePacket = std::make_shared<HWNS::Packet>(HWNS::PacketType::PT_ChatMessage);
	*welcomeMessagePacket << std::string("Welcome from server!");
	newConnection.OutgoingPacketManager.Append(welcomeMessagePacket);

	std::shared_ptr<HWNS::Packet> connectionLostPacket = std::make_shared<HWNS::Packet>(HWNS::PacketType::PT_ChatMessage);
	*connectionLostPacket << std::string("A user disconnected!");
	for (auto& connection : m_Connections)
	{
		if (&connection == &newConnection)
		{
			continue;
		}

		connection.OutgoingPacketManager.Append(connectionLostPacket);
	}
}

void MyServer::OnDisconnect(HWNS::TCPConnection& lostConnection, std::string reason)
{
	std::cout << "[" << reason << "]" << " Connection lost: " << lostConnection.ToString() << "." << std::endl;

	std::shared_ptr<HWNS::Packet> connectionLostPacket = std::make_shared<HWNS::Packet>(HWNS::PacketType::PT_ChatMessage);
	*connectionLostPacket << std::string("A user disconnected!");
	for (auto& connection : m_Connections)
	{
		if (&connection == &lostConnection)
		{
			continue;
		}

		connection.OutgoingPacketManager.Append(connectionLostPacket);
	}
}

bool MyServer::ProcessPacket(std::shared_ptr<HWNS::Packet> packet)
{
	switch (packet->GetPacketType())
	{
	case HWNS::PacketType::PT_ChatMessage:
	{
		std::string message;
		*packet >> message;
		std::cout << "Chat Message: " << message << std::endl;
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
	{
		std::cout << "Unrecognized packet type: " << packet->GetPacketType() << std::endl;
		break;
	}
	}

	return true;
}