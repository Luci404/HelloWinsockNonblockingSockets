#include "IPEndpoint.h"

#include <assert.h>
#include <iostream>

namespace HWNS
{

	IPEndpoint::IPEndpoint(const char* ipAddress, uint16_t port)
		: m_IPVersion(IPVersion::Unknown), m_Port(port)
	{
		in_addr address;
		int result = inet_pton(AF_INET, ipAddress, &address);
		if (result == -1)
		{
			int error = WSAGetLastError();
			return;
		}

		if (result == 1)
		{
			if (address.S_un.S_addr != INADDR_NONE)
			{
				m_IPString = ipAddress;
				m_Hostname = ipAddress;

				m_IPBytes.resize(sizeof(ULONG));
				memcpy(&m_IPBytes[0], &address.S_un.S_addr, sizeof(ULONG));

				m_IPVersion = IPVersion::IPv4;
				return;
			}
		}

		// Attempt to resolve hostname to IPv4 address
		addrinfo hints = {}; // Hints will filter the results we get back for getaddrinfo
		hints.ai_family = AF_INET; // IPv4 addresses only
		addrinfo* hostInfo = nullptr;
		result = getaddrinfo(ipAddress, NULL, &hints, &hostInfo);
		if (result == 0)
		{
			sockaddr_in* hostAddr = reinterpret_cast<sockaddr_in*>(hostInfo->ai_addr);
		
			m_IPString.resize(16);
			inet_ntop(AF_INET, &hostAddr->sin_addr, &m_IPString[0], 16);

			m_Hostname = ipAddress;

			ULONG ipLong = hostAddr->sin_addr.S_un.S_addr;
			m_IPBytes.resize(sizeof(ULONG));
			memcpy(&m_IPBytes[0], &ipLong, sizeof(ULONG));

			m_IPVersion = IPVersion::IPv4;

			freeaddrinfo(hostInfo);
			return;
		}
	}

	IPEndpoint::IPEndpoint(sockaddr* addr)
	{
		assert(addr->sa_family == AF_INET);
		sockaddr_in* addrv4 = reinterpret_cast<sockaddr_in*>(addr);
		m_IPVersion = IPVersion::IPv4;
		m_Port = ntohs(addrv4->sin_port);
		m_IPBytes.resize(sizeof(ULONG));
		memcpy(&m_IPBytes[0], &addrv4->sin_addr, sizeof(ULONG));
		m_IPString.resize(16);
		inet_ntop(AF_INET, &addrv4->sin_addr, &m_IPString[0], 16);
		m_Hostname = m_IPString;
	}

	IPVersion IPEndpoint::GetIPVersion()
	{
		return m_IPVersion;
	}

	std::string IPEndpoint::GetHostname()
	{
		return m_Hostname;
	}

	std::string IPEndpoint::GetIPString()
	{
		return m_IPString;
	}

	std::vector<uint8_t> IPEndpoint::GetIPBytes()
	{
		return m_IPBytes;
	}

	uint16_t IPEndpoint::GetPort()
	{
		return m_Port;
	}
	sockaddr_in IPEndpoint::GetSockaddrIPv4()
	{
		assert(m_IPVersion == IPVersion::IPv4);

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		memcpy(&addr.sin_addr, &m_IPBytes[0], sizeof(ULONG));
		addr.sin_port = htons(m_Port);

		return addr;

	}
	void IPEndpoint::Print()
	{
		std::cout << "IP Version: " << ((m_IPVersion == IPVersion::IPv4) ? "IPv4" : "???") << std::endl;
		std::cout << "Hostname: " << m_Hostname << std::endl;
		std::cout << "IP: " << m_IPString<< std::endl;
		std::cout << "Port: " << m_Port << std::endl;
		std::cout << "IP Bytes..." << std::endl;
		for (uint8_t digit : m_IPBytes)
		{
			std::cout << (int)digit << std::endl;
		}
	}
}

