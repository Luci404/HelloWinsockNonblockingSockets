#include "Client.h"

#include <iostream>

namespace HWNS
{
	bool Client::Connect(HWNS::IPEndpoint endpoint)
	{
		m_IsConnected = false;

		std::cout << "Successfully initialized network" << std::endl;

		Socket socket = HWNS::Socket(endpoint.GetIPVersion());
		if (socket.Create() == HWNS::PResult::P_Success)
		{
			if (socket.SetBlocking(true) != HWNS::PResult::P_Success)
			{
				return false;
			}

			std::cout << "Successfully created socket." << std::endl;
			if (socket.Connect(endpoint) == HWNS::PResult::P_Success)
			{
				if (socket.SetBlocking(false) == HWNS::PResult::P_Success)
				{
					m_Connection = TCPConnection(socket, endpoint);
					m_MasterFD.fd = m_Connection.ConnectionSocket.GetHandle();
					m_MasterFD.events = POLLRDNORM | POLLWRNORM;
					m_MasterFD.revents = 0;
					m_IsConnected = true;
					OnConnect();
					return true;
				}
			}
			else
			{
			}
			socket.Close();
		}
		else
		{
			std::cout << "Failed to create socket." << std::endl;
		}
		OnConnectFail();
		return false;
	}

	bool Client::IsConnected()
	{
		return m_IsConnected;
	}

	bool Client::Frame()
	{
		m_UseFD = m_MasterFD;

		// TODO: Consider using 0 for timeout
		if (WSAPoll(&m_UseFD, 1, 1) > 0)
		{
			if (m_UseFD.revents & POLLERR) // If poll error occured on this socket
			{
				CloseConnection("POLLERR");
				return false;
			}

			if (m_UseFD.revents & POLLHUP) // If poll hangup occured on this socket
			{
				CloseConnection("POLLHUP");
				return false;
			}

			if (m_UseFD.revents & POLLNVAL) // If invalid socket
			{
				CloseConnection("POLLNVAL");
				return false;
			}

			if (m_UseFD.revents & POLLRDNORM) // If normal data can be read without blocking
			{
				// TMP
				int bytesReceived = 0;

				if (m_Connection.IncomingPacketManager.CurrentTask == HWNS::PacketManagerTask::ProcessPacketSize)
				{
					// ExtractionOffset handles edge case with partial reads.
					bytesReceived = recv(m_UseFD.fd, (char*)&m_Connection.IncomingPacketManager.CurrentPacketSize + m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset, sizeof(uint16_t) - m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset, 0);
				}
				else // HWNS::PacketTask::ProcessPacketContent
				{
					bytesReceived = recv(m_UseFD.fd, (char*)&m_Connection.Buffer + m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset, m_Connection.IncomingPacketManager.CurrentPacketSize - m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset, 0);
				}

				if (bytesReceived == 0) // m_Connection lost
				{
					CloseConnection("Recv=0");
					return false;
				}

				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection("Recv<0");
						return false;
					}
				}
				if (bytesReceived > 0)
				{
					m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset += bytesReceived;
					if (m_Connection.IncomingPacketManager.CurrentTask == HWNS::PacketManagerTask::ProcessPacketSize)
					{
						if (m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset == sizeof(uint16_t))
						{
							m_Connection.IncomingPacketManager.CurrentPacketSize = ntohs(m_Connection.IncomingPacketManager.CurrentPacketSize);
							if (m_Connection.IncomingPacketManager.CurrentPacketSize > HWNS::g_MaxPacketSize)
							{
								CloseConnection("Packet size too large.");
								return false;;
							}
							m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset = 0;
							m_Connection.IncomingPacketManager.CurrentTask = HWNS::PacketManagerTask::ProcessPacketContent;
						}
					}
					else //Processing packet contents
					{
						if (m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset == m_Connection.IncomingPacketManager.CurrentPacketSize)
						{
							std::shared_ptr<HWNS::Packet> packet = std::make_shared<HWNS::Packet>();
							packet->Buffer.resize(m_Connection.IncomingPacketManager.CurrentPacketSize);
							memcpy(&packet->Buffer[0], m_Connection.Buffer, m_Connection.IncomingPacketManager.CurrentPacketSize);

							m_Connection.IncomingPacketManager.Append(packet);

							m_Connection.IncomingPacketManager.CurrentPacketSize = 0;
							m_Connection.IncomingPacketManager.CurrentPacketExtractionOffset = 0;
							m_Connection.IncomingPacketManager.CurrentTask = HWNS::PacketManagerTask::ProcessPacketSize;
						}
					}
				}
			}

