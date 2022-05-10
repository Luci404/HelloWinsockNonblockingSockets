#pragma once
#include "HWNS/Socket.h"

namespace HWNS
{
	class TCPConnection
	{
	public:
		TCPConnection(Socket socket, IPEndpoint endpoint);
		void Close();
		std::string ToString();
		

	private:
		Socket m_Socket;
		IPEndpoint m_Endpoint;
		std::string m_StringRepresentation;
	};
}