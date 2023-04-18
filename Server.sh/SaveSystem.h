#pragma once
#include <iostream>
int if_freeze = 0;
int if_freeze_to_exit = 0;
string theCCH = "";
string theCCH3 = "";
string theCASE4 = "";
string packet_logs = "";
void crashLog() {
	ofstream a("database/server_logs/packet.txt", ios::app);
	a << "last packet sent: " + theCCH + "\nlast world: " + theCCH3 + "\nlast case 4: " + theCASE4 << endl;
	a.close();
}
struct test {
	string name = "";
	int totals = 0;
};
bool issaving = false;

static void Server_Saving() {
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or not pInfo(currentPeer)->isIn) continue;
		save_player(pInfo(currentPeer), false);
	}
	lastServerSave = Algorithm::get_time();
	{
		ifstream ifs("database/ipban.json");
		if (ifs.is_open()) {
			json j;
			ifs >> j;
			vector<string> blacklistedRID = j["ip"];
			ipbans = blacklistedRID;
		}
		ifs.close();
	}
	{
		ifstream ifs("database/ridban.json");
		if (ifs.is_open()) {
			json j;
			ifs >> j;
			vector<string> blacklistedRID = j["rid"];
			ridbans = blacklistedRID;
		}
		ifs.close();
	}
	save_guilds();
}

void save_w(int e) {
	crashLog();
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or not pInfo(currentPeer)->isIn) continue;
		save_player(pInfo(currentPeer), false);
	}
	for (int i = 0; i < static_cast<int>(worlds.size()); i++) {
		string world_name = worlds[i].name;
		save_world(world_name, false);
	}
	save_guilds();
	hoshi_warn("ENet server crashed.");
	hoshi_db_logs("Saved all database");
	system("pause");
	exit(1);
}

static void Server_Reset() {
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or not pInfo(currentPeer)->isIn) continue;
		uint32_t guild_id = pInfo(currentPeer)->guild_id;
		vector<Guild>::iterator p = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
		if (p != guilds.end()) {
			Guild* guild_information = &guilds[p - guilds.begin()];
			uint32_t my_rank = 0;
			string name = "";
			for (GuildMember member_search : guild_information->guild_members) {
				if (member_search.member_name == pInfo(currentPeer)->tankIDName) {
					my_rank = member_search.role_id;
					name = member_search.member_name;
					break;
				}
			}
			if (my_rank < 3 && name != pInfo(currentPeer)->tankIDName && pInfo(currentPeer)->guild_id == guild_information->guild_id) {
				pInfo(currentPeer)->guild_id = 0;
			}
			else if (my_rank < 3 && name == pInfo(currentPeer)->tankIDName && pInfo(currentPeer)->guild_id != guild_information->guild_id) {
				pInfo(currentPeer)->guild_id = guild_information->guild_id;
			}
		}
		for (int i_ = 0, remove = 0; i_ < pInfo(currentPeer)->inv.size(); i_++) {
			if (items[pInfo(currentPeer)->inv[i_].id].name.find("null_item") != string::npos) {
				remove = pInfo(currentPeer)->inv[i_].count * -1;
				modify_inventory(currentPeer, pInfo(currentPeer)->inv[i_].id, remove);
			}
		}
	}
	time_t currentTime;
	time(&currentTime);
	const auto localTime = localtime(&currentTime);
	const auto Hour = localTime->tm_hour; const auto Min = localTime->tm_min; const auto Sec = localTime->tm_sec; const auto Year = localTime->tm_year + 1900; const auto Day = localTime->tm_mday; const auto Month = localTime->tm_mon + 1;
	if (Hour == 0) {
		honors_reset();
		daily_quest();
		daily_ancient_riddle();
	}
	if (Hour >= 6 and Hour < 15) {
		DaylightDragon.param1 = 0, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 2;
	}
	if (Hour >= 15 and Hour < 18) {
		DaylightDragon.param1 = 1, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 0;
	}
	if (Hour >= 18 and Hour < 23 or Hour >= 0 and Hour < 6) {
		DaylightDragon.param1 = 2, DaylightDragon.param2 = 0, DaylightDragon.param3 = 1, DaylightDragon.param4 = 5, DaylightDragon.param5 = 0, DaylightDragon.param6 = 1;
	}
}