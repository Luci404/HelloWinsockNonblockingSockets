#include "HWNS/PacketManager.h"

namespace HWNS
{
	void PacketManager::Clear()
	{
		m_Packets = std::queue<std::shared_ptr<Packet>>{};//Clear out packet queue
	}

	bool PacketManager::HasPendingPackets()
	{
		return !m_Packets.empty(); //returns true if packets are pending
	}

	void PacketManager::Append(std::shared_ptr<Packet> packet)
	{
		m_Packets.push(std::move(packet)); //Add packet to queue
	}

	std::shared_ptr<Packet> PacketManager::Retrieve()
	{
		std::shared_ptr<Packet> packet = m_Packets.front(); //Get packet from front of queue
		return packet; //Return packet that was removed from the queue
	}

	void PacketManager::Pop()
	{
		m_Packets.pop(); //Remove packet from front of queue
	}
}