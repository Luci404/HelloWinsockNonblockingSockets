#pragma once
#include "HWNS/HWNS.h"

class Server
{
public:
	bool Initialize(HWNS::IPEndpoint endpoint);
	void Frame();

private:
	HWNS::Socket m_ListeningSocket;
};