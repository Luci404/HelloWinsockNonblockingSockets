#pragma once

#include "HWNS/IPVersion.h"

#include <string>
#include <vector>
#include <WS2tcpip.h>

namespace HWNS
{
	class IPEndpoint
	{
	public:
		IPEndpoint(const char* ipAddress, uint16_t port);
		IPEndpoint(sockaddr* addr);

		IPVersion GetIPVersion();
		std::string GetHostname();
		std::string GetIPString();
		std::vector<uint8_t> GetIPBytes();
		uint16_t GetPort();
		sockaddr_in GetSockaddrIPv4();

		void Print();

	private:
		IPVersion m_IPVersion;
		std::string m_Hostname;
		std::string m_IPString;
		std::vector<uint8_t> m_IPBytes;
		uint16_t m_Port;
	};
}