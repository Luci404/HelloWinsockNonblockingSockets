#include "MyServer.h"

#include "HWNS/HWNS.h"

#include <iostream>

int main()
{
	if (HWNS::Network::Initialize())
	{
		MyServer server;
		if (server.Initialize(HWNS::IPEndpoint("127.0.0.1", 4791)))
		{
			while (true)
			{
				server.Frame();
			}
		}
	}

	HWNS::Network::Shutdown();
	system("pause");
	return 0;
}