			if (m_UseFD.revents & POLLWRNORM)
			{
				HWNS::PacketManager& pm = m_Connection.OutgoingPacketManager;
				while (pm.HasPendingPackets())
				{
					if (pm.CurrentTask == PacketManagerTask::ProcessPacketSize) //Sending packet size
					{
						pm.CurrentPacketSize = pm.Retrieve()->Buffer.size();
						uint16_t bigEndianPacketSize = htons(pm.CurrentPacketSize);
						int bytesSent = send(m_UseFD.fd, (char*)(&bigEndianPacketSize) + pm.CurrentPacketExtractionOffset, sizeof(uint16_t) - pm.CurrentPacketExtractionOffset, 0);
						if (bytesSent > 0)
						{
							pm.CurrentPacketExtractionOffset += bytesSent;
						}

						if (pm.CurrentPacketExtractionOffset == sizeof(uint16_t)) //If full packet size was sent
						{
							pm.CurrentPacketExtractionOffset = 0;
							pm.CurrentTask = PacketManagerTask::ProcessPacketContent;
						}
						else //If full packet size was not sent, break out of the loop for sending outgoing packets for this connection - we'll have to try again next time we are able to write normal data without blocking
						{
							break;
						}
					}
					else //Sending packet contents
					{
						char* bufferPtr = (char*)&pm.Retrieve()->Buffer[0];
						int bytesSent = send(m_UseFD.fd, (char*)(bufferPtr)+pm.CurrentPacketExtractionOffset, pm.CurrentPacketSize - pm.CurrentPacketExtractionOffset, 0);
						if (bytesSent > 0)
						{
							pm.CurrentPacketExtractionOffset += bytesSent;
						}

						if (pm.CurrentPacketExtractionOffset == pm.CurrentPacketSize) //If full packet contents have been sent
						{
							pm.CurrentPacketExtractionOffset = 0;
							pm.CurrentTask = PacketManagerTask::ProcessPacketSize;
							pm.Pop(); //Remove packet from queue after finished processing
						}
						else
						{
							break; //Added after tutorial was made 2019-06-24
						}
					}
				}
			}
		}


		while (m_Connection.IncomingPacketManager.HasPendingPackets())
		{
			std::shared_ptr<HWNS::Packet> frontPacket = m_Connection.IncomingPacketManager.Retrieve();
			if (!ProcessPacket(frontPacket))
			{
				CloseConnection("Failed to process incoming packet.");
				return false;
			}
			m_Connection.IncomingPacketManager.Pop();
		}
	}

	bool Client::ProcessPacket(std::shared_ptr<HWNS::Packet> packet)
	{
		std::cout << "Packet received with size: " << packet->Buffer.size() << std::endl;
		return true;
	}

	void Client::OnConnect()
	{
		std::cout << "Successfully connected!" << std::endl;
	}

	void Client::OnConnectFail()
	{
		std::cout << "Failed to connect!" << std::endl;
	}

	void Client::OnDisonnect(std::string reason)
	{
		std::cout << "Lost Connection. Reason: " << reason << "." << std::endl;
	}

	void Client::CloseConnection(std::string reason)
	{
		OnDisonnect(reason);
		m_MasterFD.fd = 0;
		m_IsConnected = false;
		m_Connection.Close();
	}
}