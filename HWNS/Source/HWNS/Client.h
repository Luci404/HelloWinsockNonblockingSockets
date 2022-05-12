#pragma once
#include "HWNS/HWNS.h"

namespace HWNS
{
	class Client
	{
	public:
		bool Connect(IPEndpoint endpoint);
		bool IsConnected();
		bool Frame();

	protected:
		virtual bool ProcessPacket(std::shared_ptr<HWNS::Packet> packet);
		virtual void OnConnect();
		virtual void OnConnectFail();
		virtual void OnDisonnect(std::string reason);
		void CloseConnection(std::string reason);

	protected:
		TCPConnection m_Connection;

	private:
		bool m_IsConnected = false;
		WSAPOLLFD m_MasterFD;
		WSAPOLLFD m_UseFD;
	};
}
