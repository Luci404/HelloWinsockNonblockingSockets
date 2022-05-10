#include "Socket.h"

#include <assert.h>
#include <iostream>

namespace HWNS
{

	Socket::Socket(IPVersion ipVersion, SocketHandle handle)
		: m_IPVersion(ipVersion), m_Handle(handle)
	{
		assert(m_IPVersion == IPVersion::IPv4);
	}

	PResult Socket::Create()
	{
		assert(m_IPVersion == IPVersion::IPv4);

		if (m_Handle != INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		m_Handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_Handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		if (SetBlocking(false) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		if (SetSocketOption(SocketOption::TCP_NoDelay, true) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Close()
	{
		if (m_Handle == INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		int result = closesocket(m_Handle);

		if (result != 0)
		{
			// An error occurred
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		m_Handle = INVALID_SOCKET;

		return PResult::P_Success;
	}

	PResult Socket::Bind(IPEndpoint endpoint)
	{
		sockaddr_in address = endpoint.GetSockaddrIPv4();
		
		int result = bind(m_Handle, (sockaddr*)&address, sizeof(sockaddr_in));
		if (result != 0)
		{
			// An error occurred
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (Bind(endpoint) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		int result = listen(m_Handle, backlog);
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::SetSocketOption(SocketOption option, bool enabled)
	{
		BOOL value = enabled ? TRUE : FALSE;

		int result = 0;
		switch (option)
		{
		case SocketOption::TCP_NoDelay:
		{
			result = setsockopt(m_Handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(BOOL));
			break;
		}
		default:
		{
			return PResult::P_GenericError;
		}
		}

		if (result != 0)
		{
			// An error occurred
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Accept(Socket& outSocket)
	{
		sockaddr_in addr = {};
		int len = sizeof(sockaddr_in);
		SocketHandle acceptedConnectionHandle = accept(m_Handle, (sockaddr*)(&addr), &len); // Blocking!
		if (acceptedConnectionHandle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		IPEndpoint newConnectionEndpoint((sockaddr*)&addr);
		std::cout << "Net socket accepted!" << std::endl;
		newConnectionEndpoint.Print();

		outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);

		return PResult::P_Success;
	}

	PResult Socket::Connect(IPEndpoint endpoint)
	{
		sockaddr_in addr = endpoint.GetSockaddrIPv4();
		int result = connect(m_Handle, (sockaddr*)(&addr), sizeof(sockaddr_in));
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Send(const void* data, uint32_t numberOfBytes, int& bytesSent)
	{
		bytesSent = send(m_Handle, (const char*)data, numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Receive(void* destination, int numberOfBytes, int& bytesReceived)
	{
		bytesReceived = recv(m_Handle, (char*)destination, numberOfBytes, NULL);

		if (bytesReceived == 0) // If connection was gracefully closed
		{
			return PResult::P_GenericError;
		}

		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::SendAll(const void* data, uint32_t numberOfBytes)
	{
		int totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesSent;
			int bytesSent = 0;
			char* bufferOffset = (char*)data + totalBytesSent;
			PResult result = Send(bufferOffset, bytesRemaining, bytesSent);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
			totalBytesSent += bytesSent;
		}

		return PResult::P_Success;
	}

	PResult Socket::ReceiveAll(void* destination, int numberOfBytes)
	{
		int totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesReceived;
			int bytesReceived = 0;
			char* bufferOffset = (char*)destination + totalBytesReceived;
			PResult result = Receive(bufferOffset, bytesRemaining, bytesReceived);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
			totalBytesReceived += bytesReceived;
		}

		return PResult::P_Success;
	}

	PResult Socket::SendPacket(Packet& packet)
	{
		uint16_t encodedPacketSize = htons(packet.Buffer.size());
		PResult result = SendAll(&encodedPacketSize, sizeof(uint16_t));
		if (result != PResult::P_Success)
			return PResult::P_GenericError;

		result = SendAll(packet.Buffer.data(), packet.Buffer.size());
		if (result != PResult::P_Success)
			return PResult::P_GenericError;

		return PResult::P_Success;
	}

	PResult Socket::ReceivePacket(Packet& packet)
	{
		packet.Clear();

		uint16_t encodedSize = 0;
		PResult result = ReceiveAll(&encodedSize, sizeof(uint16_t));
		if (result != PResult::P_Success)
			return PResult::P_GenericError;

		uint16_t bufferSize = ntohs(encodedSize);
		if (bufferSize > g_MaxPacketSize)
			return PResult::P_GenericError;

		packet.Buffer.resize(bufferSize);
		result = ReceiveAll(&packet.Buffer[0], bufferSize);
		if (result != PResult::P_Success)
			return PResult::P_GenericError;

		return PResult::P_Success;
	}

	PResult Socket::SetBlocking(bool isBlocking)
	{
		unsigned long blocking = 0;
		unsigned long nonBlocking = 1;
		int result = ioctlsocket(m_Handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		return PResult::P_Success;
	}

	SocketHandle Socket::GetHandle()
	{
		return m_Handle;
	}

	IPVersion Socket::GetIPVersion()
	{
		return m_IPVersion;
	}

}