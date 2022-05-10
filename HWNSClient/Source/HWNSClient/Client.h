#pragma once
#include "HWNS/HWNS.h"

class Client
{
public:
	bool Connect(HWNS::IPEndpoint endpoint);
	bool IsConnected();
	bool Frame();

private:
	bool m_IsConnected = false;
	HWNS::Socket m_Socket;
};