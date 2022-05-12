#include "MyClient.h"

#include "HWNS/HWNS.h"

#include <iostream>

int main()
{
	if (HWNS::Network::Initialize())
	{
		MyClient client;
		if (client.Connect(HWNS::IPEndpoint("127.0.0.1", 4791)))
		{
			while (client.IsConnected())
			{
				client.Frame();
			}
		}
	}

	HWNS::Network::Shutdown();
	system("pause");
	return 0;
}