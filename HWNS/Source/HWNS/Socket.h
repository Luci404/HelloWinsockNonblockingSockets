#pragma once

#include "HWNS/SocketHandle.h"
#include "HWNS/PResult.h"
#include "HWNS/IPVersion.h"
#include "HWNS/IPEndpoint.h"
#include "HWNS/Constants.h"
#include "HWNS/Packet.h"

namespace HWNS
{
	enum class SocketOption
	{
		TCP_NoDelay, // TRUE = Disable Negle's Algorithn
	};

	class Socket
	{
	public: 
		Socket(IPVersion ipVersion = IPVersion::IPv4, SocketHandle handle = INVALID_SOCKET);

		PResult Create();
		PResult Close();
		PResult Bind(IPEndpoint endpoint);
		PResult Listen(IPEndpoint endpoint, int backlog = 5);
		PResult SetSocketOption(SocketOption option, bool enabled);
		PResult Accept(Socket& outSocket, IPEndpoint* endpoint);
		PResult Connect(IPEndpoint endpoint);
		PResult Send(const void* data, uint32_t numberOfBytes, int& bytesSent);
		PResult Receive(void* destination, int numberOfBytes, int& bytesReceived);
		PResult SendAll(const void* data, uint32_t numberOfBytes);
		PResult ReceiveAll(void* destination, int numberOfBytes);
		PResult SendPacket(Packet& packet);
		PResult ReceivePacket(Packet& packet);
		PResult SetBlocking(bool isBlocking);

		SocketHandle GetHandle();
		IPVersion GetIPVersion();


	private:
		SocketHandle m_Handle;
		IPVersion m_IPVersion;
	};


}