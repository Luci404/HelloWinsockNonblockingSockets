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
	std::vector<WSAPOLLFD> useFD = m_MasterFD;

	// TODO: Consider using 0 for timeout
	if (WSAPoll(useFD.data(), useFD.size(), 1) > 0)
	{
#pragma region listener
		WSAPOLLFD& listeningSocketFD = useFD[0];
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

		for (int i = 1; i < useFD.size(); i++)
		{
			int connectionIndex = i - 1;
			HWNS::TCPConnection& connection = m_Connections[connectionIndex];

			if (useFD[i].revents & POLLERR) // If error occured on this socket
			{
				std::cout << "Poll error occured on: " << connection.ToString() << "." << std::endl;
				m_MasterFD.erase(m_MasterFD.begin() + i);
				useFD.erase(useFD.begin() + i);
				connection.Close();
				m_Connections.erase(m_Connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (useFD[i].revents & POLLHUP) // If poll hangup occured on this socket
			{
				std::cout << "Poll hangup occured on: " << connection.ToString() << "." << std::endl;
				m_MasterFD.erase(m_MasterFD.begin() + i);
				useFD.erase(useFD.begin() + i);
				connection.Close();
				m_Connections.erase(m_Connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (useFD[i].revents & POLLNVAL) // If invalid socket
			{
				std::cout << "Invalid socket used on: " << connection.ToString() << "." << std::endl;
				m_MasterFD.erase(m_MasterFD.begin() + i);
				useFD.erase(useFD.begin() + i);
				connection.Close();
				m_Connections.erase(m_Connections.begin() + connectionIndex);
				i -= 1;
				continue;
			}

			if (useFD[i].revents & POLLRDNORM) // If normal data can be read without blocking
			{
				// TMP
				char buffer[HWNS::g_MaxPacketSize];
				int bytesReceived = 0;
				bytesReceived = recv(useFD[i].fd, buffer, HWNS::g_MaxPacketSize, 0);

				if (bytesReceived == 0) // Connection lost
				{
					std::cout << "Connection lost: " << connection.ToString() << "." << std::endl;
					m_MasterFD.erase(m_MasterFD.begin() + i);
					useFD.erase(useFD.begin() + i);
					connection.Close();
					m_Connections.erase(m_Connections.begin() + connectionIndex);
					i -= 1;
					continue;
				}

				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						std::cout << "[Recv<0] Connection lost: " << connection.ToString() << "." << std::endl;
						m_MasterFD.erase(m_MasterFD.begin() + i);
						useFD.erase(useFD.begin() + i);
						connection.Close();
						m_Connections.erase(m_Connections.begin() + connectionIndex);
						i -= 1;
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
