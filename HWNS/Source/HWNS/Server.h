#pragma once
#include "HWNS/HWNS.h"
#include "HWNS/TCPConnection.h"

class Server
{
public:
	bool Initialize(HWNS::IPEndpoint endpoint);
	void Frame();

protected:
	virtual void OnConnect(HWNS::TCPConnection& newConnection);
	virtual void OnDisconnect(HWNS::TCPConnection& lostConnection, std::string reason);
	void CloseConnection(int connectionIndex, std::string reason);
	virtual bool ProcessPacket(std::shared_ptr<HWNS::Packet> packet);

protected:
	HWNS::Socket m_ListeningSocket;
	std::vector<HWNS::TCPConnection> m_Connections;
	std::vector<WSAPOLLFD> m_MasterFD;
	std::vector<WSAPOLLFD> m_UseFD;
};