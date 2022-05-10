#include "Server.h"

#include <iostream>

bool Server::Initialize(HWNS::IPEndpoint endpoint)
{
	if (HWNS::Network::Initialize())
	{
		std::cout << "Successfully initialized network" << std::endl;

		m_ListeningSocket = HWNS::Socket(endpoint.GetIPVersion());
		if (m_ListeningSocket.Create() == HWNS::PResult::P_Success)
		{
			std::cout << "Successfully created socket." << std::endl;
			if (m_ListeningSocket.Listen(endpoint) == HWNS::PResult::P_Success)
			{
				std::cout << "Socket successfully listening." << std::endl;
				return true;
			}
			else
			{
				std::cout << "Failed to listen." << std::endl;
			}
			m_ListeningSocket.Close();
		}
		else
		{
			std::cout << "Failed to create socket." << std::endl;
		}

	}

	return false;
}

void Server::Frame()
{
	// Use docs...
	WSAPOLLFD listeningSocketFD = {};
	listeningSocketFD.fd = m_ListeningSocket.GetHandle();
	listeningSocketFD.events = POLLRDNORM;
	listeningSocketFD.revents = 0;
	// TODO: Consider using 0 for timeout
	if (WSAPoll(&listeningSocketFD, 1, 1) > 0)
	{
		if (listeningSocketFD.revents & POLLRDNORM)
		{
			// Try to accept connection
			HWNS::Socket newConnection;
			if (m_ListeningSocket.Accept(newConnection) == HWNS::PResult::P_Success)
			{
				std::cout << "New connection accepted." << std::endl;
				newConnection.Close();
			}
			else
			{
				std::cerr << "Failed to accept new connection." << std::endl;
			}
		}
	}
}
