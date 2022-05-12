#pragma once

#include "HWNS/Server.h"

class MyServer : public Server
{
public:
	virtual void OnConnect(HWNS::TCPConnection& newConnection) override;
	virtual void OnDisconnect(HWNS::TCPConnection& lostConnection, std::string reason) override;
	virtual bool ProcessPacket(std::shared_ptr<HWNS::Packet> packet) override;
};