#pragma once
#include "HWNS/Socket.h"

namespace HWNS
{
	enum PacketTask
	{
		ProcessPacketSize,
		ProcessPacketContent
	};

	class TCPConnection
	{
	public:
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void Close();
		std::string ToString();
		
	public:
		PacketTask Task = PacketTask::ProcessPacketSize;
		int ExtractionOffset = 0;
		uint16_t PacketSize = 0;
		char Buffer[g_MaxPacketSize];

	private:
		Socket m_Socket;
		IPEndpoint m_Endpoint;
		std::string m_StringRepresentation;
	};
}