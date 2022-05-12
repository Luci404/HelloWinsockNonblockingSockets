#pragma once
#include "HWNS/Socket.h"
#include "HWNS/PacketManager.h"

namespace HWNS
{
	class TCPConnection
	{
	public:
		TCPConnection() : ConnectionSocket(Socket()) {};
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void Close();
		std::string ToString();
		
	public:
		PacketManager IncomingPacketManager;
		PacketManager OutgoingPacketManager;
		char Buffer[g_MaxPacketSize];
		Socket ConnectionSocket;

	private:
		IPEndpoint m_Endpoint;
		std::string m_StringRepresentation;
	};
}