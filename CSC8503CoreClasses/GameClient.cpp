#include "GameClient.h"

#include "NetworkObject.h"
#include "../CSC8503/NetworkPlayer.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
	mTimerSinceLastPacket = 0.0f;
	mPeerId = -1;
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

int GameClient::GetPeerID() const {
	return mPeerId;
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | (a);

	mNetPeer = enet_host_connect(netHandle, &address, 2, 0);

	// returm false if net peer is null
	return mNetPeer != nullptr;
}

bool GameClient::UpdateClient() {
	// if there is no net handle we cannot handle packets
	if (netHandle == nullptr)
		return false;

	mTimerSinceLastPacket++;

	// handle incoming packets
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			//erendgrmnc: I remember +1 is needed because when counting server as a player, outgoing peer Id is not increasing.
			mPeerId = mNetPeer->outgoingPeerID + 1;
			std::cout << "Connected to server!" << std::endl;
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			//std::cout << "Client Packet recieved..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
			mTimerSinceLastPacket = 0.0f;
		}
		// once packet data is handled we can destroy packet and go to next
		enet_packet_destroy(event.packet);
	}
	// return false if client is no longer receiving packets
	if (mTimerSinceLastPacket > 20.0f) {
		return false;
	}
	return true;
}

void GameClient::WriteAndSendClientInputPacket(int lastId, const PlayerInputs& playerInputs){
	ClientPlayerInputPacket packet(lastId, playerInputs);
	this->SendPacket(packet);
}

void GameClient::SendPacket(GamePacket&  payload) {
	// defines packet to send and sends packet
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(mNetPeer, 0, dataPacket);
}
