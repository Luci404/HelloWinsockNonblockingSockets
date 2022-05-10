#pragma once
#include <queue>
#include <memory>
#include "HWNS/Packet.h"

namespace HWNS
{
	enum PacketManagerTask
	{
		ProcessPacketSize,
		ProcessPacketContent
	};

	class PacketManager
	{
	public:
		void Clear();
		bool HasPendingPackets();
		void Append(std::shared_ptr<Packet> packet);
		std::shared_ptr<Packet> Retrieve();
		void Pop();

	public:
		int CurrentPacketExtractionOffset = 0;
		uint16_t CurrentPacketSize = 0;
		PacketManagerTask CurrentTask = PacketManagerTask::ProcessPacketSize;

	private:
		std::queue<std::shared_ptr<Packet>> m_Packets;
	};
}