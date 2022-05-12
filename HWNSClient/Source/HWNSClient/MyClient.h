#pragma once

#include "HWNS/Client.h"

class MyClient : public HWNS::Client
{
protected:
	bool ProcessPacket(std::shared_ptr<HWNS::Packet> packet) override;
	void OnConnect() override;
	// virtual void OnConnectFail() override;
	// virtual void OnDisonnect(std::string reason) override;
};