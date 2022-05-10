#include "TCPConnection.h"

namespace HWNS
{

	TCPConnection::TCPConnection(Socket socket, IPEndpoint endpoint)
		: m_Socket(socket), m_Endpoint(endpoint)
	{
		m_StringRepresentation = "[" + m_Endpoint.GetIPString();
		m_StringRepresentation += ":" + std::to_string(m_Endpoint.GetPort()) + "]";
	}

	void TCPConnection::Close()
	{
	}

	std::string TCPConnection::ToString()
	{
		return m_StringRepresentation;
	}

}