#pragma once
#include "HWNS/HWNS.h"

class Server
{
public:
	bool Initialize(HWNS::IPEndpoint endpoint);
	void Frame();

private:
	void CloseConnection(int connectionIndex, std::string reason);
	bool ProcessPacket(std::shared_ptr<HWNS::Packet> packet);

private:
	HWNS::Socket m_ListeningSocket;
	std::vector<HWNS::TCPConnection> m_Connections;
	std::vector<WSAPOLLFD> m_MasterFD;
	std::vector<WSAPOLLFD> m_UseFD;

};