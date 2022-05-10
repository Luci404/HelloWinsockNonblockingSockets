#include "Client.h"

#include <iostream>

bool Client::Connect(HWNS::IPEndpoint endpoint)
{
	m_IsConnected = false;

	if (HWNS::Network::Initialize())
	{
		std::cout << "Successfully initialized network" << std::endl;

		m_Socket = HWNS::Socket(endpoint.GetIPVersion());
		if (m_Socket.Create() == HWNS::PResult::P_Success)
		{
			// TMP
			if (m_Socket.SetBlocking(true) != HWNS::PResult::P_Success)
				return false;

			std::cout << "Successfully created socket." << std::endl;
			if (m_Socket.Connect(endpoint) == HWNS::PResult::P_Success)
			{
				m_IsConnected = true;
				std::cout << "Successfully connected to server." << std::endl;
				return true;
			}
			else
			{
				std::cerr << "Failed to connect to server." << std::endl;

			}

			m_Socket.Close();
		}
		else
		{
			std::cout << "Failed to create socket." << std::endl;
		}
	}

	return false;
}

bool Client::IsConnected()
{
	return m_IsConnected;
}

bool Client::Frame()
{
	HWNS::Packet stringPacket(HWNS::PacketType::PT_ChatMessage);
	stringPacket << std::string("This is my string packet!");

	HWNS::Packet integerArrayPacket(HWNS::PacketType::PT_IntegerArray);
	const uint32_t arraySize = 6;
	const uint32_t integerArray[arraySize] = { 1, 2, 3, 4, 5, 6 };
	integerArrayPacket << arraySize;
	for (uint32_t i : integerArray) integerArrayPacket << i;

	HWNS::PResult result;

	if (rand() % 2 == 0)
	{
		result = m_Socket.SendPacket(stringPacket);
	}
	else
	{
		result = m_Socket.SendPacket(integerArrayPacket);
	}

	if (result != HWNS::PResult::P_Success)
	{
		m_IsConnected = false;
		return false;
	}

	std::cout << "Attempting to send chunk of data..." << std::endl;
	Sleep(500);

	return true;
}
