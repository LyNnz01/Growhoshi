﻿#pragma warning(disable:4552, 4309)
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "enet/include/enet.h"
#include <nlohmann/json.hpp>
#include "Item.h"
#include "Base.h"
#include "Player.h"
#include "Packet.h"
#include "Guilds.h"
#include "World.h"
#include "Webhook.h"
#include "SaveSystem.h"
#pragma comment(lib, "Ws2_32.lib")
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) {
	save_all();
	return FALSE;
}
void doLog(string txt) {
	ofstream a("case4logs.txt", ios::app);
	a << txt + "\n";
	a.close();
}
void doLog2(string txt) {
	ofstream a2("case2logs.txt", ios::app);
	a2 << txt + "\n";
	a2.close();
}
vector<string> split_string_by_newline(const string& str)
{
	auto result = vector<string>{};
	auto ss = stringstream{ str };

	for (string line; getline(ss, line, '\n');)
		result.push_back(line);

	return result;
}
BOOL WINAPI ConsoleHandler(DWORD dwType)
{
	switch (dwType) {
	case CTRL_LOGOFF_EVENT: case CTRL_SHUTDOWN_EVENT: case CTRL_CLOSE_EVENT:
	{
		down_save();
		return TRUE;
	}
	default:
	{
		break;
	}
	}
	return FALSE;
}
long long last_time = 0, last_guild_save = time(NULL) + 60, last_time_ = 0, last_time2_ = 0, last_hm_time = 0;// , last_growganoth_time = 0;
void loop_worlds() {
	long long ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	if (last_hm_time - ms_time <= 0) {
		last_hm_time = ms_time + 60000;
		for (int i = 0; i < monitors.size(); i++) {
			string name_ = monitors[i].world_name;
			vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				WorldBlock* monitor = &world_->blocks[monitors[i].x + (monitors[i].y * 100)];
				if (!items[monitor->fg].heart_monitor) {
					monitors.erase(monitors.begin() + i);
					i--;
					continue;
				}
				monitors[i].active = 0;
				string find_mon = monitor->heart_monitor;
				if (find_mon.size() >= 2) find_mon.resize(find_mon.size() - 2); // remove `` is galo
				for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
					if (pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName == find_mon) {
						monitors[i].active = 1;
						break;
					}
				}
				PlayerMoving data_{};
				data_.packetType = 5, data_.punchX = monitors[i].x, data_.punchY = monitors[i].y, data_.characterState = 0x8;
				BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, monitor));
				BYTE* blc = raw + 56;
				form_visual(blc, *monitor, *world_, NULL, false, false, monitors[i].x, monitors[i].y);
				for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
					if (pInfo(currentPeer)->world == world_->name) {
						send_raw(currentPeer, 4, raw, 112 + alloc_(world_, monitor), ENET_PACKET_FLAG_RELIABLE);
					}
				}
				delete[] raw, blc;
			}
		}
	}
	if (last_time2_ - ms_time <= 0 && restart_server_status) {
		gamepacket_t p;
		p.Insert("OnConsoleMessage"), p.Insert("`4Global System Message``: Restarting server for update in `4" + to_string(restart_server_time) + "`` minutes");
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
			packet_(currentPeer, "action|play_sfx\nfile|audio/ogg/suspended.ogg\ndelayMS|700");
			p.CreatePacket(currentPeer);
		}
		restart_server_time -= 1;
		if (restart_server_time == 0) {
			last_time2_ = ms_time + 10000, restart_server_status_seconds = true, restart_server_status = false;
			restart_server_time = 50;
		}
		else last_time2_ = ms_time + 60000;
	}
	if (restart_server_status_seconds && last_time2_ - ms_time <= 0) {
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
			gamepacket_t p;
			p.Insert("OnConsoleMessage"), p.Insert("`4Global System Message``: Restarting server for update in `4" + (restart_server_time > 0 ? to_string(restart_server_time) : "ZERO") + "`` seconds" + (restart_server_time > 0 ? "" : "! Should be back up in a minute or so. BYE!") + "");
			p.CreatePacket(currentPeer);
		}
		last_time2_ = ms_time + 10000;
		if (restart_server_time == 0) {
			restart_server_status_seconds = false;
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
				only_role = true;
				enet_peer_disconnect_later(currentPeer, 0);
			}
		}
		restart_server_time -= 10;
	}
	if (last_time - ms_time <= 0) {
		last_time = ms_time + 1000;
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world.empty() or pInfo(currentPeer)->tankIDName.empty()) continue;
			if (pInfo(currentPeer)->last_fish_catch + pInfo(currentPeer)->fish_seconds < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count() && pInfo(currentPeer)->fishing_used != 0 && rand() % 100 < (pInfo(currentPeer)->hand == 3010 ? 9 : 6)) {
				PlayerMoving data_{};
				data_.packetType = 17, data_.netID = 34, data_.YSpeed = 34, data_.x = pInfo(currentPeer)->f_x * 32 + 16, data_.y = pInfo(currentPeer)->f_y * 32 + 16;
				pInfo(currentPeer)->last_fish_catch = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
				BYTE* raw = packPlayerMoving(&data_);
				gamepacket_t p3(0, pInfo(currentPeer)->netID);
				p3.Insert("OnPlayPositioned"), p3.Insert("audio/splash.wav");
				for (ENetPeer* currentPeer_event = server->peers; currentPeer_event < &server->peers[server->peerCount]; ++currentPeer_event) {
					if (currentPeer_event->state != ENET_PEER_STATE_CONNECTED or currentPeer_event->data == NULL or pInfo(currentPeer_event)->world != pInfo(currentPeer)->world) continue;
					send_raw(currentPeer_event, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE), p3.CreatePacket(currentPeer_event);
				}
				delete[] raw;
			}
			if (pInfo(currentPeer)->save_time + 600000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
				if (pInfo(currentPeer)->save_time != 0) {
					pInfo(currentPeer)->opc++;
					if (pInfo(currentPeer)->gp) pInfo(currentPeer)->opc++;
				}
				pInfo(currentPeer)->save_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			}
			/*
			string name_ = pInfo(currentPeer)->world;
			vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
			if (p != worlds.end()) {
				World* world_ = &worlds[p - worlds.begin()];
				/*
				if (last_growganoth_time - ms_time <= 0 && world_->name == "GROWGANOTH") {
					last_growganoth_time = ms_time + 15000;
					for (int i_ = 0; i_ < growganoth_platform.size(); i_++) update_tile(currentPeer, growganoth_platform[i_] % 100, growganoth_platform[i_] / 100, 0, false, false);
					growganoth_platform.clear();
					for (int i_ = 0; i_ < 50; i_++) update_tile(currentPeer, rand() % 60 + 20, rand() % 32 + 20, (i_ < 40 ? 1222 : 7048), true, false);
				}*/
				/*
					if (world_->special_event && world_->last_special_event + 30000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						gamepacket_t p, p2;
						p.Insert("OnAddNotification"), p.Insert("interface/large/special_event.rttex"), p.Insert("`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oTime's up! Nobody found it!``" : "`oTime's up! " + to_string(world_->special_event_item_taken) + " of " + to_string(items[world_->special_event_item].event_total) + " items found.``") + ""), p.Insert("audio/cumbia_horns.wav"), p.Insert(0);
						p2.Insert("OnConsoleMessage"), p2.Insert("`2" + items[world_->special_event_item].event_name + ":`` " + (items[world_->special_event_item].event_total == 1 ? "`oTime's up! Nobody found it!``" : "`oTime's up! " + to_string(world_->special_event_item_taken) + " of " + to_string(items[world_->special_event_item].event_total) + " items found.``") + "");
						for (ENetPeer* currentPeer_event = server->peers; currentPeer_event < &server->peers[server->peerCount]; ++currentPeer_event) {
							if (currentPeer_event->state != ENET_PEER_STATE_CONNECTED or currentPeer_event->data == NULL or pInfo(currentPeer_event)->world != name_) continue;
							p.CreatePacket(currentPeer_event), p2.CreatePacket(currentPeer_event);
							PlayerMoving data_{};
							data_.effect_flags_check = 1, data_.packetType = 14, data_.netID = 0;
							for (int i_ = 0; i_ < world_->drop.size(); i_++) {
								if (world_->drop[i_].special == true) {
									data_.plantingTree = world_->drop[i_].uid;
									BYTE* raw = packPlayerMoving(&data_);
									send_raw(currentPeer_event, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[]raw;
									world_->drop[i_].id = 0, world_->drop[i_].x = -1, world_->drop[i_].y = -1;
								}
							}
						}
						world_->last_special_event = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count(), world_->special_event_item = 0, world_->special_event_item_taken = 0, world_->special_event = false;
					}
				}*/
			if (pInfo(currentPeer)->hand == 3578 && pInfo(currentPeer)->hand_torch + 60000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
				if (pInfo(currentPeer)->hand_torch != 0) {
					int got = 0;
					modify_inventory(currentPeer, 3578, got);
					if (got - 1 >= 1) {
						gamepacket_t p;
						p.Insert("OnTalkBubble"), p.Insert(pInfo(currentPeer)->netID), p.Insert("`4My torch went out, but I have " + to_string(got - 1) + " more!``"), p.CreatePacket(currentPeer);
					}
					modify_inventory(currentPeer, 3578, got = -1);
				}
				pInfo(currentPeer)->hand_torch = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
			}
			if (pInfo(currentPeer)->hand == 2204 and pInfo(currentPeer)->x != -1 and pInfo(currentPeer)->y != -1) {
				if (not has_playmod(pInfo(currentPeer), "Irradiated")) {
					if (pInfo(currentPeer)->geiger_x == -1 and pInfo(currentPeer)->geiger_y == -1) pInfo(currentPeer)->geiger_x = (rand() % 100) * 32, pInfo(currentPeer)->geiger_y = (rand() % 54) * 32;
					int a_ = pInfo(currentPeer)->geiger_x + ((pInfo(currentPeer)->geiger_y * 100) / 32), b_ = pInfo(currentPeer)->x + ((pInfo(currentPeer)->y * 100) / 32), diff = abs(a_ - b_) / 32;
					if (diff > 30) { // nieko
					}
					else if (diff >= 30) { // raudona
						if (pInfo(currentPeer)->geiger_time + 1500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(currentPeer)->geiger_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							PlayerMoving data_{};
							data_.packetType = 17, data_.x = pInfo(currentPeer)->x + 10, data_.y = pInfo(currentPeer)->y + 16, data_.characterState = 0x8, data_.XSpeed = 0, data_.YSpeed = 114;
							BYTE* raw = packPlayerMoving(&data_, 56);
							raw[3] = 114;
							double rotation = -4.13;
							memcpy(raw + 40, &rotation, 4);
							send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							delete[] raw;
						}
					}
					else if (diff >= 15) { // geltona
						if (pInfo(currentPeer)->geiger_time + 1500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(currentPeer)->geiger_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							PlayerMoving data_{};
							data_.packetType = 17, data_.x = pInfo(currentPeer)->x + 10, data_.y = pInfo(currentPeer)->y + 16, data_.characterState = 0x8, data_.XSpeed = 1, data_.YSpeed = 114;
							BYTE* raw = packPlayerMoving(&data_, 56);
							raw[3] = 114;
							double rotation = -4.13;
							memcpy(raw + 40, &rotation, 4);
							send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
							delete[] raw;
						}
					}
					else { // zalia
						if (diff <= 1) { // surado
							{
								if (pInfo(currentPeer)->geiger_time + 2500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
									pInfo(currentPeer)->geiger_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
									pInfo(currentPeer)->geiger_x = -1, pInfo(currentPeer)->geiger_y = -1;
									{
										int c_ = -1;
										modify_inventory(currentPeer, 2204, c_);
										int c_2 = 1;
										if (modify_inventory(currentPeer, 2286, c_2) != 0) {
											string name_ = pInfo(currentPeer)->world;
											vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
											if (p != worlds.end()) {
												World* world_ = &worlds[p - worlds.begin()];
												WorldDrop drop_block_{};
												drop_block_.id = 2286, drop_block_.count = 1, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(currentPeer)->x + rand() % 17, drop_block_.y = pInfo(currentPeer)->y + rand() % 17;
												dropas_(world_, drop_block_);
											}
										}
										PlayMods give_playmod{};
										give_playmod.id = 10;
										give_playmod.time = time(nullptr) + (thedaytoday == 3 ? 600 : 900);
										pInfo(currentPeer)->playmods.push_back(give_playmod);
										pInfo(currentPeer)->hand = 2286;
										update_clothes(currentPeer);
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert(a + "You are aglow with radiation! (`$Irradiated`` mod added, `$" + (thedaytoday == 3 ? "10" : "15") + " mins`` left)");
										p.CreatePacket(currentPeer);
										packet_(currentPeer, "action|play_sfx\nfile|audio/dialog_confirm.wav\ndelayMS|0");
									}
									int chanced = 0;
									if (thedaytoday == 3) chanced = 3;
									add_role_xp(currentPeer, 1, "geiger");
									// GEIGER QUEST
									vector<int> geiger_items = { 6416,3196,2206,1500,1498,2806,2804,8270,8272,8274,2244,2246,2242,3792,3306,4676,4678,4680,4682,4652,4650,4648,4646,11186,10086 };
									vector<int> rare_cr = { 2248,2250,3792,10084 };
									vector<int> rarest = { 4654 , 9380 , 11562, 1486 };
									int item_ = geiger_items[rand() % geiger_items.size()];
									if (rand() % 50 < 10 + chanced) item_ = rare_cr[rand() % rare_cr.size()];
									if (rand() % 50 < 2 + chanced) item_ = rarest[rand() % rarest.size()];
									int c_ = 1;
									if (modify_inventory(currentPeer, item_, c_) != 0) {
										string name_ = pInfo(currentPeer)->world;
										vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
										if (p != worlds.end()) {
											World* world_ = &worlds[p - worlds.begin()];
											WorldDrop drop_block_{};
											drop_block_.id = item_, drop_block_.count = 1, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(currentPeer)->x + rand() % 17, drop_block_.y = pInfo(currentPeer)->y + rand() % 17;
											dropas_(world_, drop_block_);
										}
									}
									gamepacket_t p;
									p.Insert("OnParticleEffect");
									p.Insert(48);
									p.Insert((float)pInfo(currentPeer)->x + 10, (float)pInfo(currentPeer)->y + 16);
									p.CreatePacket(currentPeer);
									{
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(currentPeer)->netID);
										p.Insert("I found `21 " + items[item_].name + "``!");
										p.Insert(0);
										p.CreatePacket(currentPeer);
										gamepacket_t p2;
										p2.Insert("OnConsoleMessage");
										p2.Insert(pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName + "`` found `21 " + items[item_].name + "``!");
										PlayerMoving data_{};
										data_.packetType = 19, data_.plantingTree = 0, data_.netID = 0;
										data_.punchX = item_;
										data_.x = pInfo(currentPeer)->x + 10, data_.y = pInfo(currentPeer)->y + 16;
										int32_t to_netid = pInfo(currentPeer)->netID;
										BYTE* raw = packPlayerMoving(&data_);
										raw[3] = 5;
										memcpy(raw + 8, &to_netid, 4);
										for (ENetPeer* currentPeer2 = server->peers; currentPeer2 < &server->peers[server->peerCount]; ++currentPeer2) {
											if (currentPeer2->state != ENET_PEER_STATE_CONNECTED or currentPeer2->data == NULL) continue;
											if (pInfo(currentPeer2)->world == pInfo(currentPeer)->world) {
												send_raw(currentPeer2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
												p2.CreatePacket(currentPeer2);
											}
										}
										delete[]raw;
									}
								}
							}
						}
						else {
							int t_ = 0;
							if (diff >= 6) t_ = 1350;
							else t_ = 1000;
							if (pInfo(currentPeer)->geiger_time + t_ < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
								pInfo(currentPeer)->geiger_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
								PlayerMoving data_{};
								data_.packetType = 17, data_.x = pInfo(currentPeer)->x + 10, data_.y = pInfo(currentPeer)->y + 16, data_.characterState = 0x8;
								data_.XSpeed = 2, data_.YSpeed = 114;
								BYTE* raw = packPlayerMoving(&data_, 56);
								raw[3] = 114;
								double rotation = -4.13;
								memcpy(raw + 40, &rotation, 4);
								send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
								delete[] raw;
							}
						}
					}
				}
			}
			long long time_ = time(nullptr);
			for (int i_ = 0; i_ < pInfo(currentPeer)->playmods.size(); i_++) {
				if (pInfo(currentPeer)->playmods[i_].id == 12) {
					if (pInfo(currentPeer)->valentine_time + 2500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(currentPeer)->valentine_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
						for (ENetPeer* valentine = server->peers; valentine < &server->peers[server->peerCount]; ++valentine) {
							if (valentine->state != ENET_PEER_STATE_CONNECTED or valentine->data == NULL) continue;
							if (pInfo(valentine)->world == pInfo(currentPeer)->world and pInfo(valentine)->tankIDName == pInfo(currentPeer)->playmods[i_].user) {
								if (not pInfo(valentine)->invis and not pInfo(currentPeer)->invis and pInfo(currentPeer)->x != -1 and pInfo(currentPeer)->y != -1 and pInfo(valentine)->x != -1 and pInfo(valentine)->y != -1) {
									gamepacket_t p;
									p.Insert("OnParticleEffect");
									p.Insert(13);
									p.Insert((float)pInfo(valentine)->x + 10, (float)pInfo(valentine)->y + 16);
									p.Insert((float)0), p.Insert((float)pInfo(currentPeer)->netID);
									bool double_send = false;
									for (int i_2 = 0; i_2 < pInfo(valentine)->playmods.size(); i_2++) {
										if (pInfo(valentine)->playmods[i_2].id == 12 and pInfo(valentine)->playmods[i_2].user == pInfo(currentPeer)->tankIDName) {
											double_send = true;
											break;
										}
									}
									gamepacket_t p2;
									p2.Insert("OnParticleEffect");
									p2.Insert(13);
									p2.Insert((float)pInfo(currentPeer)->x + 10, (float)pInfo(currentPeer)->y + 16);
									p2.Insert((float)0), p2.Insert((float)pInfo(valentine)->netID);
									for (ENetPeer* valentine_bc = server->peers; valentine_bc < &server->peers[server->peerCount]; ++valentine_bc) {
										if (valentine_bc->state != ENET_PEER_STATE_CONNECTED or valentine_bc->data == NULL) continue;
										if (pInfo(valentine_bc)->world == pInfo(currentPeer)->world) {
											p.CreatePacket(valentine_bc);
											if (double_send) p2.CreatePacket(valentine_bc);
										}
									}
								}
								break;
							}
						}
					}
				}
				if (pInfo(currentPeer)->playmods[i_].time - time_ < 0) {
					for (vector<string> get_ : info_about_playmods) {
						uint32_t playmod_id = atoi(get_[0].c_str());
						if (playmod_id == pInfo(currentPeer)->playmods[i_].id) {
							string playmod_name = get_[2];
							string playmod_on_remove = get_[4];
							pInfo(currentPeer)->playmods.erase(pInfo(currentPeer)->playmods.begin() + i_);
							packet_(currentPeer, "action|play_sfx\nfile|audio/dialog_confirm.wav\ndelayMS|0");
							gamepacket_t p;
							p.Insert("OnConsoleMessage");
							p.Insert(playmod_on_remove + " (`$" + playmod_name + "`` mod removed)");
							p.CreatePacket(currentPeer);
							update_clothes(currentPeer);
							break;
						}
					}
				}
			}
		}
	}
}
bool isASCII(const std::string& s)
{
	return !std::any_of(s.begin(), s.end(), [](char c) {
		return static_cast<unsigned char>(c) > 127;
		});
}
inline void Loopos() {
	while (true) {
		Sleep(500);
		loop_worlds();
	}
}
int main(int argc, char* argv[]) {
	BOOL ret = SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	system("COLOR 9");
	system("TITLE Growhoshi Server");
	hoshi_info("Please wait while we setuping the server");
	if (atexit(save_all)) {
		hoshi_db_logs("Saving all Database!");
	}
	srand(unsigned int(time(nullptr)));
	hoshi_db_logs("Loading saved guild data");
	for (const auto& entry : fs::directory_iterator("database/guilds")) {
		if (!fs::is_directory(entry.path())) {
			string guild_id = explode(".", entry.path().filename().string())[0];
			json guild_read;
			ifstream read_guild(entry.path(), ifstream::binary);
			read_guild >> guild_read;
			read_guild.close();
			Guild new_guild{};
			new_guild.guild_id = atoi(guild_id.c_str());
			new_guild.guild_name = guild_read["guild_name"].get<string>();
			new_guild.guild_description = guild_read["guild_description"].get<string>();
			new_guild.guild_mascot = guild_read["guild_mascot"].get<vector<uint16_t>>();
			new_guild.guild_level = guild_read["guild_level"].get<uint16_t>();
			new_guild.guild_xp = guild_read["guild_xp"].get<uint32_t>();
			new_guild.coleader_access = guild_read["coleader_access"].get<bool>();
			new_guild.coleader_elder_access = guild_read["coleader_elder_access"].get<bool>();
			new_guild.all_access = guild_read["all_access"].get<bool>();
			new_guild.guild_world = guild_read["guild_world"].get<string>();
			json a_ = guild_read["guild_members"].get<json>();
			for (int i_ = 0; i_ < a_.size(); i_++) {
				GuildMember new_member{};
				new_member.member_name = a_[i_]["member_name"].get<string>();
				new_member.role_id = a_[i_]["role_id"].get<int>();
				new_member.public_location = a_[i_]["public_location"].get<bool>();
				new_member.show_notifications = a_[i_]["show_notifications"].get<bool>();
				new_member.last_online = a_[i_]["last_online"].get<long long>();
				new_guild.guild_members.push_back(new_member);
			}
			json b_ = guild_read["guild_logs"].get<json>();
			for (int i_ = 0; i_ < b_.size(); i_++) {
				GuildLog new_log{};
				new_log.info = b_[i_]["info"].get<string>();
				new_log.display_id = b_[i_]["display_id"].get<uint16_t>();
				new_log.date = b_[i_]["date"].get<long long>();
				new_guild.guild_logs.push_back(new_log);
			}
			guilds.push_back(new_guild);
		}
	}
	if (items_dat() == -1)
		hoshi_warn("items.dat error maybe missing or moved");
	else {
		hoshi_info(setGems(items.size()) + " items loaded");
		//cout << setGems(items.size()) << " items loaded" << endl;
	}
	server_port = 28028;
	if (init_enet(server_port) == -1)
		hoshi_warn("ENet error or port 28028 already used");
	else
		system("CLS");
	cout << "===== Growhoshi Growtopia Private Server =====" << endl;
	cout << "" << endl;
	cout << "Server written in C++ and developed by Ritshu" << endl;
	time_t timeNow = time(0);
	char* dt = ctime(&timeNow);
	cout << "Server Running: " << dt << endl;
	hoshi_info("ENet Service started on port: " + to_string(server_port));
	hoshi_info("Items data loaded containing : " + setGems(items.size()) + " items");
	hoshi_db_logs("All data successfully loaded.");
	thread runautosave(autosave);
	if (runautosave.joinable()) {
		runautosave.detach();
	}
	thread a__(Server_Saving);
	thread b__(Server_Reset);
	thread c__(Loopos);
	a__.detach();
	b__.detach();
	c__.detach();
	signal(SIGSEGV, save_w);
	signal(SIGABRT, save_w);
	daily_quest();
	honors_reset();

	ifstream ifs("news.json");
	if (ifs.is_open()) {
		json j;
		ifs >> j;
		news_text = j["news"].get<string>();
	}
	// }
	struct tm newtime;
	time_t now = time(0);
	localtime_s(&newtime, &now);
	thedaytoday = newtime.tm_wday;
	{
		const char* months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
		string month = months[newtime.tm_mon], translated = "", str = to_string(today_day), locke = "";
		if (str == "01" || str == "21") translated = "st";
		else if (str == "02" || str == "22") translated = "nd";
		else if (str == "03") translated = "rd";
		else translated = "th";
		if (thedaytoday == 0) locke = "\nadd_spacer|small|\nadd_textbox|`oToday Growhoshi is being paid a visit by `5Locke`` the traveling salesman! He comes one day a week to hawk his fabulous wares, though this time he'll stick around a day and a half to introduce himself. Checkout the world `5LOCKE``!``|left|";
		news_texture = "set_default_color|`o\nadd_label_with_icon|big|`wThe Growhoshi Gazette``|left|5016|\nadd_spacer|small|\nadd_image_button||interface/large/hoshi_banner.rttex|bannerlayout|||\nadd_spacer|small|\nadd_textbox|`w" + month + " " + to_string(today_day) + "" + translated + ": `1Server Release``: x5 Gems from breaking blocks/harvest trees.``|left|\nadd_spacer|small|\nadd_image_button|iotm_layout|interface/large/gazette/gazette_3columns_feature_btn04.rttex|3imageslayout|||\nadd_image_button|iotm_layout|interface/large/gazette/gazette_3columns_feature_btn03.rttex|3imageslayout|||\nadd_image_button|iotm_layout|interface/large/gazette/gazette_3columns_feature_btn10.rttex|3imageslayout|||" + locke + news_text + "\nadd_quick_exit|";
	}
	if (thedaytoday == 1) theitemtoday = 5040;
	else if (thedaytoday == 2) theitemtoday = 5042;
	else if (thedaytoday == 3) theitemtoday = 5044;
	else if (thedaytoday == 4) theitemtoday = 5032;
	else if (thedaytoday == 5)theitemtoday = 5034;
	else if (thedaytoday == 6) theitemtoday = 5036;
	else if (thedaytoday == 0)theitemtoday = 5038;
	ENetEvent event;
	while (true) {
		while (enet_host_service(server, &event, 1000) > 0 && f_saving_ == false) {
			if (saving_ or f_saving_) continue;
			ENetPeer* peer = event.peer;
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
			{
				bool quit = false;
				char clientConnection[16];
				enet_address_get_host_ip(&peer->address, clientConnection, 16);
				if (quit) break;
				send_(peer, 1, nullptr, 0);
				peer->data = new Player;
				pInfo(peer)->id = peer->connectID;
				pInfo(peer)->ip = clientConnection;
				string error = "";
				int logged = 0;
				for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
					if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->ip != pInfo(currentPeer)->ip) continue;
					logged++;
				}
				if (logged == 200) error = "`4OOPS:`` Sorry the server is currently overloaded with `4200 ``Player online, we are currently working for this issue.";
				if (logged > 4) error = "`4OOPS:`` Too many people logging in at once. Please press `5CANCEL`` and try again in a few seconds.";
				for (int i = 0; i < ipbans.size(); i++) if (pInfo(peer)->ip == ipbans[i]) error = "action|log\nmsg|CT:[S]_ `4Sorry, you are not allowed to enter the game from this location. Contact `5Discord Staff Team `4if you have any questions.";
				if (pInfo(peer)->ip != pInfo(peer)->meta and pInfo(peer)->meta != "") error = "action|log\nmsg|CT:[S]_ `4Can not make new account!`` Sorry, but IP " + pInfo(peer)->ip + " is not permitted to create NEW Growtopia accounts at this time. (This can be because there is an open proxy/VPN here or abuse has from this IP) Please try again from another IP address.";
				//if (pInfo(peer)->gameVersion != "3.93") error = "action|log\nmsg|CT:[S]_ `5Update Required: `oNow version Growtopia is 3.93, Please update your growtopia.";
				if (error != "") packet_(peer, error, ""), enet_peer_disconnect_later(peer, 0);

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE: // TODO DETECT CLIENT SENDING PACKET
			{
				loop_worlds();
				switch (message_(event.packet)) {
				case 2:
				{
					string cch = text_(event.packet);
					theCCH = cch;
					int lpse = 10;
					if (cch.size() < 8) {
						if (pInfo(peer)->tankIDName != "") {
							add_ban(peer, 6.307e+7, "Usage of Third Party Program", "System");
							gamepacket_t p, p1;
							p.Insert("OnConsoleMessage");
							p1.Insert("OnConsoleMessage");
							p.Insert("CT:[FC]_>> `^>> Server-Operator: `w" + pInfo(peer)->tankIDName + "`` (in: `w" + pInfo(peer)->world + "``) has been disconnected because it was detected sending suspicious packet.");
							p1.Insert("CT:[FC]_>> `^>> Server-Operator: `w" + pInfo(peer)->tankIDName + "`` has been BANNED for `w730 days``.");
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
								if (pInfo(currentPeer)->dev || pInfo(currentPeer)->superdev) {
									p.CreatePacket(currentPeer);
									p1.CreatePacket(currentPeer);
								}
							}
						}
					}
					if (cch == "action|getDRAnimations\n") break;
					if (pInfo(peer)->lpps + 1000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(peer)->pps = 0;
						pInfo(peer)->lpps = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						pInfo(peer)->pps++;
						if (pInfo(peer)->pps >= lpse) {
							hoshi_warn("Detected Packet (2) Spam by " + pInfo(peer)->tankIDName);
							packet_(peer, "action|log\nmsg|`7Your client sending too many packets, atempt to reconnect.");
							enet_peer_disconnect_later(peer, 0);
							break;
						}
					}
					//doLog2(cch);
					if (cch.size() < 5) break;
					if (cch.size() > 1024 || not isASCII(cch)) break;
					//cout << cch << endl;
					if (pInfo(peer)->last_open_packet + 1000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(peer)->packet_to_much = 0;
						pInfo(peer)->last_open_packet = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						pInfo(peer)->packet_to_much++;
						if (pInfo(peer)->packet_to_much >= 30) {
							add_modlogs(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "`6 [" + pInfo(peer)->ip + " BANNED BY SYSTEM (PACKET BYPASS/SPAM) +" + text_(event.packet), "");
							hoshi_info(pInfo(peer)->tankIDName + " Auto Banned for bypassing / spamming packets ");
							add_ban(peer, 6.307e+7, "Spaming packet (maybe dupe/crash)!", "System");
							enet_peer_disconnect_later(peer, 0);
							break;
						}
					}

					if (pInfo(peer)->world != "") {
						if (pInfo(peer)->x / 32 == -1 || pInfo(peer)->y / 32 == -1) break;
					}
					if (pInfo(peer)->requestedName.empty()) {
						if (pInfo(peer)->enter_game != 0 || pInfo(peer)->world != "") enet_peer_disconnect_later(peer, 0);
						else player_login(peer, cch);
					}
					else if (cch.find("action|validate_world") != string::npos) {
						vector<string> t_ = explode("|", cch);
						if (t_.size() < 3) break;
						string Name = explode("\n", t_[2])[0];
						int Available = 0;
						if (fs::is_directory("database/worlds/" + Name + "_.json")) Available = 1;
						packet_(peer, "action|world_validated\navailable|" + to_string(Available) + "\nworld_name|" + Name + "\n");
						break;
					}
					if (!pInfo(peer)->isIn) {
						if (!player_login(peer, cch)) {
							break;
						}
					}
					else if (cch.find("action|input") != string::npos) {
						vector<string> t_ = explode("|", cch);
						if (t_.size() < 4) break;
						string msg = explode("\n", t_[3])[0];
						if (pInfo(peer)->tankIDName == "") break;
						if (msg.length() <= 0 || msg.length() > 120 || msg.empty() || std::all_of(msg.begin(), msg.end(), [](char c) {return std::isspace(c); })) continue;
						for (char c : msg) if (c < 0x20 || c>0x7A) continue;
						space_(msg);
						if (msg[0] == '/') SendCmd(peer, msg);
						else {
							if (msg[0] == '`' and msg.size() <= 2) break;
							if (pInfo(peer)->world == "") break;
							if (has_playmod(pInfo(peer), "duct tape") || has_playmod(pInfo(peer), "Iron MMMFF")) {
								string msg2 = "";
								for (int i = 0; i < msg.length(); i++) {
									if (isspace(msg[i])) msg2 += " ";
									else {
										if (isupper(msg[i])) msg2 += i % 2 == 0 ? "M" : "F";
										else msg2 += i % 2 == 0 ? "m" : "f";
									}
								}
								msg = msg2;
							}
							string check_ = msg;
							transform(check_.begin(), check_.end(), check_.begin(), ::tolower);
							{
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									if (world_->silence and pInfo(peer)->superdev != 1 and world_->owner_name != pInfo(peer)->tankIDName and find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end()) {
										gamepacket_t p;
										p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`1(Peasants must not speak)"), p.Insert(1), p.CreatePacket(peer);
										break;
									}
								}
							}
							bool warned = false;
							pInfo(peer)->not_same++;
							if (pInfo(peer)->last_input_text == msg) pInfo(peer)->same_input++;
							pInfo(peer)->last_input_text = msg;
							if (pInfo(peer)->last_spam_detection + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) pInfo(peer)->last_spam_detection = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count(), pInfo(peer)->same_input = 0, pInfo(peer)->not_same = 0;
							if (pInfo(peer)->same_input >= 3 || pInfo(peer)->not_same >= 5) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage"), p.Insert("`6>>`4Spam detected! ``Please wait a bit before typing anything else.  Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!"), p.CreatePacket(peer);
							}
							else {
								replaceAll(msg, "`%", "");
								string chat_color = "`$";
								if (pInfo(peer)->d_name.empty()) {
									chat_color = pInfo(peer)->superdev ? "`5" : pInfo(peer)->dev ? "`1" : pInfo(peer)->tmod ? "`^" : "`$";
								}
								if (has_playmod(pInfo(peer), "Infected!")) {
									chat_color = "`2";
									if (rand() % 4 < 1) chat_color += "Brraaiinnss...";
								}
								if (pInfo(peer)->face == 1170)  chat_color = "`4";
								gamepacket_t p, p2;
								p.Insert("OnConsoleMessage");
								p.Insert("CP:_PL:0_OID:_CT:[W]_ `6<`w" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + (pInfo(peer)->is_legend ? " of Legend" : "") + "`6> " + chat_color + msg);
								p2.Insert("OnTalkBubble");
								p2.Insert(pInfo(peer)->netID);
								if (check_ != ":/" and check_ != ":p" and check_ != ":*" and check_ != ";)" and check_ != ":d" and check_ != ":o" and check_ != ":'(" and check_ != ":(") {
									p2.Insert("CP:_PL:0_OID:_player_chat=" + (chat_color == "`$" ? "" : chat_color) + msg);
								}
								else p2.Insert(msg);
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (pInfo(currentPeer)->world == pInfo(peer)->world) {
										bool muted_ = false;
										for (int c_ = 0; c_ < pInfo(currentPeer)->friends.size(); c_++) {
											if (pInfo(currentPeer)->friends[c_].name == pInfo(peer)->tankIDName) {
												if (pInfo(currentPeer)->friends[c_].mute) {
													muted_ = true;
													break;
												}
											}
										}
										if (not muted_) {
											p.CreatePacket(currentPeer);
											p2.CreatePacket(currentPeer);
										}
									}
								}
							}
						}
						break;
					}
					else if (cch.find("action|mod_trade") != string::npos or cch.find("action|rem_trade") != string::npos) {
						vector<string> t_ = explode("|", cch);
						if (t_.size() < 3) break;
						int item_id = atoi(explode("\n", t_[2])[0].c_str()), c_ = 0;
						modify_inventory(peer, item_id, c_);
						if (c_ == 0) break;
						if (items[item_id].untradeable) {
							gamepacket_t p;
							p.Insert("OnTextOverlay");
							p.Insert("You'd be sorry if you lost that!");
							p.CreatePacket(peer);
							break;
						} if (c_ == 1 or cch.find("action|rem_trade") != string::npos) {
							mod_trade(peer, item_id, c_, (cch.find("action|rem_trade") != string::npos ? true : false));
							break;
						}
						if (cch.find("action|rem_trade") == string::npos) {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\nadd_label_with_icon|big|`2Trade`` `w" + items[item_id].name + "``|left|" + to_string(item_id) + "|\nadd_textbox|`2Trade how many?``|left|\nadd_text_input|count||" + to_string(c_) + "|5|\nembed_data|itemID|" + to_string(item_id) + "\nend_dialog|trade_item|Cancel|OK|");
							p.CreatePacket(peer);
						}
						break;
					}
					else if (cch.find("action|trade_accept") != string::npos) {
						if (pInfo(peer)->trading_with != -1) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 3) break;
							string status_ = explode("\n", t_[2])[0];
							if (status_ != "1" and status_ != "0") break;
							bool f_ = false;
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
								if (pInfo(currentPeer)->world == pInfo(peer)->world) {
									if (pInfo(currentPeer)->netID == pInfo(peer)->trading_with and pInfo(peer)->netID == pInfo(currentPeer)->trading_with) {
										string name_ = pInfo(peer)->world;
										vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
										if (p != worlds.end()) {
											World* world_ = &worlds[p - worlds.begin()];
											if (status_ == "1")
												pInfo(peer)->trade_accept = 1;
											else
												pInfo(peer)->trade_accept = 0;
											if (pInfo(peer)->trade_accept and pInfo(currentPeer)->trade_accept) {
												// check inv space
												if (not trade_space_check(peer, currentPeer)) {
													pInfo(peer)->trade_accept = 0, pInfo(currentPeer)->trade_accept = 0;
													gamepacket_t p;
													p.Insert("OnTradeStatus");
													p.Insert(pInfo(peer)->netID);
													p.Insert("");
													p.Insert("`o" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "``'s offer.``");
													p.Insert(make_trade_offer(pInfo(peer), true) + "locked|0\naccepted|0");
													p.CreatePacket(peer);
													{
														gamepacket_t p;
														p.Insert("OnTradeStatus");
														p.Insert(pInfo(peer)->netID);
														p.Insert("");
														p.Insert("`o" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "``'s offer.``");
														p.Insert(make_trade_offer(pInfo(peer), true) + "locked|0\nreset_locks|1\naccepted|0");
														p.CreatePacket(currentPeer);
													}
													f_ = true;
													break;
												}
												else if (not trade_space_check(currentPeer, peer)) {
													pInfo(peer)->trade_accept = 0, pInfo(currentPeer)->trade_accept = 0;
													gamepacket_t p;
													p.Insert("OnTradeStatus");
													p.Insert(pInfo(currentPeer)->netID);
													p.Insert("");
													p.Insert("`o" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName) + "``'s offer.``");
													p.Insert(make_trade_offer(pInfo(currentPeer), true) + "locked|0\naccepted|0");
													p.CreatePacket(currentPeer);
													{
														gamepacket_t p;
														p.Insert("OnTradeStatus");
														p.Insert(pInfo(currentPeer)->netID);
														p.Insert("");
														p.Insert("`o" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName) + "``'s offer.``");
														p.Insert(make_trade_offer(pInfo(currentPeer), true) + "locked|0\nreset_locks|1\naccepted|0");
														p.CreatePacket(peer);
													}
													f_ = true;
													break;
												}
												{
													gamepacket_t p;
													p.Insert("OnForceTradeEnd");
													p.CreatePacket(peer);
												}
												send_trade_confirm_dialog(peer, currentPeer);
												break;
											}
											gamepacket_t p;
											p.Insert("OnTradeStatus");
											p.Insert(pInfo(peer)->netID);
											p.Insert("");
											p.Insert("`o" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "``'s offer.``");
											p.Insert(make_trade_offer(pInfo(peer), true) + "locked|0\naccepted|" + status_);
											p.CreatePacket(peer);
											{
												{
													gamepacket_t p;
													p.Insert("OnTradeStatus");
													p.Insert(pInfo(currentPeer)->netID);
													p.Insert("");
													p.Insert("`o" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName) + "``'s offer.``");
													p.Insert("locked|0\nreset_locks|1\naccepted|0");
													p.CreatePacket(currentPeer);
												}
												gamepacket_t p;
												p.Insert("OnTradeStatus");
												p.Insert(pInfo(currentPeer)->netID);
												p.Insert("");
												p.Insert("`o" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName) + "``'s offer.``");
												p.Insert("locked|0\naccepted|1");
												p.CreatePacket(currentPeer);
												{
													gamepacket_t p;
													p.Insert("OnTradeStatus");
													p.Insert(pInfo(currentPeer)->netID);
													p.Insert("");
													p.Insert("`o" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName) + "``'s offer.``");
													p.Insert(make_trade_offer(pInfo(currentPeer), true) + "locked|0\nreset_locks|1\naccepted|0");
													p.CreatePacket(currentPeer);
												}
												{
													gamepacket_t p;
													p.Insert("OnTradeStatus");
													p.Insert(pInfo(peer)->netID);
													p.Insert("");
													p.Insert("`o" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "``'s offer.``");
													p.Insert(make_trade_offer(pInfo(peer), true) + "locked|0\nreset_locks|1\naccepted|" + status_);
													p.CreatePacket(currentPeer);
												}
											}
										}
										f_ = true;
										break;
									}
								}
							} if (not f_) {
								if (status_ == "1")
									pInfo(peer)->trade_accept = 1;
								else
									pInfo(peer)->trade_accept = 0;
							}
						}
						break;
					}
					else if (cch == "action|trade_cancel\n") cancel_trade(peer);
					if (pInfo(peer)->trading_with == -1) {
						if (cch.find("action|dialog_return\ndialog_name|dnaproc") != string::npos) {
							int DNAID;
							int remove = 0 - 1;
							int add = 1;
							int x_ = atoi(explode("\n", explode("tilex|", cch)[1])[0].c_str()), y_ = atoi(explode("\n", explode("tiley|", cch)[1])[0].c_str());
							std::stringstream ss(cch);
							std::string to;
							try {
								while (std::getline(ss, to, '\n')) {
									vector<string> infoDat = explode("|", to);
									if (infoDat.at(0) == "choose") {
										DNAID = atoi(infoDat.at(1).c_str());
										if (items[DNAID].name.find("Dino DNA Strand") != string::npos || items[DNAID].name.find("Plant DNA Strand") != string::npos || items[DNAID].name.find("Raptor DNA Strand") != string::npos) {
											if (pInfo(peer)->DNAInput == 0) {
												pInfo(peer)->IDDNA1 = DNAID;
												pInfo(peer)->DNAInput = 1;
												modify_inventory(peer, DNAID, remove);
												SendDNAProcessor(peer, x_, y_, false, true, false, 0, true, false);
											}
											else if (pInfo(peer)->DNAInput == 1) {
												pInfo(peer)->IDDNA2 = DNAID;
												pInfo(peer)->DNAInput = 2;
												modify_inventory(peer, DNAID, remove);
												SendDNAProcessor(peer, x_, y_, false, true, false, 0, true, false);
											}
											else if (pInfo(peer)->DNAInput == 2) {
												pInfo(peer)->IDDNA3 = DNAID;
												pInfo(peer)->DNAInput = 3;
												modify_inventory(peer, DNAID, remove);
												int DnaNumber1 = 0, DnaNumber2 = 0, DnaNumber3 = 0, What;
												ifstream infile("DnaRecipe.txt");
												for (string line; getline(infile, line);) {
													if (line.length() > 3 && line.at(0) != '/' && line.at(1) != '/') {
														auto ex = explode("|", line);
														int id1 = atoi(ex.at(0).c_str());
														int id2 = atoi(ex.at(1).c_str());
														int id3 = atoi(ex.at(2).c_str());
														if (id1 == pInfo(peer)->IDDNA1 && id2 == pInfo(peer)->IDDNA2 && id3 == pInfo(peer)->IDDNA3) {
															DnaNumber1 = atoi(ex.at(0).c_str());
															DnaNumber2 = atoi(ex.at(1).c_str());
															DnaNumber3 = atoi(ex.at(2).c_str());
															What = atoi(ex.at(3).c_str());
															break;
														}
													}
												}
												infile.close();
												if (pInfo(peer)->IDDNA1 == DnaNumber1 && pInfo(peer)->IDDNA2 == DnaNumber2 && pInfo(peer)->IDDNA3 == DnaNumber3 && DnaNumber3 != 0 && DnaNumber2 != 0 && DnaNumber1 != 0 && What != 0) {
													SendDNAProcessor(peer, x_, y_, false, true, false, 0, true, false);
												}
												else {
													if (pInfo(peer)->DNAInput >= 1) {
														SendDNAProcessor(peer, x_, y_, false, true, false, 0, true, true);
													}
													else {
														SendDNAProcessor(peer, x_, y_, false, false, false, 0, true, true);
													}
												}
											}
										}
										else {
											if (pInfo(peer)->DNAInput >= 1) {
												SendDNAProcessor(peer, x_, y_, true, false, false, 0, true, false);
											}
											else {
												SendDNAProcessor(peer, x_, y_, true, false, false, 0, false, false);
											}
										}
									}
								}
							}
							catch (const std::out_of_range& e) {
								std::cout << e.what() << std::endl;
							}
							if (cch.find("buttonClicked|remove0") != string::npos) {
								if (pInfo(peer)->DNAInput == 1) {
									int DNARemoved = pInfo(peer)->IDDNA1;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA1 = 0;
									pInfo(peer)->DNAInput = 0;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, false, false);
								}
								if (pInfo(peer)->DNAInput == 2) {
									int DNARemoved = pInfo(peer)->IDDNA1;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA1 = pInfo(peer)->IDDNA2;
									pInfo(peer)->IDDNA2 = 0;
									pInfo(peer)->DNAInput = 1;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, true, false);
								}
								if (pInfo(peer)->DNAInput == 3) {
									int DNARemoved = pInfo(peer)->IDDNA1;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA1 = pInfo(peer)->IDDNA2;
									pInfo(peer)->IDDNA2 = pInfo(peer)->IDDNA3;
									pInfo(peer)->IDDNA3 = 0;
									pInfo(peer)->DNAInput = 2;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, true, false);
								}
							}
							if (cch.find("buttonClicked|remove1") != string::npos) {
								if (pInfo(peer)->DNAInput == 2) {
									int DNARemoved = pInfo(peer)->IDDNA2;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA2 = 0;
									pInfo(peer)->DNAInput = 1;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, true, false);
								}
								if (pInfo(peer)->DNAInput == 3) {
									int DNARemoved = pInfo(peer)->IDDNA2;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA2 = pInfo(peer)->IDDNA3;
									pInfo(peer)->IDDNA3 = 0;
									pInfo(peer)->DNAInput = 2;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, true, false);
								}
							}
							if (cch.find("buttonClicked|remove2") != string::npos) {
								if (pInfo(peer)->DNAInput == 3) {
									int DNARemoved = pInfo(peer)->IDDNA3;
									modify_inventory(peer, DNARemoved, add);
									pInfo(peer)->IDDNA3 = 0;
									pInfo(peer)->DNAInput = 2;
									SendDNAProcessor(peer, x_, y_, false, false, true, DNARemoved, true, false);
								}
							}
							if (cch.find("buttonClicked|combine") != string::npos) {
								if (pInfo(peer)->DNAInput == 3) {
									int DnaNumber1 = 0, DnaNumber2 = 0, DnaNumber3 = 0, What;
									ifstream infile("DnaRecipe.txt");
									for (string line; getline(infile, line);) {
										if (line.length() > 3 && line.at(0) != '/' && line.at(1) != '/') {
											auto ex = explode("|", line);
											int id1 = atoi(ex.at(0).c_str());
											int id2 = atoi(ex.at(1).c_str());
											int id3 = atoi(ex.at(2).c_str());
											if (id1 == pInfo(peer)->IDDNA1 && id2 == pInfo(peer)->IDDNA2 && id3 == pInfo(peer)->IDDNA3) {
												DnaNumber1 = atoi(ex.at(0).c_str());
												DnaNumber2 = atoi(ex.at(1).c_str());
												DnaNumber3 = atoi(ex.at(2).c_str());
												What = atoi(ex.at(3).c_str());
												break;
											}
										}
									}
									infile.close();
									if (pInfo(peer)->IDDNA1 == DnaNumber1 && pInfo(peer)->IDDNA2 == DnaNumber2 && pInfo(peer)->IDDNA3 == DnaNumber3 && DnaNumber3 != 0 && DnaNumber2 != 0 && DnaNumber1 != 0 && What != 0) {
										int count = items[What].blockType == BlockTypes::FOREGROUND ? 10 : 5;
										modify_inventory(peer, What, count);
										if (items[What].clothType == ClothTypes::FEET) pInfo(peer)->feet = What;
										else if (items[What].clothType == ClothTypes::HAND) pInfo(peer)->hand = What;
										pInfo(peer)->IDDNA1 = 0; pInfo(peer)->IDDNA2 = 0; pInfo(peer)->IDDNA3 = 0; pInfo(peer)->DNAInput = 0;
										gamepacket_t p, p2;
										p.Insert("OnConsoleMessage"), p.Insert("DNA Processing complete! The DNA combined into a `2" + items[What].name + "``!"), p.CreatePacket(peer);
										p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("DNA Processing complete! The DNA combined into a `2" + items[What].name + "``!"), p2.Insert(0), p2.Insert(0), p2.CreatePacket(peer);
										update_clothes(peer);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
											if (pInfo(currentPeer)->world == pInfo(peer)->world) {
												{
													gamepacket_t p;
													p.Insert("OnParticleEffect"); p.Insert(44); p.Insert((float)x_ * 32 + 16, (float)y_ * 32 + 16); p.CreatePacket(currentPeer);
												}
												{
													PlayerMoving data_{};
													data_.packetType = 19, data_.plantingTree = 150, data_.netID = pInfo(peer)->netID;
													data_.punchX = What, data_.punchY = What;
													int32_t to_netid = pInfo(peer)->netID;
													BYTE* raw = packPlayerMoving(&data_);
													raw[3] = 3;
													memcpy(raw + 8, &to_netid, 4);
													send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
													delete[]raw;
												}
											}
										}
									}
									else {
										SendDNAProcessor(peer, x_, y_, false, false, false, 0, true, true);
									}
								}
							}
						}
						if (cch.find("action|dialog_return\ndialog_name|dnaget") != string::npos) {
							int x_ = atoi(explode("\n", explode("tilex|", cch)[1])[0].c_str()), y_ = atoi(explode("\n", explode("tiley|", cch)[1])[0].c_str());
							int item = atoi(explode("\n", explode("item|", cch)[1])[0].c_str());
							int remove = -1;
							if (modify_inventory(peer, item, remove) == 0) {
								int Random = rand() % 100, reward = 0, count = 1;
								vector<int> list{ 4082,4084,4086,4088,4090,4092,4120,4122,5488 };
								gamepacket_t p, p2;
								p.Insert("OnTalkBubble"), p2.Insert("OnConsoleMessage"); p.Insert(pInfo(peer)->netID);
								if (Random >= 4 and Random <= 10) {
									reward = list[rand() % list.size()];
									p.Insert("You ground up a " + items[item].name + ", `9and found " + items[reward].name + " inside!``"), p2.Insert("You ground up a " + items[item].name + ", `9and found " + items[reward].name + " inside!``");
									modify_inventory(peer, reward, count);
								}
								else if (Random >= 1 and Random <= 3) {
									gamepacket_t a;
									a.Insert("OnConsoleMessage");
									a.Insert("Wow! You discovered the missing link between cave-rayman and the modern Growtopian.");
									reward = 5488;
									p.Insert("You ground up a " + items[item].name + ", `9and found " + items[reward].name + " inside!``"), p2.Insert("You ground up a " + items[item].name + ", `9and found " + items[reward].name + " inside!``");
									modify_inventory(peer, reward, count);
									a.CreatePacket(peer);
								}
								else {
									p.Insert("You ground up a " + items[item].name + ", `3but any DNA inside was destroyed in the process.``"), p2.Insert("You ground up a " + items[item].name + ", `3but any DNA inside was destroyed in the process.``");
								}
								p.Insert(0), p.Insert(0);
								p2.CreatePacket(peer), p.CreatePacket(peer);
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
									if (pInfo(currentPeer)->world == pInfo(peer)->world) {
										if (reward != 0) {
											packet_(currentPeer, "action|play_sfx\nfile|audio/bell.wav\ndelayMS|0");
											PlayerMoving data_{};
											data_.packetType = 19, data_.plantingTree = 150, data_.netID = pInfo(peer)->netID;
											data_.punchX = reward, data_.punchY = reward;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 3;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[]raw;
										}
										else {
											packet_(currentPeer, "action|play_sfx\nfile|audio/ch_start.wav\ndelayMS|0");
										}
									}
								}
							}
						}
						if (cch.find("action|dialog_return\ndialog_name|artifact_upgrade") != string::npos) {
							/*
							5104|Celestial Kaleidoscope
							5106|Harmonic Chimes
							5204|Plasma Globe
							5070|Crystallized Reality
							5072|Crystallized Wealth
							5074|Crystallized Brilliance
							5076|Crystallized Nature|999
							*/
							// Wisdom
							if (cch.find("buttonClicked|upgrade-5126") != string::npos) {
								SendArtifactUpgrade(peer, 5078, 1, 5126, 5104, 1, 5074, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5128") != string::npos) {
								SendArtifactUpgrade(peer, 5126, 1, 5128, 5106, 1, 5074, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5130") != string::npos) {
								SendArtifactUpgrade(peer, 5128, 2, 5130, 5204, 1, 5074, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5132") != string::npos) {
								SendArtifactUpgrade(peer, 5130, 2, 5132, 5104, 2, 5074, 2);
							}
							else if (cch.find("buttonClicked|upgrade-5134") != string::npos) {
								SendArtifactUpgrade(peer, 5132, 3, 5134, 5106, 3, 5074, 2);
							}
							// Tesseract
							else if (cch.find("buttonClicked|upgrade-5144") != string::npos) {
								SendArtifactUpgrade(peer, 5080, 1, 5144, 5204, 1, 5070, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5146") != string::npos) {
								SendArtifactUpgrade(peer, 5144, 1, 5146, 5104, 1, 5070, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5148") != string::npos) {
								SendArtifactUpgrade(peer, 5146, 2, 5148, 5106, 1, 5070, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5150") != string::npos) {
								SendArtifactUpgrade(peer, 5148, 2, 5150, 5204, 2, 5070, 2);
							}
							else if (cch.find("buttonClicked|upgrade-5152") != string::npos) {
								SendArtifactUpgrade(peer, 5150, 2, 5152, 5104, 3, 5070, 2);
							}
							// Seed of Life
							else if (cch.find("buttonClicked|upgrade-5162") != string::npos) {
								SendArtifactUpgrade(peer, 5082, 1, 5162, 5106, 1, 5076, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5164") != string::npos) {
								SendArtifactUpgrade(peer, 5162, 1, 5164, 5204, 1, 5076, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5166") != string::npos) {
								SendArtifactUpgrade(peer, 5164, 2, 5166, 5104, 1, 5076, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5168") != string::npos) {
								SendArtifactUpgrade(peer, 5166, 2, 5168, 5106, 2, 5076, 2);
							}
							else if (cch.find("buttonClicked|upgrade-5170") != string::npos) {
								SendArtifactUpgrade(peer, 5168, 2, 5170, 5204, 3, 5076, 2);
							}
							// Riches
							else if (cch.find("buttonClicked|upgrade-5180") != string::npos) {
								SendArtifactUpgrade(peer, 5084, 1, 5180, 5104, 2, 5072, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5182") != string::npos) {
								SendArtifactUpgrade(peer, 5180, 1, 5182, 5204, 2, 5072, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5184") != string::npos) {
								SendArtifactUpgrade(peer, 5182, 2, 5184, 5106, 2, 5072, 1);
							}
							else if (cch.find("buttonClicked|upgrade-5186") != string::npos) {
								SendArtifactUpgrade(peer, 5184, 2, 5186, 5104, 3, 5072, 2);
							}
							else if (cch.find("buttonClicked|upgrade-5188") != string::npos) {
								SendArtifactUpgrade(peer, 5186, 2, 5188, 5204, 3, 5072, 2);
							}
							// Orb
							else if (cch.find("buttonClicked|upgrade-7168") != string::npos) {
								SendArtifactUpgrade(peer, 7166, 1, 7168, 5106, 2, 7186, 1);
							}
							else if (cch.find("buttonClicked|upgrade-7170") != string::npos) {
								SendArtifactUpgrade(peer, 7168, 1, 7170, 5104, 1, 7186, 1);
							}
							else if (cch.find("buttonClicked|upgrade-7172") != string::npos) {
								SendArtifactUpgrade(peer, 7170, 2, 7172, 5204, 1, 7186, 1);
							}
							else if (cch.find("buttonClicked|upgrade-7174") != string::npos) {
								SendArtifactUpgrade(peer, 7172, 2, 7174, 5106, 2, 7186, 2);
							}
							else if (cch.find("buttonClicked|upgrade-9212") != string::npos) {
								SendArtifactUpgrade(peer, 7174, 2, 9212, 5104, 3, 7186, 2);
							}
							// Upgrading
							if (cch.find("buttonClicked|completecraft-" + to_string(pInfo(peer)->Upgradeto) + "") != string::npos) {
								int SoulStone = 0, Crystalized = 0, Celestial = 0, Riddles = 0, HaveAnces = 0;
								modify_inventory(peer, 5202, SoulStone);
								modify_inventory(peer, pInfo(peer)->IDCrystalized, Crystalized);
								modify_inventory(peer, pInfo(peer)->IDCeles, Celestial);
								modify_inventory(peer, pInfo(peer)->DailyRiddles, Riddles);
								modify_inventory(peer, pInfo(peer)->AncesID, HaveAnces);
								if (SoulStone >= pInfo(peer)->HowmuchSoulStone && Crystalized >= pInfo(peer)->JumlahCrystalized && Celestial >= pInfo(peer)->JumlahCeles && Riddles >= 5 && HaveAnces >= 1) {
									int del_a = -pInfo(peer)->HowmuchSoulStone, del_b = -pInfo(peer)->JumlahCrystalized, del_c = -pInfo(peer)->JumlahCeles, del_d = -5, del_e = -1;
									modify_inventory(peer, 5202, del_a);
									modify_inventory(peer, pInfo(peer)->IDCrystalized, del_b);
									modify_inventory(peer, pInfo(peer)->IDCeles, del_c);
									modify_inventory(peer, pInfo(peer)->DailyRiddles, del_d);
									modify_inventory(peer, pInfo(peer)->AncesID, del_e);
									/*if (pInfo(peer)->Upgradeto == 7174 || pInfo(peer)->Upgradeto == 5186 || pInfo(peer)->Upgradeto == 5150 || pInfo(peer)->Upgradeto == 5168 || pInfo(peer)->Upgradeto == 5132) {
										SendSuccesAchievement(peer, "Ancestral Being", false, "Ancestral Being", 137, 1);
									}*/
									if (pInfo(peer)->ances == pInfo(peer)->AncesID) {
										pInfo(peer)->ances = 0;
										update_clothes(peer);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED || currentPeer->data == NULL) continue;
											if (pInfo(currentPeer)->world == pInfo(peer)->world) {
												packet_(currentPeer, "action|play_sfx\nfile|audio/change_clothes.wav\ndelayMS|0");
											}
										}
									}
									int c_ = 1;
									if (modify_inventory(peer, pInfo(peer)->Upgradeto, c_) == 0) {
										gamepacket_t p;
										p.Insert("OnDialogRequest");
										p.Insert("add_label_with_icon|big|`9Ancient Goddess``|left|5086|\nadd_spacer|small|\nadd_textbox|`8You've pleased me, clever one.``|left|\nadd_spacer|small|\nend_dialog|artifact_upgrade|Return|");
										p.CreatePacket(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnDialogRequest");
										p.Insert("add_label_with_icon|big|`9Ancient Goddess``|left|5086|\nadd_spacer|small|\nadd_textbox|`8You didn't have enough inventory.``|left|\nadd_spacer|small|\nend_dialog|artifact_upgrade|Return|");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|dialog_minokawa_wings") != string::npos) {
							bool Minokawa_1 = atoi(explode("\n", explode("checkbox_minokawa_wings|", cch)[1])[0].c_str()), Minokawa_2 = atoi(explode("\n", explode("checkbox_minokawa_pet|", cch)[1])[0].c_str());
							if (Minokawa_1) pInfo(peer)->MKW = true;
							else pInfo(peer)->MKW = false;
							if (Minokawa_2) pInfo(peer)->MKP = true;
							else pInfo(peer)->MKP = false;
							update_clothes(peer);
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|halloween_tasks_popup_handle\nbuttonClicked|halloween_claim_task_reward_") != string::npos) {
							if (Halloween) {
								int Task = atoi(cch.substr(104, cch.length() - 104).c_str()), Give = 0;
								string Tasks = "";
								if (Task == 1) Give = 5, Tasks = "Sacrifice Dark King's Offering"; if (Task == 2) Give = 1, Tasks = "Purchase a Dark Ticket"; if (Task == 3) Give = 2, Tasks = "Purchase a Gift of Growganoth"; if (Task == 4) Give = 10, Tasks = "Purchase Weather Machine - Dark Mountains";
								int Total = Give;
								gamepacket_t p, p1;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p1.Insert("OnConsoleMessage");
								if (modify_inventory(peer, 12766, Give) == 0) {
									if (Task == 1) pInfo(peer)->Task_Darking--; if (Task == 2) pInfo(peer)->Task_Dark_Ticket--; if (Task == 3) pInfo(peer)->Task_Gift_Growganoth--; if (Task == 4) pInfo(peer)->Task_Mountain--;
									p.Insert("You received `2" + to_string(Total) + " Halloween Candy`` from Complete " + Tasks + " quest.");
									p1.Insert("You received `2" + to_string(Total) + " Halloween Candy`` from Complete " + Tasks + " quest.");
									Send_Halloween_Task(peer, pInfo(peer));
								}
								else p.Insert("You doesn't have enough inventory to claim this reward!"), p1.Insert("You doesn't have enough inventory to claim this reward!");
								p.Insert(0), p.Insert(1), p.CreatePacket(peer), p1.CreatePacket(peer);
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|dialog_rift_cape") != string::npos) {
							if (cch.find("buttonClicked|button_manual") != string::npos) {
								SendDialogRiftCape(peer, true);
								break;
							}
							else if (cch.find("buttonClicked|restore_default") != string::npos) {
								// Rift Cape
								pInfo(peer)->Time_Change = true;
								pInfo(peer)->Cycle_Time = 30;
								// Cape 1
								pInfo(peer)->Cape_R_0 = 147, pInfo(peer)->Cape_G_0 = 56, pInfo(peer)->Cape_B_0 = 143, pInfo(peer)->Collar_R_0 = 147, pInfo(peer)->Collar_G_0 = 56, pInfo(peer)->Collar_B_0 = 143;
								pInfo(peer)->Cape_Collar_0 = true, pInfo(peer)->Closed_Cape_0 = false, pInfo(peer)->Open_On_Move_0 = true, pInfo(peer)->Aura_0 = true, pInfo(peer)->Aura_1st_0 = false, pInfo(peer)->Aura_2nd_0 = false, pInfo(peer)->Aura_3rd_0 = true;
								// Cape 2
								pInfo(peer)->Cape_R_1 = 137, pInfo(peer)->Cape_G_1 = 30, pInfo(peer)->Cape_B_1 = 43, pInfo(peer)->Collar_R_1 = 34, pInfo(peer)->Collar_G_1 = 35, pInfo(peer)->Collar_B_1 = 63;
								pInfo(peer)->Cape_Collar_1 = true, pInfo(peer)->Closed_Cape_1 = true, pInfo(peer)->Open_On_Move_1 = false, pInfo(peer)->Aura_1 = true, pInfo(peer)->Aura_1st_1 = false, pInfo(peer)->Aura_2nd_1 = true, pInfo(peer)->Aura_3rd_1 = false;
								// Total
								pInfo(peer)->Cape_Value = 1952541691, pInfo(peer)->Cape_Collor_0_Value = 2402849791, pInfo(peer)->Cape_Collar_0_Value = 2402849791, pInfo(peer)->Cape_Collor_1_Value = 723421695, pInfo(peer)->Cape_Collar_1_Value = 1059267327;
								// End
								update_clothes(peer);
								break;
							}
							else {
								try {
									pInfo(peer)->Time_Change = atoi(explode("\n", explode("checkbox_time_cycle|", cch)[1])[0].c_str()) == 1 ? true : false;
									if (!is_number(explode("\n", explode("text_input_time_cycle|", cch)[1])[0])) break;
									pInfo(peer)->Cycle_Time = atoi(explode("\n", explode("text_input_time_cycle|", cch)[1])[0].c_str());
									{ // Cape_1
										pInfo(peer)->Cape_Collar_0 = atoi(explode("\n", explode("checkbox_cape_collar0|", cch)[1])[0].c_str()) == 1 ? true : false;
										{
											auto Cape_Color0 = explode(",", explode("\n", explode("text_input_cape_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_cape_color0|", cch)[1])[0].c_str());
											if (Cape_Color0.size() != 3 || t_.size() < 2 || Cape_Color0[0] == "" || Cape_Color0[1] == "" || Cape_Color0[2] == "" || Cape_Color0[0].empty() || Cape_Color0[1].empty() || Cape_Color0[2].empty()) {
												SendDialogRiftCape(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Cape_Color0[0]) || !is_number(Cape_Color0[1]) || !is_number(Cape_Color0[2]) || atoi(Cape_Color0[0].c_str()) > 255 || atoi(Cape_Color0[1].c_str()) > 255 || atoi(Cape_Color0[2].c_str()) > 255 || atoi(Cape_Color0[0].c_str()) < 0 || atoi(Cape_Color0[1].c_str()) < 0 || atoi(Cape_Color0[2].c_str()) < 0) {
												SendDialogRiftCape(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Cape_R_0 = atoi(Cape_Color0.at(0).c_str());
											pInfo(peer)->Cape_G_0 = atoi(Cape_Color0.at(1).c_str());
											pInfo(peer)->Cape_B_0 = atoi(Cape_Color0.at(2).c_str());
											pInfo(peer)->Cape_Collor_0_Value = 255 + (((256 * atoi(Cape_Color0.at(0).c_str())) + atoi(Cape_Color0.at(1).c_str()) * 65536) + atoi(Cape_Color0.at(2).c_str()) * (long long int)16777216);
										}
										{
											auto Cape_Collar_Color0 = explode(",", explode("\n", explode("text_input_collar_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_collar_color0|", cch)[1])[0].c_str());
											if (Cape_Collar_Color0.size() != 3 || t_.size() < 2 || Cape_Collar_Color0[0] == "" || Cape_Collar_Color0[1] == "" || Cape_Collar_Color0[2] == "" || Cape_Collar_Color0[0].empty() || Cape_Collar_Color0[1].empty() || Cape_Collar_Color0[2].empty()) {
												SendDialogRiftCape(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Cape_Collar_Color0[0]) || !is_number(Cape_Collar_Color0[1]) || !is_number(Cape_Collar_Color0[2]) || atoi(Cape_Collar_Color0[0].c_str()) > 255 || atoi(Cape_Collar_Color0[1].c_str()) > 255 || atoi(Cape_Collar_Color0[2].c_str()) > 255 || atoi(Cape_Collar_Color0[0].c_str()) < 0 || atoi(Cape_Collar_Color0[1].c_str()) < 0 || atoi(Cape_Collar_Color0[2].c_str()) < 0) {
												SendDialogRiftCape(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Collar_R_0 = atoi(Cape_Collar_Color0.at(0).c_str());
											pInfo(peer)->Collar_G_0 = atoi(Cape_Collar_Color0.at(1).c_str());
											pInfo(peer)->Collar_B_0 = atoi(Cape_Collar_Color0.at(2).c_str());
											pInfo(peer)->Cape_Collar_0_Value = 255 + (((256 * atoi(Cape_Collar_Color0.at(0).c_str())) + atoi(Cape_Collar_Color0.at(1).c_str()) * 65536) + atoi(Cape_Collar_Color0.at(2).c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Closed_Cape_0 = atoi(explode("\n", explode("checkbox_closed_cape0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Open_On_Move_0 = atoi(explode("\n", explode("checkbox_open_on_move0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_0 = atoi(explode("\n", explode("checkbox_aura0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_1st_0 = atoi(explode("\n", explode("checkbox_aura_1st0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_2nd_0 = atoi(explode("\n", explode("checkbox_aura_2nd0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_3rd_0 = atoi(explode("\n", explode("checkbox_aura_3rd0|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
									{ // Cape_2
										pInfo(peer)->Cape_Collar_1 = atoi(explode("\n", explode("checkbox_cape_collar1|", cch)[1])[0].c_str()) == 1 ? true : false;
										{
											auto Cape_Color1 = explode(",", explode("\n", explode("text_input_cape_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_cape_color1|", cch)[1])[0].c_str());
											if (Cape_Color1.size() != 3 || t_.size() < 2 || Cape_Color1[0] == "" || Cape_Color1[1] == "" || Cape_Color1[2] == "" || Cape_Color1[0].empty() || Cape_Color1[1].empty() || Cape_Color1[2].empty()) {
												SendDialogRiftCape(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Cape_Color1[0]) || !is_number(Cape_Color1[1]) || !is_number(Cape_Color1[2]) || atoi(Cape_Color1[0].c_str()) > 255 || atoi(Cape_Color1[1].c_str()) > 255 || atoi(Cape_Color1[2].c_str()) > 255 || atoi(Cape_Color1[0].c_str()) < 0 || atoi(Cape_Color1[1].c_str()) < 0 || atoi(Cape_Color1[2].c_str()) < 0) {
												SendDialogRiftCape(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Cape_R_1 = atoi(Cape_Color1.at(0).c_str());
											pInfo(peer)->Cape_G_1 = atoi(Cape_Color1.at(1).c_str());
											pInfo(peer)->Cape_B_1 = atoi(Cape_Color1.at(2).c_str());
											pInfo(peer)->Cape_Collor_1_Value = 255 + (((256 * atoi(Cape_Color1.at(0).c_str())) + atoi(Cape_Color1.at(1).c_str()) * 65536) + atoi(Cape_Color1.at(2).c_str()) * (long long int)16777216);
										}
										{
											auto Cape_Collar_Color1 = explode(",", explode("\n", explode("text_input_collar_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_collar_color1|", cch)[1])[0].c_str());
											if (Cape_Collar_Color1.size() != 3 || t_.size() < 2 || Cape_Collar_Color1[0] == "" || Cape_Collar_Color1[1] == "" || Cape_Collar_Color1[2] == "" || Cape_Collar_Color1[0].empty() || Cape_Collar_Color1[1].empty() || Cape_Collar_Color1[2].empty()) {
												SendDialogRiftCape(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Cape_Collar_Color1[0]) || !is_number(Cape_Collar_Color1[1]) || !is_number(Cape_Collar_Color1[2]) || atoi(Cape_Collar_Color1[0].c_str()) > 255 || atoi(Cape_Collar_Color1[1].c_str()) > 255 || atoi(Cape_Collar_Color1[2].c_str()) > 255 || atoi(Cape_Collar_Color1[0].c_str()) < 0 || atoi(Cape_Collar_Color1[1].c_str()) < 0 || atoi(Cape_Collar_Color1[2].c_str()) < 0) {
												SendDialogRiftCape(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Collar_R_1 = atoi(Cape_Collar_Color1.at(0).c_str());
											pInfo(peer)->Collar_G_1 = atoi(Cape_Collar_Color1.at(1).c_str());
											pInfo(peer)->Collar_B_1 = atoi(Cape_Collar_Color1.at(2).c_str());
											pInfo(peer)->Cape_Collar_1_Value = 255 + (((256 * atoi(Cape_Collar_Color1.at(0).c_str())) + atoi(Cape_Collar_Color1.at(1).c_str()) * 65536) + atoi(Cape_Collar_Color1.at(2).c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Closed_Cape_1 = atoi(explode("\n", explode("checkbox_closed_cape1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Open_On_Move_1 = atoi(explode("\n", explode("checkbox_open_on_move1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_1 = atoi(explode("\n", explode("checkbox_aura1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_1st_1 = atoi(explode("\n", explode("checkbox_aura_1st1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_2nd_1 = atoi(explode("\n", explode("checkbox_aura_2nd1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Aura_3rd_1 = atoi(explode("\n", explode("checkbox_aura_3rd1|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
								}
								catch (...) {
									break;
								}
								int TValue = 0;
								if (pInfo(peer)->Time_Change) TValue += 4096;
								if (pInfo(peer)->Cape_Collar_0) TValue += 1;
								if (pInfo(peer)->Cape_Collar_1) TValue += 2;
								if (pInfo(peer)->Closed_Cape_0) TValue += 4;
								if (pInfo(peer)->Closed_Cape_1) TValue += 8;
								if (pInfo(peer)->Open_On_Move_0) TValue += 16;
								if (pInfo(peer)->Open_On_Move_1) TValue += 32;
								if (pInfo(peer)->Aura_0) TValue += 64;
								if (pInfo(peer)->Aura_1) TValue += 128;
								if (pInfo(peer)->Aura_1st_0) TValue += 256;
								if (pInfo(peer)->Aura_1st_1) TValue += 1024;
								if (pInfo(peer)->Aura_2nd_0) TValue += 256 * 2;
								if (pInfo(peer)->Aura_2nd_1) TValue += 1024 * 2;
								if (pInfo(peer)->Aura_3rd_0) TValue += 256 * 3;
								if (pInfo(peer)->Aura_3rd_1) TValue += 1024 * 3;
								pInfo(peer)->Cape_Value = TValue;
								update_clothes(peer);
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|dialog_cernuous_mask") != string::npos) {
							if (cch.find("buttonClicked|restore_default") != string::npos) {
								pInfo(peer)->Aura_Season = 2;
								pInfo(peer)->Trail_Season = 2;
								update_clothes(peer);
								break;
							}
							else {
								pInfo(peer)->Aura_Season = (atoi(explode("\n", explode("checkbox_none0|", cch)[1])[0].c_str()) == 1 ? 0 : (atoi(explode("\n", explode("checkbox_spring0|", cch)[1])[0].c_str()) == 1 ? 1 : (atoi(explode("\n", explode("checkbox_summer0|", cch)[1])[0].c_str()) == 1 ? 2 : (atoi(explode("\n", explode("checkbox_autumn0|", cch)[1])[0].c_str()) == 1 ? 3 : (atoi(explode("\n", explode("checkbox_winter0|", cch)[1])[0].c_str()) == 1 ? 4 : 0)))));
								pInfo(peer)->Trail_Season = (atoi(explode("\n", explode("checkbox_none1|", cch)[1])[0].c_str()) == 1 ? 0 : (atoi(explode("\n", explode("checkbox_spring1|", cch)[1])[0].c_str()) == 1 ? 1 : (atoi(explode("\n", explode("checkbox_summer1|", cch)[1])[0].c_str()) == 1 ? 2 : (atoi(explode("\n", explode("checkbox_autumn1|", cch)[1])[0].c_str()) == 1 ? 3 : (atoi(explode("\n", explode("checkbox_winter1|", cch)[1])[0].c_str()) == 1 ? 4 : 0)))));
								update_clothes(peer);
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|sessionlength_edit") != string::npos) {
							try {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									if (pInfo(peer)->tankIDName != world_->owner_name) break;
									world_->World_Time = (atoi(explode("\n", explode("checkbox_5|", cch)[1])[0].c_str()) == 1 ? 5 : (atoi(explode("\n", explode("checkbox_10|", cch)[1])[0].c_str()) == 1 ? 10 : (atoi(explode("\n", explode("checkbox_20|", cch)[1])[0].c_str()) == 1 ? 20 : (atoi(explode("\n", explode("checkbox_30|", cch)[1])[0].c_str()) == 1 ? 30 : (atoi(explode("\n", explode("checkbox_40|", cch)[1])[0].c_str()) == 1 ? 40 : (atoi(explode("\n", explode("checkbox_50|", cch)[1])[0].c_str()) == 1 ? 50 : (atoi(explode("\n", explode("checkbox_60|", cch)[1])[0].c_str()) == 1 ? 60 : 0)))))));
									gamepacket_t p;
									p.Insert("OnTalkBubble"); p.Insert(pInfo(peer)->netID);
									p.Insert((world_->World_Time == 0 ? "World Timer limit removed!" : "World Timer limit set to `2" + to_string(world_->World_Time) + " minutes``."));
									p.Insert(0); p.Insert(0); p.CreatePacket(peer);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->world == world_->name) {
											if (pInfo(currentPeer)->tankIDName != world_->owner_name && world_->World_Time != 0) {
												pInfo(currentPeer)->World_Timed = time(nullptr) + (world_->World_Time * 60);
												pInfo(currentPeer)->WorldTimed = true;
											}
										}
									}
								}
							}
							catch (out_of_range) {
								cout << "Server error invalid (out of range) on " + cch << endl;
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|worldcategory") != string::npos) {
							string Cat = "None";
							if (cch.find("buttonClicked|cat10") != string::npos) {
								Cat = "Storage";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|10\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat11") != string::npos) {
								Cat = "Story";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|11\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat12") != string::npos) {
								Cat = "Trade";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|12\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat14") != string::npos) {
								Cat = "Puzzle";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|14\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat15") != string::npos) {
								Cat = "Music";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|15\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat0") != string::npos) {
								Cat = "None";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|0\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat1") != string::npos) {
								Cat = "Adventure";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|1\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat2") != string::npos) {
								Cat = "Art";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|2\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat3") != string::npos) {
								Cat = "Farm";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|3\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat4") != string::npos) {
								Cat = "Game";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|4\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat5") != string::npos) {
								Cat = "Information";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|5\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat6") != string::npos) {
								Cat = "Parkour";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|6\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat7") != string::npos) {
								Cat = "Roleplay";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|7\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat8") != string::npos) {
								Cat = "Shop";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|8\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|cat9") != string::npos) {
								Cat = "Social";
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSet World Category``|left|3802|\nembed_data|chosencat|9\nadd_textbox|Are you sure you want to change your world's category to " + Cat + "?|left|\nadd_smalltext|`4Warning:`` Changing your category will delete all ratings on your world .|left|\nend_dialog|worldcategory|Nevermind|Change Category|");
								p.CreatePacket(peer);
								break;
							}
							else {
								int Category = atoi(explode("\n", explode("chosencat|", cch)[1])[0].c_str());
								if (Category < 0 or Category > 15) break;
								string Cat = "None";
								if (Category == 0) Cat = "None"; if (Category == 1) Cat = "Adventure"; if (Category == 2) Cat = "Art"; if (Category == 3) Cat = "Farm"; if (Category == 4) Cat = "Game"; if (Category == 5) Cat = "Information"; if (Category == 15) Cat = "Music"; if (Category == 6) Cat = "Parkour"; if (Category == 14) Cat = "Puzzle"; if (Category == 7) Cat = "Roleplay"; if (Category == 8) Cat = "Shop"; if (Category == 9) Cat = "Social"; if (Category == 10) Cat = "Storage"; if (Category == 11) Cat = "Story"; if (Category == 12) Cat = "Trade";
								try {
									string name_ = pInfo(peer)->world;
									vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
									if (p != worlds.end()) {
										World* world_ = &worlds[p - worlds.begin()];
										if (pInfo(peer)->tankIDName != world_->owner_name) break;
										world_->World_Rating = 0;
										world_->Category = Cat;
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("This world has been moved to the '" + Cat + "' category! Everyone, please type `2/rate`` to rate it from 1-5 stars.");
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(currentPeer)->world == world_->name) {
												p.CreatePacket(currentPeer);
											}
										}
									}
								}
								catch (out_of_range) {
									cout << "Server error invalid (out of range) on " + cch << endl;
									break;
								}
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|bannerbandolier") != string::npos) {
							if (cch.find("buttonClicked|patternpicker") != string::npos) {
								string dialog = "";
								if (pInfo(peer)->Banner_Flag == 0) dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|5838|";
								else if (pInfo(peer)->Banner_Flag == 1) dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|5844|";
								else if (pInfo(peer)->Banner_Flag == 2) dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|5848|";
								else if (pInfo(peer)->Banner_Flag == 3) dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|5846|";
								else if (pInfo(peer)->Banner_Flag == 4) dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|5842|";
								dialog += "\nadd_spacer|small|\nadd_textbox|Pick a pattern for your banner.|left|\nadd_spacer|small|";
								dialog += "\nadd_label_with_icon_button|big|Harlequin|left|5838|pattern_1|\nadd_spacer|small|";
								dialog += "\nadd_label_with_icon_button|big|Slant|left|5844|pattern_2|\nadd_spacer|small|";
								dialog += "\nadd_label_with_icon_button|big|Stripe|left|5848|pattern_3|\nadd_spacer|small|";
								dialog += "\nadd_label_with_icon_button|big|Panel|left|5846|pattern_4|\nadd_spacer|small|";
								dialog += "\nadd_label_with_icon_button|big|Cross|left|5842|pattern_5|\nadd_spacer|small|";
								dialog += "\nend_dialog|bannerbandolier|Cancel||\nadd_quick_exit|";
								gamepacket_t p;
								p.Insert("OnDialogRequest"), p.Insert(dialog), p.CreatePacket(peer);
								break;
							}
							else if (cch.find("buttonClicked|pattern_") != string::npos) {
								int Pattern = atoi(cch.substr(49 + 22, cch.length() - 49 + 22).c_str());
								pInfo(peer)->CBanner_Item = pInfo(peer)->Banner_Item;
								pInfo(peer)->CBanner_Flag = Pattern - 1;
								SendBannerBandolier2(peer);
								break;
							}
							else if (cch.find("buttonClicked|reset") != string::npos) {
								pInfo(peer)->CBanner_Item = 0; pInfo(peer)->CBanner_Flag = 0; pInfo(peer)->Banner_Item = 0; pInfo(peer)->Banner_Flag = 0;
								SendBannerBandolier2(peer);
								break;
							}
							else if (!(cch.find("buttonClicked|patternpicker") != string::npos || cch.find("buttonClicked|pattern_") != string::npos || cch.find("\nbanneritem|") != string::npos)) {
								if (pInfo(peer)->CBanner_Item != 0) pInfo(peer)->Banner_Item = pInfo(peer)->CBanner_Item;
								if (pInfo(peer)->CBanner_Flag != 0) pInfo(peer)->Banner_Flag = pInfo(peer)->CBanner_Flag;
								pInfo(peer)->CBanner_Item = 0; pInfo(peer)->CBanner_Flag = 0;
								update_clothes(peer);
								break;
							}
							else {
								pInfo(peer)->CBanner_Flag = pInfo(peer)->CBanner_Flag;
								pInfo(peer)->CBanner_Item = atoi(explode("\n", explode("banneritem|", cch)[1])[0].c_str());
								SendBannerBandolier2(peer);
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|magic_compass") != string::npos) {
							if (cch.find("buttonClicked|clear_item") != string::npos) {
								pInfo(peer)->Magnet_Item = 0;
								update_clothes(peer);
								gamepacket_t p;
								p.Insert("OnMagicCompassTrackingItemIDChanged");
								p.Insert(pInfo(peer)->Magnet_Item);
								p.CreatePacket(peer);
								break;
							}
							else {
								pInfo(peer)->Magnet_Item = atoi(explode("\n", explode("magic_compass_item|", cch)[1])[0].c_str());
								update_clothes(peer);
								gamepacket_t p;
								p.Insert("OnMagicCompassTrackingItemIDChanged");
								p.Insert(atoi(explode("\n", explode("magic_compass_item|", cch)[1])[0].c_str()));
								p.CreatePacket(peer);
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|dialog_rift_wings") != string::npos) {
							if (cch.find("buttonClicked|button_manual") != string::npos) {
								SendDialogRiftWings(peer, true);
								break;
							}
							else if (cch.find("buttonClicked|restore_default") != string::npos) {
								// Rift Wings
								pInfo(peer)->Wing_Time_Change = true;
								pInfo(peer)->Wing_Cycle_Time = 30;
								// Wings 1
								pInfo(peer)->Wing_R_0 = 93, pInfo(peer)->Wing_G_0 = 22, pInfo(peer)->Wing_B_0 = 200, pInfo(peer)->Wing_Metal_R_0 = 220, pInfo(peer)->Wing_Metal_G_0 = 72, pInfo(peer)->Wing_Metal_B_0 = 255;
								pInfo(peer)->Open_Wing_0 = false, pInfo(peer)->Closed_Wing_0 = false, pInfo(peer)->Trail_On_0 = true, pInfo(peer)->Stamp_Particle_0 = true, pInfo(peer)->Trail_1st_0 = true, pInfo(peer)->Trail_2nd_0 = false, pInfo(peer)->Trail_3rd_0 = false, pInfo(peer)->Material_1st_0 = true, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = false;
								// Wings 2
								pInfo(peer)->Wing_R_1 = 137, pInfo(peer)->Wing_G_1 = 30, pInfo(peer)->Wing_B_1 = 43, pInfo(peer)->Wing_Metal_R_1 = 34, pInfo(peer)->Wing_Metal_G_1 = 35, pInfo(peer)->Wing_Metal_B_1 = 65;
								pInfo(peer)->Open_Wing_1 = false, pInfo(peer)->Closed_Wing_1 = false, pInfo(peer)->Trail_On_1 = true, pInfo(peer)->Stamp_Particle_1 = false, pInfo(peer)->Trail_1st_1 = false, pInfo(peer)->Trail_2nd_1 = true, pInfo(peer)->Trail_3rd_1 = false, pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = true, pInfo(peer)->Material_3rd_1 = false;
								// Total
								pInfo(peer)->Wing_Value = 104912, pInfo(peer)->Wing_Color_0_Value = 3356909055, pInfo(peer)->Wing_Metal_0_Value = 4282965247, pInfo(peer)->Wing_Color_1_Value = 723421695, pInfo(peer)->Wing_Metal_1_Value = 1059267327;
								// End
								update_clothes(peer);
								break;
							}
							else {
								try {
									pInfo(peer)->Wing_Time_Change = atoi(explode("\n", explode("checkbox_time_cycle|", cch)[1])[0].c_str()) == 1 ? true : false;
									if (!is_number(explode("\n", explode("text_input_time_cycle|", cch)[1])[0])) break;
									pInfo(peer)->Wing_Cycle_Time = atoi(explode("\n", explode("text_input_time_cycle|", cch)[1])[0].c_str());
									{ // Wing_1
										{
											auto Wings_Color0 = explode(",", explode("\n", explode("text_input_wings_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_wings_color0|", cch)[1])[0].c_str());
											if (Wings_Color0.size() != 3 || t_.size() < 2 || Wings_Color0[0] == "" || Wings_Color0[1] == "" || Wings_Color0[2] == "" || Wings_Color0[0].empty() || Wings_Color0[1].empty() || Wings_Color0[2].empty()) {
												SendDialogRiftWings(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Wings_Color0[0]) || !is_number(Wings_Color0[1]) || !is_number(Wings_Color0[2]) || atoi(Wings_Color0[0].c_str()) > 255 || atoi(Wings_Color0[1].c_str()) > 255 || atoi(Wings_Color0[2].c_str()) > 255 || atoi(Wings_Color0[0].c_str()) < 0 || atoi(Wings_Color0[1].c_str()) < 0 || atoi(Wings_Color0[2].c_str()) < 0) {
												SendDialogRiftWings(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Wing_R_0 = atoi(Wings_Color0.at(0).c_str());
											pInfo(peer)->Wing_G_0 = atoi(Wings_Color0.at(1).c_str());
											pInfo(peer)->Wing_B_0 = atoi(Wings_Color0.at(2).c_str());
											pInfo(peer)->Wing_Color_0_Value = 255 + (((256 * atoi(Wings_Color0.at(0).c_str())) + atoi(Wings_Color0.at(1).c_str()) * 65536) + atoi(Wings_Color0.at(2).c_str()) * (long long int)16777216);
										}
										{
											auto Metal_Color0 = explode(",", explode("\n", explode("text_input_metal_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_metal_color0|", cch)[1])[0].c_str());
											if (Metal_Color0.size() != 3 || t_.size() < 2 || Metal_Color0[0] == "" || Metal_Color0[1] == "" || Metal_Color0[2] == "" || Metal_Color0[0].empty() || Metal_Color0[1].empty() || Metal_Color0[2].empty()) {
												SendDialogRiftWings(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Metal_Color0[0]) || !is_number(Metal_Color0[1]) || !is_number(Metal_Color0[2]) || atoi(Metal_Color0[0].c_str()) > 255 || atoi(Metal_Color0[1].c_str()) > 255 || atoi(Metal_Color0[2].c_str()) > 255 || atoi(Metal_Color0[0].c_str()) < 0 || atoi(Metal_Color0[1].c_str()) < 0 || atoi(Metal_Color0[2].c_str()) < 0) {
												SendDialogRiftWings(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Wing_Metal_R_0 = atoi(Metal_Color0.at(0).c_str());
											pInfo(peer)->Wing_Metal_G_0 = atoi(Metal_Color0.at(1).c_str());
											pInfo(peer)->Wing_Metal_B_0 = atoi(Metal_Color0.at(2).c_str());
											pInfo(peer)->Wing_Metal_0_Value = 255 + (((256 * atoi(Metal_Color0.at(0).c_str())) + atoi(Metal_Color0.at(1).c_str()) * 65536) + atoi(Metal_Color0.at(2).c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Open_Wing_0 = atoi(explode("\n", explode("checkbox_open_wings0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Closed_Wing_0 = atoi(explode("\n", explode("checkbox_closed_wings0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Stamp_Particle_0 = atoi(explode("\n", explode("checkbox_stamp_particle0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_On_0 = atoi(explode("\n", explode("checkbox_trail0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_1st_0 = atoi(explode("\n", explode("checkbox_trail_1st0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_2nd_0 = atoi(explode("\n", explode("checkbox_trail_2nd0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_3rd_0 = atoi(explode("\n", explode("checkbox_trail_3rd0|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
									{ // Wing_2
										{
											auto Wings_Color1 = explode(",", explode("\n", explode("text_input_wings_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_wings_color1|", cch)[1])[0].c_str());
											if (Wings_Color1.size() != 3 || t_.size() < 2 || Wings_Color1[0] == "" || Wings_Color1[1] == "" || Wings_Color1[2] == "" || Wings_Color1[0].empty() || Wings_Color1[1].empty() || Wings_Color1[2].empty()) {
												SendDialogRiftWings(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Wings_Color1[0]) || !is_number(Wings_Color1[1]) || !is_number(Wings_Color1[2]) || atoi(Wings_Color1[0].c_str()) > 255 || atoi(Wings_Color1[1].c_str()) > 255 || atoi(Wings_Color1[2].c_str()) > 255 || atoi(Wings_Color1[0].c_str()) < 0 || atoi(Wings_Color1[1].c_str()) < 0 || atoi(Wings_Color1[2].c_str()) < 0) {
												SendDialogRiftWings(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Wing_R_1 = atoi(Wings_Color1.at(0).c_str());
											pInfo(peer)->Wing_G_1 = atoi(Wings_Color1.at(1).c_str());
											pInfo(peer)->Wing_B_1 = atoi(Wings_Color1.at(2).c_str());
											pInfo(peer)->Wing_Color_1_Value = 255 + (((256 * atoi(Wings_Color1.at(0).c_str())) + atoi(Wings_Color1.at(1).c_str()) * 65536) + atoi(Wings_Color1.at(2).c_str()) * (long long int)16777216);
										}
										{
											auto Metal_Color1 = explode(",", explode("\n", explode("text_input_metal_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_metal_color1|", cch)[1])[0].c_str());
											if (Metal_Color1.size() != 3 || t_.size() < 2 || Metal_Color1[0] == "" || Metal_Color1[1] == "" || Metal_Color1[2] == "" || Metal_Color1[0].empty() || Metal_Color1[1].empty() || Metal_Color1[2].empty()) {
												SendDialogRiftWings(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Metal_Color1[0]) || !is_number(Metal_Color1[1]) || !is_number(Metal_Color1[2]) || atoi(Metal_Color1[0].c_str()) > 255 || atoi(Metal_Color1[1].c_str()) > 255 || atoi(Metal_Color1[2].c_str()) > 255 || atoi(Metal_Color1[0].c_str()) < 0 || atoi(Metal_Color1[1].c_str()) < 0 || atoi(Metal_Color1[2].c_str()) < 0) {
												SendDialogRiftWings(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Wing_Metal_R_1 = atoi(Metal_Color1.at(0).c_str());
											pInfo(peer)->Wing_Metal_G_1 = atoi(Metal_Color1.at(1).c_str());
											pInfo(peer)->Wing_Metal_B_1 = atoi(Metal_Color1.at(2).c_str());
											pInfo(peer)->Wing_Metal_1_Value = 255 + (((256 * atoi(Metal_Color1.at(0).c_str())) + atoi(Metal_Color1.at(1).c_str()) * 65536) + atoi(Metal_Color1.at(2).c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Open_Wing_1 = atoi(explode("\n", explode("checkbox_open_wings1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Closed_Wing_1 = atoi(explode("\n", explode("checkbox_closed_wings1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Stamp_Particle_1 = atoi(explode("\n", explode("checkbox_stamp_particle1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_On_1 = atoi(explode("\n", explode("checkbox_trail1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_1st_1 = atoi(explode("\n", explode("checkbox_trail_1st1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_2nd_1 = atoi(explode("\n", explode("checkbox_trail_2nd1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Trail_3rd_1 = atoi(explode("\n", explode("checkbox_trail_3rd1|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
								}
								catch (...) {
									break;
								}
								int Feathers_0 = atoi(explode("\n", explode("checkbox_material_1st0|", cch)[1])[0].c_str()), Blades_0 = atoi(explode("\n", explode("checkbox_material_2nd0|", cch)[1])[0].c_str()), Scael_0 = atoi(explode("\n", explode("checkbox_material_3rd0|", cch)[1])[0].c_str());
								int Feathers_1 = atoi(explode("\n", explode("checkbox_material_1st1|", cch)[1])[0].c_str()), Blades_1 = atoi(explode("\n", explode("checkbox_material_2nd1|", cch)[1])[0].c_str()), Scael_1 = atoi(explode("\n", explode("checkbox_material_3rd1|", cch)[1])[0].c_str());
								int TValue = 0;
								if (Feathers_0 == 0 && Blades_0 == 0 && Scael_0 == 0) {
									SendDialogRiftWings(peer, false, "You need to select one material for style 1");
									break;
								}
								if (Feathers_1 == 0 && Blades_1 == 0 && Scael_1 == 0) {
									SendDialogRiftWings(peer, false, "You need to select one material for style 2");
									break;
								}
								if (Feathers_0 == 1 && Feathers_1 == 1) {
									TValue += 20480;
									pInfo(peer)->Material_1st_0 = true, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = true, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Feathers_0 == 1 && Blades_1 == 1) {
									TValue += 20480 + 16384;
									pInfo(peer)->Material_1st_0 = true, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = true, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Feathers_0 == 1 && Scael_1 == 1) {
									TValue += 20480 + (16384 * 2);
									pInfo(peer)->Material_1st_0 = true, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = true;
								}
								if (Blades_0 == 1 && Feathers_1 == 1) {
									TValue += 20480 + 4096;
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = true, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = true, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Blades_0 == 1 && Blades_1 == 1) {
									TValue += 20480 + 4096 + 16384;
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = true, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = true, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Blades_0 == 1 && Scael_1 == 1) {
									TValue += 20480 + 4096 + (16384 * 2);
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = true, pInfo(peer)->Material_3rd_0 = false;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = true;
								}
								if (Scael_0 == 1 && Feathers_1 == 1) {
									TValue += 20480 + (4096 * 2);
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = true;
									pInfo(peer)->Material_1st_1 = true, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Scael_0 == 1 && Blades_1 == 1) {
									TValue += 20480 + (4096 * 2) + 16384;
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = true;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = true, pInfo(peer)->Material_3rd_1 = false;
								}
								if (Scael_0 == 1 && Scael_1 == 1) {
									TValue += 20480 + (4096 * 2) + (16384 * 2);
									pInfo(peer)->Material_1st_0 = false, pInfo(peer)->Material_2nd_0 = false, pInfo(peer)->Material_3rd_0 = true;
									pInfo(peer)->Material_1st_1 = false, pInfo(peer)->Material_2nd_1 = false, pInfo(peer)->Material_3rd_1 = true;
								}
								if (pInfo(peer)->Wing_Time_Change) TValue += 65536;
								if (pInfo(peer)->Open_Wing_0) TValue += 1;
								if (pInfo(peer)->Open_Wing_1) TValue += 2;
								if (pInfo(peer)->Closed_Wing_0) TValue += 4;
								if (pInfo(peer)->Closed_Wing_1) TValue += 8;
								if (pInfo(peer)->Trail_On_0) TValue += 16;
								if (pInfo(peer)->Trail_On_1) TValue += 32;
								if (pInfo(peer)->Stamp_Particle_0) TValue += 64;
								if (pInfo(peer)->Stamp_Particle_1) TValue += 128;
								if (pInfo(peer)->Trail_1st_0) TValue += 256;
								if (pInfo(peer)->Trail_1st_1) TValue += 1024;
								if (pInfo(peer)->Trail_2nd_0) TValue += 256 * 2;
								if (pInfo(peer)->Trail_2nd_1) TValue += 1024 * 2;
								if (pInfo(peer)->Trail_3rd_0) TValue += 256 * 3;
								if (pInfo(peer)->Trail_3rd_1) TValue += 1024 * 3;
								pInfo(peer)->Wing_Value = TValue;
								update_clothes(peer);
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|dialog_infinity_crown") != string::npos) {
							if (cch.find("buttonClicked|button_manual") != string::npos) {
								SendDialogInfinityCrown(peer, true);
								break;
							}
							else if (cch.find("buttonClicked|restore_default") != string::npos) {
								// Infinity Crown
								pInfo(peer)->Crown_Time_Change = true;
								pInfo(peer)->Crown_Cycle_Time = 15;
								// Crown 1
								pInfo(peer)->Base_R_0 = 255, pInfo(peer)->Base_G_0 = 255, pInfo(peer)->Base_B_0 = 255;
								pInfo(peer)->Gem_R_0 = 255, pInfo(peer)->Gem_G_0 = 0, pInfo(peer)->Gem_B_0 = 255;
								pInfo(peer)->Crystal_R_0 = 0, pInfo(peer)->Crystal_G_0 = 205, pInfo(peer)->Crystal_B_0 = 249;
								pInfo(peer)->Crown_Floating_Effect_0 = false, pInfo(peer)->Crown_Laser_Beam_0 = true, pInfo(peer)->Crown_Crystals_0 = true, pInfo(peer)->Crown_Rays_0 = true;
								// Crown 2
								pInfo(peer)->Base_R_1 = 255, pInfo(peer)->Base_G_1 = 200, pInfo(peer)->Base_B_1 = 37;
								pInfo(peer)->Gem_R_1 = 255, pInfo(peer)->Gem_G_1 = 0, pInfo(peer)->Gem_B_1 = 64;
								pInfo(peer)->Crystal_R_1 = 26, pInfo(peer)->Crystal_G_1 = 45, pInfo(peer)->Crystal_B_1 = 140;
								pInfo(peer)->Crown_Floating_Effect_1 = false, pInfo(peer)->Crown_Laser_Beam_1 = true, pInfo(peer)->Crown_Crystals_1 = true, pInfo(peer)->Crown_Rays_1 = true;
								// Total
								pInfo(peer)->Crown_Value = 1768716607;
								pInfo(peer)->Crown_Value_0_0 = 4294967295, pInfo(peer)->Crown_Value_0_1 = 4278255615, pInfo(peer)->Crown_Value_0_2 = 4190961919;
								pInfo(peer)->Crown_Value_1_0 = 633929727, pInfo(peer)->Crown_Value_1_1 = 1073807359, pInfo(peer)->Crown_Value_1_2 = 2351766271;
								// End
								update_clothes(peer);
								break;
							}
							else {
								try {
									pInfo(peer)->Crown_Time_Change = atoi(explode("\n", explode("checkbox_time_cycle|", cch)[1])[0].c_str()) == 1 ? true : false;
									if (!is_number(explode("\n", explode("text_input_time_cycle|", cch)[1])[0])) break;
									pInfo(peer)->Crown_Cycle_Time = atoi(explode("\n", explode("text_input_time_cycle|", cch)[1])[0].c_str());
									{ // Crown 1
										pInfo(peer)->Crown_Floating_Effect_0 = atoi(explode("\n", explode("checkbox_floating0|", cch)[1])[0].c_str()) == 1 ? true : false;
										{
											auto Crown_Base_0 = explode(",", explode("\n", explode("text_input_base_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_base_color0|", cch)[1])[0].c_str());
											if (Crown_Base_0.size() != 3 || t_.size() < 2 || Crown_Base_0[0] == "" || Crown_Base_0[1] == "" || Crown_Base_0[2] == "" || Crown_Base_0[0].empty() || Crown_Base_0[1].empty() || Crown_Base_0[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Crown_Base_0[0]) || !is_number(Crown_Base_0[1]) || !is_number(Crown_Base_0[2]) || atoi(Crown_Base_0[0].c_str()) > 255 || atoi(Crown_Base_0[1].c_str()) > 255 || atoi(Crown_Base_0[2].c_str()) > 255 || atoi(Crown_Base_0[0].c_str()) < 0 || atoi(Crown_Base_0[1].c_str()) < 0 || atoi(Crown_Base_0[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Base_R_0 = atoi(Crown_Base_0[0].c_str());
											pInfo(peer)->Base_G_0 = atoi(Crown_Base_0[1].c_str());
											pInfo(peer)->Base_B_0 = atoi(Crown_Base_0[2].c_str());
											pInfo(peer)->Crown_Value_0_0 = (long long int)(255 + (256 * pInfo(peer)->Base_R_0) + pInfo(peer)->Base_G_0 * 65536 + (pInfo(peer)->Base_B_0 * (long long int)16777216));
										}
										{
											pInfo(peer)->Crown_Laser_Beam_0 = atoi(explode("\n", explode("checkbox_laser_beam0|", cch)[1])[0].c_str()) == 1 ? true : false;
											auto Crown_Gem_0 = explode(",", explode("\n", explode("text_input_gem_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_gem_color0|", cch)[1])[0].c_str());
											if (Crown_Gem_0.size() != 3 || t_.size() < 2 || Crown_Gem_0[0] == "" || Crown_Gem_0[1] == "" || Crown_Gem_0[2] == "" || Crown_Gem_0[0].empty() || Crown_Gem_0[1].empty() || Crown_Gem_0[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Crown_Gem_0[0]) || !is_number(Crown_Gem_0[1]) || !is_number(Crown_Gem_0[2]) || atoi(Crown_Gem_0[0].c_str()) > 255 || atoi(Crown_Gem_0[1].c_str()) > 255 || atoi(Crown_Gem_0[2].c_str()) > 255 || atoi(Crown_Gem_0[0].c_str()) < 0 || atoi(Crown_Gem_0[1].c_str()) < 0 || atoi(Crown_Gem_0[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Gem_R_0 = atoi(Crown_Gem_0[0].c_str());
											pInfo(peer)->Gem_G_0 = atoi(Crown_Gem_0[1].c_str());
											pInfo(peer)->Gem_B_0 = atoi(Crown_Gem_0[2].c_str());
											pInfo(peer)->Crown_Value_0_1 = 255 + (256 * atoi(Crown_Gem_0[0].c_str())) + atoi(Crown_Gem_0[1].c_str()) * 65536 + (atoi(Crown_Gem_0[2].c_str()) * (long long int)16777216);
										}
										{
											auto Crown_Crystal_0 = explode(",", explode("\n", explode("text_input_crystal_color0|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_crystal_color0|", cch)[1])[0].c_str());
											if (Crown_Crystal_0.size() != 3 || t_.size() < 2 || Crown_Crystal_0[0] == "" || Crown_Crystal_0[1] == "" || Crown_Crystal_0[2] == "" || Crown_Crystal_0[0].empty() || Crown_Crystal_0[1].empty() || Crown_Crystal_0[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											};
											if (!is_number(Crown_Crystal_0[0]) || !is_number(Crown_Crystal_0[1]) || !is_number(Crown_Crystal_0[2]) || atoi(Crown_Crystal_0[0].c_str()) > 255 || atoi(Crown_Crystal_0[1].c_str()) > 255 || atoi(Crown_Crystal_0[2].c_str()) > 255 || atoi(Crown_Crystal_0[0].c_str()) < 0 || atoi(Crown_Crystal_0[1].c_str()) < 0 || atoi(Crown_Crystal_0[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Crystal_R_0 = atoi(Crown_Crystal_0[0].c_str());
											pInfo(peer)->Crystal_G_0 = atoi(Crown_Crystal_0[1].c_str());
											pInfo(peer)->Crystal_B_0 = atoi(Crown_Crystal_0[2].c_str());
											pInfo(peer)->Crown_Value_0_2 = 255 + (256 * atoi(Crown_Crystal_0[0].c_str())) + atoi(Crown_Crystal_0[1].c_str()) * 65536 + (atoi(Crown_Crystal_0[2].c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Crown_Crystals_0 = atoi(explode("\n", explode("checkbox_crystals0|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Crown_Rays_0 = atoi(explode("\n", explode("checkbox_rays0|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
									{ // Crown 2
										pInfo(peer)->Crown_Floating_Effect_1 = atoi(explode("\n", explode("checkbox_floating1|", cch)[1])[0].c_str()) == 1 ? true : false;
										{
											auto Crown_Base_1 = explode(",", explode("\n", explode("text_input_base_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_base_color1|", cch)[1])[0].c_str());
											if (Crown_Base_1.size() != 3 || t_.size() < 2 || Crown_Base_1[0] == "" || Crown_Base_1[1] == "" || Crown_Base_1[2] == "" || Crown_Base_1[0].empty() || Crown_Base_1[1].empty() || Crown_Base_1[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Crown_Base_1[0]) || !is_number(Crown_Base_1[1]) || !is_number(Crown_Base_1[2]) || atoi(Crown_Base_1[0].c_str()) > 255 || atoi(Crown_Base_1[1].c_str()) > 255 || atoi(Crown_Base_1[2].c_str()) > 255 || atoi(Crown_Base_1[0].c_str()) < 0 || atoi(Crown_Base_1[1].c_str()) < 0 || atoi(Crown_Base_1[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Base_R_1 = atoi(Crown_Base_1[0].c_str());
											pInfo(peer)->Base_G_1 = atoi(Crown_Base_1[1].c_str());
											pInfo(peer)->Base_B_1 = atoi(Crown_Base_1[2].c_str());
											pInfo(peer)->Crown_Value_1_1 = 255 + (256 * atoi(Crown_Base_1[0].c_str())) + atoi(Crown_Base_1[1].c_str()) * 65536 + (atoi(Crown_Base_1[2].c_str()) * (long long int)16777216);
										}
										{
											pInfo(peer)->Crown_Laser_Beam_1 = atoi(explode("\n", explode("checkbox_laser_beam1|", cch)[1])[0].c_str()) == 1 ? true : false;
											auto Crown_Gem_1 = explode(",", explode("\n", explode("text_input_gem_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_gem_color1|", cch)[1])[0].c_str());
											if (Crown_Gem_1.size() != 3 || t_.size() < 2 || Crown_Gem_1[0] == "" || Crown_Gem_1[1] == "" || Crown_Gem_1[2] == "" || Crown_Gem_1[0].empty() || Crown_Gem_1[1].empty() || Crown_Gem_1[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Crown_Gem_1[0]) || !is_number(Crown_Gem_1[1]) || !is_number(Crown_Gem_1[2]) || atoi(Crown_Gem_1[0].c_str()) > 255 || atoi(Crown_Gem_1[1].c_str()) > 255 || atoi(Crown_Gem_1[2].c_str()) > 255 || atoi(Crown_Gem_1[0].c_str()) < 0 || atoi(Crown_Gem_1[1].c_str()) < 0 || atoi(Crown_Gem_1[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Gem_R_1 = atoi(Crown_Gem_1[0].c_str());
											pInfo(peer)->Gem_G_1 = atoi(Crown_Gem_1[1].c_str());
											pInfo(peer)->Gem_B_1 = atoi(Crown_Gem_1[2].c_str());
											pInfo(peer)->Crown_Value_1_1 = 255 + (256 * atoi(Crown_Gem_1[0].c_str())) + atoi(Crown_Gem_1[1].c_str()) * 65536 + (atoi(Crown_Gem_1[2].c_str()) * (long long int)16777216);
										}
										{
											auto Crown_Crystal_1 = explode(",", explode("\n", explode("text_input_crystal_color1|", cch)[1])[0].c_str());
											vector<string> t_ = explode(",", explode("\n", explode("text_input_crystal_color1|", cch)[1])[0].c_str());
											if (Crown_Crystal_1.size() != 3 || t_.size() < 2 || Crown_Crystal_1[0] == "" || Crown_Crystal_1[1] == "" || Crown_Crystal_1[2] == "" || Crown_Crystal_1[0].empty() || Crown_Crystal_1[1].empty() || Crown_Crystal_1[2].empty()) {
												SendDialogInfinityCrown(peer, false, "you need to enter an RGB (Red, Blue, Green) value");
												break;
											}
											if (!is_number(Crown_Crystal_1[0]) || !is_number(Crown_Crystal_1[1]) || !is_number(Crown_Crystal_1[2]) || atoi(Crown_Crystal_1[0].c_str()) > 255 || atoi(Crown_Crystal_1[1].c_str()) > 255 || atoi(Crown_Crystal_1[2].c_str()) > 255 || atoi(Crown_Crystal_1[0].c_str()) < 0 || atoi(Crown_Crystal_1[1].c_str()) < 0 || atoi(Crown_Crystal_1[2].c_str()) < 0) {
												SendDialogInfinityCrown(peer, false, "you need to enter values betwwen 0 and 255");
												break;
											}
											pInfo(peer)->Crystal_R_1 = atoi(Crown_Crystal_1[0].c_str());
											pInfo(peer)->Crystal_G_1 = atoi(Crown_Crystal_1[1].c_str());
											pInfo(peer)->Crystal_B_1 = atoi(Crown_Crystal_1[2].c_str());
											pInfo(peer)->Crown_Value_1_2 = 255 + (256 * atoi(Crown_Crystal_1[0].c_str())) + atoi(Crown_Crystal_1[1].c_str()) * 65536 + (atoi(Crown_Crystal_1[2].c_str()) * (long long int)16777216);
										}
										pInfo(peer)->Crown_Crystals_1 = atoi(explode("\n", explode("checkbox_crystals1|", cch)[1])[0].c_str()) == 1 ? true : false;
										pInfo(peer)->Crown_Rays_1 = atoi(explode("\n", explode("checkbox_rays1|", cch)[1])[0].c_str()) == 1 ? true : false;
									}
								}
								catch (...) {
									break;
								}
								int Total_Value = 1768716288;
								if (pInfo(peer)->Crown_Time_Change) Total_Value += 256;
								if (pInfo(peer)->Crown_Floating_Effect_0) Total_Value += 64;
								if (pInfo(peer)->Crown_Laser_Beam_0) Total_Value += 1;
								if (pInfo(peer)->Crown_Crystals_0) Total_Value += 4;
								if (pInfo(peer)->Crown_Rays_0) Total_Value += 16;
								if (pInfo(peer)->Crown_Floating_Effect_1) Total_Value += 128;
								if (pInfo(peer)->Crown_Laser_Beam_1) Total_Value += 2;
								if (pInfo(peer)->Crown_Crystals_1) Total_Value += 8;
								if (pInfo(peer)->Crown_Rays_1) Total_Value += 32;
								pInfo(peer)->Crown_Value = Total_Value;
								update_clothes(peer);
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|wfcalendar_reward_list_dialog") != string::npos) {
							int x_ = atoi(explode("\n", explode("tilex|", cch)[1])[0].c_str()), y_ = atoi(explode("\n", explode("tiley|", cch)[1])[0].c_str());
							if (cch.find("buttonClicked|goto_maindialog") != string::npos) {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
									if (world_->owner_name == pInfo(peer)->tankIDName || pInfo(peer)->dev || guild_access(peer, world_->guild_id) || find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end()) {
										string Day = "";
										Day += "\nadd_button_with_icon|calendarSystem_1|`$Day 1``|" + string(block_->pr >= 1 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_2|`$Day 2``|" + string(block_->pr >= 2 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_3|`$Day 3``|" + string(block_->pr >= 3 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_4|`$Day 4``|" + string(block_->pr >= 4 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_5|`$Day 5``|" + string(block_->pr >= 5 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_6|`$Day 6``|" + string(block_->pr >= 6 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_7|`$Day 7``|" + string(block_->pr >= 7 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_8|`$Day 8``|" + string(block_->pr >= 8 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_9|`$Day 9``|" + string(block_->pr >= 9 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_10|`$Day 10``|" + string(block_->pr >= 10 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_11|`$Day 11``|" + string(block_->pr >= 11 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_12|`$Day 12``|" + string(block_->pr >= 12 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_13|`$Day 13``|" + string(block_->pr >= 13 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_14|`$Day 14``|" + string(block_->pr >= 14 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_15|`$Day 15``|" + string(block_->pr >= 15 ? "noflags|6292||" : "frame|9202||");
										Day += "\nadd_button_with_icon|calendarSystem_16|`$Day 16``|" + string(block_->pr >= 16 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_17|`$Day 17``|" + string(block_->pr >= 17 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_18|`$Day 18``|" + string(block_->pr >= 18 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_19|`$Day 19``|" + string(block_->pr >= 19 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_20|`$Day 20``|" + string(block_->pr >= 20 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_21|`$Day 21``|" + string(block_->pr >= 21 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_22|`$Day 22``|" + string(block_->pr >= 22 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_23|`$Day 23``|" + string(block_->pr >= 23 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_24|`$Day 24``|" + string(block_->pr >= 24 ? "noflags|6292||" : "frame|10486||");
										Day += "\nadd_button_with_icon|calendarSystem_25|`$Day 25``|" + string(block_->pr >= 25 ? "noflags|6292||" : "frame|9202||");
										gamepacket_t p(500);
										p.Insert("OnDialogRequest");
										p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wWinterfest Calendar - 2022``|left|12986|\nadd_textbox|Tap on a button below to view the `2rewards`` available on that day.|left|\nadd_spacer|small|\ntext_scaling_string|Defibrillators|" + Day + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_button|cancel|Close|noflags|0|0|\nend_dialog|wfcalendar_dailyrewards|||\nadd_quick_exit|\n");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|wfcalendar_dailyrewards") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 7) break;
							int x_ = atoi(explode("\n", explode("tilex|", cch)[1])[0].c_str()), y_ = atoi(explode("\n", explode("tiley|", cch)[1])[0].c_str());
							if (cch.find("buttonClicked|calendarSystem_") != string::npos) {
								string test = explode("\n", t_[7])[0].c_str();
								replace_str(test, "calendarSystem_", "");
								int System = atoi(test.c_str());
								string Prizes = "";
								if (System == 1 || System == 5 || System == 9 || System == 13 || System == 18 || System == 22) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wCrystal of Hoa``|left|11244|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wCrystal Block Seed``|left|263|\nadd_label_with_icon|small|`wEzio's Armguards``|left|7088|\nadd_label_with_icon|small|`wEzio's Boots``|left|7094|\nadd_label_with_icon|small|`wFrozen Fountain``|left|11520|\nadd_label_with_icon|small|`wLuxurious Wall Clock``|left|7864|\nadd_label_with_icon|small|`wDroid Pet``|left|8976|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wTwo Turtle Doves``|left|12918|\nadd_label_with_icon|small|`wLumberjack Coat - Scarlet``|left|5428|\nadd_label_with_icon|small|`wLumberjack Coat - Olive``|left|5430|\nadd_label_with_icon|small|`wLumberjack Coat - Navy``|left|5432|\nadd_label_with_icon|small|`wLumberjack Coat - Midnight``|left|5434|\nadd_label_with_icon|small|`wSnowy Tree Block``|left|10486|\nadd_label_with_icon|small|`wGavle Goat``|left|10444|\nadd_label_with_icon|small|`wFuturistic Sunglasses``|left|11202|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wNutcracker Boots``|left|7498|\nadd_label_with_icon|small|`wSnowy Fence``|left|10488|\nadd_label_with_icon|small|`wKitty Block``|left|8280|\nadd_label_with_icon|small|`wDoggy Block``|left|8278|\nadd_spacer|small|";
								if (System == 2 || System == 6 || System == 10 || System == 14 || System == 19 || System == 23) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wZodiac Wings``|left|10054|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wSnowtopian``|left|9202|\nadd_label_with_icon|small|`wGlowing Nose``|left|1384|\nadd_label_with_icon|small|`wConveyor Block``|left|11192|\nadd_label_with_icon|small|`wPuffy Blue Jacket``|left|5446|\nadd_label_with_icon|small|`wChristmas Pudding Block``|left|11514|\nadd_label_with_icon|small|`wGem Hair``|left|8480|\nadd_label_with_icon|small|`wCaduceaxe``|left|8554|\nadd_label_with_icon|small|`wNutcracker Pants``|left|7496|\nadd_label_with_icon|small|`wEzio's Beard``|left|7090|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wTwo Turtle Doves``|left|12918|\nadd_label_with_icon|small|`wLumberjack Coat - Midnight``|left|5434|\nadd_label_with_icon|small|`wLumberjack Coat - Navy``|left|5432|\nadd_label_with_icon|small|`wLumberjack Coat - Olive``|left|5430|\nadd_label_with_icon|small|`wLumberjack Coat - Scarlet``|left|5428|\nadd_label_with_icon|small|`wVile Vial - Ecto-Bones``|left|8546|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wSnowy Fence``|left|10488|\nadd_label_with_icon|small|`wChocolate Block (x5)``|left|2564|\nadd_label_with_icon|small|`wWinter Gift (x5)``|left|1360|\nadd_spacer|small|";
								if (System == 3 || System == 7 || System == 11 || System == 16 || System == 20 || System == 24) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wZodiac Ring``|left|11158|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wNutcracker Hat``|left|7490|\nadd_label_with_icon|small|`wSurgWorld Blast``|left|8556|\nadd_label_with_icon|small|`wPigeon``|left|10258|\nadd_label_with_icon|small|`wFrozen Fountain``|left|11520|\nadd_label_with_icon|small|`wPet Trainer Whistle``|left|3676|\nadd_label_with_icon|small|`wEzio's Trousers``|left|7086|\nadd_label_with_icon|small|`wEzio's Vest``|left|7082|\nadd_label_with_icon|small|`wWaist Tied Sweater - Green``|left|10036|\nadd_label_with_icon|small|`wWaist Tied Sweater - Yellow``|left|10042|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wFuturistic Sunglasses``|left|11202|\nadd_label_with_icon|small|`wTwo Turtle Doves``|left|12918|\nadd_label_with_icon|small|`wPuffy Blue Jacket``|left|5446|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wGavle Goat``|left|10444|\nadd_label_with_icon|small|`wLumberjack Coat - Midnight``|left|5434|\nadd_label_with_icon|small|`wLumberjack Coat - Navy``|left|5432|\nadd_label_with_icon|small|`wLumberjack Coat - Olive``|left|5430|\nadd_label_with_icon|small|`wLumberjack Coat - Scarlet``|left|5428|\nadd_spacer|small|";
								if (System == 4 || System == 8 || System == 12 || System == 17 || System == 21) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wLightning Horns``|left|12302|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wSnowtopian``|left|9202|\nadd_label_with_icon|small|`wStar Tool Droid``|left|6414|\nadd_label_with_icon|small|`wEzio's Hood``|left|7080|\nadd_label_with_icon|small|`wSnowfrost's Suit``|left|7422|\nadd_label_with_icon|small|`wPrehistoric Dragon Claw``|left|7758|\nadd_label_with_icon|small|`wRobot Legs``|left|6384|\nadd_label_with_icon|small|`wAxolotl Slippers``|left|10032|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wChristmas Pudding Block``|left|11514|\nadd_label_with_icon|small|`wConveyor Block``|left|11192|\nadd_label_with_icon|small|`wSnowball (x10)``|left|1368|\nadd_label_with_icon|small|`wTwo Turtle Doves``|left|12918|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wS'mores Block (x2)``|left|5382|\nadd_label_with_icon|small|`wLumberjack Coat - Midnight``|left|5434|\nadd_label_with_icon|small|`wLumberjack Coat - Navy``|left|5432|\nadd_label_with_icon|small|`wLumberjack Coat - Olive``|left|5430|\nadd_label_with_icon|small|`wLumberjack Coat - Scarlet``|left|5428|\nadd_label_with_icon|small|`wWinter Gift (x5)``|left|1360|\nadd_spacer|small|";
								if (System == 15) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wCute Mutant Plonky``|left|11080|\nadd_label_with_icon|small|`wCute Mutant Boogle``|left|11082|\nadd_label_with_icon|small|`wEzio's Hair``|left|7092|\nadd_label_with_icon|small|`wPegasus Lance``|left|10922|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wWinter Swept Hair``|left|10440|\nadd_label_with_icon|small|`wCrystal Block Seed``|left|263|\nadd_label_with_icon|small|`wCute Mutant Wonky``|left|11078|\nadd_label_with_icon|small|`wSad Snowfrost's Mask``|left|11500|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wRiding Eagle Owl``|left|9184|\nadd_label_with_icon|small|`wHay Cart``|left|7096|\nadd_label_with_icon|small|`wStar Dragon Claw``|left|7760|\nadd_label_with_icon|small|`wWaist Tied Sweater - Red``|left|10038|\nadd_label_with_icon|small|`wSnowfrost's Mask``|left|7420|\nadd_label_with_icon|small|`wWaist Tied Sweater - Blue``|left|10040|\nadd_label_with_icon|small|`wSnowfrost's Top Hat``|left|7418|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wSpring-Loaded Fists``|left|8960|\nadd_label_with_icon|small|`wExo Suit Helmet``|left|9692|\nadd_label_with_icon|small|`wSpeeder``|left|8948|\nadd_label_with_icon|small|`wFlower Headband``|left|8722|\nadd_spacer|small|";
								if (System == 25) Prizes = "\nadd_textbox|`5Epic``|left|\nadd_label_with_icon|small|`wThe Beast with a Thousand Eyes Mask``|left|8358|\nadd_label_with_icon|small|`wBeowulf's Blade``|left|11084|\nadd_label_with_icon|small|`wGiant Eye Head``|left|8372|\nadd_spacer|small|\nadd_textbox|`1Rare``|left|\nadd_label_with_icon|small|`wRadical Rider``|left|10878|\nadd_label_with_icon|small|`wFrosty Wings``|left|9182|\nadd_label_with_icon|small|`wCandy Cane Cruiser``|left|11468|\nadd_spacer|small|\nadd_textbox|`6Uncommon``|left|\nadd_label_with_icon|small|`wSilverstar Bow``|left|5456|\nadd_label_with_icon|small|`wEzio's Cape``|left|7084|\nadd_label_with_icon|small|`wSnowfrost's Candy Cane Blade``|left|7424|\nadd_label_with_icon|small|`wSnowflake Eyes``|left|7396|\nadd_label_with_icon|small|`wWinter Light Launcher``|left|10442|\nadd_spacer|small|\nadd_textbox|Common|left|\nadd_label_with_icon|small|`wDouble Growsaber``|left|812|\nadd_label_with_icon|small|`wRed Growsaber``|left|802|\nadd_label_with_icon|small|`wGrowboard``|left|1758|\nadd_label_with_icon|small|`wStrange Eyes``|left|9370|\nadd_label_with_icon|small|`wDragon Throne``|left|7738|\nadd_label_with_icon|small|`wRiding Chicken``|left|5018|\nadd_label_with_icon|small|`wHowly Hat``|left|8468|\nadd_label_with_icon|small|`wSurgWorld Blast``|left|8556|\nadd_label_with_icon|small|`wGourmet Dragon Claw``|left|7752|\nadd_label_with_icon|small|`wCupcake Hat``|left|8474|\nadd_spacer|small|";
								gamepacket_t p(500);
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wDay " + to_string(System) + " Rewards``|left|12986|\nadd_spacer|small|\nadd_textbox|" + string(System == 15 || System == 25 ? "Merry Winterfest and a happy new reward. You have a chance of obtaining the following items:" : "You have a chance of obtaining the following items:") + "|left|\nadd_spacer|small|" + Prizes + "\nembed_data|tilex|" + to_string(x_) + "\nembed_data|tiley|" + to_string(y_) + "\nadd_button|goto_maindialog|Thanks for the info!|0|0|\nend_dialog|wfcalendar_reward_list_dialog|||\nadd_quick_exit|\n");
								p.CreatePacket(peer);
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|xenonite_edit") != string::npos) {
							int x_ = atoi(explode("\n", explode("tilex|", cch)[1])[0].c_str()), y_ = atoi(explode("\n", explode("tiley|", cch)[1])[0].c_str());
							try {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									WorldBlock* block_ = &world_->blocks.at(x_ + (y_ * 100));
									uint16_t t_ = (block_->fg ? block_->fg : block_->bg);
									if (not items.at(t_).xeno) break;
									string owner_name = world_->owner_name, user_name = pInfo(peer)->tankIDName;
									if (owner_name != user_name and not pInfo(peer)->dev and not world_->owner_name.empty()) {
										if (block_->locked) {
											WorldBlock* check_lock = &world_->blocks.at(block_->lock_origin);
											if (check_lock->owner_name != pInfo(peer)->tankIDName) break;
										}
										else {
											break;
										}
									}
									int Xeno_1 = atoi(explode("\n", explode("checkbox_force_dbl|", cch)[1])[0].c_str());
									int Xeno_2 = atoi(explode("\n", explode("checkbox_block_dbl|", cch)[1])[0].c_str());
									int Xeno_3 = atoi(explode("\n", explode("checkbox_force_hig|", cch)[1])[0].c_str());
									int Xeno_4 = atoi(explode("\n", explode("checkbox_block_hig|", cch)[1])[0].c_str());
									int Xeno_5 = atoi(explode("\n", explode("checkbox_force_asb|", cch)[1])[0].c_str());
									int Xeno_6 = atoi(explode("\n", explode("checkbox_block_asb|", cch)[1])[0].c_str());
									int Xeno_7 = atoi(explode("\n", explode("checkbox_force_pun|", cch)[1])[0].c_str());
									int Xeno_8 = atoi(explode("\n", explode("checkbox_block_pun|", cch)[1])[0].c_str());
									int Xeno_9 = atoi(explode("\n", explode("checkbox_force_lng|", cch)[1])[0].c_str());
									int Xeno_10 = atoi(explode("\n", explode("checkbox_block_lng|", cch)[1])[0].c_str());
									int Xeno_11 = atoi(explode("\n", explode("checkbox_force_spd|", cch)[1])[0].c_str());
									int Xeno_12 = atoi(explode("\n", explode("checkbox_block_spd|", cch)[1])[0].c_str());
									int Xeno_13 = atoi(explode("\n", explode("checkbox_force_lngb|", cch)[1])[0].c_str());
									int Xeno_14 = atoi(explode("\n", explode("checkbox_block_lngb|", cch)[1])[0].c_str());
									int Xeno_15 = atoi(explode("\n", explode("checkbox_force_spiw|", cch)[1])[0].c_str());
									int Xeno_16 = atoi(explode("\n", explode("checkbox_block_spiw|", cch)[1])[0].c_str());
									int Xeno_17 = atoi(explode("\n", explode("checkbox_force_pdmg|", cch)[1])[0].c_str());
									int Xeno_18 = atoi(explode("\n", explode("checkbox_block_pdmg|", cch)[1])[0].c_str());
									int Xeno_19 = atoi(explode("\n", explode("checkbox_block_pwr|", cch)[1])[0].c_str());
									world_->X_1 = Xeno_1 == 1 ? true : false;
									world_->X_2 = Xeno_2 == 1 and !world_->X_1 ? true : false;
									world_->X_3 = Xeno_3 == 1 ? true : false;
									world_->X_4 = Xeno_4 == 1 and !world_->X_3 ? true : false;
									world_->X_5 = Xeno_5 == 1 ? true : false;
									world_->X_6 = Xeno_6 == 1 and !world_->X_5 ? true : false;
									world_->X_7 = Xeno_7 == 1 ? true : false;
									world_->X_8 = Xeno_8 == 1 and !world_->X_7 ? true : false;
									world_->X_9 = Xeno_9 == 1 ? true : false;
									world_->X_10 = Xeno_10 == 1 and !world_->X_9 ? true : false;
									world_->X_11 = Xeno_11 == 1 ? true : false;
									world_->X_12 = Xeno_12 == 1 and !world_->X_11 ? true : false;
									world_->X_13 = Xeno_13 == 1 ? true : false;
									world_->X_14 = Xeno_14 == 1 and !world_->X_13 ? true : false;
									world_->X_15 = Xeno_15 == 1 ? true : false;
									world_->X_16 = Xeno_16 == 1 and !world_->X_15 ? true : false;
									world_->X_17 = Xeno_17 == 1 ? true : false;
									world_->X_18 = Xeno_18 == 1 and !world_->X_17 ? true : false;
									world_->X_19 = Xeno_19 == 1 ? true : false;
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->world == pInfo(peer)->world) {
											Send_Xenonite_Update(currentPeer);
										}
									}
								}
							}
							catch (out_of_range) {
								cout << "Server error invalid (out of range) on " + cch << endl;
								break;
							}
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|punish_view\nbuttonClicked|punish_player") != string::npos) {
							string result = cch.substr(74, cch.length() - 74);
							vector<string> hasil = split_string_by_newline(result);
							vector<string> days = explode("|", hasil[0]);
							vector<string> hours = explode("|", hasil[1]);
							vector<string> mins = explode("|", hasil[2]);
							vector<string> reason = explode("|", hasil[3]);
							vector<string> mute = explode("|", hasil[4]);
							vector<string> curse = explode("|", hasil[5]);
							vector<string> ban = explode("|", hasil[6]);
							if (not onlyDigit(days[1]) or not onlyDigit(hours[1]) or not onlyDigit(mins[1])) break;
							int Days = atoi(days[1].c_str());
							int Hours = atoi(hours[1].c_str());
							int Mins = atoi(mins[1].c_str());
							string Reason = reason[1];
							if (Reason.length() >= 180) break;
							if (Reason.length() <= 0) Reason = "No Reason Provided";
							int isMute = atoi(mute[1].c_str());
							int isCurse = atoi(curse[1].c_str());
							int isBan = atoi(ban[1].c_str());
							long long int banTime = Days * 86400 + (Hours * 3600) + (Mins * 60);
							if (banTime == 0) break;
							if (isMute == 1 and isCurse == 0 and isBan == 0) {
								if (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1) {
									if (to_lower(pInfo(peer)->last_wrenched) == "ritshu" || pInfo(peer)->superdev) break;
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
											string p_username = ("**" + pInfo(currentPeer)->tankIDName + "** (IP: `" + pInfo(currentPeer)->ip + "`) was muted!");
											string p_reason = ("> " + Reason);
											string p_duration = (detailSecond(banTime));
											string p_punisher = (pInfo(peer)->tankIDName);
											send_Punishment(p_username, p_reason, p_punisher, p_duration);
											add_mute(currentPeer, banTime, Reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``");
										}
									}
								}
								break;
							}
							else if (isMute == 0 and isCurse == 1 and isBan == 0) {
								if (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1) {
									if (to_lower(pInfo(peer)->last_wrenched) == "ritshu" || pInfo(peer)->superdev) break;
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
											string p_username = ("**" + pInfo(currentPeer)->tankIDName + "** (IP: `" + pInfo(currentPeer)->ip + "`) was cursed!");
											string p_reason = ("> " + Reason);
											string p_duration = (detailSecond(banTime));
											string p_punisher = (pInfo(peer)->tankIDName);
											send_Punishment(p_username, p_reason, p_punisher, p_duration);
											add_curse(currentPeer, banTime, Reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``");
										}
									}
								}
								break;
							}
							else if (isMute == 0 and isCurse == 0 and isBan == 1) {
								if (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1) {
									if (to_lower(pInfo(peer)->last_wrenched) == "ritshu" || pInfo(peer)->superdev) break;
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
											string p_username = ("**" + pInfo(currentPeer)->tankIDName + "** (IP: `" + pInfo(currentPeer)->ip + "`) was banned!");
											string p_reason = ("> " + Reason);
											string p_duration = (detailSecond(banTime));
											string p_punisher = (pInfo(peer)->tankIDName);
											send_Punishment(p_username, p_reason, p_punisher, p_duration);
											add_ban(currentPeer, banTime, Reason, pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``");
										}
									}
								}
								break;
							}
							else {
								gamepacket_t p;
								p.Insert("OnTextOverlay");
								p.Insert("Please choose only one option within Mute/Curse/Ban.");
								p.CreatePacket(peer);
								break;
							}
						}
						if (cch.find("action|dialog_return\ndialog_name|punish_view\nbuttonClicked|warp_to_") != string::npos) {
							if (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1) {
								string world_name = cch.substr(67, cch.length() - 67);
								vector<string> test = split_string_by_newline(world_name);
								join_world(peer, test[0]);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|blast\nname|") != string::npos) {
							if (pInfo(peer)->lastchoosenitem == 830 || pInfo(peer)->lastchoosenitem == 9164 || pInfo(peer)->lastchoosenitem == 9602 || pInfo(peer)->lastchoosenitem == 942 || pInfo(peer)->lastchoosenitem == 1060 || pInfo(peer)->lastchoosenitem == 1136 || pInfo(peer)->lastchoosenitem == 1402 || pInfo(peer)->lastchoosenitem == 9582 || pInfo(peer)->lastchoosenitem == 1532 || pInfo(peer)->lastchoosenitem == 3562 || pInfo(peer)->lastchoosenitem == 4774 || pInfo(peer)->lastchoosenitem == 7380 || pInfo(peer)->lastchoosenitem == 7588 || pInfo(peer)->lastchoosenitem == 8556) {
								int blast = pInfo(peer)->lastchoosenitem, got = 0;
								modify_inventory(peer, blast, got);
								if (got == 0) break;
								string world = cch.substr(44, cch.length() - 44).c_str();
								replace_str(world, "\n", "");
								transform(world.begin(), world.end(), world.begin(), ::toupper);
								if (find_if(worlds.begin(), worlds.end(), [world](const World& a) { return a.name == world; }) != worlds.end() || not check_blast(world) || _access_s(("database/worlds/" + world + "_.json").c_str(), 0) == 0) {
									gamepacket_t p;
									p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("That world name already exists. You'll have to be more original. Maybe add some numbers after it?"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
								}
								else {
									if (modify_inventory(peer, blast, got = -1) == 0) {
										create_world_blast(peer, world, blast);
										if (blast == 830) modify_inventory(peer, 834, got = -100);
										join_world(peer, world);
										//pInfo(peer)->worlds_owned.push_back(world);
										gamepacket_t p(750), p2(750);
										p.Insert("OnConsoleMessage"), p.Insert("** `5" + pInfo(peer)->tankIDName + " activates a " + items[blast].name + "! ``**"), p.CreatePacket(peer);
										p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("** `5" + pInfo(peer)->tankIDName + " activates a " + items[blast].name + "! ``**"), p2.Insert(0), p2.Insert(1), p2.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|world_spray\n") {
							int got = 0;
							modify_inventory(peer, 12600, got);
							if (got == 0) break;
							string name = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name](const World& a) { return a.name == name; });
							if (p != worlds.end()) {
								World* world = &worlds[p - worlds.begin()];
								if (world->owner_name == pInfo(peer)->tankIDName || pInfo(peer)->superdev) {
									int remove = -1;
									modify_inventory(peer, 12600, remove);
									for (int i_ = 0; i_ < world->blocks.size(); i_++) if (world->blocks[i_].fg % 2 != 0)  world->blocks[i_].planted = _int64(2.592e+6);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->world == name) {
											int x = pInfo(currentPeer)->x, y = pInfo(currentPeer)->y;
											exit_(currentPeer, true);
											join_world(currentPeer, name);
										}
									}
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("`wYou must own the world!``"), p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|consumehgem\nitemID|7784|\ncount|") != string::npos) {
							string itemCount = cch.substr(64, cch.length() - 64).c_str();
							int intItemCount = atoi(itemCount.c_str());
							int got = 0;
							modify_inventory(peer, 7784, got);
							if (got == 0) break;
							if (intItemCount >= 1) {
								int remove = -intItemCount;
								modify_inventory(peer, 7784, remove);
								int total = intItemCount * 500;
								pInfo(peer)->gems += total;
								{
									gamepacket_t p;
									p.Insert("OnSetBux");
									p.Insert(pInfo(peer)->gems);
									p.Insert(0);
									p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
									if (pInfo(peer)->supp >= 2) {
										p.Insert((float)33796, (float)1, (float)0);
									}
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|find_item\nitemID|") != string::npos) {
							cout << cch << endl;
						}
						else if (cch.find("action|dialog_return\ndialog_name|megaphone\nitemID|2480|\nwords|") != string::npos) {
							string text = cch.substr(62, cch.length() - 62).c_str();
							bool cansb = true;
							int c_ = 0;
							modify_inventory(peer, 2480, c_);
							if (c_ == 0) continue;
							if (has_playmod(pInfo(peer), "duct tape")) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("`6>> That's sort of hard to do while duct-taped.``");
								p.CreatePacket(peer);
								cansb = false;
								continue;
							}
							if (has_playmod(pInfo(peer), "megaphone!")) {
								int time_ = 0;
								for (PlayMods peer_playmod : pInfo(peer)->playmods) {
									if (peer_playmod.id == 13) {
										time_ = peer_playmod.time - time(nullptr);
										break;
									}
								}
								packet_(peer, "action|log\nmsg|>> (" + to_playmod_time(time_) + "before you can broadcast again)", "");
								break;
							}
							if (cansb) {
								replace_str(text, "\n", "");
								pInfo(peer)->usedmegaphone = 1;
								SendCmd(peer, "/sb " + text, false);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|lockemegaphone\nitemID|11230|\nwords|") != string::npos) {
							string text = cch.substr(62, cch.length() - 62).c_str();
							bool cansb = true;
							int c_ = 0;
							modify_inventory(peer, 11230, c_);
							if (c_ == 0) continue;
							if (has_playmod(pInfo(peer), "duct tape")) {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("`6>> That's sort of hard to do while duct-taped.``");
								p.CreatePacket(peer);
								cansb = false;
								continue;
							}
							if (has_playmod(pInfo(peer), "Locke's Megaphone!")) {
								int time_ = 0;
								for (PlayMods peer_playmod : pInfo(peer)->playmods) {
									if (peer_playmod.id == 13) {
										time_ = peer_playmod.time - time(nullptr);
										break;
									}
								}
								packet_(peer, "action|log\nmsg|>> (" + to_playmod_time(time_) + "before you can broadcast again)", "");
								break;
							}
							if (cansb) {
								replace_str(text, "\n", "");
								pInfo(peer)->usedmegaphone = 1;
								SendCmd(peer, "/lsb " + text, false);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|notebook_edit\nbuttonClicked|cancel") != string::npos || cch.find("action|dialog_return\ndialog_name|notebook_edit\nbuttonClicked|clear") != string::npos) {
							if (cch.find("action|dialog_return\ndialog_name|notebook_edit\nbuttonClicked|clear") != string::npos) 	pInfo(peer)->note = "";
							send_wrench_self(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|notebook_edit\nbuttonClicked|save\n\npersonal_note|") != string::npos) {
							string btn = cch.substr(81, cch.length() - 81).c_str();
							replace_str(btn, "\n", "");
							if (btn.length() > 128) continue;
							pInfo(peer)->note = btn;
							send_wrench_self(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|3898\nbuttonClicked|") != string::npos || cch == "action|dialog_return\ndialog_name|zurgery_back\nbuttonClicked|53785\n\n" || cch == "action|dialog_return\ndialog_name|zombie_back\nbuttonClicked|53785\n\n") {
							string btn = cch.substr(52, cch.length() - 52).c_str();
							if (cch == "action|dialog_return\ndialog_name|zurgery_back\nbuttonClicked|53785\n\n" || cch == "action|dialog_return\ndialog_name|zombie_back\nbuttonClicked|53785\n\n") btn = "53785";
							replace_str(btn, "\n", "");
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (btn == "12345") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wCrazy Jim's Quest Emporium``|left|3902|\nadd_textbox|HEEEEYYY there Growtopian! I'm Crazy Jim, and my quests are so crazy they're KERRRRAAAAZZY!! And that is clearly very crazy, so please, be cautious around them. What can I do ya for, partner?|left|\nadd_button|chc1_1|Daily Quest|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							else if (btn == "53785") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man``|left|4358|\nadd_textbox|It is I, Sales-Man, savior of the wealthy! Let me rescue you from your riches. What would you like to buy today?|left|\nadd_button|chc4_1|Surgery Items|noflags|0|0|"/*\nadd_button|chc3_1|Zombie Defense Items|noflags|0|0|*/"\nadd_button|chc2_1|Blue Gem Lock|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							else if (btn == "chc1_1") {
								if (today_day != pInfo(peer)->dd) {
									int haveitem1 = 0, haveitem2 = 0, received = 0;
									modify_inventory(peer, item1, haveitem1);
									modify_inventory(peer, item2, haveitem2);
									if (haveitem1 >= item1c && haveitem2 >= item2c) received = 1;
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wCrazy Jim's Daily Quest``|left|3902|\nadd_textbox|I guess some people call me Crazy Jim because I'm a bit of a hoarder. But I'm very particular about what I want! And today, what I want is this:|left|\nadd_label_with_icon|small|`2" + to_string(item1c) + " " + items[item1].name + "|left|" + to_string(item1) + "|\nadd_smalltext|and|left|\nadd_label_with_icon|small|`2" + to_string(item2c) + " " + items[item2].name + "|left|" + to_string(item2) + "|\nadd_spacer|small|\nadd_smalltext|You shove all that through the phone (it works, I've tried it), and I will hand you one of the `2Growtokens`` from my personal collection!  But hurry, this offer is only good until midnight, and only one `2Growtoken`` per person!|left|\nadd_spacer|small|\nadd_smalltext|`6(You have " + to_string(haveitem1) + " " + items[item1].name + " and " + to_string(haveitem2) + " " + items[item2].name + ")``|left|\nadd_spacer|small|" + (received == 1 ? "\nadd_button|turnin|Turn in items|noflags|0|0|" : "") + "\nadd_spacer|small|\nadd_button|12345|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
								}
								else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wCrazy Jim's Daily Quest``|left|3902|\nadd_textbox|You've already completed my Daily Quest for today! Call me back after midnight to hear about my next cravings.|left|\nadd_button|12345|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							}
							else if (btn == "turnin") {
								if (today_day != pInfo(peer)->dd) {
									int haveitem1 = 0, haveitem2 = 0, received = 0, remove = -1, remove2 = -1, giveitem = 1;
									modify_inventory(peer, item1, haveitem1);
									modify_inventory(peer, item2, haveitem2);
									if (haveitem1 >= item1c && haveitem2 >= item2c) received = 1;
									if (received == 1) {
										if (pInfo(peer)->quest_active && pInfo(peer)->quest_step == 6 && pInfo(peer)->quest_progress < 28) {
											pInfo(peer)->quest_progress += received;
											if (pInfo(peer)->quest_progress >= 28) {
												pInfo(peer)->quest_progress = 28;
												gamepacket_t p;
												p.Insert("OnTalkBubble");
												p.Insert(pInfo(peer)->netID);
												p.Insert("`9Legendary Quest step complete! I'm off to see a Wizard!");
												p.Insert(0), p.Insert(0);
												p.CreatePacket(peer);
											}
										}
										pInfo(peer)->dd = today_day;
										modify_inventory(peer, item1, remove *= item1c);
										modify_inventory(peer, item2, remove2 *= item2c);
										modify_inventory(peer, 1486, giveitem);
										gamepacket_t p, p4;
										p.Insert("OnConsoleMessage");
										p.Insert("[`6You jammed " + to_string(item1c) + " " + items[item1].name + " and " + to_string(item2c) + " " + items[item2].name + " into the phone, and 1 `2Growtoken`` popped out!``]");
										p4.Insert("OnTalkBubble");
										p4.Insert(pInfo(peer)->netID);
										p4.Insert("Thanks, pardner! Have 1 `2Growtoken`w!");
										p4.Insert(0), p4.Insert(0);
										p.CreatePacket(peer), p4.CreatePacket(peer);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(currentPeer)->world == pInfo(peer)->world) {
												gamepacket_t p3;
												p3.Insert("OnParticleEffect");
												p3.Insert(198);
												p3.Insert((float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
												p3.CreatePacket(currentPeer);
											}
										}
									}
								}
								else {
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wCrazy Jim's Daily Quest``|left|3902|\nadd_textbox|You've already completed my Daily Quest for today! Call me back after midnight to hear about my next cravings.|left|\nadd_button|12345|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
									p.CreatePacket(peer);
								}
							}
							else if (btn == "chc2_1") {
								int c_ = 0;
								modify_inventory(peer, 1796, c_);
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlue Gem Lock``|left|7188|\nadd_textbox|Excellent! I'm happy to sell you a Blue Gem Lock in exchange for 100 Diamond Lock..|left|\nadd_smalltext|`6You have " + to_string(c_) + " Diamond Lock.``|left|" + (c_ >= 100 ? "\nadd_button|chc2_2_1|Thank you!|noflags|0|0|" : "") + "\nadd_button|53785|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							}
							else if (btn == "chc2_2_1") {
								int c7188 = 0, c1796 = 0, additem = 0;
								modify_inventory(peer, 1796, c1796);
								if (c1796 < 100) continue;
								modify_inventory(peer, 7188, c7188);
								if (c7188 >= 200) {
									gamepacket_t p;
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(peer)->netID);
									p.Insert("You don't have room in your backpack!");
									p.Insert(0), p.Insert(1);
									p.CreatePacket(peer);
									{
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("You don't have room in your backpack!");
										p.CreatePacket(peer);
									}
									continue;
								}
								if (c1796 >= 100) {
									if (get_free_slots(pInfo(peer)) >= 2) {
										int cz_ = 1;
										if (modify_inventory(peer, 1796, additem = -100) == 0) {
											modify_inventory(peer, 7188, additem = 1);
											{
												{
													string name_ = pInfo(peer)->world;
													vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
													if (p != worlds.end()) {
														World* world_ = &worlds[p - worlds.begin()];
														PlayerMoving data_{};
														data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16;
														data_.packetType = 19, data_.plantingTree = 500;
														data_.punchX = 7188, data_.punchY = pInfo(peer)->netID;
														int32_t to_netid = pInfo(peer)->netID;
														BYTE* raw = packPlayerMoving(&data_);
														raw[3] = 5;
														memcpy(raw + 8, &to_netid, 4);
														for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
															if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
															if (pInfo(currentPeer)->world == world_->name) {
																send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
															}
														}
														delete[] raw;
													}
												}
												gamepacket_t p;
												p.Insert("OnConsoleMessage");
												p.Insert("[`6You spent 100 Diamond Lock to get 1 Blue Gem Lock``]");
												p.CreatePacket(peer);
											}
										}
										int c_ = 0;
										modify_inventory(peer, 1796, c_);
										p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlue Gem Lock``|left|7188|\nadd_textbox|Excellent! I'm happy to sell you a Blue Gem Lock in exchange for 100 Diamond Lock..|left|\nadd_smalltext|`6You have " + to_string(c_) + " Diamond Lock.``|left|" + (c_ >= 100 ? "\nadd_button|chc2_2_1|Thank you!|noflags|0|0|" : "") + "\nadd_button|53785|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
									}
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("You don't have enough inventory space!");
									p.CreatePacket(peer);
								}
							}
							else if (btn == "chc3_1") {
								int zombie_brain = 0, pile = 0, total = 0;
								modify_inventory(peer, 4450, zombie_brain);
								modify_inventory(peer, 4452, pile);
								total += zombie_brain + (pile * 100);
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man: Zombie Defense``|left|4358|\nadd_textbox|Excellent! I'm happy to sell you Zombie Defense supplies in exchange for Zombie Brains.|left|\nadd_smalltext|You currently have " + setGems(total) + " Zombie Brains.|left|\nadd_spacer|small|\ntext_scaling_string|5,000ZB|\n" + zombie_list + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_button|53785|Back|noflags|0|0|\nend_dialog|zombie_back|Hang Up||\n");
								p.CreatePacket(peer);
							}
							else if (btn == "chc4_1") {
								int zombie_brain = 0, pile = 0, total = 0;
								modify_inventory(peer, 4450, zombie_brain);
								modify_inventory(peer, 4300, pile);
								total += zombie_brain + (pile * 100);
								gamepacket_t p;
								p.Insert("OnDialogRequest");
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man: Surgery``|left|4358|\nadd_textbox|Excellent! I'm happy to sell you rare and precious Surgery prizes in exchange for Caduceus medals.|left|\nadd_smalltext|You currently have " + setGems(total) + " Caducei.|left|\nadd_spacer|small|\ntext_scaling_string|5,000ZB|\n" + surgery_list + "\nadd_button_with_icon||END_LIST|noflags|0||\nadd_button|53785|Back|noflags|0|0|\nend_dialog|zurgery_back|Hang Up||\n");
								p.CreatePacket(peer);
							}
							else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wDisconnected``|left|774|\nadd_textbox|The number you have tried to reach is disconnected. Please check yourself before you wreck yourself.|left|\nend_dialog|3898|Hang Up||\n");
							if (btn != "turnin") p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|punish_view\nbuttonClicked|view_inventory") != string::npos) {
							if (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1) {
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
										string recently_visited = "";
										for (auto it = pInfo(currentPeer)->last_visited_worlds.rbegin(); it != pInfo(currentPeer)->last_visited_worlds.rend(); ++it) {
											string a_ = *it + (next(it) != pInfo(currentPeer)->last_visited_worlds.rend() ? "``, " : "``");
											recently_visited += "`#" + a_;
										}
										string inventory = "";
										int thats5 = 0, thatsadded = 0;
										for (int i_ = 0; i_ < pInfo(currentPeer)->inv.size(); i_++) {
											if (pInfo(currentPeer)->inv[i_].id == 0 || pInfo(currentPeer)->inv[i_].id == 18 || pInfo(currentPeer)->inv[i_].id == 32) continue;
											thats5++;
											thatsadded = 0;
											inventory += "\nadd_button_with_icon|" + (pInfo(peer)->dev == 1 ? to_string(pInfo(currentPeer)->inv[i_].id) : "") + "||staticBlueFrame|" + to_string(pInfo(currentPeer)->inv[i_].id) + "|" + to_string(pInfo(currentPeer)->inv[i_].count) + "|";
											if (thats5 >= 6) {
												thats5 = 0;
												thatsadded = 1;
												inventory += "\nadd_button_with_icon||END_LIST|noflags|0||";
											}
										}
										gamepacket_t p;
										p.Insert("OnDialogRequest");
										p.Insert("set_default_color|`o\nadd_label_with_icon|small|`0Inventory of " + pInfo(currentPeer)->name_color + pInfo(currentPeer)->tankIDName + "``'s (" + pInfo(currentPeer)->requestedName + ") - #" + to_string(pInfo(currentPeer)->netID) + "|left|3802|\nadd_spacer|small|\nadd_textbox|Last visited: " + recently_visited + "|\nadd_textbox|Gems: `w" + setGems(pInfo(currentPeer)->gems) + "|\nadd_textbox|Backpack slots: `w" + to_string(pInfo(currentPeer)->inv.size() - 1) + "|" + inventory + "" + (thatsadded == 1 ? "" : "\nadd_button_with_icon||END_LIST|noflags|0||") + "|\nend_dialog|view_inventory|Continue||\nadd_quick_exit|");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|top\nbuttonClicked|warp_to_") != string::npos) {
							string world_name = cch.substr(59, cch.length() - 59);
							replace_str(world_name, "\n", "");
							join_world(peer, world_name);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|top\nbuttonClicked|wotwlistback\n\n") {
							SendCmd(peer, "/top", true);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|shopgemsconfirm\ngemspurchase|") != string::npos) {
							int gems = atoi(cch.substr(62, cch.length() - 62).c_str());
							if (gems <= 0) break;
							pInfo(peer)->offergems = gems;
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Confirm Checkout``|left|9438|\n\nadd_spacer|small|\nadd_textbox|`2Buy`` `2" + setGems(pInfo(peer)->offergems * 25000) + " `wGems for `1" + to_string(gems) + " Hoshi|\nadd_button|shopmoneybuy|`0Confirm``|NOFLAGS|0|0|\nadd_button||`0Cancel``|NOFLAGS|0|0|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopmoneybuy\n\n") {
							if (pInfo(peer)->offergems <= 0) break;
							if (pInfo(peer)->gtwl >= pInfo(peer)->offergems) {
								pInfo(peer)->gems += (pInfo(peer)->offergems * 25000);
								{
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("You bought `0" + setGems(pInfo(peer)->offergems * 25000) + "`` Gems!");
									p.CreatePacket(peer);
								}
								packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
								gamepacket_t p;
								p.Insert("OnSetBux");
								p.Insert(pInfo(peer)->gems);
								p.Insert(0);
								p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
								if (pInfo(peer)->supp >= 2) {
									p.Insert((float)33796, (float)1, (float)0);
								}
								p.CreatePacket(peer);
								pInfo(peer)->gtwl -= pInfo(peer)->offergems;
							}
							break;
						}
						else if (cch == "action|claimprogressbar\n" || cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopgrowtoken\n\n" || cch == "action|dialog_return\ndialog_name|top\nbuttonClicked|toplist\n\n" || cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopmoney\n\n" || cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprank\n\n" || cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopitems\n\n" || cch == "action|dialog_return\ndialog_name|socialportal\nbuttonClicked|onlinepointhub\n\n") {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprank\n\n") p.Insert(a + "set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase Ranks``|left|9474|\n\nadd_spacer|small|\n\nadd_textbox|`wPlease choose rank that you want to purchase!``|left|\n\nadd_spacer|small|\nadd_label_with_icon_button|small|`o <-- `7Road to Glory " + (pInfo(peer)->glo ? "`c(Owned)" : "") + "``|left|9436|" + (pInfo(peer)->glo ? "" : "shoprankglory") + "|\nadd_label_with_icon_button|small|`o <-- `7Grow Pass " + (pInfo(peer)->gp ? "`c(Owned)" : "") + "``|left|9222|" + (pInfo(peer)->gp ? "" : "shoprankgrowpass") + "|\nadd_spacer|small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopitems\n\n") p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Rare Items``|left|7188|\n\nadd_spacer|small|\n\nadd_textbox|`wPlease choose item that you want to purchase!``|left|\n\nadd_spacer|small|" + shop_list + "||\nadd_button_with_icon||END_LIST|noflags|0|0|\nadd_button|shop|`0Back``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|chc0|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopmoney\n\n")p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Buy Gems``|left|9438|\n\nadd_spacer|small|\nadd_textbox|`11 Hoshi `wis `925,000 `wgems!``|\nadd_textbox|`wYou have `1" + to_string(pInfo(peer)->gtwl) + " Hoshi`w, how much hoshi you want to spend for gems? `7(Enter hoshi amount)``|\nadd_text_input|gemspurchase|`1Hoshi``||30|\nend_dialog|shopgemsconfirm|Cancel|Checkout|\n");
							if (cch == "action|dialog_return\ndialog_name|top\nbuttonClicked|toplist\n\n")p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`8Top worlds today``|left|394|\nadd_spacer|" + top_list + "\nadd_button|wotwlistback|`oBack`|NOFLAGS|0|0|\nend_dialog|top|Close||\n");
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shopgrowtoken\n\n")p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase Growtoken``|left|1486|\nadd_spacer|small|\n\nadd_textbox|`7Please choose packet you want to purchase:``|left|\n\nadd_spacer|small|\nadd_button_with_icon|gtoken_packet_1|`7Buy Packet 1|staticYellowFrame|1486|5|\nadd_button_with_icon|gtoken_packet_2|`7Buy Packet 2|staticYellowFrame|1486|10|\nadd_button_with_icon|gtoken_packet_3|`7Buy Packet 3|staticYellowFrame|1486|50|\nadd_button_with_icon|gtoken_packet_4|`7Buy Packet 4|staticYellowFrame|6802||\nadd_button_with_icon||END_LIST|noflags|0|0|\nadd_button|shop|`0Back``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|chc0|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							if (cch == "action|dialog_return\ndialog_name|socialportal\nbuttonClicked|onlinepointhub\n\n") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Online Star Hub``|left|7074|\nadd_spacer|small|\nadd_textbox|Welcome to Online Star HUB! Do you have any Online Star? You can buy items from me with them.|left|\nadd_smalltext|`2You can earn 1 Online Star every 5 minutes just by playing the game.``|left|\nadd_spacer|small|\nadd_textbox|You have `1" + setGems(pInfo(peer)->opc) + " ``Online Star``.|left|\ntext_scaling_string|99,000OPC|" + opc_list + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_button|chc0|OK|noflags|0|0|\nnend_dialog|gazette||OK|");
							//if (cch == "action|claimprogressbar\n")p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wAbout Valentine's Event``|left|384|\nadd_spacer|small|\nadd_textbox|During Valentine's Week you will gain points for opening Golden Booty Chests. Claim enough points to earn bonus rewards.|left|\nadd_spacer|small|\nadd_textbox|Current Progress: " + to_string(pInfo(peer)->booty_broken) + "/100|left|\nadd_spacer|small|\nadd_textbox|Reward:|left|\nadd_label_with_icon|small|Super Golden Booty Chest|left|9350|\nadd_smalltext|             - 4x chance of getting a Golden Heart Crystal when opening!|left|\nend_dialog|valentines_quest||OK|\n");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprankgrowpass\n\n" || cch == "action|shoprankgrowpass\n\n" || cch == "action|shoprankgrowpass\n" || cch == "action|shoprankgrowpass") {
							gamepacket_t p(500);
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase Grow Pass``|left|9222|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `1100 Hoshi``|left|\nadd_smalltext|Duration: `7[```531 Days```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\nadd_smalltext|Extra: `7Receive daily items everyday, get 2x xp and gems from breaking block or harvesting tree, receive newest growtopia items, unlock exclusive title and skin only for grow pass!``|left|\nadd_spacer|\nadd_button|shoprankgrowpassbuy|`0Purchase for `1100 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprankglory\n\n" || cch == "action|shoprankglory\n\n" || cch == "action|shoprankglory\n" || cch == "action|shoprankglory") {
							gamepacket_t p(500);
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase Road to Glory``|left|9436|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `125 Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\nadd_smalltext|Extra: `7Receive 100k gems instantly, get up to 4M gems by leveling up, and Unlock Account Security feature.``|left|\nadd_spacer|\nadd_button|shoprankglorybuy|`0Purchase for `125 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|gtoken_packet_1") != string::npos) {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase 5 Growtoken``|left|1486|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `15 Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Special Effects:``|left|\nadd_smalltext|`wNo special effects|left|\nadd_spacer|\nadd_button|buy_token_1|`0Purchase for `15 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|gtoken_packet_2") != string::npos) {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase 10 Growtoken``|left|1486|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `110 Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Special Effects:``|left|\nadd_smalltext|`wNo special effects|left|\nadd_spacer|\nadd_button|buy_token_2|`0Purchase for `110 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|gtoken_packet_3") != string::npos) {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase 50 Growtoken``|left|1486|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `150 Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Special Effects:``|left|\nadd_smalltext|`wNo special effects|left|\nadd_spacer|\nadd_button|buy_token_3|`0Purchase for `150 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|gtoken_packet_4") != string::npos) {
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase 1 Mega Growtoken``|left|6802|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `1100 Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Special Effects:``|left|\nadd_smalltext|`wThis item contain 100 Growtoken|left|\nadd_spacer|\nadd_button|buy_token_4|`0Purchase for `1100 Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|shop_price_") != string::npos) {
							int item = atoi(cch.substr(59, cch.length() - 59).c_str());
							if (item <= 0 || item >= items.size() || items[item].pwl == 0) continue;
							string special = "`eThere is none yet for that item``";
							// hoshi shop item special effects description
							if (item == 11118) special = "`wDrop x3 Gems``";
							if (item == 11118) special += ", `wHigher chance on getting ores.``";
							if (item == 5480) special = "`wBreak 3 blocks in a row.``";
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase " + items[item].name + "``|left|" + to_string(items[item].id) + "|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `1" + setGems(items[item].pwl) + " Hoshi``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Special Effects:``|left|\nadd_smalltext|" + special + "|left|\nadd_spacer|\nadd_button|shop_item_" + to_string(item) + "|`0Purchase for `1" + setGems(items[item].pwl) + " Hoshi``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|shop|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|lock_price_") != string::npos) {
							int item = atoi(cch.substr(59, cch.length() - 59).c_str());
							if (item <= 0 || item >= items.size() || items[item].gtwl == 0) continue;
							pInfo(peer)->lockeitem = item;
							int wl = 0, dl = 0;
							modify_inventory(peer, 242, wl);
							modify_inventory(peer, 1796, dl);
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`9Buy " + items[item].name + "?``|left|" + to_string(items[item].id) + "|\nadd_smalltext|`4" + items[item].description + "``|left|\nadd_smalltext|`1Price: " + (items[item].gtwl > 200 ? to_string(items[item].gtwl / 100) : to_string(items[item].gtwl)) + " " + (items[item].gtwl > 200 ? "Diamond Lock" : "World Locks") + "``|left|\nadd_spacer|\nadd_textbox|How many " + items[item].name + " do you want to buy, for " + (items[item].gtwl > 200 ? to_string(items[item].gtwl / 100) : to_string(items[item].gtwl)) + " " + (items[item].gtwl > 200 ? "Diamond Lock" : "World Locks") + " each?|left|\nadd_text_input|howmuch||1|5|\nadd_smalltext|" + (wl + dl != 0 ? "You have " + (wl != 0 ? to_string(wl) + " World Locks" : "") + "" + (dl != 0 ? ", " + to_string(dl) + " Diamond Lock." : ".") + "" : "") + "|left|\nadd_button|lock_item_|`9Purchase``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|locke|No thanks|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|locm_price_") != string::npos) {
							int item = atoi(cch.substr(59, cch.length() - 59).c_str());
							if (item <= 0 || item >= items.size() || items[item].u_gtwl == 0) continue;
							pInfo(peer)->lockeitem = item;
							int wl = 0, dl = 0;
							modify_inventory(peer, 242, wl);
							modify_inventory(peer, 1796, dl);
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`9Buy " + items[item].name + "?``|left|" + to_string(items[item].id) + "|\nadd_smalltext|`4" + items[item].description + "``|left|\nadd_smalltext|`1Price: " + setGems(items[item].u_gtwl) + " World Lock``|left|\nadd_spacer|\nadd_textbox|How many " + items[item].name + " do you want to buy, for " + (items[item].u_gtwl > 200 ? to_string(items[item].u_gtwl / 100) : to_string(items[item].u_gtwl)) + " " + (items[item].u_gtwl > 200 ? "Diamond Lock" : "World Locks") + " each?|left|\nadd_text_input|howmuch||1|5|\nadd_smalltext|" + (wl + dl != 0 ? "You have " + (wl != 0 ? to_string(wl) + " World Locks" : "") + "" + (dl != 0 ? ", " + to_string(dl) + " Diamond Lock." : ".") + "" : "") + "|left|\nadd_button|lock_item_|`9Purchase``|noflags|0|0||small|\n\nadd_quick_exit|\nadd_button|lockm|No thanks|noflags|0|0|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|locke") != string::npos) {
							if (pInfo(peer)->world == "LOCKE") {
								if (thedaytoday == 0 || pInfo(peer)->superdev) {
									int wl = 0, dl = 0;
									modify_inventory(peer, 242, wl);
									modify_inventory(peer, 1796, dl);
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert(a + "set_default_color|`o\n\nadd_label_with_icon|big|`9Locke The Traveling Salesman``|left|2398|\nadd_spacer|small|\nadd_smalltext|Ho there, friend! Locke's my name, and locks are my game. I Love 'em all, Diamond, Huge.. even Small! If you can part with some locks, I'll give you something special in return. Whaddya say?|left|\nadd_spacer|small|\nadd_smalltext|" + (wl + dl != 0 ? "`9(Hmm, smells like you care carrying " + (wl != 0 ? to_string(wl) + " World Locks" : "") + "" + (dl != 0 ? ", and and " + to_string(dl) + " Diamond Lock" : "") + ")``" : "`9(Hmm, smells like you don't care any world locks)``") + "|left|\nadd_spacer|small|" + shop_list2 + "|\nadd_button|chc0|Close|noflags|0|0|\nnend_dialog|gazette||OK|");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|lock_item_\n\nhowmuch|") != string::npos) {
							if (pInfo(peer)->world == "LOCKE") {
								int count = atoi(cch.substr(68, cch.length() - 68).c_str()), count2 = atoi(cch.substr(68, cch.length() - 68).c_str());
								if (count <= 0 || count > 200) continue;
								int item = pInfo(peer)->lockeitem;
								if (item <= 0 || item >= items.size()) continue;
								if (items[item].gtwl == 0 and items[item].u_gtwl == 0) continue;
								int allwl = 0, wl = 0, dl = 0, price = (items[item].gtwl == 0 ? items[item].u_gtwl : items[item].gtwl), priced = 0, bgl = 0;
								price *= count;
								priced = price;
								modify_inventory(peer, 242, wl);
								modify_inventory(peer, 1796, dl);
								modify_inventory(peer, 7188, bgl);
								allwl = wl + (dl * 100);
								int allbgl = bgl * 10000;
								if (allwl >= price || allbgl >= price && price > 20000) {
									int c_ = count;
									if (modify_inventory(peer, item, c_) == 0) {
										if (price <= 20000) {
											if (wl >= price) modify_inventory(peer, 242, price *= -1);
											else {
												modify_inventory(peer, 242, wl *= -1);
												modify_inventory(peer, 1796, dl *= -1);
												int givedl = (allwl - price) / 100;
												int givewl = (allwl - price) - (givedl * 100);
												modify_inventory(peer, 242, givewl);
												modify_inventory(peer, 1796, givedl);
											}
										}
										else {
											int removebgl = (price / 10000) * -1;
											modify_inventory(peer, 7188, removebgl);
										}
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world == pInfo(currentPeer)->world) {
												gamepacket_t p;
												p.Insert("OnConsoleMessage");
												p.Insert("`9[" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->tankIDName) + " bought " + to_string(count2) + " " + items[item].name + " for " + to_string(priced) + " World Lock]");
												p.CreatePacket(currentPeer);
												packet_(currentPeer, "action|play_sfx\nfile|audio/cash_register.wav\ndelayMS|0");
											}
										}
									}
									else {
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("No inventory space.");
										p.CreatePacket(peer);
									}
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`9You don't have enough wls!``");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|shop_item_") != string::npos) {
							int item = atoi(cch.substr(58, cch.length() - 58).c_str());
							if (item <= 0 || item >= items.size() || items[item].pwl == 0) continue;
							if (pInfo(peer)->gtwl >= items[item].pwl) {
								int c_ = 1;
								if (modify_inventory(peer, item, c_) == 0) {
									pInfo(peer)->gtwl -= items[item].pwl;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased " + items[item].name + "!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this item.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|buy_token_1") != string::npos) {
							if (pInfo(peer)->gtwl >= 5) {
								int c_ = 5;
								if (modify_inventory(peer, 1486, c_) == 0) {
									pInfo(peer)->gtwl -= 5;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased 5 Growtoken!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this packet.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|buy_token_2") != string::npos) {
							if (pInfo(peer)->gtwl >= 10) {
								int c_ = 10;
								if (modify_inventory(peer, 1486, c_) == 0) {
									pInfo(peer)->gtwl -= 10;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased 10 Growtoken!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this packet.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|buy_token_3") != string::npos) {
							if (pInfo(peer)->gtwl >= 50) {
								int c_ = 50;
								if (modify_inventory(peer, 1486, c_) == 0) {
									pInfo(peer)->gtwl -= 50;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased 50 Growtoken!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this packet.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|buy_token_4") != string::npos) {
							if (pInfo(peer)->gtwl >= 100) {
								int c_ = 1;
								if (modify_inventory(peer, 6802, c_) == 0) {
									pInfo(peer)->gtwl -= 100;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased 1 Mega Growtoken!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this packet.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|opop_price_") != string::npos) {
							int item = atoi(cch.substr(59, cch.length() - 59).c_str());
							if (item <= 0 || item >= items.size() || items[item].oprc == 0) continue;
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\n\nadd_label_with_icon|big|`0Purchase " + items[item].name + "``|left|" + to_string(items[item].id) + "|\nadd_smalltext|`4Make sure to read this information clearly!``|left|\nadd_smalltext|Price: `3" + setGems(items[item].oprc) + "`` `0Online Star``|left|\nadd_smalltext|Duration: `7[```4~```7]``|left|\nadd_smalltext|Stock: `7[```4~```7]``|left|\n\nadd_textbox|`6Other information:``|left|\nadd_smalltext|" + items[item].description + "|left|\nadd_spacer|\nadd_button|opop_item_" + to_string(item) + "|`0Purchase `1" + setGems(items[item].oprc) + " Star``|noflags|0|0||small|\n\nadd_quick_exit|\nnend_dialog|gazette||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|opop_item_") != string::npos) {
							int item = atoi(cch.substr(58, cch.length() - 58).c_str());
							if (item <= 0 || item >= items.size() || items[item].oprc == 0) continue;
							if (pInfo(peer)->opc >= items[item].oprc) {
								int c_ = 1;
								if (modify_inventory(peer, item, c_) == 0) {
									if (item == 1486 && pInfo(peer)->quest_active && pInfo(peer)->quest_step == 6 && pInfo(peer)->quest_progress < 28) {
										pInfo(peer)->quest_progress++;
										if (pInfo(peer)->quest_progress >= 28) {
											pInfo(peer)->quest_progress = 28;
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("`9Legendary Quest step complete! I'm off to see a Wizard!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
									}
									pInfo(peer)->opc -= items[item].oprc;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased " + items[item].name + "!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Online Star`` to purchase this item.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprankgrowpassbuy\n\n" || cch == "action|shoprankgrowpassbuy\n\n" || cch == "action|shoprankgrowpassbuy\n" || cch == "action|shoprankgrowpassbuy") {
							if (pInfo(peer)->gtwl >= 100) {
								int c_ = 1;
								if (modify_inventory(peer, 11304, c_) == 0) {
									pInfo(peer)->gtwl -= 100;
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased Royal Grow Pass Token, check your inventory and consume to get the benefits!");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("No inventory space.");
									p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage");
								p.Insert("You don't have enough `1Hoshi`` to buy this.");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shoprankglorybuy\n\n" || cch == "action|shoprankglorybuy\n\n" || cch == "action|shoprankglorybuy\n" || cch == "action|shoprankglorybuy") {
							if (pInfo(peer)->gtwl >= 25) {
								pInfo(peer)->gems += 100000;
								gamepacket_t p;
								p.Insert("OnSetBux");
								p.Insert(pInfo(peer)->gems);
								p.Insert(0);
								p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
								if (pInfo(peer)->supp >= 2) {
									p.Insert((float)33796, (float)1, (float)0);
								}
								p.CreatePacket(peer);
								pInfo(peer)->gtwl -= 25;
								packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
								pInfo(peer)->glo = 1;
								{
									gamepacket_t p;
									p.Insert("OnConsoleMessage");
									p.Insert("`o>> You purchased Road to Glory! Wrench yourself and press on Road to Glory button!``");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|showblarneyprogress") != string::npos) {
							gamepacket_t p(550);
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlarney Bonanza!``|left|528|\nadd_spacer|small|\nadd_textbox|Welcome to the Blarney Bonanza|left|\nadd_spacer|small|\nadd_textbox|As you, as a community, complete Blarneys and kiss the most magical stone, items will unlock for you to pick up in the store.|left|\nadd_spacer|small|\nadd_textbox|There are 4 items to unlock throughout the event.|left|\nadd_spacer|small|\nadd_textbox|Items will only remain unlocked for a short amount of time, so make sure you check back often! These items can be unlocked multiple times throughout the week.|left|\nadd_spacer|small|\nend_dialog|blarney_dialog||OK|\n");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shop\n\n" || cch == "action|dialog_return\ndialog_name|\nbuttonClicked|deposit\n\n") {
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|shop\n\n") SendCmd(peer, "/shop", true);
							if (cch == "action|dialog_return\ndialog_name|\nbuttonClicked|deposit\n\n") SendCmd(peer, "/topup", true);
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|claim_reward\n\n") {
							if (pInfo(peer)->gp == 1) {
								if (today_day != pInfo(peer)->gd) {
									vector<int> list2{ 1796, 10396, 11476, 542, 10386, 722, 10826 };
									int receive = 1, item = list2[rand() % list2.size()], received = 1;
									if (item == 10386 || item == 722) receive = 20, received = 20;
									if (item == 542) receive = 100, received = 100;
									if (item == 10826) receive = 5, received = 5;
									if (modify_inventory(peer, item, receive) == 0) {
										pInfo(peer)->gd = today_day;
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("`9 >> You claimed your Grow Pass reward:");
											p.CreatePacket(peer);
										}
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("Given `0" + to_string(received) + " " + items[item].name + "``.");
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17;
										data_.netID = 48;
										data_.YSpeed = 48;
										data_.x = pInfo(peer)->x + 16;
										data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(currentPeer)->world == pInfo(peer)->world) {
												send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											}
										}
										delete[] raw;
									}
									else {
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("Clean your inventory and try again!");
											p.CreatePacket(peer);
										}
									}
								}
								else {
									{
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("You already claimed your reward today!");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nitemid|") != string::npos) {
							int item = atoi(cch.substr(57, cch.length() - 57).c_str());
							if (item <= 0 || item >= items.size()) break;
							if (pInfo(peer)->lastwrenchb != 4516 and items[item].untradeable == 1 or item == 1424) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You can't store Untradeable items there!"), p.CreatePacket(peer);
							}
							else if (pInfo(peer)->lastwrenchb == 4516 and items[item].untradeable == 0 or item == 18 || item == 32 || item == 6336 || item == 1424) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You can't store Tradeable items there!"), p.CreatePacket(peer);
							}
							else {
								int got = 0, receive = 0;
								modify_inventory(peer, item, got);
								if (got == 0) break;
								pInfo(peer)->lastchoosenitem = item;
								gamepacket_t p;
								p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`w" + items[pInfo(peer)->lastwrenchb].name + "``|left|" + to_string(pInfo(peer)->lastwrenchb) + "|\nadd_textbox|You have " + to_string(got) + " " + items[item].name + ". How many to store?|left|\nadd_text_input|itemcount||" + to_string(got) + "|3|\nadd_spacer|small|\nadd_button|do_add|Store Items|noflags|0|0|\nend_dialog|storageboxxtreme|Cancel||\n"), p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nbuttonClicked|itm") != string::npos) {
							int itemnr = atoi(cch.substr(67, cch.length() - 67).c_str()), itemcount = 0;
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_access(peer, world_, block_, false, true)) {
									for (int i_ = 0; i_ < world_->sbox1.size(); i_++) {
										if (world_->sbox1[i_].x == pInfo(peer)->lastwrenchx and world_->sbox1[i_].y == pInfo(peer)->lastwrenchy) {
											itemcount++;
											if (itemnr == itemcount) {
												pInfo(peer)->lastchoosennr = itemnr;
												gamepacket_t p;
												p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`w" + items[pInfo(peer)->lastwrenchb].name + "``|left|" + to_string(pInfo(peer)->lastwrenchb) + "|\nadd_textbox|You have `w" + to_string(world_->sbox1[i_].count) + " " + items[world_->sbox1[i_].id].name + "`` stored.|left|\nadd_textbox|Withdraw how many?|left|\nadd_text_input|itemcount||" + to_string(world_->sbox1[i_].count) + "|3|\nadd_spacer|small|\nadd_button|do_take|Remove Items|noflags|0|0|\nadd_button|cancel|Back|noflags|0|0|\nend_dialog|storageboxxtreme|Exit||\n"), p.CreatePacket(peer);
											}
										}
									}
								}
							}
							break;
						}

						else if (cch.find("action|dialog_return\ndialog_name|donation_box_edit\nitemid|") != string::npos) {
							int item = atoi(cch.substr(58, cch.length() - 58).c_str()), got = 0;
							modify_inventory(peer, item, got);
							if (got == 0) break;
							if (items[item].untradeable == 1 || item == 1424 || items[item].blockType == BlockTypes::FISH) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`7[```4You can't place that in the box, you need it!`7]``"), p.CreatePacket(peer);
							}
							else if (items[item].rarity == 1) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`7[```4This box only accepts items rarity 2+ or greater`7]``"), p.CreatePacket(peer);
							}
							else {
								pInfo(peer)->lastchoosenitem = item;
								gamepacket_t p;
								p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|" + items[item].name + "``|left|" + to_string(item) + "|\nadd_textbox|How many to put in the box as a gift? (Note: You will `4LOSE`` the items you give!)|left|\nadd_text_input|count|Count:|" + to_string(got) + "|5|\nadd_text_input|sign_text|Optional Note:||128|\nadd_spacer|small|\nadd_button|give|`4Give the item(s)``|noflags|0|0|\nadd_spacer|small|\nadd_button|cancel|`wCancel``|noflags|0|0|\nend_dialog|give_item|||\n");
								p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|donation_box_edit\nbuttonClicked|takeall\n") != string::npos) {
							bool took = false, fullinv = false;
							gamepacket_t p3;
							p3.Insert("OnTalkBubble"), p3.Insert(pInfo(peer)->netID);
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (not block_access(peer, world_, block_)) break;
								if (!items[block_->fg].donation) break;
								for (int i_ = 0; i_ < block_->donates.size(); i_++) {
									int receive = block_->donates[i_].count;
									if (modify_inventory(peer, block_->donates[i_].item, block_->donates[i_].count) == 0) {
										took = true;
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("`7[``" + pInfo(peer)->tankIDName + " receives `5" + to_string(receive) + "`` `w" + items[block_->donates[i_].item].name + "`` from `w" + block_->donates[i_].name + "``, how nice!`7]``");
										block_->donates.erase(block_->donates.begin() + i_);
										i_--;
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											p.CreatePacket(currentPeer);
											packet_(currentPeer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
										}
									}
									else fullinv = true;
								}
								if (block_->donates.size() == 0) {
									if (block_->flags & 0x00400000) block_->flags ^= 0x00400000;
									PlayerMoving data_{};
									data_.packetType = 5, data_.punchX = pInfo(peer)->lastwrenchx, data_.punchY = pInfo(peer)->lastwrenchy, data_.characterState = 0x8;
									BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
									BYTE* blc = raw + 56;
									form_visual(blc, *block_, *world_, peer, false);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
										send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
									}
									delete[] raw, blc;
									if (block_->locked) upd_lock(*block_, *world_, peer);
								}
							}
							if (fullinv) {
								p3.Insert("I don't have enough room in my backpack to get the item(s) from the box!");
								gamepacket_t p2;
								p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("`2(Couldn't get all of the gifts)``"), p2.CreatePacket(peer);
							}
							else if (took) p3.Insert("`2Box emptied.``");
							p3.CreatePacket(peer);
							packet_(peer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|surge\n") {
							if (pInfo(peer)->lastwrenchb == 4296 || pInfo(peer)->lastwrenchb == 8558) {
								setstats(peer, rand() % 30, "", items[pInfo(peer)->lastwrenchb].name);
								pInfo(peer)->lastwrenchb = 0;
							}
							break;
						}
						// 
						else if (cch.find("action|dialog_return\ndialog_name|give_item\nbuttonClicked|give\n\ncount|") != string::npos) {
							int count = atoi(cch.substr(69, cch.length() - 69).c_str()), got = 0;
							string text = cch.substr(80 + to_string(count).length(), cch.length() - 80 + to_string(count).length()).c_str();
							replace_str(text, "\n", "");
							modify_inventory(peer, pInfo(peer)->lastchoosenitem, got);
							if (text.size() > 128 || got <= 0 || count <= 0 || count > items.size()) break;
							if (count > got || items[pInfo(peer)->lastchoosenitem].untradeable == 1 || pInfo(peer)->lastchoosenitem == 1424 || items[pInfo(peer)->lastchoosenitem].blockType == BlockTypes::FISH) {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(peer)->netID);
								if (count > got) p.Insert("You don't have that to give!");
								else p.Insert("`7[```4You can't place that in the box, you need it!`7]``");
								p.CreatePacket(peer);
							}
							else {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
									if (!items[block_->fg].donation) break;
									Donate donate_{};
									donate_.item = pInfo(peer)->lastchoosenitem, donate_.count = count, donate_.name = pInfo(peer)->tankIDName, donate_.text = text;
									block_->donates.push_back(donate_);
									{
										PlayerMoving data_{};
										data_.packetType = 5, data_.punchX = pInfo(peer)->lastwrenchx, data_.punchY = pInfo(peer)->lastwrenchy, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
										BYTE* blc = raw + 56;
										block_->flags = (block_->flags & 0x00400000 ? block_->flags : block_->flags | 0x00400000);
										form_visual(blc, *block_, *world_, peer, false, true);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw, blc;
										if (block_->locked) upd_lock(*block_, *world_, peer);
									}
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
										gamepacket_t p, p2;
										p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`7[```5[```w" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->tankIDName) + "`` places `5" + to_string(count) + "`` `2" + items[pInfo(peer)->lastchoosenitem].name + "`` into the " + items[pInfo(peer)->lastwrenchb].name + "`5]```7]``");
										p.CreatePacket(currentPeer);
										p2.Insert("OnConsoleMessage");
										p2.Insert("`7[```5[```w" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->tankIDName) + "`` places `5" + to_string(count) + "`` `2" + items[pInfo(peer)->lastchoosenitem].name + "`` into the " + items[pInfo(peer)->lastwrenchb].name + "`5]```7]``");
										p2.CreatePacket(currentPeer);
										packet_(currentPeer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
									}
									//webhook here
									modify_inventory(peer, pInfo(peer)->lastchoosenitem, count *= -1);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nbuttonClicked|cancel") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (world_->owner_name == pInfo(peer)->tankIDName || pInfo(peer)->dev)load_storagebox(peer, world_, block_);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nbuttonClicked|do_take\n\nitemcount|") != string::npos) {
							int itemnr = pInfo(peer)->lastchoosennr, countofremoval = atoi(cch.substr(83, cch.length() - 83).c_str()), removed = 0, itemcount = 0;
							removed = countofremoval;
							if (countofremoval <= 0) break;
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_access(peer, world_, block_, false, true)) {
									for (int i_ = 0; i_ < world_->sbox1.size(); i_++) {
										if (world_->sbox1[i_].x == pInfo(peer)->lastwrenchx and world_->sbox1[i_].y == pInfo(peer)->lastwrenchy) {
											itemcount++;
											if (itemnr == itemcount and countofremoval < world_->sbox1[i_].count) {
												if (world_->sbox1[i_].count <= 0) break;
												world_->sbox1[i_].count -= removed;
												if (modify_inventory(peer, world_->sbox1[i_].id, countofremoval) == 0) {
													gamepacket_t p, p2;
													p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("Removed `w" + to_string(removed) + " " + items[world_->sbox1[i_].id].name + "`` in " + items[pInfo(peer)->lastwrenchb].name + "."), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
													p2.Insert("OnConsoleMessage"), p2.Insert("Removed `w" + to_string(removed) + " " + items[world_->sbox1[i_].id].name + "`` in the " + items[pInfo(peer)->lastwrenchb].name + "."), p2.CreatePacket(peer);
													PlayerMoving data_{};
													data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16, data_.packetType = 19, data_.plantingTree = 500, data_.punchX = world_->sbox1[i_].id, data_.punchY = pInfo(peer)->netID;
													int32_t to_netid = pInfo(peer)->netID;
													BYTE* raw = packPlayerMoving(&data_);
													raw[3] = 5;
													memcpy(raw + 8, &to_netid, 4);
													send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
													delete[] raw;
													i_ = world_->sbox1.size();
												}
											}
											else if (itemnr == itemcount and world_->sbox1[i_].count == countofremoval) {
												if (world_->sbox1[i_].count <= 0) break;
												if (modify_inventory(peer, world_->sbox1[i_].id, countofremoval) == 0) {
													gamepacket_t p, p2;
													p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("Removed `w" + to_string(removed) + " " + items[world_->sbox1[i_].id].name + "`` in " + items[pInfo(peer)->lastwrenchb].name + "."), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
													p2.Insert("OnConsoleMessage"), p2.Insert("Removed `w" + to_string(removed) + " " + items[world_->sbox1[i_].id].name + "`` in the " + items[pInfo(peer)->lastwrenchb].name + "."), p2.CreatePacket(peer);
													PlayerMoving data_{};
													data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16;
													data_.packetType = 19, data_.plantingTree = 500;
													data_.punchX = world_->sbox1[i_].id, data_.punchY = pInfo(peer)->netID;
													int32_t to_netid = pInfo(peer)->netID;
													BYTE* raw = packPlayerMoving(&data_);
													raw[3] = 5;
													memcpy(raw + 8, &to_netid, 4);
													send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
													delete[] raw;
													world_->sbox1.erase(world_->sbox1.begin() + i_);
													i_ = world_->sbox1.size();
												}
											}
										}
									}
								}
							}
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|builder_reward\n\n" || cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|fishing_reward\n\n" || cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|geiger_reward\n\n" || cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|farmer_reward\n\n" || cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|provider_reward\n\n") {
							if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|farmer_reward\n\n") farmer_reward_show(peer);
							if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|provider_reward\n\n")provider_reward_show(peer);
							if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|geiger_reward\n\n") geiger_reward_show(peer);
							if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|fishing_reward\n\n") 	fishing_reward_show(peer);
							if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|builder_reward\n\n")	builder_reward_show(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|dialog_eq_aura\nbutton_item_selection|") != string::npos) {
							int item = atoi(cch.substr(70, cch.length() - 70).c_str());
							if (item > 0 && item < items.size()) {
								if (items[item].toggleable) {
									pInfo(peer)->eq_a_1 = item;
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wEQ Aura``|left|12634|\nadd_spacer|small|\nadd_textbox|Play music wherever you are with the EQ Aura! Choose a musical block from your inventory to play the song.|left|\nadd_spacer|small|" + (string(pInfo(peer)->eq_a_1 != 0 ? "\nadd_label_with_icon|small|`w" + items[pInfo(peer)->eq_a_1].name + "``|left|" + to_string(pInfo(peer)->eq_a_1) + "|\nadd_spacer|small|" : "")) + "\nadd_item_picker|button_item_selection|`wChange Block Item``|Choose Musical Block Item!|\nadd_button|restore_default|`wRemove Block Item``|noflags|0|0|\nadd_spacer|small|\nadd_spacer|small|\nend_dialog|dialog_eq_aura|Cancel|Update|\nadd_quick_exit|");
									p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wEQ Aura``|left|12634|\nadd_spacer|small|\nadd_spacer|small|\nadd_textbox|`4Invalid item! You can only use musical block items! Please choose something else.``|left|\nadd_spacer|small|\nadd_textbox|Play music wherever you are with the EQ Aura! Choose a musical block from your inventory to play the song.|left|\nadd_spacer|small|" + (string(pInfo(peer)->eq_a != 0 ? "\nadd_label_with_icon|small|`w" + items[pInfo(peer)->eq_a].name + "``|left|" + to_string(pInfo(peer)->eq_a) + "|\nadd_spacer|small|" : "")) + "\nadd_item_picker|button_item_selection|`wChange Block Item``|Choose Musical Block Item!|\nadd_button|restore_default|`wRemove Block Item``|noflags|0|0|\nadd_spacer|small|\nadd_spacer|small|\nend_dialog|dialog_eq_aura|Cancel|Update|\nadd_quick_exit|");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|dialog_eq_aura\nbuttonClicked|restore_default") != string::npos) {
							pInfo(peer)->eq_a_1 = 0;
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wEQ Aura``|left|12634|\nadd_spacer|small|\nadd_textbox|Play music wherever you are with the EQ Aura! Choose a musical block from your inventory to play the song.|left|\nadd_spacer|small|" + (string(pInfo(peer)->eq_a_1 != 0 ? "\nadd_label_with_icon|small|`w" + items[pInfo(peer)->eq_a_1].name + "``|left|" + to_string(pInfo(peer)->eq_a_1) + "|\nadd_spacer|small|" : "")) + "\nadd_item_picker|button_item_selection|`wChange Block Item``|Choose Musical Block Item!|\nadd_button|restore_default|`wRemove Block Item``|noflags|0|0|\nadd_spacer|small|\nadd_spacer|small|\nend_dialog|dialog_eq_aura|Cancel|Update|\nadd_quick_exit|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|dialog_eq_aura") != string::npos) {
							if (pInfo(peer)->eq_a_1 != 0 && !pInfo(peer)->eq_a_update) pInfo(peer)->eq_a = pInfo(peer)->eq_a_1, pInfo(peer)->eq_a_update = true;
							if (pInfo(peer)->eq_a_1 == 0) pInfo(peer)->eq_a_1 = 0, pInfo(peer)->eq_a = 0;
							update_clothes(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|\nbuttonClicked|trans_") != string::npos) {
							int item = atoi(cch.substr(54, cch.length() - 54).c_str());
							if (item <= 0 || item >= items.size()) break;
							if (item == 256) {
								gamepacket_t p(0, pInfo(peer)->netID);
								p.Insert("OnFlagMay2019"), p.Insert(256);
								pInfo(peer)->flagmay = 256;
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
									if (pInfo(peer)->world == pInfo(currentPeer)->world) p.CreatePacket(currentPeer);
								}
							}
							int got = 0;
							modify_inventory(peer, item, got);
							if (got == 0) break;
							if (items[item].flagmay == 256) break;
							gamepacket_t p(0, pInfo(peer)->netID);
							pInfo(peer)->flagmay = items[item].flagmay;
							p.Insert("OnFlagMay2019"), p.Insert(items[item].flagmay);
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
								if (pInfo(peer)->world == pInfo(currentPeer)->world) p.CreatePacket(currentPeer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|t_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 98, 228, 1746, 1778, 1830, 5078, 1966, 6948, 6946, 4956 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 228 || list[reward - 1] == 1746 || list[reward - 1] == 1778) count = 200;
							if (find(pInfo(peer)->t_p.begin(), pInfo(peer)->t_p.end(), lvl = reward * 5) == pInfo(peer)->t_p.end()) {
								if (pInfo(peer)->t_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->t_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Farmer Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										farmer_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|punish_view\nbuttonClicked|viewnetwork") != string::npos) {
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
								if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
									string worlds_owned_ = "";
									for (int w_ = 0; w_ < pInfo(currentPeer)->worlds_owned.size(); w_++) worlds_owned_ += pInfo(currentPeer)->worlds_owned[w_] + ", ";
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert("\nadd_label|big|" + pInfo(peer)->last_wrenched + "'s Network and Assets|\nadd_spacer|small|\nadd_textbox|`6Network Info``|left|\nadd_smalltext|Status: `2ONLINE``|left|\nadd_smalltext | IP: `5" + pInfo(currentPeer)->ip + "``|left|\nadd_smalltext|RID: `5" + pInfo(currentPeer)->rid + "``|left|\nadd_smalltext|MAC Address: `5" + pInfo(currentPeer)->mac + "``|left|\nadd_smalltext|Country Code: `5" + to_upper(pInfo(currentPeer)->country) + "``|left|\nadd_spacer|small\nadd_textbox|`6Assets Info``|left|Ranks: `5[``" + (pInfo(currentPeer)->supp == 1 ? "`2Supporter``/" : pInfo(currentPeer)->supp == 2 ? "`5Super Supporter``/" : "`4NOT_SUPPORTER``") + (pInfo(currentPeer)->mod == 1 ? "`#Moderator``/" : "") + (pInfo(currentPeer)->smod == 1 ? "`#Super Moderator``/" : "") + (pInfo(currentPeer)->dev == 1 ? "`6Developer``/" : "") + (pInfo(currentPeer)->superdev == 1 ? "`6Super Developer" : "") + "`5]``|left|\nadd_smalltext|Online Star: `5" + setGems(pInfo(currentPeer)->opc) + "``|\nadd_smalltext|Level: `5" + to_string(pInfo(currentPeer)->level) + "``|left|\nadd_smalltext|Gems: `5" + setGems(pInfo(currentPeer)->gems) + "``|left|\nadd_smalltext|XP: `5" + setGems(pInfo(currentPeer)->xp) + "``|left|\nadd_smalltext|Owned Worlds: `5" + worlds_owned_ + "``|left|\nadd_smalltext|This player needs " + setGems((50 * (pInfo(currentPeer)->level * pInfo(currentPeer)->level) + 2) - pInfo(currentPeer)->xp) + " XP to be level " + to_string(pInfo(currentPeer)->level + 1) + "|left|\nadd_smalltext|This account was created `w~`` days ago.``|left|\nadd_spacer|small|\nend_dialog||OK!||\nadd_quick_exit|\n");
									p.CreatePacket(peer);
								}
							}
						}
						else if (cch.find("action|dialog_return\ndialog_name|view_inventory\nbuttonClicked|") != string::npos) {
							if (pInfo(peer)->superdev) {
								int item = atoi(cch.substr(62, cch.length() - 62).c_str()), got = 0;
								pInfo(peer)->choosenitem = item;
								if (item <= 0 || item > items.size()) break;
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
										modify_inventory(currentPeer, pInfo(peer)->choosenitem, got);
										gamepacket_t p;
										p.Insert("OnDialogRequest");
										p.Insert("set_default_color|`o\nadd_label_with_icon|big|`4Take`` `w" + items[pInfo(peer)->choosenitem].name + " from`` `#" + pInfo(currentPeer)->tankIDName + "``|left|" + to_string(pInfo(peer)->choosenitem) + "|\nadd_textbox|How many to `4take``? (player has " + to_string(got) + ")|left|\nadd_text_input|count||" + to_string(got) + "|5|\nend_dialog|take_item|Cancel|OK|");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|take_item\ncount|") != string::npos) {
							if (pInfo(peer)->superdev) {
								int count = atoi(cch.substr(49, cch.length() - 49).c_str()), receive = atoi(cch.substr(49, cch.length() - 49).c_str());
								int remove = count * -1;
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
										if (count <= 0 || count > 200) break;
										if (modify_inventory(peer, pInfo(peer)->choosenitem, count) == 0) {
											int total = 0;
											modify_inventory(currentPeer, pInfo(peer)->choosenitem, total += remove);
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("Collected `w" + to_string(receive) + " " + items[pInfo(peer)->choosenitem].name + "``." + (items[pInfo(peer)->choosenitem].rarity > 363 ? "" : " Rarity: `w" + to_string(items[pInfo(peer)->choosenitem].rarity) + "``") + "");
											p.CreatePacket(peer);
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|p_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 1008,1044,872,10450,870,5084,876,6950,6952,1674 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 1008) count = 5;
							if (list[reward - 1] == 1044) count = 50;
							if (list[reward - 1] == 872) count = 200;
							if (list[reward - 1] == 10450) count = 3;
							if (find(pInfo(peer)->p_p.begin(), pInfo(peer)->p_p.end(), lvl = reward * 5) == pInfo(peer)->p_p.end()) {
								if (pInfo(peer)->p_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->p_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Provider Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										provider_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|g_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 4654,262,826,828,9712,3146,2266,5072,5070,9716 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 262 || list[reward - 1] == 826 || list[reward - 1] == 828) count = 50;
							if (list[reward - 1] == 3146) count = 10;
							if (find(pInfo(peer)->g_p.begin(), pInfo(peer)->g_p.end(), lvl = reward * 5) == pInfo(peer)->g_p.end()) {
								if (pInfo(peer)->g_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->g_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Geiger Hunting Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										geiger_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|f_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 3010, 3018, 3020, 3044, 5740, 3042, 3098, 3100, 3040, 10262 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 3018) count = 200;
							if (list[reward - 1] == 3020 || list[reward - 1] == 3098) count = 50;
							if (list[reward - 1] == 3044) count = 25;
							if (find(pInfo(peer)->ff_p.begin(), pInfo(peer)->ff_p.end(), lvl = reward * 5) == pInfo(peer)->ff_p.end()) {
								if (pInfo(peer)->ff_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->ff_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Fishing Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										fishing_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|roadtoglory\nbuttonClicked|claimreward") != string::npos) {
							int count = atoi(cch.substr(70, cch.length() - 70).c_str());
							if (count < 1 || count >10) break;
							if (std::find(pInfo(peer)->glo_p.begin(), pInfo(peer)->glo_p.end(), count) == pInfo(peer)->glo_p.end()) {
								if (pInfo(peer)->level >= count * 10) {
									pInfo(peer)->glo_p.push_back(count);
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnSetBux");
									p.Insert(pInfo(peer)->gems += count * 100000);
									p.Insert(0);
									p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
									if (pInfo(peer)->supp >= 2) {
										p.Insert((float)33796, (float)1, (float)0);
									}
									p.CreatePacket(peer);
									{
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("Congratulations! You have received your Road to Glory Reward!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
									PlayerMoving data_{};
									data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
									BYTE* raw = packPlayerMoving(&data_);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
										send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									}
									delete[] raw;
									glory_show(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|bulletin_edit\nbuttonClicked|clear\n") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							packet_(peer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (items[block_->fg].blockType == BlockTypes::BULLETIN_BOARD || items[block_->fg].blockType == BlockTypes::MAILBOX) {
									for (int i_ = 0; i_ < world_->bulletin.size(); i_++) {
										if (world_->bulletin[i_].x == pInfo(peer)->lastwrenchx and world_->bulletin[i_].y == pInfo(peer)->lastwrenchy) {
											world_->bulletin.erase(world_->bulletin.begin() + i_);
											i_--;
										}
									}
									{
										gamepacket_t p;
										p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(items[pInfo(peer)->lastwrenchb].blockType == BlockTypes::MAILBOX ? "`2Mailbox emptied.``" : "`2Text cleared.``"), p.Insert(0), p.Insert(0), p.CreatePacket(peer);
									}
									if (block_->flags & 0x00400000) block_->flags ^= 0x00400000;
									PlayerMoving data_{};
									data_.packetType = 5, data_.punchX = pInfo(peer)->lastwrenchx, data_.punchY = pInfo(peer)->lastwrenchy, data_.characterState = 0x8;
									BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
									BYTE* blc = raw + 56;
									form_visual(blc, *block_, *world_, peer, false);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
										send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
									}
									delete[] raw, blc;
									if (block_->locked) upd_lock(*block_, *world_, peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|remove_bulletin") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								int letter = 0;
								World* world_ = &worlds[p - worlds.begin()];
								for (int i_ = 0; i_ < world_->bulletin.size(); i_++) {
									if (world_->bulletin[i_].x == pInfo(peer)->lastwrenchx and world_->bulletin[i_].y == pInfo(peer)->lastwrenchy) {
										letter++;
										if (pInfo(peer)->lastchoosennr == letter) {
											world_->bulletin.erase(world_->bulletin.begin() + i_);
											{
												gamepacket_t p;
												p.Insert("OnTalkBubble");
												p.Insert(pInfo(peer)->netID);
												p.Insert("`2Bulletin removed.``");
												p.Insert(0), p.Insert(0);
												p.CreatePacket(peer);
											}
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|bulletin_edit\nbuttonClicked|edit") != string::npos) {
							int count = atoi(cch.substr(65, cch.length() - 65).c_str()), letter = 0;
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								for (int i_ = 0; i_ < world_->bulletin.size(); i_++) {
									if (world_->bulletin[i_].x == pInfo(peer)->lastwrenchx and world_->bulletin[i_].y == pInfo(peer)->lastwrenchy) {
										letter++;
										if (count == letter) {
											pInfo(peer)->lastchoosennr = count;
											gamepacket_t p;
											p.Insert("OnDialogRequest");
											p.Insert("set_default_color|`o\nadd_label_with_icon|small|Remove`` \"`w" + world_->bulletin[i_].text + "\"`` from your board?|left|" + to_string(pInfo(peer)->lastwrenchb) + "|\nend_dialog|remove_bulletin|Cancel|OK|");
											p.CreatePacket(peer);
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|bulletin_edit\nbuttonClicked|send\n\nsign_text|") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							string text = explode("\n", t_[4])[0].c_str();
							replace_str(text, "\n", "");
							if (text.length() <= 2 || text.length() >= 100) {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(peer)->netID);
								p.Insert("That's not interesting enough to post.");
								p.Insert(0), p.Insert(0);
								p.CreatePacket(peer);
							}
							else {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									{
										World* world_ = &worlds[p - worlds.begin()];
										if (items[pInfo(peer)->lastwrenchb].blockType == BlockTypes::MAILBOX || items[pInfo(peer)->lastwrenchb].blockType == BlockTypes::BULLETIN_BOARD) {
											int letter = 0;
											for (int i_ = 0; i_ < world_->bulletin.size(); i_++) if (world_->bulletin[i_].x == pInfo(peer)->lastwrenchx and world_->bulletin[i_].y == pInfo(peer)->lastwrenchy) letter++;
											if (letter == 21) world_->bulletin.erase(world_->bulletin.begin() + 0);
											WorldBulletin bulletin_{};
											bulletin_.x = pInfo(peer)->lastwrenchx, bulletin_.y = pInfo(peer)->lastwrenchy;
											if (pInfo(peer)->name_color == "`4@Dr. " || pInfo(peer)->name_color == "`6@" || pInfo(peer)->name_color == "`9@" || pInfo(peer)->name_color == "`#@" || pInfo(peer)->name_color == "`0") bulletin_.name = (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "``";
											else bulletin_.name = "`0" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->tankIDName) + "``";
											bulletin_.text = text;
											world_->bulletin.push_back(bulletin_);
											{
												gamepacket_t p;
												p.Insert("OnTalkBubble");
												p.Insert(pInfo(peer)->netID);
												p.Insert(items[pInfo(peer)->lastwrenchb].blockType == BlockTypes::MAILBOX ? "`2You place your letter in the mailbox.``" : "`2Bulletin posted.``");
												p.Insert(0), p.Insert(0);
												p.CreatePacket(peer);
												packet_(peer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
											}
											if (items[pInfo(peer)->lastwrenchb].blockType == BlockTypes::MAILBOX) {
												WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
												block_->flags = (block_->flags & 0x00400000 ? block_->flags : block_->flags | 0x00400000);
												PlayerMoving data_{};
												data_.packetType = 5, data_.punchX = pInfo(peer)->lastwrenchx, data_.punchY = pInfo(peer)->lastwrenchy, data_.characterState = 0x8;
												BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_));
												BYTE* blc = raw + 56;
												form_visual(blc, *block_, *world_, peer, false, true);
												for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
													if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
													send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_), ENET_PACKET_FLAG_RELIABLE);
												}
												delete[] raw, blc;
												if (block_->locked) upd_lock(*block_, *world_, peer);
											}
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nbuttonClicked|change_password") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_->fg == 8878) {
									if (block_access(peer, world_, block_, false, true)) {
										gamepacket_t p;
										p.Insert("OnDialogRequest");
										p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSafe Vault``|left|8878|\nadd_smalltext|The ingenious minds at GrowTech bring you the `2Safe Vault`` - a place to store your items safely with its integrated password option!|left|\nadd_smalltext|How the password works:|left|\nadd_smalltext|The Safe Vault requires both a `2password`` and a `2recovery answer`` to be entered to use a password.|left|\nadd_smalltext|Enter your `2password`` and `2recovery answer`` below - keep them safe and `4DO NOT`` share them with anyone you do not trust!|left|\nadd_smalltext|The password and recovery answer can be no longer than 12 characters in length - number and alphabet only please, no special characters are allowed!|left|\nadd_smalltext|If you forget your password, enter your recovery answer to access the Safe Vault - The Safe Vault will `4NOT be password protected now``. You will need to enter a new password.|left|\nadd_smalltext|You can change your password, however you will need to enter the old password before a new one can be used.|left|\nadd_smalltext|`4WARNING``: DO NOT forget your password and recovery answer or you will not be able to access the Safe Vault!|left|\nadd_textbox|`4There is no password currently set on this Safe Vault.``|left|\nadd_textbox|Enter a new password.|left|\nadd_text_input_password|storage_newpassword|||18|\nadd_textbox|Enter a recovery answer.|left|\nadd_text_input|storage_recoveryanswer|||12|\nadd_button|set_password|Update Password|noflags|0|0|\nend_dialog|storageboxpassword|Exit||\nadd_quick_exit|");
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxpassword") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_->fg == 8878) {
									if (world_->owner_name == pInfo(peer)->tankIDName || pInfo(peer)->dev) {
										vector<string> t_ = explode("|", cch);
										if (t_.size() < 4) break;
										string button = explode("\n", t_[3])[0].c_str();
										if (button == "set_password") {
											string text = explode("\n", t_[4])[0].c_str(), text2 = explode("\n", t_[5])[0].c_str();
											replace_str(text, "\n", "");
											replace_str(text2, "\n", "");
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											if (not check_blast(text) || not check_blast(text2)) p.Insert("`4Your input contains special characters. It should only contain alphanumeric characters!``");
											else if (text == "") p.Insert("`4You did not enter a new password!``");
											else if (text2 == "") p.Insert("`4You did not enter a recovery answer!``");
											else if (text.length() > 12 || text2.length() > 12) p.Insert("`4The password is too long! You can only use a maximum of 12 characters!``");
											else {
												p.Insert("`2Your password has been updated!``");
												block_->door_destination = text;
												block_->door_id = text2;
											}
											p.Insert(0), p.Insert(1);
											p.CreatePacket(peer);
										}
										else if (button == "check_password") {
											string password = cch.substr(99, cch.length() - 100).c_str();
											if (password == block_->door_destination) load_storagebox(peer, world_, block_);
											else {
												gamepacket_t p;
												p.Insert("OnTalkBubble");
												p.Insert(pInfo(peer)->netID), p.Insert("`4The password you e did not match!``"), p.Insert(0), p.Insert(1);
												p.CreatePacket(peer);
											}
										}
										else if (button == "show_recoveryanswer") {
											gamepacket_t p;
											p.Insert("OnDialogRequest");
											p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSafe Vault``|left|8878|\nadd_textbox|Please enter recovery answer.|left|\nadd_text_input|storage_recovery_answer|||12|\nadd_button|check_recovery|Enter Recovery Answer|noflags|0|0|\nend_dialog|storageboxpassword|Exit||\nadd_quick_exit|");
											p.CreatePacket(peer);
										}
										else if (button == "check_recovery") {
											string password = cch.substr(106, cch.length() - 107).c_str();
											if (password == block_->door_id) {
												block_->door_destination = "", block_->door_id = "";
												load_storagebox(peer, world_, block_);
											}
											else {
												gamepacket_t p;
												p.Insert("OnTalkBubble");
												p.Insert(pInfo(peer)->netID), p.Insert("`4The recovery answer you entered does not match!``"), p.Insert(0), p.Insert(1);
												p.CreatePacket(peer);
											}
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|advbegins\nnameEnter|") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_->fg == 4722) {
									if (block_access(peer, world_, block_)) {
										string text = cch.substr(53, cch.length() - 54).c_str();
										if (text.size() > 32) break;
										block_->heart_monitor = text;
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID), p.Insert("Updated adventure!"), p.Insert(0), p.Insert(0), p.CreatePacket(peer);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world or pInfo(currentPeer)->adventure_begins == false) continue;
											pInfo(currentPeer)->adventure_begins = false;
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|storageboxxtreme\nbuttonClicked|do_add\n\nitemcount|") != string::npos) {
							int count = atoi(cch.substr(82, cch.length() - 82).c_str());
							if (pInfo(peer)->lastchoosenitem <= 0 || pInfo(peer)->lastchoosenitem >= items.size()) break;
							if (pInfo(peer)->lastwrenchb != 4516 and items[pInfo(peer)->lastchoosenitem].untradeable == 1 or pInfo(peer)->lastchoosenitem == 1424 or items[pInfo(peer)->lastchoosenitem].blockType == BlockTypes::FISH) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You can't store Untradeable items there!"), p.CreatePacket(peer);
							}
							else if (pInfo(peer)->lastwrenchb == 4516 and items[pInfo(peer)->lastchoosenitem].untradeable == 0 or pInfo(peer)->lastchoosenitem == 1424 || items[pInfo(peer)->lastchoosenitem].blockType == BlockTypes::FISH || pInfo(peer)->lastchoosenitem == 18 || pInfo(peer)->lastchoosenitem == 32 || pInfo(peer)->lastchoosenitem == 6336) {
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You can't store Tradeable items there!"), p.CreatePacket(peer);
							}
							else {
								int got = 0, receive = 0;
								modify_inventory(peer, pInfo(peer)->lastchoosenitem, got);
								if (count <= 0 || count > got) {
									gamepacket_t p;
									p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You don't have that many!"), p.CreatePacket(peer);
								}
								else {
									receive = count * -1;
									string name_ = pInfo(peer)->world;
									vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
									if (p != worlds.end()) {
										World* world_ = &worlds[p - worlds.begin()];
										WorldBlock block_ = world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
										if (items[pInfo(peer)->lastchoosenitem].untradeable == 1 && block_.fg != 4516) break;
										if (items[block_.fg].blockType != BlockTypes::STORAGE) break;
										gamepacket_t p1, p2;
										p1.Insert("OnTalkBubble"), p1.Insert(pInfo(peer)->netID), p1.Insert("Stored `w" + to_string(count) + " " + items[pInfo(peer)->lastchoosenitem].name + "`` in " + items[pInfo(peer)->lastwrenchb].name + "."), p1.CreatePacket(peer);
										p2.Insert("OnConsoleMessage"), p2.Insert("Stored `w" + to_string(count) + " " + items[pInfo(peer)->lastchoosenitem].name + "`` in the " + items[pInfo(peer)->lastwrenchb].name + "."), p2.CreatePacket(peer);
										modify_inventory(peer, pInfo(peer)->lastchoosenitem, receive);
										bool dublicated = true;
										for (int i_ = 0; i_ < world_->sbox1.size(); i_++) {
											if (dublicated) {
												if (world_->sbox1[i_].x == pInfo(peer)->lastwrenchx and world_->sbox1[i_].y == pInfo(peer)->lastwrenchy and world_->sbox1[i_].id == pInfo(peer)->lastchoosenitem and world_->sbox1[i_].count + count <= 200) {
													world_->sbox1[i_].count += count;
													dublicated = false;
												}
											}
										}
										if (dublicated) {
											WorldSBOX1 sbox1_{};
											sbox1_.x = pInfo(peer)->lastwrenchx, sbox1_.y = pInfo(peer)->lastwrenchy;
											sbox1_.id = pInfo(peer)->lastchoosenitem, sbox1_.count = count;
											world_->sbox1.push_back(sbox1_);
										}
										PlayerMoving data_{};
										data_.packetType = 19, data_.netID = -1, data_.plantingTree = 0;
										data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16;
										data_.XSpeed = pInfo(peer)->x + 16, data_.YSpeed = pInfo(peer)->y + 16;
										data_.punchX = pInfo(peer)->lastchoosenitem;
										BYTE* raw = packPlayerMoving(&data_);
										raw[3] = 6;
										send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										delete[] raw;
										load_storagebox(peer, world_, &block_);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|punish_view\nbuttonClicked|ipban") != string::npos) {
							if (pInfo(peer)->dev == 1) {
								if (to_lower(pInfo(peer)->last_wrenched) == "ritshu") break;
								string his_ip = "";
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (to_lower(pInfo(currentPeer)->tankIDName) == to_lower(pInfo(peer)->last_wrenched)) {
										add_ban(currentPeer, 6.307e+7, "No Reason Provided", pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``");
										writelog(pInfo(peer)->tankIDName + " IP Banned (" + pInfo(currentPeer)->ip + ") - " + pInfo(currentPeer)->tankIDName);
										add_ipban(currentPeer);
									}
								}
								if (not his_ip.empty()) {
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(currentPeer)->ip == pInfo(peer)->ip) {
											add_ban(currentPeer, 6.307e+7, "No Reason Provided", pInfo(peer)->name_color + pInfo(peer)->tankIDName + "``");
											writelog(pInfo(peer)->tankIDName + " IP Banned (" + pInfo(currentPeer)->ip + ") - " + pInfo(currentPeer)->tankIDName);
											add_ipban(currentPeer);
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|drop") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								vector<string> t_ = explode("|", cch);
								if (t_.size() < 4) break;
								int id_ = atoi(explode("\n", t_[3])[0].c_str()), c_ = 0;
								if (id_ <= 0 or id_ >= items.size()) break;
								if (find(world_->active_jammers.begin(), world_->active_jammers.end(), 4758) != world_->active_jammers.end()) {
									if (world_->owner_name != (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->tankIDName) and not pInfo(peer)->dev and find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end()) {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("The Mini-Mod says no dropping items in this world!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
										break;
									}
								}
								if (items[id_].untradeable or id_ == 1424) {
									gamepacket_t p;
									p.Insert("OnTextOverlay");
									p.Insert("You can't drop that.");
									p.CreatePacket(peer);
									break;
								}
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									int a_ = rand() % 12;
									int x = (pInfo(peer)->state == 16 ? pInfo(peer)->x - (a_ + 20) : (pInfo(peer)->x + 20) + a_);
									int y = pInfo(peer)->y + rand() % 16;
									//BlockTypes type_ = FOREGROUND;
									int where_ = (pInfo(peer)->state == 16 ? x / 32 : round((double)x / 32)) + (y / 32 * 100);
									if (where_ < 0 || x < 0 || y < 0 || where_ > 5399) continue;
									WorldBlock* block_ = &world_->blocks[where_];
									if (items[block_->fg].collisionType == 1 || block_->fg == 6 || items[block_->fg].entrance || items[block_->fg].toggleable and is_false_state(world_->blocks[(pInfo(peer)->state == 16 ? x / 32 : round((double)x / 32)) + (y / 32 * 100)], 0x00400000)) {
										gamepacket_t p;
										p.Insert("OnTextOverlay");
										p.Insert(items[block_->fg].blockType == BlockTypes::MAIN_DOOR ? "You can't drop items on the white door." : "You can't drop that here, face somewhere with open space.");
										p.CreatePacket(peer);
										break;
									}
									int count_ = 0;
									bool dublicated = false;
									for (int i_ = 0; i_ < world_->drop.size(); i_++) {
										if (abs(world_->drop[i_].y - y) <= 16 and abs(world_->drop[i_].x - x) <= 16) {
											count_ += 1;
										}
										if (world_->drop[i_].id == id_) if (world_->drop[i_].count < 200) dublicated = true;
									}
									if (!dublicated) {
										if (count_ > 20) {
											gamepacket_t p;
											p.Insert("OnTextOverlay");
											p.Insert("You can't drop that here, find an emptier spot!");
											p.CreatePacket(peer);
											break;
										}
									}
								}
								modify_inventory(peer, id_, c_);
								{
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`w" + items[id_].ori_name + "``|left|" + to_string(id_) + "|\nadd_textbox|How many to drop?|left|\nadd_text_input|count||" + to_string(c_) + "|5|\nembed_data|itemID|" + to_string(id_) + "" + (world_->owner_name != pInfo(peer)->tankIDName and not pInfo(peer)->dev and (!guild_access(peer, world_->guild_id) and find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end()) ? "\nadd_textbox|If you are trying to trade an item with another player, use your wrench on them instead to use our Trade System! `4Dropping items is not safe!``|left|" : "") + "\nend_dialog|drop_item|Cancel|OK|");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|setRoleIcon") != string::npos || cch.find("action|setRoleSkin") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 3) break;
							string id_ = explode("\n", t_[2])[0];
							if (not isdigit(id_[0])) break;
							uint32_t role_t = atoi(id_.c_str());
							if (cch.find("action|setRoleIcon") != string::npos) {
								if (role_t == 6) pInfo(peer)->roleIcon = role_t;
								else if (role_t == 0 and pInfo(peer)->t_lvl >= 50) pInfo(peer)->roleIcon = role_t;
								else if (role_t == 1 and pInfo(peer)->bb_lvl >= 50) pInfo(peer)->roleIcon = role_t;
							}
							else {
								if (role_t == 6) pInfo(peer)->roleSkin = role_t;
								else if (role_t == 0 and pInfo(peer)->t_lvl >= 50) pInfo(peer)->roleSkin = role_t;
								else if (role_t == 1 and pInfo(peer)->bb_lvl >= 50) pInfo(peer)->roleSkin = role_t;
							}
							gamepacket_t p(0, pInfo(peer)->netID);
							p.Insert("OnSetRoleSkinsAndIcons"), p.Insert(pInfo(peer)->roleSkin), p.Insert(pInfo(peer)->roleIcon), p.Insert(0);
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != pInfo(peer)->world) continue;
								p.CreatePacket(currentPeer);
							}
							break;
						}
						else if (cch.find("action|setSkin") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 3) break;
							string id_ = explode("\n", t_[2])[0];
							if (not isdigit(id_[0])) break;
							char* endptr = NULL;
							unsigned int skin_ = strtoll(id_.c_str(), &endptr, 10);
							if (skin_ == 3531226367 and pInfo(peer)->supp == 2 or skin_ == 4023103999 and pInfo(peer)->supp == 2 or skin_ == 1345519520 and pInfo(peer)->supp == 2 or skin_ == 194314239 and pInfo(peer)->supp == 2) pInfo(peer)->skin = skin_;
							else if (skin_ == 3578898848 and pInfo(peer)->gp or skin_ == 3317842336 and pInfo(peer)->gp) pInfo(peer)->skin = skin_;
							else if (skin_ != 1348237567 and skin_ != 1685231359 and skin_ != 2022356223 and skin_ != 2190853119 and skin_ != 2527912447 and skin_ != 2864971775 and skin_ != 3033464831 and skin_ != 3370516479) {
								if (pInfo(peer)->supp >= 1) {
									if (skin_ != 2749215231 and skin_ != 3317842431 and skin_ != 726390783 and skin_ != 713703935 and skin_ != 3578898943 and skin_ != 4042322175) break;
									else pInfo(peer)->skin = skin_;
								}
								else break;
							}
							else pInfo(peer)->skin = skin_;
							update_clothes(peer);
							break;
						}
						else if (cch.find("action|trash") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							int id_ = atoi(explode("\n", t_[3])[0].c_str()), c_ = 0;
							if (id_ <= 0 or id_ >= items.size()) break;
							gamepacket_t p;
							if (id_ == 18 || id_ == 32 || id_ == 6336) {
								packet_(peer, "action|play_sfx\nfile|audio/cant_place_tile.wav\ndelayMS|0");
								p.Insert("OnTextOverlay"), p.Insert("You'd be sorry if you lost that!"), p.CreatePacket(peer);
								break;
							}
							modify_inventory(peer, id_, c_); // gauna itemo kieki
							p.Insert("OnDialogRequest");
							if (pInfo(peer)->supp == 0) p.Insert("set_default_color|`o\nadd_label_with_icon|big|`4Trash`` `w" + items[id_].ori_name + "``|left|" + to_string(id_) + "|\nadd_textbox|How many to `4destroy``? (you have " + to_string(c_) + ")|left|\nadd_text_input|count||0|5|\nembed_data|itemID|" + to_string(id_) + "\nend_dialog|trash_item|Cancel|OK|");
							else {
								int item = id_, maxgems = 0, maximum_gems = 0;
								if (id_ % 2 != 0) item -= 1;
								maxgems = items[item].max_gems2;
								if (items[item].max_gems3 != 0) maximum_gems = items[item].max_gems3;
								string recycle_text = "0" + (maxgems == 0 ? "" : "-" + to_string(maxgems)) + "";
								if (maximum_gems != 0) recycle_text = to_string(maximum_gems);
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`4Recycle`` `w" + items[id_].ori_name + "``|left|" + to_string(id_) + "|\nadd_textbox|You will get " + recycle_text + " gems per item.|\nadd_textbox|How many to `4destroy``? (you have " + to_string(c_) + ")|left|\nadd_text_input|count||0|5|\nembed_data|itemID|" + to_string(id_) + "\nend_dialog|trash_item|Cancel|OK|");
							}
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|info") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							int id_ = atoi(explode("\n", t_[3])[0].c_str());
							if (id_ % 2 != 0) p.Insert("set_default_color|`o\nadd_label_with_ele_icon|big|`wAbout " + items[id_].ori_name + "``|left|" + to_string(id_) + "|" + to_string(items[id_ - 1].chi) + "|\nadd_spacer|small|\nadd_textbox|Plant this seed to grow a `0" + items[id_ - 1].ori_name + " Tree.``|left|\nadd_spacer|small|\nadd_textbox|Rarity: `0" + to_string(items[id_].rarity) + "``|left|\nadd_spacer|small|\nend_dialog|continue||OK|\n");
							else {
								string extra_ = "\nadd_textbox|";
								if (id_ == 18) {
									extra_ += "You've punched `w" + to_string(pInfo(peer)->punch_count) + "`` times.";
								} if (items[id_].blockType == BlockTypes::LOCK) {
									extra_ += "A lock makes it so only you (and designated friends) can edit an area.";
								} if (items[id_].r_1 == 0 or items[id_].r_2 == 0) {
									extra_ += "<CR>This item can't be spliced.";
								}
								else {
									extra_ += "Rarity: `w" + to_string(items[id_].rarity) + "``<CR><CR>To grow, plant a `w" + items[id_ + 1].name + "``.   (Or splice a `w" + items[items[id_].r_1].name + "`` with a `w" + items[items[id_].r_2].name + "``)<CR>";
								} if (items[id_].properties & Property_Dropless or items[id_].rarity == 999) {
									extra_ += "<CR>`1This item never drops any seeds.``";
								} if (items[id_].properties & Property_Untradable) {
									extra_ += "<CR>`1This item cannot be dropped or traded.``";
								} if (items[id_].properties & Property_AutoPickup) {
									extra_ += "<CR>`1This item can't be destroyed - smashing it will return it to your backpack if you have room!``";
								} if (items[id_].properties & Property_Wrenchable) {
									extra_ += "<CR>`1This item has special properties you can adjust with the Wrench.``";
								} if (items[id_].properties & Property_MultiFacing) {
									extra_ += "<CR>`1This item can be placed in two directions, depending on the direction you're facing.``";
								} if (items[id_].properties & Property_NoSelf) {
									extra_ += "<CR>`1This item has no use... by itself.``";
								}
								extra_ += "|left|";
								if (extra_ == "\nadd_textbox||left|") extra_ = "";
								else extra_ = replace_str(extra_, "add_textbox|<CR>", "add_textbox|");

								string extra_ore = "";
								if (id_ == 9386) extra_ore = rare_text;
								if (id_ == 9384) extra_ore = rare2_text;
								if (id_ == 7782) extra_ore = rainbow_text;
								p.Insert("set_default_color|`o\nadd_label_with_ele_icon|big|`wAbout " + items[id_].name + "``|left|" + to_string(id_) + "|" + to_string(items[id_].chi) + "|\nadd_spacer|small|\nadd_textbox|" + items[id_].description + "|left|" + (extra_ore != "" ? "\nadd_spacer|small|\nadd_textbox|This item also drops:|left|" + extra_ore : "") + "" + (id_ == 8552 ? "\nadd_spacer|small|\nadd_textbox|Angelic Healings: " + to_string(pInfo(peer)->surgery_done) + "|left|" : "") + "\nadd_spacer|small|" + extra_ + "\nadd_spacer|small|\nembed_data|itemID|" + to_string(id_) + "\nend_dialog|continue||OK|\n");
							}
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|wrench") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							int netID = atoi(explode("\n", t_[3])[0].c_str());
							if (pInfo(peer)->netID == netID) {
								send_wrench_self(peer);
							}
							else {
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (pInfo(currentPeer)->netID == netID and pInfo(currentPeer)->world == pInfo(peer)->world) {
										bool already_friends = false, trade_blocked = false, muted = false;
										for (int c_ = 0; c_ < pInfo(peer)->friends.size(); c_++) {
											if (pInfo(peer)->friends[c_].name == pInfo(currentPeer)->tankIDName) {
												already_friends = true;
												if (pInfo(peer)->friends[c_].block_trade)
													trade_blocked = true;
												if (pInfo(peer)->friends[c_].mute)
													muted = true;
												break;
											}
										}
										pInfo(peer)->last_wrenched = pInfo(currentPeer)->tankIDName;
										string name_ = pInfo(peer)->world;
										vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
										if (p != worlds.end()) {
											World* world_ = &worlds[p - worlds.begin()];
											string msg2 = "";
											for (int i = 0; i < to_string(pInfo(currentPeer)->level).length(); i++) msg2 += "?";
											string inv_guild = "";
											string extra = "";
											if (pInfo(currentPeer)->guild_id != 0) {
												uint32_t guild_id = pInfo(currentPeer)->guild_id;
												vector<Guild>::iterator find_guild = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
												if (find_guild != guilds.end()) {
													Guild* guild_information = &guilds[find_guild - guilds.begin()];
													for (GuildMember member_search : guild_information->guild_members) {
														if (member_search.member_name == pInfo(currentPeer)->tankIDName) {
															if (guild_information->guild_mascot[1] == 0 and guild_information->guild_mascot[0] == 0) {
																extra += "\nadd_label_with_icon|small|`9Guild: `2" + guild_information->guild_name + "``|left|5814|\nadd_textbox|`9Rank: `2" + (member_search.role_id == 0 ? "Member" : (member_search.role_id == 1 ? "Elder" : (member_search.role_id == 2 ? "Co-Leader" : "Leader"))) + "``|left|\nadd_spacer|small|";
															}
															else {
																extra += "\nadd_dual_layer_icon_label|small|`9Guild: `2" + guild_information->guild_name + "``|left|" + to_string(guild_information->guild_mascot[1]) + "|" + to_string(guild_information->guild_mascot[0]) + "|1.0|1|\nadd_smalltext|`9Rank: `2" + (member_search.role_id == 0 ? "Member" : (member_search.role_id == 1 ? "Elder" : (member_search.role_id == 2 ? "Co-Leader" : "Leader"))) + "``|left|\nadd_spacer|small|";
															}
															break;
														}
													}
												}
											} if (pInfo(peer)->guild_id != 0 and pInfo(currentPeer)->guild_id == 0) {
												uint32_t guild_id = pInfo(peer)->guild_id;
												vector<Guild>::iterator find_guild = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
												if (find_guild != guilds.end()) {
													Guild* guild_information = &guilds[find_guild - guilds.begin()];
													for (GuildMember member_search : guild_information->guild_members) {
														if (member_search.member_name == pInfo(peer)->tankIDName) {
															if (member_search.role_id >= 1) {
																inv_guild = "\nadd_button|invitetoguild|`2Invite to Guild``|noflags|0|0|";
															}
															break;
														}
													}
												}
											}
											string surgery = "\nadd_spacer|small|\nadd_button|start_surg|`$Perform Surgery``|noflags|0|0|\nadd_smalltext|Surgeon Skill: " + to_string(pInfo(peer)->surgery_skill) + "|left|";
											for (int i_ = 0; i_ < pInfo(currentPeer)->playmods.size(); i_++) if (pInfo(currentPeer)->playmods[i_].id == 89) surgery = "\nadd_spacer|small|\nadd_textbox|Recovering from surgery...|left|";
											if (pInfo(currentPeer)->hospital_bed == false) surgery = "";
											gamepacket_t p;
											p.Insert("OnDialogRequest");
											p.Insert("embed_data|netID|" + to_string(pInfo(peer)->netID) + "\nset_default_color|`o\nadd_label_with_icon|big|" + (pInfo(currentPeer)->mod == 1 || pInfo(currentPeer)->dev == 1 ? pInfo(currentPeer)->name_color : "`0") + "" + (not pInfo(currentPeer)->d_name.empty() ? pInfo(currentPeer)->d_name : pInfo(currentPeer)->tankIDName) + "`` `0(```2" + (pInfo(currentPeer)->dev == 1 ? msg2 : to_string(pInfo(currentPeer)->level)) + "```0)``|left|18|" + surgery + "\nembed_data|netID|" + to_string(netID) + "\nadd_spacer|small|" + extra + (trade_blocked ? "\nadd_button||`4Trade Blocked``|disabled|||" : "\nadd_button|trade|`wTrade``|noflags|0|0|") + "\nadd_textbox|(No Battle Leash equipped)|left|\nadd_textbox|Your opponent needs a valid license to battle!|left|" + (world_->owner_name == pInfo(peer)->tankIDName or (guild_access(peer, world_->guild_id) or find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) != world_->admins.end()) or pInfo(peer)->dev + pInfo(peer)->mod > 0 ? "\nadd_button|kick|`4Kick``|noflags|0|0|\nadd_button|pull|`5Pull``|noflags|0|0|\nadd_button|worldban|`4World Ban``|noflags|0|0|" : "") + (pInfo(peer)->mod == 1 || pInfo(peer)->dev == 1 ? "\nadd_button|punish_view|`5Punish/View``|noflags|0|0|" : "") + inv_guild + (!already_friends ? "\nadd_button|friend_add|`wAdd as friend``|noflags|0|0|" : "") + (muted ? "\nadd_button|unmute_player|`wUnmute``|noflags|0|0|" : (already_friends ? "\nadd_button|mute_player|`wMute``|noflags|0|0|" : "")) + ""/*"\nadd_button|ignore_player|`wIgnore Player``|noflags|0|0|\nadd_button|report_player|`wReport Player``|noflags|0|0|"*/"\nadd_spacer|small|\nend_dialog|popup||Continue|\nadd_quick_exit|");
											p.CreatePacket(peer);
										}
										break;
									}
								}
							}
							break;
						}
						else if (cch.find("action|friends") != string::npos) {
							send_social(peer);
							break;
						}
						else if (cch == "action|battlepasspopup\n") {
							gamepacket_t p(550);
							p.Insert("OnDialogRequest");
							int growpassid = 6124;
							if (today_day == pInfo(peer)->gd) growpassid = 6292;
							if (pInfo(peer)->gp == 1) p.Insert("set_default_color|`o\nadd_label_with_icon|big|Grow Pass Rewards|left|9222|\nadd_smalltext|`9You can claim your daily reward everyday here.``|left|\nadd_button_with_icon|claim_reward||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|claim_reward||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|claim_reward||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|claim_reward||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|claim_reward||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
							else p.Insert("set_default_color|`o\nadd_label_with_icon|big|Grow Pass Rewards|left|9222|\nadd_button|deposit|`2Purchase``|noflags|0|0|\nadd_smalltext|`4You must purchase the Grow Pass role to claim your prize!``|left|\nadd_button_with_icon|||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon|||staticBlueFrame|" + to_string(growpassid) + "||\nadd_button_with_icon||END_LIST|noflags|0||\nadd_spacer|small|\nend_dialog|||Back|\nadd_quick_exit|\n");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|storenavigate\nitem|main\nselection|deposit\n" || cch == "action|storenavigate\nitem|locks\nselection|upgrade_backpack\n" || cch == "action|storenavigate\nitem|main\nselection|bonanza\n" || cch == "action|storenavigate\nitem|main\nselection|calendar\n" || cch == "action|store\nlocation|bottommenu\n" || cch == "action|store\nlocation|gem\n" || cch == "action|store\nlocation|pausemenu\n" || cch == "action|storenavigate\nitem|main\nselection|gems_rain\n") {
							if (cch == "action|store\nlocation|bottommenu\n" || cch == "action|store\nlocation|gem\n" || cch == "action|store\nlocation|pausemenu\n") shop_tab(peer, "tab1");
							if (cch == "action|storenavigate\nitem|main\nselection|gems_rain\n") shop_tab(peer, "tab1_1");
							if (cch == "action|storenavigate\nitem|main\nselection|calendar\n") shop_tab(peer, "tab1_2");
							if (cch == "action|storenavigate\nitem|main\nselection|bonanza\n")	shop_tab(peer, "tab1_3");
							if (cch == "action|storenavigate\nitem|locks\nselection|upgrade_backpack\n") shop_tab(peer, "tab2_1");
							if (cch == "action|storenavigate\nitem|main\nselection|deposit\n") SendCmd(peer, "/deposit", true);
							break;
						}
						else if (cch.find("action|buy") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 3) break;
							string item = explode("\n", t_[2])[0];
							int price = 0, free = get_free_slots(pInfo(peer)), slot = 1, getcount = 0, get_counted = 0, random_pack = 0, token = 0;
							gamepacket_t p2;
							p2.Insert("OnStorePurchaseResult");
							if (item == "main") shop_tab(peer, "tab1");
							else if (item == "locks") shop_tab(peer, "tab2");
							else if (item == "itempack") shop_tab(peer, "tab3");
							else if (item == "bigitems") shop_tab(peer, "tab4");
							else if (item == "weather") shop_tab(peer, "tab5");
							else if (item == "token") shop_tab(peer, "tab6");
							else if (item == "upgrade_backpack") {
								price = 100 * ((((pInfo(peer)->inv.size() - 17) / 10) * ((pInfo(peer)->inv.size() - 17) / 10)) + 1);
								if (price > pInfo(peer)->gems) {
									packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
									p2.Insert("You can't afford `0Upgrade Backpack`` (`w10 Slots``)!  You're `$" + setGems(price - pInfo(peer)->gems) + "`` Gems short.");
								}
								else {
									if (pInfo(peer)->inv.size() < 246) {
										{
											gamepacket_t p;
											p.Insert("OnConsoleMessage");
											p.Insert("You've purchased `0Upgrade Backpack`` (`010 Slots``) for `$" + setGems(price) + "`` Gems.\nYou have `$" + setGems(pInfo(peer)->gems - price) + "`` Gems left.");
											p.CreatePacket(peer);
										}
										p2.Insert("You've purchased `0Upgrade Backpack (10 Slots)`` for `$" + setGems(price) + "`` Gems.\nYou have `$" + setGems(pInfo(peer)->gems - price) + "`` Gems left.\n\n`5Received: ```0Backpack Upgrade``\n");
										pInfo(peer)->gems -= price;
										{
											packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
											gamepacket_t p;
											p.Insert("OnSetBux");
											p.Insert(pInfo(peer)->gems);
											p.Insert(0);
											p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
											if (pInfo(peer)->supp >= 2) {
												p.Insert((float)33796, (float)1, (float)0);
											}
											p.CreatePacket(peer);
										}
										for (int i_ = 0; i_ < 10; i_++) { // default inv dydis
											Items itm_{};
											itm_.id = 0, itm_.count = 0;
											pInfo(peer)->inv.push_back(itm_);
										}
										send_inventory(peer);
										update_clothes(peer);
										shop_tab(peer, "tab2");
									}
								}
								p2.CreatePacket(peer);
							}
							else {
								vector<int> list;
								vector<vector<int>> itemai;
								string item_name = "";
								ifstream ifs("database/shop/-" + item + ".json");
								if (ifs.is_open()) {
									json j;
									ifs >> j;
									price = j["g"].get<int>();
									item_name = j["p"].get<string>();
									if (j.find("itemai") != j.end()) { // mano sistema
										if (pInfo(peer)->gems < price) {
											packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
											p2.Insert("You can't afford `o" + item_name + "``!  You're `$" + setGems(price - pInfo(peer)->gems) + "`` Gems short."), p2.CreatePacket(peer);
											break;
										}
										itemai = j["itemai"].get<vector<vector<int>>>();
										int reik_slots = itemai.size();
										int turi_slots = get_free_slots(pInfo(peer));
										for (vector<int> item_info : itemai) {
											int turi_dabar = 0;
											modify_inventory(peer, item_info[0], turi_dabar);
											if (turi_dabar != 0) reik_slots--;
											if (turi_dabar + item_info[1] > 200) goto fail;
										}
										if (turi_slots < reik_slots) goto fail;
										{
											//if (item == "g4good_Gem_Charity") grow4good(peer, false, "donate_gems", 0);
											//if (item != "arm_guy" and item != "g4good_Gem_Charity") grow4good(peer, false, "gems", price);
											pInfo(peer)->gems -= price;
											gamepacket_t gem_upd;
											gem_upd.Insert("OnSetBux"), gem_upd.Insert(pInfo(peer)->gems), gem_upd.Insert(0), gem_upd.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
											if (pInfo(peer)->supp >= 2) {
												gem_upd.Insert((float)33796, (float)1, (float)0);
											}
											gem_upd.CreatePacket(peer);
											vector<string> received_items{}, received_items2{};
											for (vector<int> item_info : itemai) {
												uint32_t item_id = item_info[0];
												int item_count = item_info[1];
												modify_inventory(peer, item_id, item_count);
												received_items.push_back("Got " + to_string(item_info[1]) + " `#" + items[item_id].ori_name + "``."), received_items2.push_back(to_string(item_info[1]) + " " + items[item_id].ori_name);
											}
											packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
											//if (item == "arm_guy") grow4good(peer, false, "purchase_waving", 0);
											gamepacket_t p_;
											p_.Insert("OnConsoleMessage"), p_.Insert("You've purchased `o" + item_name + "`` for `$" + setGems(price) + "`` Gems.\nYou have `$" + setGems(pInfo(peer)->gems) + "`` Gems left." + "\n" + join(received_items, "\n")), p_.CreatePacket(peer);
											p2.Insert("You've purchased `o" + item_name + "`` for `$" + setGems(price) + "`` Gems.\nYou have `$" + setGems(pInfo(peer)->gems) + "`` Gems left." + "\n\n`5Received: ``" + join(received_items2, ", ") + "\n"), p2.CreatePacket(peer);
											break;
										}
									fail:
										packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
										p2.Insert("You don't have enough space in your inventory to buy that. You may be carrying to many of one of the items you are trying to purchase or you don't have enough free spaces to fit them all in your backpack!");
										p2.CreatePacket(peer);
										break;
									}
									list = j["i"].get<vector<int>>();
									getcount = j["h"].get<int>();
									get_counted = j["h"].get<int>();
									slot = j["c"].get<int>();
									token = j["t"].get<int>();
									random_pack = j["random"].get<int>();
									int totaltoken = 0, tokencount = 0, mega_token = 0, inventoryfull = 0;
									modify_inventory(peer, 1486, tokencount);
									modify_inventory(peer, 6802, mega_token);
									totaltoken = tokencount + (mega_token * 100);
									vector<pair<int, int>> receivingitems;
									if (token == 0 ? price > pInfo(peer)->gems : token > totaltoken) {
										packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
										p2.Insert("You can't afford `o" + item_name + "``!  You're `$" + (token == 0 ? "" + setGems(price - pInfo(peer)->gems) + "`` Gems short." : "" + setGems(token - totaltoken) + "`` Growtokens short."));
									}
									else {
										if (free >= slot) {
											string received = "", received2 = "";
											if (item == "basic_splice") {
												slot++;
												receivingitems.push_back(make_pair(11, 10));
											}
											if (item == "race_packa") {
												slot++;
												receivingitems.push_back(make_pair(11, 10));
											}
											for (int i = 0; i < slot; i++) receivingitems.push_back(make_pair((random_pack == 1 ? list[rand() % list.size()] : list[i]), getcount));
											for (int i = 0; i < slot; i++) {
												int itemcount = 0;
												modify_inventory(peer, receivingitems[i].first, itemcount);
												if (itemcount + getcount > 200) inventoryfull = 1;
											}
											if (inventoryfull == 0) {
												int i = 0;
												for (i = 0; i < slot; i++) {
													received += (i != 0 ? ", " : "") + items[receivingitems[i].first].name;
													received2 += "Got " + to_string(receivingitems[i].second) + " `#" + items[receivingitems[i].first].name + "``." + (i == (slot - 1) ? "" : "\n") + "";
													modify_inventory(peer, receivingitems[i].first, receivingitems[i].second);
												}
											}
											else {
												packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
												p2.Insert("You don't have enough space in your inventory that. You may be carrying to many of one of the items you are trying to purchase or you don't have enough free spaces to fit them all in your backpack!");
												p2.CreatePacket(peer);
												break;
											}
											{
												gamepacket_t p;
												p.Insert("OnConsoleMessage");
												p.Insert("You've purchased `o" + received + "`` for `$" + (token == 0 ? "" + setGems(price) + "`` Gems." : "" + setGems(token) + "`` Growtokens.") + "\nYou have `$" + (token == 0 ? "" + setGems(pInfo(peer)->gems - price) + "`` Gems left." : "" + setGems(totaltoken - token) + "`` Growtokens left.") + "\n" + received2);
												p.CreatePacket(peer);
											}
											p2.Insert("You've purchased `o" + received + "`` for `$" + (token == 0 ? "" + setGems(price) + "`` Gems." : "" + setGems(token) + "`` Growtokens.") + "\nYou have `$" + (token == 0 ? "" + setGems(pInfo(peer)->gems - price) + "`` Gems left." : "" + setGems(totaltoken - token) + "`` Growtokens left.") + "\n\n`5Received: ``" + (get_counted <= 1 ? "" : "`0" + to_string(get_counted)) + "`` " + received + "\n"), p2.CreatePacket(peer);
											if (token == 0) {
												pInfo(peer)->gems -= price;
												gamepacket_t p;
												p.Insert("OnSetBux");
												p.Insert(pInfo(peer)->gems);
												p.Insert(0);
												p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
												if (pInfo(peer)->supp >= 2) {
													p.Insert((float)33796, (float)1, (float)0);
												}
												p.CreatePacket(peer);
											}
											else {
												if (tokencount >= token) modify_inventory(peer, 1486, token *= -1);
												else {
													modify_inventory(peer, 1486, tokencount *= -1);
													modify_inventory(peer, 6802, mega_token *= -1);
													int givemegatoken = (totaltoken - token) / 100;
													int givetoken = (totaltoken - token) - (givemegatoken * 100);
													modify_inventory(peer, 1486, givetoken);
													modify_inventory(peer, 6802, givemegatoken);
												}
											}
											packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										}
										else {
											packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
											p2.Insert(slot > 1 ? "You'll need " + to_string(slot) + " slots free to buy that! You have " + to_string(free) + " slots." : "You don't have enough space in your inventory that. You may be carrying to many of one of the items you are trying to purchase or you don't have enough free spaces to fit them all in your backpack!");
										}
									}
								}
								else {
									packet_(peer, "action|play_sfx\nfile|audio/bleep_fail.wav\ndelayMS|0");
									p2.Insert("This item was not found. Server error.");
									p2.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch == "action|AccountSecurity\nlocation|pausemenu\n") {
							gamepacket_t p(500);
							p.Insert("OnDialogRequest");
							p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wAdvanced Account Protection ``|left|3732|\nadd_textbox|`1You are about to enable the Advanced Account Protection.``|left|\nadd_textbox|`1After that, every time you try to log in from a new device and IP you will receive an email with a login confirmation link.``|left|\nadd_textbox|`1This will significantly increase your account security.``|left|\nend_dialog|secureaccount|Cancel|Ok|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|respawn") != string::npos) SendRespawn(peer, false, 0, (cch.find("action|respawn_spike") != string::npos) ? false : true);
						else if (cch == "action|refresh_item_data\n") {
							if (pInfo(peer)->world.empty()) {
								save_player(pInfo(peer), (f_saving_ ? false : true));
								enet_peer_send(peer, 0, enet_packet_create(item_data, item_data_size + 60, ENET_PACKET_FLAG_RELIABLE));
								enet_peer_disconnect_later(peer, 0);
							}
							else writelogd(pInfo(peer)->tankIDName + " tried to disconnent while in world");
							break;
						}
						else if (cch == "action|enter_game\n") {
							pInfo(peer)->enter_game++;
							if (pInfo(peer)->world == "" && pInfo(peer)->enter_game == 1) {
								if (pInfo(peer)->tankIDName.empty()) {
									gamepacket_t p;
									p.Insert("OnDialogRequest"), p.Insert(r_dialog("")), p.CreatePacket(peer);
								}
								else {
									pInfo(peer)->tmod = pInfo(peer)->mod;
									pInfo(peer)->name_color = (pInfo(peer)->dev == 1 ? "`6@" : (pInfo(peer)->tmod == 1) ? "`#@" : "`0");
									//string thetag = (pInfo(peer)->mod || pInfo(peer)->dev ? "@" : "");
									//if (pInfo(peer)->drt) pInfo(peer)->d_name = (pInfo(peer)->drt ? "`4" + thetag : pInfo(peer)->name_color) + (pInfo(peer)->drt ? "Dr." : "") + pInfo(peer)->tankIDName + (pInfo(peer)->is_legend ? " of Legend" : "");
									int on_ = 0, t_ = 0;
									//if (gotall) pInfo(peer)->superdev = 1, pInfo(peer)->dev = 1, pInfo(peer)->mod = 1, pInfo(peer)->gems = 999999, pInfo(peer)->gtwl = 99999;
									vector<string> friends_;
									for (int c_ = 0; c_ < pInfo(peer)->friends.size(); c_++) friends_.push_back(pInfo(peer)->friends[c_].name);
									if (not pInfo(peer)->invis and not pInfo(peer)->m_h) {
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											t_ += 1 + rand() % 3 + 1;
											if (find(friends_.begin(), friends_.end(), pInfo(currentPeer)->tankIDName) != friends_.end()) {
												if (pInfo(currentPeer)->show_friend_notifications_) {
													gamepacket_t p;
													p.Insert("OnConsoleMessage"), p.Insert("`3FRIEND ALERT:`` " + (pInfo(peer)->name_color == "`0" ? "`o" : pInfo(peer)->name_color + "" + pInfo(peer)->tankIDName) + "`` has `2logged on``.");
													packet_(currentPeer, "action|play_sfx\nfile|audio/friend_logon.wav\ndelayMS|0");
													p.CreatePacket(currentPeer);
												}
												on_++;
											}
										}
									}
									{
										gamepacket_t p;
										p.Insert("OnEmoticonDataChanged");
										p.Insert(151749376);
										p.Insert("(wl)|ā|1&(yes)|Ă|1&(no)|ă|1&(love)|Ą|1&(oops)|ą|1&(shy)|Ć|1&(wink)|ć|1&(tongue)|Ĉ|1&(agree)|ĉ|1&(sleep)|Ċ|1&(punch)|ċ|1&(music)|Č|1&(build)|č|1&(megaphone)|Ď|1&(sigh)|ď|1&(mad)|Đ|1&(wow)|đ|1&(dance)|Ē|1&(see-no-evil)|ē|1&(bheart)|Ĕ|1&(heart)|ĕ|1&(grow)|Ė|1&(gems)|ė|1&(kiss)|Ę|1&(gtoken)|ę|1&(lol)|Ě|1&(smile)|Ā|1&(cool)|Ĝ|1&(cry)|ĝ|1&(vend)|Ğ|1&(bunny)|ě|1&(cactus)|ğ|1&(pine)|Ĥ|1&(peace)|ģ|1&(terror)|ġ|1&(troll)|Ġ|1&(evil)|Ģ|1&(fireworks)|Ħ|1&(football)|ĥ|1&(alien)|ħ|1&(party)|Ĩ|1&(pizza)|ĩ|1&(clap)|Ī|1&(song)|ī|1&(ghost)|Ĭ|1&(nuke)|ĭ|1&(halo)|Į|1&(turkey)|į|1&(gift)|İ|1&(cake)|ı|1&(heartarrow)|Ĳ|1&(lucky)|ĳ|1&(shamrock)|Ĵ|1&(grin)|ĵ|1&(ill)|Ķ|1&");
										p.CreatePacket(peer);
									}
									/*
									{
										gamepacket_t p;
										p.Insert("OnEmoticonDataChanged");
										p.Insert(151749376);
										string other = "";
										for (int i = 0; i < pInfo(peer)->gr.size(); i++) other += pInfo(peer)->gr[i];
										p.Insert(other + "" + (pInfo(peer)->supp == 2 ? "(yes)|Ă|1" : "(yes)|Ă|0") + "&" + (pInfo(peer)->supp != 0 ? "(no)|ă|1" : "(no)|ă|0") + "&" + (pInfo(peer)->supp == 2 ? "(love)|Ą|1" : "(love)|Ą|0") + "&" + (pInfo(peer)->supp != 0 ? "(shy)|Ć|1&(wink)|ć|1" : "(shy)|Ć|0&(wink)|ć|0") + "&" + (pInfo(peer)->level >= 5 ? "(tongue)|Ĉ|1" : "(tongue)|Ĉ|0") + "&" + (pInfo(peer)->friends.size() >= 20 ? "(agree)|ĉ|1" : "(agree)|ĉ|0") + "&" + (pInfo(peer)->supp != 0 ? "(music)|Č|1" : "(music)|Č|0") + "&" + (pInfo(peer)->friends.size() >= 50 ? "(build)|č|1" : "(build)|č|0") + "&" + (pInfo(peer)->supp == 2 ? "(megaphone)|Ď|1" : "(megaphone)|Ď|0") + "&" + (pInfo(peer)->level >= 5 ? "(sigh)|ď|1&(mad)|Đ|1&(wow)|đ|1" : "(sigh)|ď|0&(mad)|Đ|0&(wow)|đ|0") + "&" + (pInfo(peer)->friends.size() >= 40 ? "(dance)|Ē|1" : "(dance)|Ē|0") + "&" + (pInfo(peer)->friends.size() >= 30 ? "(see-no-evil)|ē|1" : "(see-no-evil)|ē|0") + "&" + (pInfo(peer)->supp == 2 ? "(heart)|ĕ|1" : "(heart)|ĕ|0") + "&" + (pInfo(peer)->friends.size() >= 10 ? "(kiss)|Ę|1" : "(kiss)|Ę|0") + "&" + (pInfo(peer)->supp != 0 ? "(lol)|Ě|1" : "(lol)|Ě|1") + "&" + (pInfo(peer)->level >= 5 ? "(smile)|Ā|1" : "(smile)|Ā|0") + "&" + (pInfo(peer)->supp == 2 ? "(cool)|Ĝ|1" : "(cool)|Ĝ|0") + "&(lucky)|ĳ|1&");
										p.CreatePacket(peer);
									}*/
									if (pInfo(peer)->surgery_type == -1) pInfo(peer)->surgery_type = rand() % 30;
									SendReceive(peer);
									{
										if (pInfo(peer)->pinata_day != today_day) {
											pInfo(peer)->pinata_prize = false;
											pInfo(peer)->pinata_claimed = false;
										}
										gamepacket_t p;
										p.Insert("OnProgressUISet"), p.Insert(1), p.Insert(0), p.Insert(to_string(pInfo(peer)->pinata_claimed)), p.Insert(1), p.Insert(""), p.Insert(to_string(pInfo(peer)->pinata_prize)), p.CreatePacket(peer);
									}
									// ENTER GAME NOTIFICATION
									gamepacket_t p1;//, p5;
									p1.Insert("OnConsoleMessage"), p1.Insert("Welcome back, `w" + (not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + "````." + (pInfo(peer)->friends.size() == 0 ? "" : (on_ != 0 ? " `w" + to_string(on_) + "`` friend is online." : " No friends are online."))), p1.CreatePacket(peer);
									//p3.Insert("OnConsoleMessage"), p3.Insert("`3Limited time event``: 3x Gems from breaking blocks, Trees time reduced by 50%, Providers time reduced by 50%!"), p3.CreatePacket(peer);
									{
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										if (thedaytoday == 1) p.Insert("`3Today is Trees Day!`` 50% higher chance to get `2extra block`` from harvesting tree.");
										else if (thedaytoday == 2) p.Insert("`3Today is Breaking Day!`` 15% higher chance to get `2extra seed``.");
										else if (thedaytoday == 3) p.Insert("`3Today is Geiger Day!`` Higher chance of getting a `2better Geiger prize`` & Irradiated mod will last only `210 minutes``.");
										else if (thedaytoday == 4) p.Insert("`3Today is Level Day!`` Get extra `2500 gems`` bonus for leveling up.");
										else if (thedaytoday == 5) p.Insert("`3Today is Gems Day!`` 50% higher chance to get `2extra`` gem drop.");
										else if (thedaytoday == 6) p.Insert("`3Today is Surgery Day!`` Malpractice takes `215 minutes`` and Recovering takes `230 minutes`` & receive `2different prizes``.");
										else if (thedaytoday == 0) p.Insert("`3Today is Fishing Day!`` Catch a fish and receive `2extra lb``.");
										p.CreatePacket(peer);
									}
									//p5.Insert("OnConsoleMessage"), p5.Insert("`3It's Approximately `2Cinco `wDe `4Mayo!`` Party with your friends and smash Pinatas for prizes!``"), p5.CreatePacket(peer);
									if (pInfo(peer)->mod) {
										if (get_free_slots(pInfo(peer)) >= 1) {
											if (today_day != pInfo(peer)->mds) {
												pInfo(peer)->mds = today_day;
												vector<int> list2{ 408, 274, 276 };
												int receive = 1, item = list2[rand() % list2.size()], got = 1;
												modify_inventory(peer, item, receive);
												gamepacket_t p, p2;
												p.Insert("OnConsoleMessage"), p.Insert("Your mod appreciation bonus (feel free keep, trade, or use for prizes) for today is:"), p.CreatePacket(peer);
												p2.Insert("OnConsoleMessage"), p2.Insert("Given `0" + to_string(got) + " " + items[item].name + "``."), p2.CreatePacket(peer);
											}
										}
									}
									world_menu(peer);
									news(peer);
								}
							}
							else enet_peer_disconnect_later(peer, 0);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|account_security\nchange|") != string::npos) {
							string change = cch.substr(57, cch.length() - 57).c_str();
							replace_str(change, "\n", "");
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (change == "email") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|Having an up-to-date email address attached to your account is a great step toward improved account security.|left|\nadd_smalltext|Email: `5" + pInfo(peer)->email + "``|left|\nadd_spacer|small|\nadd_smalltext|Type your new `5email address``|left|\nadd_text_input|change|||50|\nend_dialog|change_email|OK|Continue|\n");
							else if (change == "password") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|A hacker may attempt to access your account more than once over a period of time.|left|\nadd_smalltext|Changing your password `2often reduces the risk that they will have frequent access``.|left|\nadd_spacer|small|\nadd_smalltext|Type your new `5password``|left|\nadd_text_input|change|||18|\nend_dialog|change_password|OK|Continue|\n");
							if (change == "email" or change == "password") p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|change_email\nchange|") != string::npos) {
							string change = cch.substr(53, cch.length() - 53).c_str();
							replace_str(change, "\n", "");
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (change == "") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|Having an up-to-date email address attached to your account is a great step toward improved account security.|left|\nadd_smalltext|Email: `5" + pInfo(peer)->email + "``|left|\nadd_spacer|small|\nadd_smalltext|Type your new `5email address``|left|\nadd_text_input|change|||50|\nend_dialog|change_email|OK|Continue|\n");
							else {
								pInfo(peer)->email = change;
								save_player(pInfo(peer), false);
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|Having an up-to-date email address attached to your account is a great step toward improved account security.|left|\nadd_smalltext|Your new Email: `5" + pInfo(peer)->email + "``|left|\nadd_spacer|small|\nend_dialog|changedemail|OK||\n");
							}
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|change_password\nchange|") != string::npos) {
							string change = cch.substr(56, cch.length() - 56).c_str();
							replace_str(change, "\n", "");
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (change == "") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|A hacker may attempt to access your account more than once over a period of time.|left|\nadd_smalltext|Changing your password `2often reduces the risk that they will have frequent access``.|left|\nadd_spacer|small|\nadd_smalltext|Type your new `5password``|left|\nadd_text_input|change|||18|\nend_dialog|change_password|OK|Continue|\n");
							else {
								{
									gamepacket_t p;
									p.Insert("SetHasGrowID"), p.Insert(1), p.Insert(pInfo(peer)->tankIDName), p.Insert(pInfo(peer)->tankIDPass = change);
									p.CreatePacket(peer);
								}
								save_player(pInfo(peer), false);
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`0Account Security``|left|1424|\nadd_spacer|small|\nadd_textbox|`6Information``|left|\nadd_smalltext|A hacker may attempt to access your account more than once over a period of time.|left|\nadd_smalltext|Changing your password `2often reduces the risk that they will have frequent access``.|left|\nadd_smalltext|Your new password: `5" + pInfo(peer)->tankIDPass + "``|left|\nadd_spacer|small|\nend_dialog|changedpassword|OK||\n");
							}
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|world_swap\nname_box|") != string::npos) {
							string world = cch.substr(53, cch.length() - 53).c_str(), currentworld = pInfo(peer)->world;
							int got = 0;
							replace_str(world, "\n", "");
							transform(world.begin(), world.end(), world.begin(), ::toupper);
							if (not check_blast(world) || currentworld == world) {
								gamepacket_t p;
								p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSwap World Names``|left|2580|\nadd_textbox|`4World swap failed - you don't own both worlds!``|left|\nadd_smalltext|This will swap the name of the world you are standing in with another world `4permanently``.  You must own both worlds, with a World Lock in place.<CR>Your `wChange of Address`` will be consumed if you press `5Swap 'Em``.|left|\nadd_textbox|Enter the other world's name:|left|\nadd_text_input|name_box|||32|\nadd_spacer|small|\nend_dialog|world_swap|Cancel|Swap 'Em!|"), p.CreatePacket(peer);
							}
							else create_address_world(peer, world, currentworld);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|surgery\nbuttonClicked|tool") != string::npos) {
							if (pInfo(peer)->surgery_started) {
								int count = atoi(cch.substr(59, cch.length() - 59).c_str());
								if (count == 999) end_surgery(peer);
								else load_surgery(peer, count);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|compactor\ncount|") != string::npos) {
							int count = atoi(cch.substr(49, cch.length() - 49).c_str()), item = pInfo(peer)->lastchoosenitem, got = 0;
							modify_inventory(peer, item, got);
							if (got < count) break;
							if (items[item].r_1 == 2037 || items[item].r_2 == 2037 || items[item].r_1 == 2035 || items[item].r_2 == 2035 || items[item].r_1 + items[item].r_2 == 0 || items[item].blockType != BlockTypes::CLOTHING || items[item].untradeable || item == 1424 || items[item].rarity > 200) break;
							else {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									string received = "";
									vector<pair<int, int>> receivingitems;
									vector<int> list = { items[item].r_1,  items[item].r_2,  items[item].r_1 - 1 ,  items[item].r_2 - 1 }, random_compactor_rare = { 3178, 2936, 5010, 2644, 2454, 2456, 2458, 2460, 6790, 9004, 11060 };
									for (int i = 0; i < count; i++) {
										if (rand() % items[item].newdropchance < 20) {
											bool dublicate = false;
											int given_item = list[rand() % list.size()];
											for (int i = 0; i < receivingitems.size(); i++) {
												if (receivingitems[i].first == given_item) {
													dublicate = true;
													receivingitems[i].second += 1;
												}
											}
											if (dublicate == false) receivingitems.push_back(make_pair(given_item, 1));
										}
										else if (rand() % 50 < 1) {
											bool dublicate = false;
											int given_item = 0;
											if (rand() % 100 < 1) given_item = random_compactor_rare[rand() % random_compactor_rare.size()];
											else given_item = 2462;
											for (int i = 0; i < receivingitems.size(); i++) {
												if (receivingitems[i].first == given_item) {
													dublicate = true;
													receivingitems[i].second += 1;
												}
											}
											if (dublicate == false) receivingitems.push_back(make_pair(given_item, 1));
										}
										else {
											bool dublicate = false;
											int given_item = 112, given_count = (rand() % items[item].max_gems) / 2 + 1;
											if (rand() % 3 < 1) given_item = 856, given_count = 1;
											for (int i = 0; i < receivingitems.size(); i++) {
												if (receivingitems[i].first == given_item) {
													dublicate = true;
													receivingitems[i].second += given_count;
												}
											}
											if (dublicate == false) receivingitems.push_back(make_pair(given_item, given_count));
										}
									}
									int remove = count * -1;
									modify_inventory(peer, item, remove);
									for (int i = 0; i < receivingitems.size(); i++) {
										if (receivingitems.size() == 1) received += to_string(receivingitems[i].second) + " " + (items[item].r_1 == receivingitems[i].first || items[item].r_2 == receivingitems[i].first || items[item].r_2 - 1 == receivingitems[i].first || items[item].r_1 - 1 == receivingitems[i].first ? "`2" + items[receivingitems[i].first].name + "``" : (receivingitems[i].first == 112) ? items[receivingitems[i].first].name : "`1" + items[receivingitems[i].first].name + "``");
										else {
											if (receivingitems.size() - 1 == i)received += "and " + to_string(receivingitems[i].second) + " " + (items[item].r_1 == receivingitems[i].first || items[item].r_2 == receivingitems[i].first || items[item].r_2 - 1 == receivingitems[i].first || items[item].r_1 - 1 == receivingitems[i].first ? "`2" + items[receivingitems[i].first].name + "``" : (receivingitems[i].first == 112) ? items[receivingitems[i].first].name : "`1" + items[receivingitems[i].first].name + "``");
											else if (i != receivingitems.size()) received += to_string(receivingitems[i].second) + " " + (items[item].r_1 == receivingitems[i].first || items[item].r_2 == receivingitems[i].first || items[item].r_2 - 1 == receivingitems[i].first || items[item].r_1 - 1 == receivingitems[i].first ? "`2" + items[receivingitems[i].first].name + "``" : (receivingitems[i].first == 112) ? items[receivingitems[i].first].name : "`1" + items[receivingitems[i].first].name + "``") + ", ";
										}
										int given_count = receivingitems[i].second;
										if (receivingitems[i].first != 112) {
											if (modify_inventory(peer, receivingitems[i].first, given_count) == 0) {
											}
											else {
												WorldDrop drop_block_{};
												drop_block_.id = receivingitems[i].first, drop_block_.count = given_count, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = (pInfo(peer)->lastwrenchx * 32) + rand() % 17, drop_block_.y = (pInfo(peer)->lastwrenchy * 32) + rand() % 17;
												dropas_(world_, drop_block_);
											}
										}
										else {
											gamepacket_t p;
											p.Insert("OnSetBux"), p.Insert(pInfo(peer)->gems += given_count), p.Insert(0), p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
											if (pInfo(peer)->supp >= 2) {
												p.Insert((float)33796, (float)1, (float)0);
											}
											p.CreatePacket(peer);
										}
									}
									gamepacket_t p, p2;
									p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`7[``From crushing " + to_string(count) + " " + items[item].name + ", " + pInfo(peer)->tankIDName + " extracted " + received + ".`7]``"), p.Insert(0);
									p2.Insert("OnConsoleMessage"), p2.Insert("`7[``From crushing " + to_string(count) + " " + items[item].name + ", " + pInfo(peer)->tankIDName + " extracted " + received + ".`7]``");
									for (ENetPeer* currentPeer_event = server->peers; currentPeer_event < &server->peers[server->peerCount]; ++currentPeer_event) {
										if (currentPeer_event->state != ENET_PEER_STATE_CONNECTED or currentPeer_event->data == NULL or pInfo(currentPeer_event)->world != name_) continue;
										p.CreatePacket(currentPeer_event), p2.CreatePacket(currentPeer_event);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|levelup\nbuttonClicked|claimreward") != string::npos) {
							int count = atoi(cch.substr(66, cch.length() - 66).c_str());
							if (count < 1 || count >125) break;
							if (std::find(pInfo(peer)->lvl_p.begin(), pInfo(peer)->lvl_p.end(), count) == pInfo(peer)->lvl_p.end()) {
								if (pInfo(peer)->level >= count) {
									pInfo(peer)->lvl_p.push_back(count);
									packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
									gamepacket_t p;
									p.Insert("OnSetBux");
									p.Insert(pInfo(peer)->gems += count * 100);
									p.Insert(0);
									p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
									if (pInfo(peer)->supp >= 2) {
										p.Insert((float)33796, (float)1, (float)0);
									}
									p.CreatePacket(peer);
									{
										gamepacket_t p;
										p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("Congratulations! You have received your Level Up Reward!"), p.Insert(0), p.Insert(0), p.CreatePacket(peer);
									}
									PlayerMoving data_{};
									data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
									BYTE* raw = packPlayerMoving(&data_);
									send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[] raw;
									level_show(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblock\nisStatsWorldBlockUsableByPublic") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							bool world_public = atoi(explode("\n", t_[3])[0].c_str()), floating_public = atoi(explode("\n", t_[4])[0].c_str()), changed = false;
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								if (pInfo(peer)->tankIDName == world_->owner_name) {
									for (int i_ = 0; i_ < world_->gscan.size(); i_++) {
										if (world_->gscan[i_].x == pInfo(peer)->lastwrenchx and world_->gscan[i_].y == pInfo(peer)->lastwrenchy) {
											changed = true;
											world_->gscan[i_].world_public = world_public;
											world_->gscan[i_].floating_public = floating_public;
										}
									}
								}
								if (changed == false) {
									WorldGrowscan gscan_{};
									gscan_.x = pInfo(peer)->lastwrenchx, gscan_.y = pInfo(peer)->lastwrenchy;
									gscan_.world_public = world_public, gscan_.floating_public = floating_public;
									world_->gscan.push_back(gscan_);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblock\nbuttonClicked|back_to_gscan\n") != string::npos || cch.find("action|dialog_return\ndialog_name|statsblockworld\nbuttonClicked|back_to_gscan\n") != string::npos) {
							edit_tile(peer, pInfo(peer)->lastwrenchx, pInfo(peer)->lastwrenchy, 32);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblock\nbuttonClicked|floatingItems\n") != string::npos) {
							send_growscan_floating(peer, "start", "1");
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblock\nbuttonClicked|search_") != string::npos) {
							try {
								string type = cch.substr(65, 1);
								string search = cch.substr(79, cch.length() - 79);
								replace_str(search, "\n", "");
								replace_str(type, "\n", "");
								send_growscan_floating(peer, search, type);
							}
							catch (out_of_range) {
								hoshi_warn("Server error invalid (out of range) on " + cch);
								break;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblock\nbuttonClicked|worldBlocks\n") != string::npos || cch.find("action|dialog_return\ndialog_name|statsblockworld\nbuttonClicked|worldBlocks\n") != string::npos) {
							if (cch.find("action|dialog_return\ndialog_name|statsblock\nbuttonClicked|worldBlocks\n") != string::npos) send_growscan_worldblocks(peer, "start", "1");
							if (cch.find("action|dialog_return\ndialog_name|statsblockworld\nbuttonClicked|worldBlocks\n") != string::npos) send_growscan_worldblocks(peer, "start", "1");
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|statsblockworld\nbuttonClicked|search_") != string::npos) {
							string type = cch.substr(70, 1);
							string search = cch.substr(84, cch.length() - 84);
							replace_str(search, "\n", "");
							replace_str(type, "\n", "");
							send_growscan_worldblocks(peer, search, type);
							break;
						}
						if (cch.find("action|dialog_return\ndialog_name|billboard_edit\nbillboard_toggle|") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							int billboard_active = atoi(explode("\n", t_[3])[0].c_str());
							int billboard_price = atoi(explode("\n", t_[4])[0].c_str());
							int peritem = atoi(explode("\n", t_[5])[0].c_str());
							int perlock = atoi(explode("\n", t_[6])[0].c_str());
							bool update_billboard = true;
							if (peritem == perlock or peritem == 0 and perlock == 0 or peritem == 1 and perlock == 1) {
								update_billboard = false;
								gamepacket_t p, p2;
								p.Insert("OnConsoleMessage"), p.Insert("You need to pick one pricing method - 'locks per item' or 'items per lock'"), p.CreatePacket(peer);
								p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("You need to pick one pricing method - 'locks per item' or 'items per lock'"), p2.Insert(0), p2.Insert(1), p2.CreatePacket(peer);
							}
							else {
								if (peritem == 1) pInfo(peer)->b_w = 1;
								if (perlock == 1) pInfo(peer)->b_w = 0;
							}
							if (billboard_active == 1)pInfo(peer)->b_a = 1;
							else pInfo(peer)->b_a = 0;
							if (billboard_price < 0 or billboard_price > 99999) {
								update_billboard = false;
								gamepacket_t p, p2;
								p.Insert("OnConsoleMessage"), p.Insert("Price can't be negative. That's beyond science."), p.CreatePacket(peer);
								p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("Price can't be negative. That's beyond science."), p2.Insert(0), p2.Insert(1), p2.CreatePacket(peer);
							}
							else pInfo(peer)->b_p = billboard_price;
							if (update_billboard && pInfo(peer)->b_p != 0 && pInfo(peer)->b_i != 0) {
								gamepacket_t p(0, pInfo(peer)->netID);
								p.Insert("OnBillboardChange"), p.Insert(pInfo(peer)->netID), p.Insert(pInfo(peer)->b_i), p.Insert(pInfo(peer)->b_a), p.Insert(pInfo(peer)->b_p), p.Insert(pInfo(peer)->b_w);
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != pInfo(peer)->world) continue;
									p.CreatePacket(currentPeer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|dialog_scarf_of_seasons\nbuttonClicked") != string::npos) {
							if (pInfo(peer)->necklace == 11818) pInfo(peer)->i_11818_1 = 0, pInfo(peer)->i_11818_2 = 0, update_clothes(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|title_edit\nbuttonClicked|") != string::npos) {
							try {
								vector<string> t_ = explode("|", cch);
								if (t_.size() < 4) break;
								int total = 4;
								if (pInfo(peer)->drtitle) {
									pInfo(peer)->drt = atoi(explode("\n", t_.at(total++)).at(0).c_str());
									string thetag = (pInfo(peer)->mod || pInfo(peer)->dev ? "@" : "");
									pInfo(peer)->d_name = (pInfo(peer)->drt ? "`4" + thetag : pInfo(peer)->name_color) + (pInfo(peer)->drt ? "Dr. " : "") + pInfo(peer)->tankIDName;
									if (!pInfo(peer)->drt) pInfo(peer)->d_name = "";
									{
										gamepacket_t p2(0, pInfo(peer)->netID);
										p2.Insert("OnNameChanged"), p2.Insert((not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName));
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != pInfo(peer)->world) continue;
											p2.CreatePacket(currentPeer);
										}
									}
								}
								if (pInfo(peer)->level >= 125) pInfo(peer)->lvl125 = atoi(explode("\n", t_.at(total++)).at(0).c_str());
								if (pInfo(peer)->legend) {
									pInfo(peer)->is_legend = atoi(explode("\n", t_.at(total++)).at(0).c_str());
									string modtag = (pInfo(peer)->mod || pInfo(peer)->dev ? "@" : "");
									if (!pInfo(peer)->is_legend);
									{
										gamepacket_t p2(0, pInfo(peer)->netID);
										p2.Insert("OnNameChanged"), p2.Insert((not pInfo(peer)->d_name.empty() ? pInfo(peer)->d_name : pInfo(peer)->name_color + pInfo(peer)->tankIDName) + (pInfo(peer)->is_legend ? " of Legend``" : ""));
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != pInfo(peer)->world) continue;
											p2.CreatePacket(currentPeer);
										}
									}
								}
								if (pInfo(peer)->gp) pInfo(peer)->donor = atoi(explode("\n", t_.at(total++)).at(0).c_str()), pInfo(peer)->master = atoi(explode("\n", t_.at(total++)).at(0).c_str());
								update_clothes(peer);
							}
							catch (out_of_range& e) {
								hoshi_warn(e.what());
								break;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|dialog_scarf_of_seasons\ncheckbox") != string::npos) {
							try {
								if (pInfo(peer)->necklace == 11818) {
									vector<string> t_ = explode("|", cch);
									if (t_.size() < 4) break;
									for (int i = 3; i <= 10; i++) {
										if (i <= 6 && atoi(explode("\n", t_.at(i)).at(0).c_str()) == 1) pInfo(peer)->i_11818_1 = i - 3;
										else if (atoi(explode("\n", t_.at(i)).at(0).c_str()) == 1) pInfo(peer)->i_11818_2 = i - 7;
									}
									update_clothes(peer);
								}
							}
							catch (out_of_range) {
								break;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|bulletin_edit\nsign_text|\ncheckbox_locked|") != string::npos) {
							try {
								vector<string> t_ = explode("|", cch);
								if (t_.size() < 4) break;
								bool public_can_add = atoi(explode("\n", t_.at(4)).at(0).c_str()), hide_names = atoi(explode("\n", t_.at(5)).at(0).c_str());
								bool changed = false;
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									if (pInfo(peer)->tankIDName == world_->owner_name) {
										for (int i_ = 0; i_ < world_->bulletins.size(); i_++) {
											if (world_->bulletins.at(i_).x == pInfo(peer)->lastwrenchx and world_->bulletins.at(i_).y == pInfo(peer)->lastwrenchy) {
												changed = true;
												world_->bulletins.at(i_).public_can_add = public_can_add;
												world_->bulletins.at(i_).hide_names = hide_names;
											}
										}
									}
									if (changed == false) {
										WorldBulletinSettings set_{};
										set_.x = pInfo(peer)->lastwrenchx, set_.y = pInfo(peer)->lastwrenchy, set_.public_can_add = public_can_add, set_.hide_names = hide_names;
										world_->bulletins.push_back(set_);
									}
								}
							}
							catch (out_of_range) {
								break;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|camera_edit\ncheckbox_showpick|") != string::npos) {
							try {
								vector<string> t_ = explode("|", cch);
								if (t_.size() < 4) break;
								bool show_item_taking = atoi(explode("\n", t_.at(3)).at(0).c_str()), show_item_dropping = atoi(explode("\n", t_.at(4)).at(0).c_str()), show_people_entering = atoi(explode("\n", t_.at(5)).at(0).c_str()), show_people_exiting = atoi(explode("\n", t_.at(6)).at(0).c_str()), dont_show_owner = atoi(explode("\n", t_.at(7)).at(0).c_str()), dont_show_admins = atoi(explode("\n", t_.at(8)).at(0).c_str()), dont_show_noaccess = atoi(explode("\n", t_.at(9)).at(0).c_str()), changed = false;
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									for (int i_ = 0; i_ < world_->cctv_settings.size(); i_++) {
										if (world_->cctv_settings.at(i_).x == pInfo(peer)->lastwrenchx and world_->cctv_settings.at(i_).y == pInfo(peer)->lastwrenchy) {
											changed = true;
											world_->cctv_settings.at(i_).show_item_taking = show_item_taking;
											world_->cctv_settings.at(i_).show_item_dropping = show_item_dropping;
											world_->cctv_settings.at(i_).show_people_entering = show_people_entering;
											world_->cctv_settings.at(i_).show_people_exiting = show_people_exiting;
											world_->cctv_settings.at(i_).dont_show_owner = dont_show_owner;
											world_->cctv_settings.at(i_).dont_show_admins = dont_show_admins;
											world_->cctv_settings.at(i_).dont_show_noaccess = dont_show_noaccess;
										}
									}
									if (changed == false) {
										WorldCCTVSettings cctvs_{};
										cctvs_.x = pInfo(peer)->lastwrenchx, cctvs_.y = pInfo(peer)->lastwrenchy;
										cctvs_.show_item_taking = show_item_taking, cctvs_.show_item_dropping = show_item_dropping, cctvs_.show_people_entering = show_people_entering, cctvs_.show_people_exiting = show_people_exiting, cctvs_.dont_show_owner = dont_show_owner, cctvs_.dont_show_admins = dont_show_admins, cctvs_.dont_show_noaccess = dont_show_noaccess;
										world_->cctv_settings.push_back(cctvs_);
									}
								}
							}
							catch (out_of_range) {
								break;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|camera_edit\nbuttonClicked|clear") != string::npos) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								for (int i_ = 0; i_ < world_->cctv.size(); i_++)if (world_->cctv[i_].x == pInfo(peer)->lastwrenchx and world_->cctv[i_].y == pInfo(peer)->lastwrenchy) {
									if (i_ != 0) {
										world_->cctv.erase(world_->cctv.begin() + i_);
										i_--;
									}
								}
							}
							{
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`2Camera log cleared.``"), p.Insert(0), p.Insert(0), p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|b_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 6896, 6948, 1068, 1966, 1836, 5080, 10754, 1874, 6946 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 1068) count = 10;
							if (find(pInfo(peer)->bb_p.begin(), pInfo(peer)->bb_p.end(), lvl = reward * 5) == pInfo(peer)->bb_p.end()) {
								if (pInfo(peer)->bb_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->bb_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Builder Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										builder_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|autoclave\nbuttonClicked|tool") != string::npos) {
							int itemtool = atoi(cch.substr(61, cch.length() - 61).c_str());
							if (itemtool == 1258 || itemtool == 1260 || itemtool == 1262 || itemtool == 1264 || itemtool == 1266 || itemtool == 1268 || itemtool == 1270 || itemtool == 4308 || itemtool == 4310 || itemtool == 4312 || itemtool == 4314 || itemtool == 4316 || itemtool == 4318) {
								int got = 0;
								modify_inventory(peer, itemtool, got);
								if (got >= 20) {
									pInfo(peer)->lastchoosenitem = itemtool;
									gamepacket_t p;
									p.Insert("OnDialogRequest");
									p.Insert("set_default_color|`o\nadd_label_with_icon|big|`9Autoclave``|left|4322|\nadd_spacer|small|\nadd_textbox|Are you sure you want to destroy 20 " + items[itemtool].ori_name + " in exchange for one of each of the other 12 surgical tools?|left|\nadd_button|verify|Yes!|noflags|0|0|\nend_dialog|autoclave|Cancel||");
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|autoclave\nbuttonClicked|verify") != string::npos) {
							int removeitem = pInfo(peer)->lastchoosenitem, inventory_space = 12, slots = get_free_slots(pInfo(peer)), got = 0;
							modify_inventory(peer, removeitem, got);
							if (got >= 20) {
								vector<int> noobitems{ 1262, 1266, 1264, 4314, 4312, 4318, 4308, 1268, 1258, 1270, 4310, 4316 };
								bool toobig = false;
								for (int i_ = 0, remove = 0; i_ < pInfo(peer)->inv.size(); i_++) for (int i = 0; i < noobitems.size(); i++) {
									if (pInfo(peer)->inv[i_].id == noobitems[i]) {
										if (pInfo(peer)->inv[i_].count == 200) toobig = true;
										else inventory_space -= 1;
									}
								}
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID);
								if (toobig == false && slots >= inventory_space) {
									modify_inventory(peer, removeitem, got = -20);
									for (int i = 0; i < noobitems.size(); i++) {
										if (noobitems[i] == removeitem) continue;
										modify_inventory(peer, noobitems[i], got = 1);
									}
									gamepacket_t p2;
									p.Insert("[`3I swapped 20 " + items[removeitem].ori_name + " for 1 of every other instrument!``]");
									p2.Insert("OnTalkBubble"), p2.Insert("[`3I swapped 20 " + items[removeitem].name + " for 1 of every other instrument!``]"), p2.CreatePacket(peer);
								}
								else p.Insert("No inventory space!");
								p.Insert(0), p.Insert(1), p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|extractor\nbuttonClicked|extractOnceObj_") != string::npos) {
							int got = 0;
							modify_inventory(peer, 6140, got);
							if (got >= 1) {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									int uid = atoi(cch.substr(72, cch.length() - 72).c_str());
									if (world_->owner_name != pInfo(peer)->tankIDName and not pInfo(peer)->dev and (!guild_access(peer, world_->guild_id) and find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end())) break;
									for (int i_ = 0; i_ < world_->drop.size(); i_++) {
										if (world_->drop[i_].id != 0 && world_->drop[i_].x > 0 && world_->drop[i_].y > 0 && world_->drop[i_].uid == uid) {
											gamepacket_t p;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID);
											int c_ = world_->drop[i_].count;
											if (modify_inventory(peer, world_->drop[i_].id, c_) == 0) {
												modify_inventory(peer, 6140, got = -1);
												p.Insert("You have extracted " + to_string(world_->drop[i_].count) + " " + items[world_->drop[i_].id].name + ".");
												int32_t to_netid = pInfo(peer)->netID;
												PlayerMoving data_{}, data2_{};
												data_.effect_flags_check = 1, data_.packetType = 14, data_.netID = 0, data_.plantingTree = world_->drop[i_].uid;
												data2_.x = world_->drop[i_].x, data2_.y = world_->drop[i_].y, data2_.packetType = 19, data2_.plantingTree = 500, data2_.punchX = world_->drop[i_].id, data2_.punchY = pInfo(peer)->netID;
												BYTE* raw = packPlayerMoving(&data_);
												BYTE* raw2 = packPlayerMoving(&data2_);
												raw2[3] = 5;
												memcpy(raw2 + 8, &to_netid, 4);
												for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
													if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
													send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
													send_raw(currentPeer, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
												}
												delete[]raw, raw2;
												world_->drop[i_].id = 0, world_->drop[i_].x = -1, world_->drop[i_].y = -1;
											}
											else p.Insert("No inventory space.");
											p.CreatePacket(peer);
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|zombie_back\nbuttonClicked|zomb_price_") != string::npos) {
							int item = atoi(cch.substr(70, cch.length() - 70).c_str());
							if (item <= 0 || item >= items.size() || items[item].zombieprice == 0) continue;
							pInfo(peer)->lockeitem = item;
							int zombie_brain = 0, pile = 0, total = 0;
							modify_inventory(peer, 4450, zombie_brain);
							modify_inventory(peer, 4452, pile);
							total += zombie_brain + (pile * 100);
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (total >= items[item].zombieprice) p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man``|left|4358|\nadd_textbox|" + items[item].name + " costs " + setGems(items[item].zombieprice) + " Zombie Brains. Are you sure you want to buy it? You have " + setGems(total) + " Zombie Brains.|left|\nadd_button|zomb_item_|Yes, please|noflags|0|0|\nadd_button|back|No, thanks|noflags|0|0|\nend_dialog|zombie_purchase|Hang Up||\n");
							else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man``|left|4358|\nadd_textbox|" + items[item].name + " costs " + setGems(items[item].zombieprice) + " Zombie Brains. You only have " + setGems(total) + " Zombie Brains so you can't afford it. Sorry!|left|\nadd_button|chc3_1|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|zurgery_back\nbuttonClicked|zurg_price_") != string::npos) {
							int item = atoi(cch.substr(71, cch.length() - 71).c_str());
							if (item <= 0 || item >= items.size() || items[item].surgeryprice == 0) continue;
							pInfo(peer)->lockeitem = item;
							int zombie_brain = 0, pile = 0, total = 0;
							modify_inventory(peer, 4298, zombie_brain);
							modify_inventory(peer, 4300, pile);
							total += zombie_brain + (pile * 100);
							gamepacket_t p;
							p.Insert("OnDialogRequest");
							if (total >= items[item].surgeryprice) p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man``|left|4358|\nadd_textbox|" + items[item].name + " costs " + setGems(items[item].surgeryprice) + " Caduceus. Are you sure you want to buy it? You have " + setGems(total) + " Caduceus.|left|\nadd_button|zurg_item_|Yes, please|noflags|0|0|\nadd_button|back|No, thanks|noflags|0|0|\nend_dialog|zurgery_purchase|Hang Up||\n");
							else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wSales-Man``|left|4358|\nadd_textbox|" + items[item].name + " costs " + setGems(items[item].surgeryprice) + " Caduceus. You only have " + setGems(total) + " Caduceus so you can't afford it. Sorry!|left|\nadd_button|chc4_1|Back|noflags|0|0|\nend_dialog|3898|Hang Up||\n");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|giantpotogold\namt|") != string::npos) {
							int count = atoi(cch.substr(51, cch.length() - 51).c_str()), got = 0;
							modify_inventory(peer, pInfo(peer)->lastchoosenitem, got);
							if (got <= 0 || count <= 0 || count > items.size()) break;
							int item = pInfo(peer)->lastchoosenitem;
							if (items[item].untradeable == 1 || item == 1424 || items[item].rarity >= 363 || items[item].rarity == 0 || items[item].rarity < 1 || count > got) {
								gamepacket_t p;
								p.Insert("OnTalkBubble");
								p.Insert(pInfo(peer)->netID);
								if (count > got) p.Insert("You don't have that to give!");
								else p.Insert("I'm sorry, we can't accept items without rarity!");
								p.CreatePacket(peer);
							}
							else {
								pInfo(peer)->b_ra += count * items[item].rarity;
								modify_inventory(peer, pInfo(peer)->lastchoosenitem, count *= -1);
								if (pInfo(peer)->b_ra >= 20000) pInfo(peer)->b_lvl = 2;
								int chance = 29;
								if (pInfo(peer)->b_ra > 25000) chance += 7;
								if (pInfo(peer)->b_ra > 40000) chance += 25;
								if (rand() % 100 < chance && pInfo(peer)->b_ra >= 20000) {
									int give_count = 1, given_count = 1;
									vector<int> list{ 7978,5734, 7986,5724,7980,7990,5730,5726,5728,7988,7992 };
									if (pInfo(peer)->b_ra >= 40000 && rand() % 100 < 15) list = { 7978,5734, 7986,5724,7980,7990,5730,5726,5728,7988,7992, 7996,5718,5720,9418,5732,5722,8000,5740,8002,9414,11728,11730 };
									int given_item = list[rand() % list.size()];
									if (given_item == 7978 || given_item == 5734 || given_item == 7986 || given_item == 5724 || given_item == 7992 || given_item == 7980 || given_item == 7990) give_count = 5, given_count = 5;
									if (given_item == 5730 || given_item == 5726 || given_item == 5728 || given_item == 7988 || given_item == 7980 || given_item == 7990) give_count = 10, given_count = 10;
									if (modify_inventory(peer, given_item, given_count) == 0) {
										gamepacket_t p, p2;
										p.Insert("OnConsoleMessage"), p.Insert(a + "Thanks for your generosity! The pot overflows with `6" + (pInfo(peer)->b_ra < 40000 ? "20" : "40") + ",000 rarity``! Your `6Level 2 prize`` is a fabulous `2" + items[given_item].name + "!``"), p.CreatePacket(peer);
										p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert(a + "Thanks for your generosity! The pot overflows with `6" + (pInfo(peer)->b_ra < 40000 ? "20" : "40") + ",000 rarity``! Your `6Level 2 prize`` is a fabulous `2" + items[given_item].name + "!``"), p2.CreatePacket(peer);
										pInfo(peer)->b_lvl = 1, pInfo(peer)->b_ra = 0;
									}
									else {
										gamepacket_t p;
										p.Insert("OnConsoleMessage"), p.Insert("No inventory space."), p.CreatePacket(peer);
									}
								}
								else {
									gamepacket_t p;
									p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("Thank you for your generosity!"), p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|donation_box_edit\nbuttonClicked|clear_selected\n") != string::npos) {
							try {
								bool took = false, fullinv = false;
								gamepacket_t p3;
								p3.Insert("OnTalkBubble"), p3.Insert(pInfo(peer)->netID);
								string name_ = pInfo(peer)->world;
								vector<string> t_ = explode("|", cch);
								if (t_.size() < 4) break;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									if (world_->owner_name != pInfo(peer)->tankIDName and not pInfo(peer)->dev and not world_->owner_name.empty() and (!guild_access(peer, world_->guild_id) and find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) == world_->admins.end())) break;
									WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
									if (!items[block_->fg].donation) break;
									for (int i_ = 0, remove_ = 0; i_ < block_->donates.size(); i_++, remove_++) {
										if (atoi(explode("\n", t_.at(4 + remove_)).at(0).c_str())) {
											int receive = block_->donates[i_].count;
											if (modify_inventory(peer, block_->donates[i_].item, block_->donates[i_].count) == 0) {
												took = true;
												gamepacket_t p;
												p.Insert("OnConsoleMessage"), p.Insert("`7[``" + pInfo(peer)->tankIDName + " receives `5" + to_string(receive) + "`` `w" + items[block_->donates[i_].item].name + "`` from `w" + block_->donates[i_].name + "``, how nice!`7]``");
												block_->donates.erase(block_->donates.begin() + i_), i_--;
												for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
													if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
													p.CreatePacket(currentPeer);
													packet_(currentPeer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
												}
											}
											else fullinv = true;
										}
									}
									if (block_->donates.size() == 0) {
										if (block_->flags & 0x00400000) block_->flags ^= 0x00400000;
										WorldBlock block_ = world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
										PlayerMoving data_{};
										data_.packetType = 5, data_.punchX = pInfo(peer)->lastwrenchx, data_.punchY = pInfo(peer)->lastwrenchy, data_.characterState = 0x8;
										BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, &block_));
										BYTE* blc = raw + 56;
										form_visual(blc, block_, *world_, peer, false);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 112 + alloc_(world_, &block_), ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw, blc;
									}
								}
								if (fullinv) {
									p3.Insert("I don't have enough room in my backpack to get the item(s) from the box!");
									gamepacket_t p2;
									p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("`2(Couldn't get all of the gifts)``"), p2.CreatePacket(peer);
								}
								else if (took) p3.Insert("`2Box emptied.``");
								packet_(peer, "action|play_sfx\nfile|audio/page_turn.wav\ndelayMS|0");
								p3.CreatePacket(peer);
							}
							catch (out_of_range) {
								break;
							}
							break;
						}
						else if (cch == "action|claimdailyreward\n") {
							if (pInfo(peer)->pinata_prize == false) {
								int c_ = 1;
								gamepacket_t p_c;
								p_c.Insert("OnConsoleMessage");
								if (modify_inventory(peer, 9616, c_) == 0) {
									pInfo(peer)->pinata_day = today_day;
									pInfo(peer)->pinata_prize = true;
									pInfo(peer)->pinata_claimed = false;
									gamepacket_t p, p2;
									p.Insert("OnProgressUIUpdateValue"), p.Insert(pInfo(peer)->pinata_claimed ? 1 : 0), p.Insert(pInfo(peer)->pinata_prize ? 1 : 0), p.CreatePacket(peer);
									p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("You got a Block De Mayo Block!"), p2.CreatePacket(peer);
									p_c.Insert("You got a Block De Mayo Block!");
								}
								else  p_c.Insert("You got a Block De Mayo Block!"),
									p_c.CreatePacket(peer);
							}
							break;
						}
						else if (cch == "action|showcincovolcaniccape\n" || cch == "action|showcincovolcanicwings\n") {
							gamepacket_t p(500);
							p.Insert("OnDialogRequest");
							if (cch == "action|showcincovolcanicwings\n") p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wVolcanic Ventures : Volcanic Wings``|left|11870|\nadd_spacer|small|\nadd_textbox|Every `224 hours``, a limited amount of `2Volcanic Wings`` will be released into the game!|left|\nadd_spacer|small|\nadd_textbox|For your chance to find one of these `#Rare`` items, smash a `2Lava Pinata``. |left|\nadd_spacer|small|\nadd_textbox|There will only be 48 released every 24 hours so, be quick!|left|\nadd_spacer|small|\nadd_textbox|Did you know there are 48 active Volcanoes in Mexico?|left|\nend_dialog|volcanic_quest||OK|");
							else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wVolcanic Ventures : Volcanic Cape``|left|10806|\nadd_spacer|small|\nadd_textbox|Every `224 hours``, a limited amount of `2Volcanic Cape`` will be released into the game!|left|\nadd_spacer|small|\nadd_textbox|For your chance to find one of these `#Rare`` items, smash a `2Lava Pinata``. |left|\nadd_spacer|small|\nadd_textbox|There will only be 48 released every 24 hours so, be quick!|left|\nadd_spacer|small|\nadd_textbox|Did you know there are 48 active Volcanoes in Mexico?|left|\nend_dialog|volcanic_quest||OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch == "action|dailyrewardmenu\n") {
							gamepacket_t p(500);
							p.Insert("OnDailyRewardRequest");
							if (pInfo(peer)->pinata_prize) {
								struct tm newtime;
								time_t now = time(0);
								localtime_s(&newtime, &now);
								p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlock De Mayo|left|9616|\nset_default_color|`o\nadd_image_button||interface/large/gui_shop_buybanner.rttex|bannerlayout|flag_frames:4,1,3,0|flag_surfsize:512,200|\nadd_smalltext|`7Get involved and get rewards!`` Smash an Ultra Pinata once a day during `5Cinco de Mayo Week`` and get a daily reward!|left|\nadd_spacer|small|\nadd_button|claimbutton|Come Back Later|noflags|0|0|\nadd_countdown|" + to_string(24 - newtime.tm_hour) + "H" + (60 - newtime.tm_min != 0 ? " " + to_string(60 - newtime.tm_min) + "M" : "") + "|center|disable|\nadd_quick_exit|");
							}
							else {
								if (pInfo(peer)->pinata_claimed) p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlock De Mayo|left|9616|\nset_default_color|`o\nadd_image_button||interface/large/gui_shop_buybanner.rttex|bannerlayout|flag_frames:4,1,3,0|flag_surfsize:512,200|\nadd_smalltext|`7Get involved and get rewards!`` Smash an Ultra Pinata once a day during `5Cinco de Mayo Week`` and get a daily reward!|left|\nadd_spacer|small|\nadd_button|claimbutton|CLAIM|noflags|0|0|\nadd_countdown||center|enable|\nadd_quick_exit|");
								else p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBlock De Mayo|left|9616|\nset_default_color|`o\nadd_image_button||interface/large/gui_shop_buybanner.rttex|bannerlayout|flag_frames:4,1,3,0|flag_surfsize:512,200|\nadd_smalltext|`7Get involved and get rewards!`` Smash an Ultra Pinata once a day during `5Cinco de Mayo Week`` and get a daily reward!|left|\nadd_spacer|small|\nadd_button|claimbutton|Come Back Later|noflags|0|0|\nadd_countdown||center|disable|\nadd_quick_exit|");
							}
							p.CreatePacket(peer);
							break;
						}
						/*else if (cch == "action|dialog_return\ndialog_name|grow4goodtasks_dialog\nbuttonClicked|tab_tasks\n\n") {
						grow4good(peer, true, "tab_tasks", 0);
						break;
						}
						else if (cch == "action|dialog_return\ndialog_name|grow4goodtasks_dialog\nbuttonClicked|tab_rewards\n\n") {
						grow4good(peer, true, "tab_rewards", 0);
						break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|grow4goodtasks_dialog\nbuttonClicked|item_info_11756") != string::npos) {
						gamepacket_t p(500);
						p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wReward Details`|left|11756|\nadd_spacer|small|\nadd_spacer|small|\nadd_textbox|A Hamper filled with cool stuff given to players as thank you for their generosity throughout the Grow4Good Event!|left|\nadd_spacer|small|\nadd_quick_exit|\nadd_button|tab_rewards|Back|noflags|0|0|\nend_dialog|grow4goodtasks_dialog|Close||"), p.CreatePacket(peer);
						break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|grow4goodtasks_dialog\nbuttonClicked|item_info_11758") != string::npos) {
						gamepacket_t p(500);
						p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wReward Details`|left|11758|\nadd_spacer|small|\nadd_spacer|small|\nadd_textbox|Opening this will give you an awesome prize, which could include one of the following `5EPIC LIMITED QUANTITY ITEMS``:|left|\nadd_label_with_icon|small| Phoenix Wings|left|1674|\nadd_label_with_icon|small| Sun Blade|left|8944|\nadd_label_with_icon|small| Nightmare Devil Wings|left|1970|\nadd_label_with_icon|small| Sonic Buster Katana|left|11118|\nadd_label_with_icon|small| Golden Angel Wings|left|1460|\nadd_label_with_icon|small| Horns of Growganoth|left|9036|\nadd_label_with_icon|small| Neptune's Trident|left|9758|\nadd_label_with_icon|small| Axolotl Scarf|left|10026|\nadd_label_with_icon|small| Thingamabob|left|8284|\nadd_label_with_icon|small| Swordfish Sword|left|10388|\nadd_label_with_icon|small| Possessing Scarf|left|11318|\nadd_label_with_icon|small| American Sports Ball Jersey|left|10252|\nadd_label_with_icon|small| Draconic Wings|left|5754|\nadd_label_with_icon|small| Egg Champion Cape|left|9446|\nadd_label_with_icon|small| Golden Heart Crystal|left|1458|\nadd_label_with_icon|small| Hotpants|left|4664|\nadd_label_with_icon|small| One Winged Angel|left|10748|\nadd_label_with_icon|small| Fish Tank Head|left|11768|\nadd_label_with_icon|small| Growing Guardian Armor|left|11760|\nadd_label_with_icon|small| Cashback Coupon 10,000 Gems|left|5138|\nadd_textbox|`4A MAXIMUM of only 5 of each of these `5EPIC items`` will drop over the course of the event``.|left|\nadd_spacer|small|\nadd_quick_exit|\nadd_button|tab_rewards|Back|noflags|0|0|\nend_dialog|grow4goodtasks_dialog|Close||"), p.CreatePacket(peer);
						break;
						}
						else if (cch == "action|dialog_return\ndialog_name|grow4goodtasks_dialog\nbuttonClicked|claimrewardsg4g\n\n") {
						vector<int> list;
						bool fullinv = false;
						if (pInfo(peer)->grow4good_points >= 40 && pInfo(peer)->grow4good_claimed_prize == 0) list.push_back(11756);
						if (pInfo(peer)->grow4good_points >= 80 && pInfo(peer)->grow4good_claimed_prize == 0) list.push_back(11756);
						if (pInfo(peer)->grow4good_points >= 80 && pInfo(peer)->grow4good_claimed_prize == 1) list.push_back(11756);
						if (pInfo(peer)->grow4good_points >= 240) list.push_back(11758);
						for (int i = 0; i < list.size(); i++) {
							int give = 1;
							if (modify_inventory(peer, list[i], give) == 0) {
								if (list[i] == 11756) pInfo(peer)->grow4good_claimed_prize++;
								else {
									pInfo(peer)->grow4good_points -= 240;
									pInfo(peer)->grow4good_claimed_prize = 0;
								}
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You received `21 "+ items[list[i]].ori_name +"."), p.CreatePacket(peer);
								{
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("You received `21 " + items[list[i]].ori_name + "."), p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage"), p.Insert("No inventory space."), p.CreatePacket(peer);
							}
						}
						if (pInfo(peer)->grow4good_claimed_prize == 3) pInfo(peer)->grow4good_claimed_prize = 0;
						break;
						}
						else if (cch == "action|grow4goodcomunity\n") {
						grow4good(peer, true, "tab_tasks", 0);
						break;
						}
						else if (cch == "action|grow4goodDonateRarity\n") {
						gamepacket_t p(500);
						p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label|big|`wDonate to Charity!`|left|\nadd_image_button||interface/large/wtr_lvl2_wjcxuy.rttex|bannerlayout|flag_frames:1,1,0,0|flag_surfsize:985,256|\nadd_spacer|small|\nadd_item_picker|donaterarity|`wSelect an Item to Donate``|Donate|\nadd_label|small|This will donate any items with a rarity of 100 or lower to go towards the work of Operation Smile. Ubisoft will convert this in-game donation to a monetary amount through the global milestones.|left\nadd_spacer|small|\nadd_button|cancel|Cancel|noflags|0|0|\nend_dialog|grow4goodtasks_dialog|||"), p.CreatePacket(peer);
						break;
						}
						else if (cch == "action|grow4goodDonateWL\n") {
						int got = 0;
						modify_inventory(peer, 242, got);
						if (got <= 0) {
							gamepacket_t p;
							p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("`4You dont have any World Lock in your inventory.``"), p.CreatePacket(peer);
						}
						else {
							pInfo(peer)->lockeitem = 242;
							gamepacket_t p(500);
							p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label|big|Donate to Charity|left|\nadd_image_button||interface/large/wtr_lvl2_wjcxuy.rttex|bannerlayout|flag_frames:1,1,0,0|flag_surfsize:985,256|\nadd_spacer|small|\nadd_textbox|How many to donate? (you have " + to_string(got) + ")|left|\nadd_text_input|count||0|5|\nadd_spacer|small|\nadd_smalltext|Total World Lock Donated Today: " + to_string(pInfo(peer)->grow4good_total_wl) + "|left|\nadd_spacer|small|\nadd_label|small|This will donate World Locks to go towards the work of Operation Smile. Ubisoft will convert this in-game donation to a monetary amount through the global milestones.|left\nadd_spacer|small|\nend_dialog|donate_item_rarity|Cancel|OK|"), p.CreatePacket(peer);
						}
						break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|grow4goodtasks_dialog\ndonaterarity|") != string::npos) {
						int item = atoi(cch.substr(68, cch.length() - 68).c_str()), got = 0;
							if (item <= 0 || item > items.size()) break;
							modify_inventory(peer, item, got);
							if (got == 0) break;
							pInfo(peer)->lockeitem = item;
							gamepacket_t p;
							if (items[item].untradeable || items[item].rarity <= 0 || items[item].rarity >= 367) p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("I'm sorry, we can't accept items without rarity!");
							else p.Insert("OnDialogRequest"), p.Insert("add_label_with_icon|big|Donate `w" + items[item].ori_name + "|left|" + to_string(item) + "|\nadd_textbox|How many to donate? (you have " + to_string(got) + ")|left|\nadd_text_input|count||0|5|\nadd_spacer|small|\nadd_smalltext|Item Rarity : " + to_string(items[item].rarity) + "|left|\nadd_smalltext|Total Rarity Donated Today: " + to_string(pInfo(peer)->grow4good_rarity) + "|left|\nadd_spacer|small|\nend_dialog|donate_item_rarity|Cancel|OK|");
							p.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|donate_item_rarity\ncount|") != string::npos) {
						int count = atoi(cch.substr(58, cch.length() - 58).c_str()), got = 0, item = pInfo(peer)->lockeitem;
						if (item <= 0 || item > items.size()) break;
						modify_inventory(peer, item, got);
						if (got == 0 || count > got || count <= 0) break;
						pInfo(peer)->donate_count = count;
						gamepacket_t p;
						p.Insert("OnDialogRequest"), p.Insert("set_default_color|`o\nadd_label_with_icon|big|`4Donate`` " + to_string(count) + " `w" + items[item].ori_name + "``|left|" + to_string(item) + "|\nadd_textbox|Are you sure you want to do this? There is no way to get the item back if you select yes.|left|\nend_dialog|donate_item_rarity_confirm|NO!|Yes, I am sure|"), p.CreatePacket(peer);
						break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|donate_item_rarity_confirm\n") != string::npos) {
						int item = pInfo(peer)->lockeitem, needed = pInfo(peer)->donate_count, got = 0, remove = 0;
						modify_inventory(peer, item, got);
						if (got < needed || needed <= 0) break;
						remove = needed * -1;
						modify_inventory(peer, item, remove);
						if (item == 242)grow4good(peer, false, "wl", needed);
						else grow4good(peer, false, "rarity", needed* items[item].rarity);
						break;
						}*/
						else if (cch == "action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|surgery_reward\n\n") {
							surgery_reward_show(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|password_reply\npassword|") != string::npos) {
							string password = cch.substr(57, cch.length() - 57).c_str();
							string name_ = pInfo(peer)->world;
							vector<World>::iterator pa = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (pa != worlds.end()) {
								World* world_ = &worlds[pa - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								if (block_->fg == 762 && block_->door_id != "") {
									gamepacket_t p;
									p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID);
									replace_str(password, "\n", "");
									transform(password.begin(), password.end(), password.begin(), ::toupper);
									if (block_->door_id != password) p.Insert("`4Wrong password!``");
									else {
										p.Insert(a + "`2The door opens!" + (block_->door_destination == "" ? " But nothing is behind it." : "") + "``");
										if (block_->door_destination != "") {
											gamepacket_t p3(0, pInfo(peer)->netID);
											p3.Insert("OnPlayPositioned"), p3.Insert("audio/door_open.wav"), p3.CreatePacket(peer);
											string door_target = block_->door_destination, door_id = "";
											World target_world = worlds[pa - worlds.begin()];
											int spawn_x = 0, spawn_y = 0;
											if (door_target.find(":") != string::npos) {
												vector<string> detales = explode(":", door_target);
												door_target = detales[0], door_id = detales[1];
											}
											int ySize = (int)target_world.blocks.size() / 100, xSize = (int)target_world.blocks.size() / ySize;
											if (not door_id.empty()) {
												for (int i_ = 0; i_ < target_world.blocks.size(); i_++) {
													WorldBlock block_data = target_world.blocks[i_];
													if (block_data.fg == 762) continue;
													if (block_data.fg == 1684 or items[block_data.fg].blockType == BlockTypes::DOOR or items[block_data.fg].blockType == BlockTypes::PORTAL) {
														if (block_data.door_id == door_id) {
															spawn_x = i_ % xSize, spawn_y = i_ / xSize;
															break;
														}
													}
												}
											}
											join_world(peer, target_world.name, spawn_x, spawn_y, 250, false, true);
										}
									}
									p.CreatePacket(peer);
								}
							}
							break;
						}
						else if (cch == "action|dialog_return\ndialog_name|2646\nbuttonClicked|off\n\n") {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world or block_->spotlight != pInfo(currentPeer)->tankIDName) continue;
									pInfo(currentPeer)->spotlight = false, update_clothes(currentPeer);
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("Back to anonymity. (`$In the Spotlight`` mod removed)"), p.CreatePacket(currentPeer);
								}
								gamepacket_t p;
								p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("Lights out!"), p.CreatePacket(peer);
								block_->spotlight = "";
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|2646\nID|") != string::npos) {
							int netID = atoi(cch.substr(41, cch.length() - 41).c_str());
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								string new_spotlight = "";
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[pInfo(peer)->lastwrenchx + (pInfo(peer)->lastwrenchy * 100)];
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world) continue;
									if (block_->spotlight == pInfo(currentPeer)->tankIDName || pInfo(currentPeer)->netID == netID) {
										if (pInfo(currentPeer)->netID == netID) {
											new_spotlight = pInfo(currentPeer)->tankIDName, pInfo(currentPeer)->spotlight = true;
											gamepacket_t p;
											p.Insert("OnConsoleMessage"), p.Insert("All eyes are on you! (`$In the Spotlight`` mod added)"), p.CreatePacket(currentPeer);
										}
										else {
											gamepacket_t p;
											p.Insert("OnConsoleMessage"), p.Insert("Back to anonymity. (`$In the Spotlight`` mod removed)"), p.CreatePacket(currentPeer);
											pInfo(currentPeer)->spotlight = false;
										}
										if (new_spotlight != "") for (int i_ = 0; i_ < world_->blocks.size(); i_++) if (world_->blocks[i_].spotlight == new_spotlight) world_->blocks[i_].spotlight = "";
										gamepacket_t p;
										p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You shine the light on " + (new_spotlight == pInfo(peer)->tankIDName ? "yourself" : new_spotlight) + "!"), p.CreatePacket(peer);
										update_clothes(currentPeer);
									}
								}
								block_->spotlight = new_spotlight;
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|grinder\ncount|") != string::npos) {
							int count = atoi(cch.substr(47, cch.length() - 47).c_str()), item = pInfo(peer)->lastchoosenitem, got = 0;
							modify_inventory(peer, item, got);
							if (items[item].grindable_count == 0 || got == 0 || count <= 0 || count * items[item].grindable_count > got) break;
							int remove = (count * items[item].grindable_count) * -1;
							modify_inventory(peer, item, remove);
							gamepacket_t p, p2;
							p.Insert("OnConsoleMessage"), p.Insert("Ground up " + to_string(count * items[item].grindable_count) + " " + items[item].name + " into " + to_string(count) + " " + items[items[item].grindable_prize].name + "!"), p.CreatePacket(peer);
							p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("Ground up " + to_string(count * items[item].grindable_count) + " " + items[item].name + " into " + to_string(count) + " " + items[items[item].grindable_prize].name + "!"), p2.Insert(0), p2.Insert(1), p2.CreatePacket(peer);
							{
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									PlayerMoving data_{};
									data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16, data_.packetType = 19, data_.plantingTree = 500, data_.punchX = items[item].grindable_prize, data_.punchY = pInfo(peer)->netID;
									int32_t to_netid = pInfo(peer)->netID;
									BYTE* raw = packPlayerMoving(&data_);
									raw[3] = 5;
									memcpy(raw + 8, &to_netid, 4);
									send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[] raw;
									int c_ = count;
									if (modify_inventory(peer, items[item].grindable_prize, c_) != 0) {
										WorldDrop drop_block_{};
										drop_block_.id = items[item].grindable_prize, drop_block_.count = count, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
										dropas_(world_, drop_block_);
									}
									{
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 221, data_.YSpeed = 221, data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16, data_.XSpeed = item;
										BYTE* raw = packPlayerMoving(&data_);
										send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										delete[] raw;
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worlds_list\nbuttonClicked|s_claimreward") != string::npos) {
							int reward = atoi(cch.substr(72, cch.length() - 72).c_str()), lvl = 0, count = 1;
							vector<int> list{ 6900, 6982, 6212, 3172, 9068, 6912, 10836, 3130, 8284 };
							if (reward <= 0 || reward > list.size()) break;
							if (list[reward - 1] == 10836) count = 100;
							if (list[reward - 1] == 6212) count = 50;
							if (list[reward - 1] == 3172 || list[reward - 1] == 6912) count = 25;
							if (find(pInfo(peer)->surg_p.begin(), pInfo(peer)->surg_p.end(), lvl = reward * 5) == pInfo(peer)->surg_p.end()) {
								if (pInfo(peer)->s_lvl >= lvl) {
									if (modify_inventory(peer, list[reward - 1], count) == 0) {
										pInfo(peer)->surg_p.push_back(lvl);
										packet_(peer, "action|play_sfx\nfile|audio/piano_nice.wav\ndelayMS|0");
										{
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("Congratulations! You have received your Surgeon Reward!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
										PlayerMoving data_{};
										data_.packetType = 17, data_.netID = 198, data_.YSpeed = 198, data_.x = pInfo(peer)->x + 16, data_.y = pInfo(peer)->y + 16;
										BYTE* raw = packPlayerMoving(&data_);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
											if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
											send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
										}
										delete[] raw;
										{
											PlayerMoving data_{};
											data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16, data_.packetType = 19, data_.plantingTree = 100, data_.punchX = list[reward - 1], data_.punchY = pInfo(peer)->netID;
											int32_t to_netid = pInfo(peer)->netID;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = 5;
											memcpy(raw + 8, &to_netid, 4);
											send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											delete[] raw;
										}
										surgery_reward_show(peer);
									}
									else {
										gamepacket_t p;
										p.Insert("OnTalkBubble");
										p.Insert(pInfo(peer)->netID);
										p.Insert("You have full inventory space!");
										p.Insert(0), p.Insert(0);
										p.CreatePacket(peer);
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|zombie_purchase\nbuttonClicked|zomb_item_") != string::npos) {
							int item = pInfo(peer)->lockeitem;
							if (item <= 0 || item >= items.size() || items[item].zombieprice == 0) continue;
							int allwl = 0, wl = 0, dl = 0, price = items[item].zombieprice;
							modify_inventory(peer, 4450, wl);
							modify_inventory(peer, 4452, dl);
							allwl = wl + (dl * 100);
							if (allwl >= price) {
								int c_ = 1;
								if (modify_inventory(peer, item, c_) == 0) {
									if (wl >= price) modify_inventory(peer, 4450, price *= -1);
									else {
										modify_inventory(peer, 4450, wl *= -1);
										modify_inventory(peer, 4452, dl *= -1);
										int givedl = (allwl - price) / 100;
										int givewl = (allwl - price) - (givedl * 100);
										modify_inventory(peer, 4450, givewl);
										modify_inventory(peer, 4452, givedl);
									}
									PlayerMoving data_{};
									data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16, data_.packetType = 19, data_.plantingTree = 500, data_.punchX = item, data_.punchY = pInfo(peer)->netID;
									int32_t to_netid = pInfo(peer)->netID;
									BYTE* raw = packPlayerMoving(&data_);
									raw[3] = 5;
									memcpy(raw + 8, &to_netid, 4);
									send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[] raw;
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("`3You bought " + items[item].name + " for " + setGems(items[item].zombieprice) + " Zombie Brains."), p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("No inventory space."), p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage"), p.Insert("`9You don't have enough Zombie Brains!``"), p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|zurgery_purchase\nbuttonClicked|zurg_item_") != string::npos) {
							int item = pInfo(peer)->lockeitem;
							if (item <= 0 || item >= items.size() || items[item].surgeryprice == 0) continue;
							int allwl = 0, wl = 0, dl = 0, price = items[item].surgeryprice;
							modify_inventory(peer, 4298, wl);
							modify_inventory(peer, 4300, dl);
							allwl = wl + (dl * 100);
							if (allwl >= price) {
								int c_ = 1;
								if (modify_inventory(peer, item, c_) == 0) {
									if (item == 1486 && pInfo(peer)->quest_active && pInfo(peer)->quest_step == 6 && pInfo(peer)->quest_progress < 28) {
										pInfo(peer)->quest_progress++;
										if (pInfo(peer)->quest_progress >= 28) {
											pInfo(peer)->quest_progress = 28;
											gamepacket_t p;
											p.Insert("OnTalkBubble");
											p.Insert(pInfo(peer)->netID);
											p.Insert("`9Legendary Quest step complete! I'm off to see a Wizard!");
											p.Insert(0), p.Insert(0);
											p.CreatePacket(peer);
										}
									}
									if (wl >= price) modify_inventory(peer, 4298, price *= -1);
									else {
										modify_inventory(peer, 4298, wl *= -1);
										modify_inventory(peer, 4300, dl *= -1);
										int givedl = (allwl - price) / 100;
										int givewl = (allwl - price) - (givedl * 100);
										modify_inventory(peer, 4298, givewl);
										modify_inventory(peer, 4300, givedl);
									}
									PlayerMoving data_{};
									data_.x = pInfo(peer)->lastwrenchx * 32 + 16, data_.y = pInfo(peer)->lastwrenchy * 32 + 16, data_.packetType = 19, data_.plantingTree = 500, data_.punchX = item, data_.punchY = pInfo(peer)->netID;
									int32_t to_netid = pInfo(peer)->netID;
									BYTE* raw = packPlayerMoving(&data_);
									raw[3] = 5;
									memcpy(raw + 8, &to_netid, 4);
									send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									delete[] raw;
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("`3You bought " + items[item].name + " for " + setGems(items[item].surgeryprice) + " Caduceus."), p.CreatePacket(peer);
								}
								else {
									gamepacket_t p;
									p.Insert("OnConsoleMessage"), p.Insert("No inventory space."), p.CreatePacket(peer);
								}
							}
							else {
								gamepacket_t p;
								p.Insert("OnConsoleMessage"), p.Insert("`9You don't have enough Caduceus!``"), p.CreatePacket(peer);
							}
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|worldreport\nreport_reason|") != string::npos) {
							if (pInfo(peer)->tankIDName == "") break;
							string report = cch.substr(59, cch.length() - 59).c_str();
							replace_str(report, "\n", "");
							add_modlogs(peer, pInfo(peer)->name_color + pInfo(peer)->tankIDName, "`4reports:`` " + report, "");
							gamepacket_t p2;
							p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("Thank you for your report. Now leave this world so you don't get punished along with the scammers!"), p2.CreatePacket(peer);
							break;
						}
						else if (cch.find("action|dialog_return\ndialog_name|billboard_edit\nbillboard_item|") != string::npos) {
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 4) break;
							int billboard_item = atoi(explode("\n", t_[3])[0].c_str());
							if (billboard_item > 0 && billboard_item < items.size()) {
								int got = 0;
								modify_inventory(peer, billboard_item, got);
								if (got == 0) break;
								if (items[billboard_item].untradeable == 1 or billboard_item == 1424 or items[billboard_item].blockType == BlockTypes::LOCK or items[billboard_item].blockType == BlockTypes::FISH) {
									gamepacket_t p, p2;
									p.Insert("OnConsoleMessage"), p.Insert("Item can not be untradeable."), p.CreatePacket(peer);
									p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("Item can not be untradeable."), p2.Insert(0), p2.Insert(1), p2.CreatePacket(peer);
								}
								else {
									pInfo(peer)->b_i = billboard_item;
									if (pInfo(peer)->b_p != 0 && pInfo(peer)->b_i != 0) {
										gamepacket_t p(0, pInfo(peer)->netID);
										p.Insert("OnBillboardChange"), p.Insert(pInfo(peer)->netID), p.Insert(pInfo(peer)->b_i), p.Insert(pInfo(peer)->b_a), p.Insert(pInfo(peer)->b_p), p.Insert(pInfo(peer)->b_w);
										for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
											if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != pInfo(peer)->world) continue;
											p.CreatePacket(currentPeer);
										}
									}
								}
							}
							break;
						}
						else if (cch.find("action|dialog_return") != string::npos) {
							call_dialog(peer, cch);
							break;
						}
					}
					else if (cch.find("action|dialog_return") != string::npos) {
						call_dialog(peer, cch);
						break;
					}
					break;
				}
				case 3: // world/enter
				{
					//auto start = chrono::steady_clock::now();
					if (pInfo(peer)->trading_with != -1) {
						cancel_trade(peer, false);
						break;
					}
					string cch = text_(event.packet);
					if (pInfo(peer)->lpps2 + 1000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(peer)->pps2 = 0;
						pInfo(peer)->lpps2 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						pInfo(peer)->pps2++;
						if (pInfo(peer)->pps2 >= 10) {
							hoshi_warn("Over packet 3 limit from " + pInfo(peer)->tankIDName + " in world " + pInfo(peer)->world + " packet was " + cch);
							enet_peer_disconnect_later(peer, 0);
							break;
						}
					}
					//cout << cch << endl;
					if (cch == "action|quit") { // kai quit issaugo dar bus settings ar captcha bypassed disconnect
						if (not pInfo(peer)->tankIDName.empty()) // jeigu yra growid
							save_player(pInfo(peer)); // issaugoti zaidejo json
						if (pInfo(peer)->trading_with != -1) {
							cancel_trade(peer, false);
						}
						enet_peer_disconnect_later(peer, 0); // turetu nesuveikti tada antra karta save
						delete peer->data;
						peer->data = NULL;
					}
					else if (cch == "action|quit_to_exit") {
						exit_(peer);
					}
					else if (cch.find("action|join_request") != string::npos) {
						if (pInfo(peer)->last_world_enter + 500 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->last_world_enter = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							vector<string> t_ = explode("|", cch);
							if (t_.size() < 3) break;
							string world_name = explode("\n", t_[2])[0];
							transform(world_name.begin(), world_name.end(), world_name.begin(), ::toupper);
							join_world(peer, world_name);
						}
					}
					//auto end = chrono::steady_clock::now();
					//int uztruko = chrono::duration_cast<chrono::milliseconds>(end - start).count();
					//if (uztruko >= 500) {
						//cout << "Lag case 3 warning: " << cch << " took " << uztruko << " ms\n";
					//}
					break;
				}
				case 4:
				{
					string cch = text_(event.packet);
					//auto start = chrono::steady_clock::now();
					if (pInfo(peer)->lpps23 + 1000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
						pInfo(peer)->pps23 = 0;
						pInfo(peer)->lpps23 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
					}
					else {
						pInfo(peer)->pps23++;
						if (pInfo(peer)->pps23 >= 360) {
							hoshi_warn("Over packet 4 limit from " + pInfo(peer)->tankIDName + " in world " + pInfo(peer)->world + " packet was " + cch);
							packet_(peer, "action|log\nmsg|`7Your client sending too many packets, atempt to reconnect.");
							enet_peer_disconnect_later(peer, 0);
							break;
						}
					}
					if (pInfo(peer)->world.empty()) break;
					BYTE* data_ = get_struct(event.packet);
					if (data_ == nullptr) break;
					PlayerMoving* p_ = unpackPlayerMoving(data_);
					switch (p_->packetType) {
					case 0: /*Kai zaidejas pajuda*/
					{
						int currentX = pInfo(peer)->x / 32;
						int currentY = pInfo(peer)->y / 32;
						int targetX = p_->x / 32;
						int targetY = p_->y / 32;

						if (pInfo(peer)->hand == 3066) {
							if ((p_->punchX > 0 && p_->punchX < 100) && (p_->punchY > 0 && p_->punchY < 60) && p_->plantingTree == 0) edit_tile(peer, p_->punchX, p_->punchY, 18);
						}
						if ((int)p_->characterState == 268435472 || (int)p_->characterState == 268435488 || (int)p_->characterState == 268435504 || (int)p_->characterState == 268435616 || (int)p_->characterState == 268435632 || (int)p_->characterState == 268435456 || (int)p_->characterState == 224 || (int)p_->characterState == 112 || (int)p_->characterState == 80 || (int)p_->characterState == 96 || (int)p_->characterState == 224 || (int)p_->characterState == 65584 || (int)p_->characterState == 65712 || (int)p_->characterState == 65696 || (int)p_->characterState == 65536 || (int)p_->characterState == 65552 || (int)p_->characterState == 65568 || (int)p_->characterState == 65680 || (int)p_->characterState == 192 || (int)p_->characterState == 65664 || (int)p_->characterState == 65600 || (int)p_->characterState == 67860 || (int)p_->characterState == 64) {
							if (pInfo(peer)->lava_time + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
								pInfo(peer)->lavaeffect = 0;
								pInfo(peer)->lava_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							}
							else {
								if (pInfo(peer)->lavaeffect >= (pInfo(peer)->feet == 250 ? 7 : 3) || pInfo(peer)->lavaeffect >= (pInfo(peer)->necklace == 5426 ? 7 : 3)) {
									pInfo(peer)->lavaeffect = 0;
									SendRespawn(peer, false, 0, true);
								}
								else pInfo(peer)->lavaeffect++;
							}
						}
						if (pInfo(peer)->fishing_used != 0) {
							if (pInfo(peer)->f_xy != pInfo(peer)->x + pInfo(peer)->y) pInfo(peer)->move_warning++;
							if (pInfo(peer)->move_warning > 1) stop_fishing(peer, true, "Sit still if you wanna fish!");
							if (p_->punchX > 0 && p_->punchY > 0) {
								pInfo(peer)->punch_warning++;
								if (pInfo(peer)->punch_warning >= 2) stop_fishing(peer, false, "");
							}
						}
						string name_ = pInfo(peer)->world;
						vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
						if (p != worlds.end()) {
							World* world_ = &worlds[p - worlds.begin()];
							if (pInfo(peer)->x != -1 and pInfo(peer)->y != -1) {
								//try {
								int x_ = (pInfo(peer)->state == 16 ? (int)p_->x / 32 : round((double)p_->x / 32));
								int y_ = (int)p_->y / 32;
								if (x_ < 0 or x_ >= 100 or y_ < 0 or y_ >= 60) continue;
								WorldBlock* block_ = &world_->blocks[x_ + (y_ * 100)];
								if (block_->fg == 1256) pInfo(peer)->hospital_bed = true;
								else pInfo(peer)->hospital_bed = false;
								if (pInfo(peer)->c_x * 32 != (int)p_->x and pInfo(peer)->c_y * 32 != (int)p_->y and not pInfo(peer)->ghost) {
									bool impossible = ar_turi_noclipaa(world_, pInfo(peer)->x, pInfo(peer)->y, block_, peer);
									bool canGhost = false;
									if (pInfo(peer)->dev || pInfo(peer)->mod || pInfo(peer)->superdev) canGhost = true;
									if (impossible) {
										if (items[block_->fg].actionType != 31) {
											gamepacket_t p(0, pInfo(peer)->netID);
											p.Insert("OnSetPos");
											p.Insert(pInfo(peer)->x, pInfo(peer)->y);
											p.CreatePacket(peer);
											pInfo(peer)->hack_++;
											if (pInfo(peer)->hack_ >= 4 and not canGhost) {
												enet_peer_disconnect_later(peer, 0);
												hoshi_warn("Detected Suspicious Activity (Trying to NoClip?) from " + pInfo(peer)->tankIDName);
												//add_ban(peer, 6.307e+7, "Usage of Third Party Program", "System");
											}
											break;
										}
									}
								}
								if (block_->fg == 1508 and not world_->name.empty()) {
									char blarney_world = world_->name.back();
									if (isdigit(blarney_world)) {
										long long current_time = time(nullptr);
										vector<vector<long long>> av_blarneys = pInfo(peer)->completed_blarneys;
										for (int i_ = 0; i_ < av_blarneys.size(); i_++) {
											int t_blarney_world = av_blarneys[i_][0];
											if ((int)blarney_world - 48 == t_blarney_world) {
												long long blarney_time = av_blarneys[i_][1];
												if (blarney_time - current_time <= 0) {
													av_blarneys[i_][1] = current_time + 86400;
													vector<vector<int>> blarney_prizes{
														//11712 11742 11710 11722
														{11712, 1},{11742, 1},{11710, 1},{11722, 1}, {528, 1},{540, 1},{1514, 5},{1544, 1},{260, 1},{1546, 1},{2400, 1},{2404, 1},{2406, 1},{2414, 1},{2416, 1},{2464, 1},{3428, 1},{3426, 1},{4532, 1},{4528, 1},{4526, 5},{4520, 1},{5740, 1},{5734, 1},{7982, 1},{7992, 1},{7994, 1},{7980, 1},{7998, 1},{7984, 3},{7988, 1},{9416, 1},{9424, 1},{10704, 1},{10680, 1},{10670, 1},{10676, 1}
													};
													vector<int> prize_ = blarney_prizes[rand() % blarney_prizes.size()];
													uint32_t give_id = prize_[0];
													uint32_t give_count = prize_[1];
													int c_ = give_count;
													if (modify_inventory(peer, give_id, c_) != 0) {
														WorldDrop drop_block_{};
														drop_block_.id = give_id, drop_block_.count = give_count, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
														dropas_(world_, drop_block_);
													}
													int c_2 = 1;
													if (modify_inventory(peer, 1510, c_2) != 0) {
														WorldDrop drop_block_{};
														drop_block_.id = 1510, drop_block_.count = c_2, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(peer)->x + rand() % 17, drop_block_.y = pInfo(peer)->y + rand() % 17;
														dropas_(world_, drop_block_);
													}
													pInfo(peer)->remind_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
													gamepacket_t p;
													p.Insert("OnTalkBubble");
													p.Insert(pInfo(peer)->netID);
													p.Insert("You kissed the " + items[block_->fg].name + " and got a `2" + items[1510].name + "`` and `2" + items[give_id].name + "``");
													p.Insert(1);
													p.CreatePacket(peer);
													{
														gamepacket_t p;
														p.Insert("OnConsoleMessage");
														p.Insert("You kissed the " + items[block_->fg].name + " and got a `2" + items[1510].name + "`` and `2" + items[give_id].name + "``");
														p.CreatePacket(peer);
													}
												}
												else {
													if (pInfo(peer)->remind_time + 8000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
														pInfo(peer)->remind_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
														gamepacket_t p;
														p.Insert("OnTalkBubble");
														p.Insert(pInfo(peer)->netID);
														p.Insert("You will be able to kiss the stone again in " + to_playmod_time(blarney_time - current_time) + "");
														p.Insert(0);
														p.CreatePacket(peer);
													}
												}
												break;
											}
										}
										pInfo(peer)->completed_blarneys = av_blarneys;
									}
								}
								if (block_->fg == 1792 and not world_->name.empty()) {
									string world_name = "LEGENDARYMOUNTAIN";
									if (pInfo(peer)->world == world_name) {
										int adaBrp = 0;
										modify_inventory(peer, 1794, adaBrp);
										if (adaBrp != 0) {
											break;
										}
										else {
											int c = 1;
											modify_inventory(peer, 1794, c);
											for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
												if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
												if (pInfo(currentPeer)->world == pInfo(peer)->world) {
													gamepacket_t p3;
													p3.Insert("OnParticleEffect");
													p3.Insert(46);
													p3.Insert((float)pInfo(peer)->x + 10, (float)pInfo(peer)->y + 16);
													p3.CreatePacket(currentPeer);
												}
											}
											break;
										}
									}
								}
								//}
								//catch (out_of_range) { // nuskrido uz worldo
									//cout << "failed to perform anticheat check for player " << pInfo(peer)->tankIDName + " invalid world??" << endl;
								//}
							}
							if (pInfo(peer)->hand == 2286) {
								if (rand() % 100 < 6) {
									pInfo(peer)->geiger_++;
									if (pInfo(peer)->geiger_ >= 100) {
										int c_ = -1;
										modify_inventory(peer, 2286, c_);
										int c_2 = 1;
										modify_inventory(peer, 2204, c_2);
										pInfo(peer)->hand = 2204;
										pInfo(peer)->geiger_ = 0;
										gamepacket_t p;
										p.Insert("OnConsoleMessage");
										p.Insert("You are detecting radiation... (`$Geiger Counting`` mod added)");
										p.CreatePacket(peer);
										packet_(peer, "action|play_sfx\nfile|audio/dialog_confirm.wav\ndelayMS|0");
										update_clothes(peer);
									}
								}
							}
							if (pInfo(peer)->gems > 0 && pInfo(peer)->back == 240) {
								if (pInfo(peer)->x != (int)p_->x) {
									if (pInfo(peer)->i240 + 750 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
										pInfo(peer)->i240 = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
										pInfo(peer)->gems -= 1;
										WorldDrop item_{};
										item_.id = 112, item_.count = 1, item_.x = (int)p_->x + rand() % 17, item_.y = (int)p_->y + rand() % 17, item_.uid = uint16_t(world_->drop.size()) + 1;
										dropas_(world_, item_);
										gamepacket_t p;
										p.Insert("OnSetBux");
										p.Insert(pInfo(peer)->gems);
										p.Insert(0);
										p.Insert((pInfo(peer)->supp >= 1) ? 1 : 0);
										if (pInfo(peer)->supp >= 2) {
											p.Insert((float)33796, (float)1, (float)0);
										}
										p.CreatePacket(peer);
									}
								}
							}
							move_(peer, p_);
							if (pInfo(peer)->x == -1 and pInfo(peer)->y == -1) {
								update_clothes(peer);
								uint32_t my_guild_role = -1;
								uint32_t guild_id = pInfo(peer)->guild_id;
								vector<Guild>::iterator find_guild = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });

								if (pInfo(peer)->guild_id != 0) {
									if (find_guild != guilds.end()) {
										Guild* guild_information = &guilds[find_guild - guilds.begin()];
										for (GuildMember member_search : guild_information->guild_members) {
											if (member_search.member_name == pInfo(peer)->tankIDName) {
												my_guild_role = member_search.role_id;
												break;
											}
										}
									}
								}
								gamepacket_t p3(0, pInfo(peer)->netID);
								p3.Insert("OnSetRoleSkinsAndIcons");
								p3.Insert(pInfo(peer)->roleSkin);
								p3.Insert(pInfo(peer)->roleIcon);
								p3.Insert(0);
								for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
									if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
									if (pInfo(currentPeer)->world == world_->name) {
										uint32_t guild_id = pInfo(peer)->guild_id;
										vector<Guild>::iterator find_guild = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
										if (find_guild != guilds.end()) {
											Guild* guild_information = &guilds[find_guild - guilds.begin()];
											const auto flag1 = (65536 * guild_information->guild_mascot[1] + guild_information->guild_mascot[0]);
											gamepacket_t p(0, pInfo(peer)->netID);
											p.Insert("OnGuildDataChanged");
											p.Insert(50478);
											p.Insert(79289404);
											p.Insert(flag1);
											p.Insert(my_guild_role);
											p.Insert(0);
											/*gamepacket_t p(0, pInfo(peer)->netID);
											p.Insert("OnGuildDataChanged");
											p.Insert(50478);  //guild_information->guild_mascot[1]
											p.Insert(79289404);
											p.Insert(0), p.Insert(my_guild_role);*/

											/*
											gamepacket_t p2(0, pInfo(peer)->netID);
											p2.Insert("OnCountryState");
											p2.Insert(pInfo(peer)->country + "|showGuild");*/
											p3.CreatePacket(currentPeer);
											if (my_guild_role != -1) {
												p.CreatePacket(currentPeer);
												//p2.CreatePacket(currentPeer);
											}
										}
										if (pInfo(currentPeer)->netID != pInfo(peer)->netID) {
											if (pInfo(currentPeer)->roleSkin != 6 or pInfo(currentPeer)->roleIcon != 6) {
												gamepacket_t p_p(0, pInfo(currentPeer)->netID);
												p_p.Insert("OnSetRoleSkinsAndIcons");
												p_p.Insert(pInfo(currentPeer)->roleSkin);
												p_p.Insert(pInfo(currentPeer)->roleIcon);
												p_p.Insert(0);
												p_p.CreatePacket(peer);
											}
										}
										if (pInfo(currentPeer)->netID != pInfo(peer)->netID and pInfo(currentPeer)->guild_id != 0) {
											uint32_t guild_id = pInfo(currentPeer)->guild_id;
											vector<Guild>::iterator find_guild = find_if(guilds.begin(), guilds.end(), [guild_id](const Guild& a) { return a.guild_id == guild_id; });
											if (find_guild != guilds.end()) {
												Guild* guild_information = &guilds[find_guild - guilds.begin()];
												uint32_t my_role = 0;
												const auto flag2 = (65536 * guild_information->guild_mascot[1] + guild_information->guild_mascot[0]);
												for (GuildMember member_search : guild_information->guild_members) {
													if (member_search.member_name == pInfo(currentPeer)->tankIDName) {
														my_role = member_search.role_id;
														break;
													}
												}
												gamepacket_t p(0, pInfo(peer)->netID);
												p.Insert("OnGuildDataChanged");
												p.Insert(50478);
												p.Insert(79289404);
												p.Insert(flag2);
												p.Insert(my_guild_role);
												p.Insert(0);
												p.CreatePacket(peer);
											}
										}
									}
								}
								long long ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
								map<string, vector<WorldNPC>>::iterator it;
								for (it = active_npc.begin(); it != active_npc.end(); it++) {
									if (it->first == world_->name) {
										for (int i_ = 0; i_ < it->second.size(); i_++) {
											try {
												WorldNPC npc_ = it->second[i_];
												if (npc_.uid == -1) continue;
												double per_sekunde_praeina_bloku = (double)npc_.projectile_speed / 32;
												double praejo_laiko = (double)(ms_time - npc_.started_moving) / 1000;
												double praejo_distancija = (double)per_sekunde_praeina_bloku * (double)praejo_laiko;
												double current_x = ((int)npc_.kryptis == 180 ? (((double)npc_.x - (double)praejo_distancija) * 32) + 16 : (((double)npc_.x + (double)praejo_distancija) * 32) + 16);
												double current_y = (double)npc_.y * 32;
												bool blocked_ = false;
												if ((int)npc_.kryptis == 180) { // check if it wasnt blocked
													vector<int> new_tiles{};
													if (items[world_->blocks[(int)(current_x / 32) + ((int)(current_y / 32) * 100)].fg].collisionType != 1) {
														new_tiles.push_back((int)(current_x / 32) + ((int)(current_y / 32) * 100));
													} int ySize = world_->blocks.size() / 100, xSize = world_->blocks.size() / ySize;
													vector<WorldBlock> shadow_copy = world_->blocks;
													for (int i2 = 0; i2 < new_tiles.size(); i2++) {
														int x_ = new_tiles[i2] % 100, y_ = new_tiles[i2] / 100;
														if (x_ < 99 and items[shadow_copy[x_ + 1 + (y_ * 100)].fg].collisionType != 1) {
															if (not shadow_copy[x_ + 1 + (y_ * 100)].scanned) {
																shadow_copy[x_ + 1 + (y_ * 100)].scanned = true;
																new_tiles.push_back(x_ + 1 + (y_ * 100));
															}
														}
														else if (items[shadow_copy[x_ + 1 + (y_ * 100)].fg].collisionType == 1 and x_ < npc_.x) {
															blocked_ = true;
															break;
														}
													}
												}
												else {
													vector<int> new_tiles{};
													if (items[world_->blocks[(int)(current_x / 32) + ((int)(current_y / 32) * 100)].fg].collisionType != 1) {
														new_tiles.push_back((int)(current_x / 32) + ((int)(current_y / 32) * 100));
													} int ySize = world_->blocks.size() / 100, xSize = world_->blocks.size() / ySize;
													vector<WorldBlock> shadow_copy = world_->blocks;
													for (int i2 = 0; i2 < new_tiles.size(); i2++) {
														int x_ = new_tiles[i2] % 100, y_ = new_tiles[i2] / 100;
														if (x_ < 99 and items[shadow_copy[x_ - 1 + (y_ * 100)].fg].collisionType != 1) {
															if (not shadow_copy[x_ - 1 + (y_ * 100)].scanned) {
																shadow_copy[x_ - 1 + (y_ * 100)].scanned = true;
																new_tiles.push_back(x_ - 1 + (y_ * 100));
															}
														}
														else if (items[shadow_copy[x_ - 1 + (y_ * 100)].fg].collisionType == 1 and x_ > npc_.x) {
															blocked_ = true;
															break;
														}
													}
												} if (blocked_) {
													continue;
												}
												PlayerMoving data_{};
												data_.packetType = 34;
												data_.x = (current_x + 16); //nuo x
												data_.y = (current_y + (npc_.id == 8020 ? 6 : 16)); //nuo y
												data_.XSpeed = (current_x + 16); // iki x
												data_.YSpeed = (current_y + (npc_.id == 8020 ? 6 : 16)); // iki y
												data_.punchY = npc_.projectile_speed;
												BYTE* raw = packPlayerMoving(&data_);
												raw[1] = (npc_.id == 8020 ? 15 : 8), raw[2] = npc_.uid, raw[3] = 2;
												memcpy(raw + 40, &npc_.kryptis, 4);
												send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
												delete[] raw;
											}
											catch (out_of_range) {
												continue;
											}
										}
										break;
									}
								}
							}

							pInfo(peer)->x = (int)p_->x, pInfo(peer)->y = (int)p_->y, pInfo(peer)->state = p_->characterState & 0x10;
						}
						break;
					}
					case 3: /* tempat player wrench atau hit block */
					{
						if (p_->plantingTree <= 0 || p_->plantingTree >= items.size()) break;
						//if (items[p_->plantingTree].blocked_place == 1) break; // crash block
						if (p_->plantingTree != 18 and p_->plantingTree != 32) {
							int c_ = 0;
							modify_inventory(peer, p_->plantingTree, c_);
							if (c_ == 0) break;
						}
						if (p_->plantingTree == 18) {
							if (pInfo(peer)->punch_time + 100 > (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) break;
							pInfo(peer)->punch_time = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							if (has_playmod(pInfo(peer), "Infected!") or pInfo(peer)->hand != 0) pInfo(peer)->last_infected = p_->punchX + (p_->punchY * 100);
						}
						if (pInfo(peer)->trading_with != -1 and p_->packetType != 0 and p_->packetType != 18) {
							cancel_trade(peer, false, true);
							break;
						}
						// EDITING FAR BLOCK BREAK
						if (p_->plantingTree == 18 and has_playmod(pInfo(peer), "rayman's fist")) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[p_->punchX + (p_->punchY * 100)];
								if (block_->fg == 0 and block_->bg == 0) break;
							}
							if (p_->punchY == pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY, p_->plantingTree);
								}
							}
							else if (p_->punchX == pInfo(peer)->x / 32) {
								if (p_->punchY > pInfo(peer)->y / 32) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY + 2, p_->plantingTree);
								}
								else if (p_->punchY < pInfo(peer)->y / 32) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
								}
							}
							else if (p_->punchY < pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY - 2, p_->plantingTree);
								}
							}
							else if (p_->punchY < pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY - 2, p_->plantingTree);
								}
							}
							else if (p_->punchY > pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY + 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY + 2, p_->plantingTree);
								}
							}
						}
						else if (p_->plantingTree == 18 and has_playmod(pInfo(peer), "legendary fist")) {
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								WorldBlock* block_ = &world_->blocks[p_->punchX + (p_->punchY * 100)];
								if (block_->fg == 0 and block_->bg == 0) break;
							}
							if (p_->punchY == pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY, p_->plantingTree);
								}
							}
							else if (p_->punchX == pInfo(peer)->x / 32) {
								if (p_->punchY > pInfo(peer)->y / 32) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY + 2, p_->plantingTree);
								}
								else if (p_->punchY < pInfo(peer)->y / 32) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
								}
							}
							else if (p_->punchY < pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY - 2, p_->plantingTree);
								}
							}
							else if (p_->punchY < pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY - 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY - 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY - 2, p_->plantingTree);
								}
							}
							else if (p_->punchY > pInfo(peer)->y / 32) {
								if (pInfo(peer)->state == 16) {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX - 1, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX - 2, p_->punchY + 2, p_->plantingTree);
								}
								else {
									edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
									edit_tile(peer, p_->punchX + 1, p_->punchY + 1, p_->plantingTree);
									edit_tile(peer, p_->punchX + 2, p_->punchY + 2, p_->plantingTree);
								}
							}
						}
						else {
							bool empty = false;
							if (p_->plantingTree == 5640) {
								string name_ = pInfo(peer)->world;
								vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
								if (p != worlds.end()) {
									World* world_ = &worlds[p - worlds.begin()];
									for (int i_ = 0; i_ < world_->machines.size(); i_++) {
										WorldMachines machine = world_->machines[i_];
										if (machine.x == pInfo(peer)->magnetron_x and machine.y == pInfo(peer)->magnetron_y and machine.id == 5638) {
											if (machine.enabled) {
												WorldBlock* itemas = &world_->blocks[machine.x + (machine.y * 100)];
												if (itemas->magnetron and itemas->id == pInfo(peer)->magnetron_id) {
													if (itemas->pr > 0) {
														p_->plantingTree = itemas->id;
														if (edit_tile(peer, p_->punchX, p_->punchY, itemas->id, true)) {
															itemas->pr--;
															if (itemas->pr <= 0) {
																PlayerMoving data_{};
																data_.packetType = 5, data_.punchX = machine.x, data_.punchY = machine.y, data_.characterState = 0x8;
																BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, itemas));
																BYTE* blc = raw + 56;
																form_visual(blc, *itemas, *world_, NULL, false);
																for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
																	if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
																	if (pInfo(currentPeer)->world == world_->name) {
																		send_raw(currentPeer, 4, raw, 112 + alloc_(world_, itemas), ENET_PACKET_FLAG_RELIABLE);
																	}
																}
																delete[] raw, blc;
															}
															break;
														}
													}
													else {
														empty = true;
														gamepacket_t p;
														p.Insert("OnTalkBubble");
														p.Insert(pInfo(peer)->netID);
														p.Insert("The `2" + items[machine.id].name + "`` is empty!");
														p.Insert(0), p.Insert(0);
														p.CreatePacket(peer);
													}
												}
											}
											break;
										}
									}
								} if (p_->plantingTree == 5640 and not empty) {
									gamepacket_t p;
									p.Insert("OnTalkBubble");
									p.Insert(pInfo(peer)->netID);
									p.Insert("There is no active `2" + items[5638].name + "``!");
									p.Insert(0), p.Insert(0);
									p.CreatePacket(peer);
								}
								break;
							}
							edit_tile(peer, p_->punchX, p_->punchY, p_->plantingTree);
						}
						break;
					}
					case 7: /*Kai zaidejas ieina pro duris arba portal*/ /*2/16/2023 update: cia dar gali buti STEAM USE*/
					{
						string name_ = pInfo(peer)->world;
						vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
						if (p != worlds.end()) {
							World* world_ = &worlds[p - worlds.begin()];
							//try {
							if (p_->punchX < 0 or p_->punchX >= 100 or p_->punchY < 0 or p_->punchY >= 60) return false;
							WorldBlock* block_ = &world_->blocks[p_->punchX + (p_->punchY * 100)];
							bool impossible = ar_turi_noclipaa(world_, pInfo(peer)->x, pInfo(peer)->y, block_, peer);
							if (impossible) break;
							if (items[items[block_->fg ? block_->fg : block_->bg].id].blockType == BlockTypes::CHECKPOINT) {
								pInfo(peer)->c_x = p_->punchX, pInfo(peer)->c_y = p_->punchY;
								gamepacket_t p(0, pInfo(peer)->netID);
								p.Insert("SetRespawnPos");
								p.Insert(pInfo(peer)->c_x + (pInfo(peer)->c_y * 100));
								p.CreatePacket(peer);
							}
							else if (items[block_->fg ? block_->fg : block_->bg].id == 6) exit_(peer);
							else if (block_->fg == 4722 && pInfo(peer)->adventure_begins == false) {
								pInfo(peer)->adventure_begins = true;
								gamepacket_t p(0);
								p.Insert("OnAddNotification"), p.Insert("interface/large/adventure.rttex"), p.Insert(block_->heart_monitor), p.Insert("audio/gong.wav"), p.Insert(0), p.CreatePacket(peer);
							}
							else if (items[block_->fg].blockType == BlockTypes::DOOR or items[block_->fg].blockType == BlockTypes::PORTAL) {
								string door_target = block_->door_destination, door_id = "";
								World target_world = worlds[p - worlds.begin()];
								bool locked = (block_->open ? false : (target_world.owner_name == pInfo(peer)->tankIDName or pInfo(peer)->dev or target_world.open_to_public or target_world.owner_name.empty() or (guild_access(peer, target_world.guild_id) or find(target_world.admins.begin(), target_world.admins.end(), pInfo(peer)->tankIDName) != target_world.admins.end()) ? false : true));
								int spawn_x = 0, spawn_y = 0;
								if (not locked && block_->fg != 762) {
									if (door_target.find(":") != string::npos) {
										vector<string> detales = explode(":", door_target);
										door_target = detales[0], door_id = detales[1];
									} if (not door_target.empty() and door_target != world_->name) {
										if (not check_name(door_target)) {
											gamepacket_t p(250, pInfo(peer)->netID);
											p.Insert("OnSetFreezeState");
											p.Insert(1);
											p.CreatePacket(peer);
											{
												gamepacket_t p(250);
												p.Insert("OnConsoleMessage");
												p.Insert(door_target);
												p.CreatePacket(peer);
											}
											{
												gamepacket_t p(250);
												p.Insert("OnZoomCamera");
												p.Insert((float)10000.000000);
												p.Insert(1000);
												p.CreatePacket(peer);
											}
											{
												gamepacket_t p(250, pInfo(peer)->netID);
												p.Insert("OnSetFreezeState");
												p.Insert(0);
												p.CreatePacket(peer);
											}
											break;
										}
										target_world = get_world(door_target);
									}
									int ySize = (int)target_world.blocks.size() / 100, xSize = (int)target_world.blocks.size() / ySize, square = (int)target_world.blocks.size();
									if (not door_id.empty()) {
										for (int i_ = 0; i_ < target_world.blocks.size(); i_++) {
											WorldBlock block_data = target_world.blocks[i_];
											if (block_data.fg == 1684 or items[block_data.fg].blockType == BlockTypes::DOOR or items[block_data.fg].blockType == BlockTypes::PORTAL) {
												if (block_data.door_id == door_id) {
													spawn_x = i_ % xSize, spawn_y = i_ / xSize;
													break;
												}
											}
										}
									}
								}
								if (block_->fg == 762) {
									pInfo(peer)->lastwrenchx = p_->punchX, pInfo(peer)->lastwrenchy = p_->punchY;
									gamepacket_t p2;
									if (block_->door_id == "") p2.Insert("OnTalkBubble"), p2.Insert(pInfo(peer)->netID), p2.Insert("No password has been set yet!"), p2.Insert(0), p2.Insert(1);
									else p2.Insert("OnDialogRequest"), p2.Insert("set_default_color|`o\nadd_label_with_icon|big|`wPassword Door``|left|762|\nadd_textbox|The door requires a password.|left|\nadd_text_input|password|Password||24|\nend_dialog|password_reply|Cancel|OK|");
									p2.CreatePacket(peer);
									gamepacket_t p(250, pInfo(peer)->netID), p3(250), p4(250, pInfo(peer)->netID);
									p.Insert("OnSetFreezeState"), p.Insert(1), p.CreatePacket(peer);
									p3.Insert("OnZoomCamera"), p3.Insert((float)10000.000000), p3.Insert(1000), p3.CreatePacket(peer);
									p4.Insert("OnSetFreezeState"), p4.Insert(0), p4.CreatePacket(peer);
								}
								if (block_->fg != 762) join_world(peer, target_world.name, spawn_x, spawn_y, 250, locked, true);
							}
							else {
								switch (block_->fg) {
								case 3270: case 3496:
								{
									Position2D steam_connector = track_steam(world_, block_, p_->punchX, p_->punchY);
									if (steam_connector.x != -1 and steam_connector.y != -1) {
										WorldBlock* block_s = &world_->blocks[steam_connector.x + (steam_connector.y * 100)];
										switch (block_s->fg) {
										case 3286: //steam door
										{
											block_s->flags = (block_s->flags & 0x00400000 ? block_s->flags ^ 0x00400000 : block_s->flags | 0x00400000);
											PlayerMoving data_{};
											data_.packetType = 5, data_.punchX = steam_connector.x, data_.punchY = steam_connector.y, data_.characterState = 0x8;
											BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_s));
											BYTE* blc = raw + 56;
											form_visual(blc, *block_s, *world_, peer, false);
											for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
												if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
												if (pInfo(currentPeer)->world == world_->name) {
													send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_s), ENET_PACKET_FLAG_RELIABLE);
												}
											}
											delete[] raw, blc;
											break;
										}
										case 3724: // spirit storage unit
										{
											uint32_t scenario = 20;
											{
												// check for ghost jars
												for (int i = 0; i < world_->drop.size(); i++) {
													WorldDrop* check_drop = &world_->drop[i];
													Position2D dropped_at{ check_drop->x / 32, check_drop->y / 32 };
													if (dropped_at.x == steam_connector.x and dropped_at.y == steam_connector.y) {
														if (check_drop->id == 3722) {
															uint32_t explo_chance = check_drop->count;
															// remove drop
															{
																PlayerMoving data_{};
																data_.packetType = 14, data_.netID = -2, data_.plantingTree = check_drop->uid;
																BYTE* raw = packPlayerMoving(&data_);
																int32_t item = -1;
																memcpy(raw + 8, &item, 4);
																for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
																	if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
																	if (pInfo(currentPeer)->world == name_) {
																		send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
																	}
																}
																world_->drop[i].id = 0, world_->drop[i].x = -1, world_->drop[i].y = -1;
																delete[] raw;
															}
															block_s->c_ += explo_chance;
															// explode or not
															{
																if (block_s->c_ * 5 >= 105) {
																	float explosion_chance = (float)((block_s->c_ * 5) - 100) * 0.5;
																	if (explosion_chance > rand() % 100) {
																		//bam bam
																		block_s->fg = 3726;
																		// drop the prize
																		{
																			vector<int> all_p{ 3734, 3732, 3748, 3712, 3706, 3708, 3718, 11136, 3728, 10056, 3730, 3788, 3750, 3738, 6060, 3738, 6840, 3736, 3750 };
																			uint32_t prize = 0;
																			if (block_s->c_ * 5 <= 115) prize = 3734;
																			else if (block_s->c_ * 5 <= 130) prize = 3732;
																			else if (block_s->c_ * 5 <= 140) prize = 3748;
																			else if (block_s->c_ * 5 <= 170) {
																				vector<int> p_drops = {
																					3712, 3706, 3708, 3718, 11136
																				};
																				prize = p_drops[rand() % p_drops.size()];
																			}
																			else if (block_s->c_ * 5 <= 190)  prize = 3728;
																			else if (block_s->c_ * 5 <= 205)  prize = 10056;
																			else if (block_s->c_ * 5 <= 220)  prize = 3730;
																			else if (block_s->c_ * 5 == 225)  prize = 3788;
																			else if (block_s->c_ * 5 <= 240)  prize = 3750;
																			else if (block_s->c_ * 5 == 245)  prize = 3738;
																			else if (block_s->c_ * 5 <= 255)  prize = 6060;
																			else if (block_s->c_ * 5 <= 265 or explo_chance * 5 >= 265) {
																				if (explo_chance * 5 >= 265) prize = all_p[rand() % all_p.size()];
																				else prize = 3738;
																			}
																			else {
																				vector<int> p_drops = {
																					6840
																				};
																				if (block_s->c_ * 5 >= 270) p_drops.push_back(3736);
																				if (block_s->c_ * 5 >= 295) p_drops.push_back(3750);
																				prize = p_drops[rand() % p_drops.size()];
																			} if (prize != 0) {
																				WorldDrop drop_block_{};
																				drop_block_.x = steam_connector.x * 32 + rand() % 17;
																				drop_block_.y = steam_connector.y * 32 + rand() % 17;
																				drop_block_.id = prize, drop_block_.count = 1, drop_block_.uid = uint16_t(world_->drop.size()) + 1;
																				dropas_(world_, drop_block_);
																				{
																					PlayerMoving data_{};
																					data_.packetType = 0x11, data_.x = steam_connector.x * 32 + 16, data_.y = steam_connector.y * 32 + 16;
																					data_.YSpeed = 97, data_.XSpeed = 3724;
																					BYTE* raw = packPlayerMoving(&data_);
																					PlayerMoving data_2{};
																					data_2.packetType = 0x11, data_2.x = steam_connector.x * 32 + 16, data_2.y = steam_connector.y * 32 + 16;
																					data_2.YSpeed = 108;
																					BYTE* raw2 = packPlayerMoving(&data_2);
																					gamepacket_t p;
																					p.Insert("OnConsoleMessage");
																					p.Insert("`#[A `9Spirit Storage Unit`` exploded, bringing forth an `9" + items[prize].name + "`` from The Other Side!]``");
																					for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
																						if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
																						if (pInfo(currentPeer)->world == world_->name) {
																							p.CreatePacket(currentPeer);
																							send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
																							send_raw(currentPeer, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
																						}
																					}
																					delete[] raw, raw2;
																				}
																				scenario = 22;
																			}
																		}
																		block_s->c_ = 0;
																	}
																}
															}
															// update visuals
															{
																PlayerMoving data_{};
																data_.packetType = 5, data_.punchX = steam_connector.x, data_.punchY = steam_connector.y, data_.characterState = 0x8;
																BYTE* raw = packPlayerMoving(&data_, 112 + alloc_(world_, block_s));
																BYTE* blc = raw + 56;
																form_visual(blc, *block_s, *world_, peer, false);
																for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
																	if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
																	if (pInfo(currentPeer)->world == world_->name) {
																		send_raw(currentPeer, 4, raw, 112 + alloc_(world_, block_s), ENET_PACKET_FLAG_RELIABLE);
																	}
																}
																delete[] raw, blc;
															}
															break;
														}
													}
												}
											}
											PlayerMoving data_{};
											data_.packetType = 32; // steam update paketas
											data_.punchX = steam_connector.x;
											data_.punchY = steam_connector.y;
											BYTE* raw = packPlayerMoving(&data_);
											raw[3] = scenario;
											for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
												if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
												if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
												send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[] raw;
											break;
										}
										default:
											break;
										}
									}
									PlayerMoving data_{};
									data_.packetType = 32; // steam update paketas
									data_.punchX = p_->punchX;
									data_.punchY = p_->punchY;
									BYTE* raw = packPlayerMoving(&data_);
									for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
										if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
										if (pInfo(peer)->world != pInfo(currentPeer)->world) continue;
										send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
									}
									delete[] raw;
									break;
								}
								default:
									break;
								}
							}
						}
						//catch (out_of_range& klaida) {
							//cout << "case 7 klaida -> " << klaida.what() << endl;
						//}
					//}
						break;
					}
					case 10: /*Kai zaidejas paspaudzia du kartus ant inventory itemo*/
					{
						if (pInfo(peer)->trading_with != -1) {
							cancel_trade(peer, false);
							break;
						}
						if (p_->plantingTree <= 0 or p_->plantingTree >= items.size()) break;
						int c_ = 0;
						modify_inventory(peer, p_->plantingTree, c_);
						if (c_ == 0) break;
						if (items[p_->plantingTree].blockType != BlockTypes::CLOTHING) {
							int free = get_free_slots(pInfo(peer)), slot = 1;
							int c242 = 242, c1796 = 1796, c6802 = 6802, c1486 = 1486, countofused = 0, getdl = 1, getwl = 100, removewl = -100, removedl = -1, countwl = 0, c4450 = 4450, c4452 = 4452;
							int c4298 = 4298, c4300 = 4300;
							int c7188 = 7188;
							modify_inventory(peer, p_->plantingTree, countofused);
							if (free >= slot) {
								if (p_->plantingTree == 242 || p_->plantingTree == 1796) {
									modify_inventory(peer, p_->plantingTree == 242 ? c1796 : c242, countwl);
									if (p_->plantingTree == 242 ? countwl <= 199 : countwl <= 100) {
										if (p_->plantingTree == 242 ? countofused >= 100 : countofused >= 1) {
											modify_inventory(peer, p_->plantingTree == 242 ? c242 : c1796, p_->plantingTree == 242 ? removewl : removedl);
											modify_inventory(peer, p_->plantingTree == 242 ? c1796 : c242, p_->plantingTree == 242 ? getdl : getwl);
											gamepacket_t p, p2;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(p_->plantingTree == 242 ? "You compressed 100 `2World Lock`` into a `2Diamond Lock``!" : "You shattered a `2Diamond Lock`` into 100 `2World Lock``!"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert(p_->plantingTree == 242 ? "You compressed 100 `2World Lock`` into a `2Diamond Lock``!" : "You shattered a `2Diamond Lock`` into 100 `2World Lock``!"), p2.CreatePacket(peer);
										}
									}
								}
								else if (p_->plantingTree == 7188) {
									modify_inventory(peer, c1796, countwl);
									if (countwl <= 100) {
										if (countofused >= 1) {
											modify_inventory(peer, c7188, removedl);
											modify_inventory(peer, c1796, getwl);
											gamepacket_t p, p2;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert("You shattered a `2Blue Gem Lock`` into 100 `2Diamond Lock``!"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert("You shattered a `2Blue Gem Lock`` into 100 `2Diamond Lock``!"), p2.CreatePacket(peer);
										}
									}
								}
								else if (p_->plantingTree == 1486 || p_->plantingTree == 6802) {
									modify_inventory(peer, p_->plantingTree == 1486 ? c6802 : c1486, countwl);
									if (p_->plantingTree == 1486 ? countwl <= 199 : countwl <= 100) {
										if (p_->plantingTree == 1486 ? countofused >= 100 : countofused >= 1) {
											modify_inventory(peer, p_->plantingTree == 1486 ? c1486 : c6802, p_->plantingTree == 1486 ? removewl : removedl);
											modify_inventory(peer, p_->plantingTree == 1486 ? c6802 : c1486, p_->plantingTree == 1486 ? getdl : getwl);
											gamepacket_t p, p2;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(p_->plantingTree == 1486 ? "You compressed 100 `2Growtoken`` into a `2Mega Growtoken``!" : "You shattered a `2Mega Growtoken`` into 100 `2Growtoken``!"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert(p_->plantingTree == 1486 ? "You compressed 100 `2Growtoken`` into a `2Mega Growtoken``!" : "You shattered a `2Mega Growtoken`` into 100 `2Growtoken``!"), p2.CreatePacket(peer);
										}
									}
								}
								else if (p_->plantingTree == 4450 || p_->plantingTree == 4452) {
									modify_inventory(peer, p_->plantingTree == 4450 ? c4452 : c4450, countwl);
									if (p_->plantingTree == 4450 ? countwl <= 199 : countwl <= 100) {
										if (p_->plantingTree == 4450 ? countofused >= 100 : countofused >= 1) {
											modify_inventory(peer, p_->plantingTree == 4450 ? c4450 : c4452, p_->plantingTree == 4450 ? removewl : removedl);
											modify_inventory(peer, p_->plantingTree == 4450 ? c4452 : c4450, p_->plantingTree == 4450 ? getdl : getwl);
											gamepacket_t p, p2;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(p_->plantingTree == 4450 ? "You compressed 100 `2Zombie Brain`` into a `2Pile of Zombie Brains``!" : "You shattered a `2Pile of Zombie Brains`` into 100 `2Zombie Brain``!"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert(p_->plantingTree == 4450 ? "You compressed 100 `2Zombie Brain`` into a `2Pile of Zombie Brains``!" : "You shattered a `2Pile of Zombie Brains`` into 100 `2Zombie Brain``!"), p2.CreatePacket(peer);
										}
									}
								}
								else if (p_->plantingTree == 4298 || p_->plantingTree == 4300) {
									modify_inventory(peer, p_->plantingTree == 4298 ? c4300 : c4298, countwl);
									if (p_->plantingTree == 4298 ? countwl <= 199 : countwl <= 100) {
										if (p_->plantingTree == 4298 ? countofused >= 100 : countofused >= 1) {
											modify_inventory(peer, p_->plantingTree == 4298 ? c4298 : c4300, p_->plantingTree == 4298 ? removewl : removedl);
											modify_inventory(peer, p_->plantingTree == 4298 ? c4300 : c4298, p_->plantingTree == 4298 ? getdl : getwl);
											gamepacket_t p, p2;
											p.Insert("OnTalkBubble"), p.Insert(pInfo(peer)->netID), p.Insert(p_->plantingTree == 4298 ? "You compressed 100 `2Caduceus`` into a `2Golden Caduceus``!" : "You shattered a `2Golden Caduceus`` into 100 `2Caduceus``!"), p.Insert(0), p.Insert(1), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert(p_->plantingTree == 4298 ? "You compressed 100 `2Caduceus`` into a `2Golden Caduceus``!" : "You shattered a `2Golden Caduceus`` into 100 `2Caduceus``!"), p2.CreatePacket(peer);
										}
									}
								}
							}
							/*compress ir t.t*/
							break;
						}
						/*equip*/
						equip_clothes(peer, p_->plantingTree);
						break;
					}
					case 11: /*Kai zaidejas paema isdropinta itema*/
					{
						if (p_->x < 0 || p_->y < 0) break;

						int currentX = pInfo(peer)->x / 32;
						int currentY = pInfo(peer)->y / 32;
						int targetX = p_->x / 32;
						int targetY = p_->y / 32;

						if (abs(targetX - currentX) >= 2 || abs(targetY - currentY) >= 2)
						{
							if (currentX != 0 && currentY != 0)
							{
								break;
							}
						}

						bool displaybox = true;
						string name_ = pInfo(peer)->world;
						vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
						if (p != worlds.end()) {
							World* world_ = &worlds[p - worlds.begin()];
							for (int i_ = 0; i_ < world_->drop.size(); i_++) {
								if (world_->drop[i_].id == 0 or world_->drop[i_].x / 32 < 0 or world_->drop[i_].x / 32 > 99 or world_->drop[i_].y / 32 < 0 or world_->drop[i_].y / 32 > 59) continue;
								if (world_->drop[i_].uid == p_->plantingTree) {
									WorldBlock* block_ = &world_->blocks[world_->drop[i_].x / 32 + (world_->drop[i_].y / 32 * 100)];
									if (block_->fg == 1422 || block_->fg == 2488) displaybox = block_access(peer, world_, block_);
									if (abs((int)p_->x / 32 - world_->drop[i_].x / 32) > 1 || abs((int)p_->x - world_->drop[i_].x) >= 32 or abs((int)p_->y - world_->drop[i_].y) >= 32) displaybox = false;
									bool noclip = false;
									if (pInfo(peer)->superdev || pInfo(peer)->dev || world_->owner_name == pInfo(peer)->tankIDName || find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) != world_->admins.end()) {
										if (pInfo(peer)->superdev || world_->owner_name == pInfo(peer)->tankIDName || find(world_->admins.begin(), world_->admins.end(), pInfo(peer)->tankIDName) != world_->admins.end()) displaybox = true;
									}
									else {
										noclip = ar_turi_noclipa(world_, p_->x, p_->y, world_->drop[i_].x / 32 + (world_->drop[i_].y / 32 * 100), peer);
									}
									if (displaybox && noclip == false) {
										int c_ = world_->drop[i_].count;
										if (world_->drop[i_].id == world_->special_event_item && world_->special_event && world_->drop[i_].special) {
											world_->special_event_item_taken++;
											if (items[world_->special_event_item].event_total == world_->special_event_item_taken) {
												gamepacket_t p, p3;
												p.Insert("OnAddNotification"), p.Insert("interface/large/special_event.rttex"), p.Insert("`2" + items[world_->special_event_item].event_name + ":`` `oSuccess! " + (items[world_->special_event_item].event_total == 1 ? "`2" + pInfo(peer)->tankIDName + "`` found it!``" : "All items found!``") + ""), p.Insert("audio/cumbia_horns.wav"), p.Insert(0);
												p3.Insert("OnConsoleMessage"), p3.Insert("`2" + items[world_->special_event_item].event_name + ":`` `oSuccess!`` " + (items[world_->special_event_item].event_total == 1 ? "`2" + pInfo(peer)->tankIDName + "`` `ofound it!``" : "All items found!``") + "");
												for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
													if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
													if (items[world_->special_event_item].event_total != 1) {
														gamepacket_t p2;
														p2.Insert("OnConsoleMessage"), p2.Insert("`2" + items[world_->special_event_item].event_name + ":`` `0" + pInfo(peer)->tankIDName + "`` found a " + items[world_->special_event_item].name + "! (" + to_string(world_->special_event_item_taken) + "/" + to_string(items[world_->special_event_item].event_total) + ")``"), p2.CreatePacket(currentPeer);
													}
													p.CreatePacket(currentPeer);
													p3.CreatePacket(currentPeer);
												}
												world_->last_special_event = 0, world_->special_event_item = 0, world_->special_event_item_taken = 0, world_->special_event = false;
											}
											else {
												gamepacket_t p2;
												p2.Insert("OnConsoleMessage"), p2.Insert("`2" + items[world_->special_event_item].event_name + ":`` `0" + pInfo(peer)->tankIDName + "`` found a " + items[world_->special_event_item].name + "! (" + to_string(world_->special_event_item_taken) + "/" + to_string(items[world_->special_event_item].event_total) + ")``");
												for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
													if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
													p2.CreatePacket(currentPeer);
												}
											}
										}
										if (modify_inventory(peer, world_->drop[i_].id, c_, false, true) == 0 or world_->drop[i_].id == 112) {
											PlayerMoving data_{};
											data_.effect_flags_check = 1, data_.packetType = 14, data_.netID = pInfo(peer)->netID, data_.plantingTree = world_->drop[i_].uid;
											BYTE* raw = packPlayerMoving(&data_);
											if (world_->drop[i_].id == 112) pInfo(peer)->gems += c_;
											else {
												add_cctv(peer, "took", to_string(world_->drop[i_].count) + " " + items[world_->drop[i_].id].name);
												gamepacket_t p;
												p.Insert("OnConsoleMessage"), p.Insert("Collected `w" + to_string(world_->drop[i_].count) + "" + (items[world_->drop[i_].id].blockType == BlockTypes::FISH ? "lb." : "") + " " + items[world_->drop[i_].id].ori_name + "``." + (items[world_->drop[i_].id].rarity > 363 ? "" : " Rarity: `w" + to_string(items[world_->drop[i_].id].rarity) + "``") + ""), p.CreatePacket(peer);
											}
											for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
												if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
												send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
											}
											delete[]raw;
											world_->drop[i_].id = 0, world_->drop[i_].x = -1, world_->drop[i_].y = -1;
										}
										else {
											if (c_ < 200 and world_->drop[i_].count >(200 - c_)) {
												int b_ = 200 - c_;
												world_->drop[i_].count -= b_;
												if (modify_inventory(peer, world_->drop[i_].id, b_, false) == 0) {
													add_cctv(peer, "took", to_string(world_->drop[i_].count) + " " + items[world_->drop[i_].id].name);
													WorldDrop drop_{};
													drop_.id = world_->drop[i_].id, drop_.count = world_->drop[i_].count, drop_.uid = uint16_t(world_->drop.size()) + 1, drop_.x = world_->drop[i_].x, drop_.y = world_->drop[i_].y;
													world_->drop.push_back(drop_);
													gamepacket_t p;
													p.Insert("OnConsoleMessage");
													p.Insert("Collected `w" + to_string(200 - c_) + " " + items[world_->drop[i_].id].ori_name + "``." + (items[world_->drop[i_].id].rarity > 363 ? "" : " Rarity: `w" + to_string(items[world_->drop[i_].id].rarity) + "``") + "");
													PlayerMoving data_{};
													data_.packetType = 14, data_.netID = -1, data_.plantingTree = world_->drop[i_].id, data_.x = world_->drop[i_].x, data_.y = world_->drop[i_].y;
													int32_t item = -1;
													float val = world_->drop[i_].count;
													BYTE* raw = packPlayerMoving(&data_);
													data_.plantingTree = world_->drop[i_].id;
													memcpy(raw + 8, &item, 4);
													memcpy(raw + 16, &val, 4);
													val = 0;
													data_.netID = pInfo(peer)->netID;
													data_.plantingTree = world_->drop[i_].uid;
													data_.x = 0, data_.y = 0;
													BYTE* raw2 = packPlayerMoving(&data_);
													BYTE val2 = 0;
													memcpy(raw2 + 8, &item, 4);
													memcpy(raw2 + 16, &val, 4);
													memcpy(raw2 + 1, &val2, 1);
													world_->drop[i_].id = 0, world_->drop[i_].x = -1, world_->drop[i_].y = -1;
													for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
														if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(currentPeer)->world != name_) continue;
														send_raw(currentPeer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
														if (pInfo(currentPeer)->netID == pInfo(peer)->netID)
															p.CreatePacket(currentPeer);
														send_raw(currentPeer, 4, raw2, 56, ENET_PACKET_FLAG_RELIABLE);
													}
													delete[]raw, raw2;
												}
											}
										}
									}
								}
							}
						}
						break;
					}
					case 18: { //chat bubble kai raso
						move_(peer, p_);
						break;
					}
					case 23: /*Kai zaidejas papunchina kita*/
					{
						if (pInfo(peer)->last_inf + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->last_inf = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							string name_ = pInfo(peer)->world;
							vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
							if (p != worlds.end()) {
								World* world_ = &worlds[p - worlds.begin()];
								bool can_cancel = true;
								if (find(world_->active_jammers.begin(), world_->active_jammers.end(), 1276) != world_->active_jammers.end()) can_cancel = false;
								if (can_cancel) {
									if (pInfo(peer)->trading_with != -1 and p_->packetType != 0 and p_->packetType != 18) {
										cancel_trade(peer, false, true);
										break;
									}
								}
							}
						}
						/*
						if (pInfo(peer)->last_inf + 5000 < (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count()) {
							pInfo(peer)->last_inf = (duration_cast<milliseconds>(system_clock::now().time_since_epoch())).count();
							bool inf = false;
							for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
								if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL or pInfo(peer)->world != pInfo(currentPeer)->world or pInfo(peer)->netID == pInfo(currentPeer)->netID) continue;
								if (abs(pInfo(currentPeer)->last_infected - p_->plantingTree) <= 3) {
									if (has_playmod(pInfo(currentPeer), "Infected!") && not has_playmod(pInfo(peer), "Infected!") && inf == false) {
										if (has_playmod(pInfo(peer), "Antidote!")) {
											for (ENetPeer* currentPeer2 = server->peers; currentPeer2 < &server->peers[server->peerCount]; ++currentPeer2) {
												if (currentPeer2->state != ENET_PEER_STATE_CONNECTED or currentPeer2->data == NULL or pInfo(peer)->world != pInfo(currentPeer2)->world) continue;
												PlayerMoving data_{};
												data_.packetType = 19, data_.punchX = 782, data_.x = pInfo(peer)->x + 10, data_.y = pInfo(peer)->y + 16;
												int32_t to_netid = pInfo(peer)->netID;
												BYTE* raw = packPlayerMoving(&data_);
												raw[3] = 5;
												memcpy(raw + 8, &to_netid, 4);
												send_raw(currentPeer2, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
												delete[]raw;
											}
										}
										else {
											pInfo(currentPeer)->last_infected = 0;
											inf = true;
											gamepacket_t p, p2;
											p.Insert("OnAddNotification"), p.Insert("interface/large/infected.rttex"), p.Insert("`4You were infected by " + pInfo(currentPeer)->tankIDName + "!"), p.CreatePacket(peer);
											p2.Insert("OnConsoleMessage"), p2.Insert("You've been infected by the g-Virus. Punch others to infect them, too! Braiiiins... (`$Infected!`` mod added, `$1 mins`` left)"), p2.CreatePacket(peer);
											PlayMods give_playmod{};
											give_playmod.id = 28, give_playmod.time = time(nullptr) + 60;
											pInfo(peer)->playmods.push_back(give_playmod);
											update_clothes(peer);
										}
									}
									if (has_playmod(pInfo(peer), "Infected!") && not has_playmod(pInfo(currentPeer), "Infected!") && inf == false) {
										inf = true;
										SendRespawn(peer, 0, true);
										for (int i_ = 0; i_ < pInfo(peer)->playmods.size(); i_++) {
											if (pInfo(peer)->playmods[i_].id == 28) {
												pInfo(peer)->playmods[i_].time = 0;
												break;
											}
										}
										string name_ = pInfo(currentPeer)->world;
										vector<World>::iterator p = find_if(worlds.begin(), worlds.end(), [name_](const World& a) { return a.name == name_; });
										if (p != worlds.end()) {
											World* world_ = &worlds[p - worlds.begin()];
											WorldDrop drop_block_{};
											drop_block_.id = rand() % 100 < 50 ? 4450 : 4490, drop_block_.count = pInfo(currentPeer)->hand == 9500 ? 2 : 1, drop_block_.uid = uint16_t(world_->drop.size()) + 1, drop_block_.x = pInfo(peer)->x, drop_block_.y = pInfo(peer)->y;
											dropas_(world_, drop_block_);
										}
									}
								}
							}
						}*/
						break;
					}
					default:
					{
						break;
					}
					}
					break;
				}
				default:
					break;
				}
				enet_event_destroy(event);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				if (saving_) break;
				if (peer->data != NULL) {
					if (pInfo(peer)->trading_with != -1) cancel_trade(peer, false);
					if (not pInfo(peer)->world.empty()) exit_(peer, true);
					if (not pInfo(peer)->invalid_data) {
						save_player(pInfo(peer), (f_saving_ ? false : true));
					}
					if (f_saving_) pInfo(peer)->saved_on_close = true;
					if (not f_saving_) {
						enet_host_flush(server);
						delete peer->data;
						peer->data = NULL;
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
	return 0;
}