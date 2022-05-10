#include "Server.h"

#include <iostream>

bool Server::Initialize(HWNS::IPEndpoint endpoint)
{
	m_MasterFD.clear();
	m_Connections.clear();

	if (HWNS::Network::Initialize())
	{
		std::cout << "Successfully initialized network" << std::endl;

		m_ListeningSocket = HWNS::Socket(endpoint.GetIPVersion());
		if (m_ListeningSocket.Create() == HWNS::PResult::P_Success)
		{
			std::cout << "Successfully created socket." << std::endl;
			if (m_ListeningSocket.Listen(endpoint) == HWNS::PResult::P_Success)
			{
				// Use docs...
				WSAPOLLFD listeningSocketFD = {};
				listeningSocketFD.fd = m_ListeningSocket.GetHandle();
				listeningSocketFD.events = POLLRDNORM;
				listeningSocketFD.revents = 0;
				m_MasterFD.push_back(listeningSocketFD);

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
	m_UseFD = m_MasterFD;

	// TODO: Consider using 0 for timeout
	if (WSAPoll(m_UseFD.data(), m_UseFD.size(), 1) > 0)
	{
#pragma region listener
		WSAPOLLFD& listeningSocketFD = m_UseFD[0];
		if (listeningSocketFD.revents & POLLRDNORM)
		{
			// Try to accept connection
			HWNS::Socket newConnectionSocket;
			HWNS::IPEndpoint newConnectionEndpoint;
			if (m_ListeningSocket.Accept(newConnectionSocket, &newConnectionEndpoint) == HWNS::PResult::P_Success)
			{
				m_Connections.emplace_back(HWNS::TCPConnection(newConnectionSocket, newConnectionEndpoint));
				HWNS::TCPConnection& acceptedConnection = m_Connections[m_Connections.size() - 1];
				std::cout << acceptedConnection.ToString() << " - New connection accepted." << std::endl;

				WSAPOLLFD newConnectionFD = {};
				newConnectionFD.fd = newConnectionSocket.GetHandle();
				newConnectionFD.events = POLLRDNORM;
				newConnectionFD.revents = 0;
				m_MasterFD.push_back(newConnectionFD);
			}
			else
			{
				std::cerr << "Failed to accept new connection." << std::endl;
			}
		}
#pragma endregion Code specific to the listening socket

		for (int i = m_UseFD.size() - 1; i >= 1; i--)
		{
			int connectionIndex = i - 1;
			HWNS::TCPConnection& connection = m_Connections[connectionIndex];

			if (m_UseFD[i].revents & POLLERR) // If poll error occured on this socket
			{
				CloseConnection(connectionIndex, "POLLERR");
				continue;
			}

			if (m_UseFD[i].revents & POLLHUP) // If poll hangup occured on this socket
			{
				CloseConnection(connectionIndex, "POLLHUP");
				continue;
			}

			if (m_UseFD[i].revents & POLLNVAL) // If invalid socket
			{
				CloseConnection(connectionIndex, "POLLNVAL");
				continue;
			}

			if (m_UseFD[i].revents & POLLRDNORM) // If normal data can be read without blocking
			{
				// TMP
				char buffer[HWNS::g_MaxPacketSize];
				int bytesReceived = 0;
				bytesReceived = recv(m_UseFD[i].fd, buffer, HWNS::g_MaxPacketSize, 0);

				if (bytesReceived == 0) // Connection lost
				{
					CloseConnection(connectionIndex, "Recv=0");
					continue;
				}

				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(connectionIndex, "Recv<0");
						continue;
					}
				}

				if (bytesReceived > 0)
				{
					std::cout << connection.ToString() << " - Message Size: " << bytesReceived << std::endl;
				}
			}
		}
	}
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
	HWNS::TCPConnection& connection = m_Connections[connectionIndex];
	std::cout << "[" << reason << "] Connection lost : " << connection.ToString() << "." << std::endl;
	m_MasterFD.erase(m_MasterFD.begin() + (connectionIndex+1));
	m_UseFD.erase(m_UseFD.begin() + (connectionIndex + 1));
	connection.Close();
	m_Connections.erase(m_Connections.begin() + connectionIndex);
}
