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
				int bytesReceived = 0;
				
				if (connection.Task == HWNS::PacketTask::ProcessPacketSize)
				{
					// ExtractionOffset handles edge case with partial reads.
					bytesReceived = recv(m_UseFD[i].fd, (char*)&connection.PacketSize + connection.ExtractionOffset, sizeof(uint16_t) - connection.ExtractionOffset, 0);
				}
				else // HWNS::PacketTask::ProcessPacketContent
				{
					bytesReceived = recv(m_UseFD[i].fd, (char*)&connection.Buffer + connection.ExtractionOffset, connection.PacketSize - connection.ExtractionOffset, 0);
				}
				
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
					connection.ExtractionOffset += bytesReceived;

					if (connection.Task == HWNS::PacketTask::ProcessPacketSize)
					{
						if (connection.ExtractionOffset == sizeof(uint16_t))
						{
							// Fully received the packet size.
							connection.PacketSize = ntohs(connection.PacketSize);
							if (connection.PacketSize > HWNS::g_MaxPacketSize)
							{
								CloseConnection(connectionIndex, "Packet size too large.");
								continue;
							}

							// We are now ready to start processing the packet content.
							connection.Task = HWNS::PacketTask::ProcessPacketContent;
							connection.ExtractionOffset = 0;
						}
					}
					else // HWNS::PacketTask::ProcessPacketContent
					{
						if (connection.ExtractionOffset == connection.PacketSize) // Check if we have the full packet
						{
							HWNS::Packet packet;
							packet.Buffer.resize(connection.PacketSize);
							memcpy(&packet.Buffer[0], &connection.Buffer[0], connection.PacketSize);
							
							if (!ProcessPacket(packet))
							{
								CloseConnection(connectionIndex, "Failed to process packet");
								continue;
							}

							// Reset
							connection.PacketSize = 0;
							connection.Task = HWNS::PacketTask::ProcessPacketSize;
							connection.ExtractionOffset = 0;
						}
					}
				}
			}
		}
	}
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
	HWNS::TCPConnection& connection = m_Connections[connectionIndex];
	std::cout << "[" << reason << "] Connection lost : " << connection.ToString() << "." << std::endl;
	m_MasterFD.erase(m_MasterFD.begin() + (connectionIndex + 1));
	m_UseFD.erase(m_UseFD.begin() + (connectionIndex + 1));
	connection.Close();
	m_Connections.erase(m_Connections.begin() + connectionIndex);
}

bool Server::ProcessPacket(HWNS::Packet& packet)
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
		std::cout << "Unrecognized packet type: " << packet.GetPacketType() << std::endl;
		break;
	}
	}

	return true;
}
