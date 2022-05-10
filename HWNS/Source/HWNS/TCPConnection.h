#pragma once
#include "HWNS/Socket.h"
#include "HWNS/PacketManager.h"

namespace HWNS
{
	class TCPConnection
	{
	public:
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void Close();
		std::string ToString();
		
	public:
		PacketManager IncomingPacketManager;
		PacketManager OutgoingPacketManager;
		char Buffer[g_MaxPacketSize];

	private:
		Socket m_Socket;
		IPEndpoint m_Endpoint;
		std::string m_StringRepresentation;
	};
}