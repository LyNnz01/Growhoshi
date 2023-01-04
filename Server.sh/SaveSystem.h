#pragma once
#include <iostream>
int if_freeze = 0;
int if_freeze_to_exit = 0;
string theCCH = "";
string theCCH3 = "";
string theCASE4 = "";
void crashLog() {
	ofstream a("database/server_logs/crash.txt", ios::app);
	a << "Last CCH: " + theCCH + "\nLast World: " + theCCH3 + "\nlast CASE 4: " + theCASE4 + "\n";
	a.close();
}
struct test {
	string name = "";
	int totals = 0;
};
bool issaving = false;
inline void Server_Saving() {
	while (true) {
		Sleep(30000);
		//Save_Winterfest_File();
		//Load_Winterfest_File();
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
			save_player(pInfo(currentPeer), false);
		}
		vector<pair<string, string>> Save;
		int Save_Size = 0;
		int saved_total = 0;
		for (int i = 0; i < static_cast<int>(worlds.size()); i++) {
			save_world(worlds[i].name, false);
			saved_total++;
			Save.push_back(make_pair(worlds[i].name, worlds[i].owner_name));
		}
		/*if (saved_total >= 800) {
			issaving = true;
			for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
				if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
				for (int i = 0; i < Save.size(); i++) {
					if (pInfo(currentPeer)->world == Save[i].first) {
						Save_Size++;
						break;
					}
				}
				Sleep(1);
			}
			for (int i = 0; i < Save.size(); i++) {
				if (Save_Size <= 0) {
					worlds.erase(
						remove_if(worlds.begin(), worlds.end(), [&](World const& w) {
							return w.name == Save[i].first;
							}),
						worlds.end());
				}
			}
		}*/
		//save_guilds();
		issaving = false;
		/*vector<test>ts;
		for (int i = 0; i < static_cast<int>(worlds.size()); i++) {
			string world_name = worlds[i].name;
			save_world(world_name, false);
			test r;
			r.name = world_name;
			r.totals = 0;
			ts.push_back(r);
			Save.push_back(make_pair(worlds[i].name, worlds[i].owner_name));
		}

		issaving = false;
		if (if_freeze == saved_total) { //Auto Close When Server Is Freezed
			if_freeze_to_exit++;
			if (if_freeze_to_exit > 2) {
				crashLog();
				exit(0);
			}
		}*/
		if_freeze = saved_total;
		time_t currentTime; struct tm* localTime; char buffer[80]; time(&currentTime); localTime = localtime(&currentTime);
		int Mon = localTime->tm_mon + 1; int Day = localTime->tm_mday; int Hour = localTime->tm_hour; int Min = localTime->tm_min; int Sec = localTime->tm_sec;
		//hoshi_db_logs("All Worlds Has Been Saved: " + saved_total);
		//hoshi_info("Server Succesfully Saved at: " + to_string(Hour) + ":" + to_string(Min) + " on " + to_string(Mon) + "/" + to_string(Day));
	}
}

void save_w(int e) {
	crashLog();
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
		save_player(pInfo(currentPeer), false);
	}
	for (int i = 0; i < static_cast<int>(worlds.size()); i++) {
		string world_name = worlds[i].name;
		save_world(world_name, false);
	}
	//save_guilds();;
	hoshi_warn("ENet Server Crashed: Hoshi Auto Saving System is now running");
	hoshi_db_logs("Saved All World and Players");
}
inline void RandomlyDailyRiddles(int ID, string Dialog) {
	ofstream IDRiddle("database/Ancestral/IDDailyRiddles.txt");
	IDRiddle << ID;
	IDRiddle.close();
	ofstream Riddle("database/Ancestral/DailyRiddles.txt");
	Riddle << Dialog;
	Riddle.close();
}

inline void Server_Reset() {
	while (true) {
		Sleep(2000);
		for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
			if (currentPeer->state != ENET_PEER_STATE_CONNECTED or currentPeer->data == NULL) continue;
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
				/*for (int i = 0, removec = 0; i < Special_Item.size(); i++) {
					if (pInfo(currentPeer)->inv[i_].id == Special_Item[i].first && pInfo(currentPeer)->tankIDName != Special_Item[i].second && !(find(Hoshi.begin(), Hoshi.end(), pInfo(currentPeer)->tankIDName) != Hoshi.end())) {
						removec = pInfo(currentPeer)->inv[i_].count * -1;
						modify_inventory(currentPeer, pInfo(currentPeer)->inv[i_].id, removec);
					}
				}*/
			}
		}
		time_t currentTime;
		time(&currentTime);
		const auto localTime = localtime(&currentTime);
		const auto Hour = localTime->tm_hour; const auto Min = localTime->tm_min; const auto Sec = localTime->tm_sec; const auto Year = localTime->tm_year + 1900; const auto Day = localTime->tm_mday; const auto Month = localTime->tm_mon + 1;
		string Already;
		ifstream fs("database/Ancestral/DailyR.txt");
		fs >> Already;
		fs.close();
		if (Hour >= 5 && Already == "True") {
			ofstream fss("database/Ancestral/DailyR.txt");
			fss << "False";
			fss.close();
		}
		else if (Hour == 4 && Already == "False") {
			daily_quest();
			int Random = rand() % 43 + 1;
			ofstream fss("database/Ancestral/DailyR.txt");
			fss << "True";
			fss.close();
			if (Random == 1) {
				RandomlyDailyRiddles(982, "I see you wish to acquire greatness. I'd never block your efforts.");
			}
			else if (Random == 2) {
				RandomlyDailyRiddles(598, "This is a part of a fantasy, but I wouldn't want to be around it when it comes to a close.");
			}
			else if (Random == 3) {
				RandomlyDailyRiddles(320, "1812 is over, sure, but there's a way to bring it back. I can go on, but you should know how by now.");
			}
			else if (Random == 4) {
				RandomlyDailyRiddles(756, "Bring me a bandit. I don't even need all of him.");
			}
			else if (Random == 5) {
				RandomlyDailyRiddles(824, "I could never tell you what I want, because I'm not a fan of spoilers, and I'd prefer to keep things cool.");
			}
			else if (Random == 6) {
				RandomlyDailyRiddles(456, "My paradise is lost, but perhaps you can bring me halfway there.");
			}
			else if (Random == 7) {
				RandomlyDailyRiddles(480, "If you're a record breaker, stay away from this.");
			}
			else if (Random == 8) {
				RandomlyDailyRiddles(784, "I'd like a huckleberry. Know where I can find one?");
			}
			else if (Random == 9) {
				RandomlyDailyRiddles(658, "If you shoot this, would it be redundant? At the very least, I'll know you're bored.");
			}
			else if (Random == 10) {
				RandomlyDailyRiddles(1430, "Everyone asks the gods for these. Just once, I'd like to get one.");
			}
			else if (Random == 11) {
				RandomlyDailyRiddles(114, "You can silence this, but it will never die.");
			}
			else if (Random == 12) {
				RandomlyDailyRiddles(596, "A store full of wonders, none of which are for sale, is exactly the mark I'd like to spot.");
			}
			else if (Random == 13) {
				RandomlyDailyRiddles(286, "I'd like to remove some underlings from my employ. Give me a place to put them.");
			}
			else if (Random == 14) {
				RandomlyDailyRiddles(1048, "Spark my memories of the open range.");
			}
			else if (Random == 15) {
				RandomlyDailyRiddles(992, "I'd like a bright idea, but remember that I'm a city girl at heart.");
			}
			else if (Random == 16) {
				RandomlyDailyRiddles(1046, "I'm undecided, but I think the cowboy life calls to me on this one.");
			}
			else if (Random == 17) {
				RandomlyDailyRiddles(220, "Block out a little time to tune this the right way and that's what you'll have.");
			}
			else if (Random == 18) {
				RandomlyDailyRiddles(64, "If you ever had to use the bathroom in a dungeon, theres a chance you will put this together.");
			}
			else if (Random == 19) {
				RandomlyDailyRiddles(666, "Don't make an assumption, presume, or suppose - just take this for what it is and igneore the distractions.");
			}
			else if (Random == 20) {
				RandomlyDailyRiddles(684, "There aren't any drinks to be found here, but if they were, they'd be good for your blood.");
			}
			else if (Random == 21) {
				RandomlyDailyRiddles(872, "A creature of roads and rubber. It is dangerous to go alone, but even moreso, where beasts such as this are concerned.");
			}
			else if (Random == 22) {
				RandomlyDailyRiddles(866, "Time to get low. Perhaps this choice will cheese you off, but I suggest you move along.");
			}
			else if (Random == 23) {
				RandomlyDailyRiddles(1044, "It's worth more than a nickel, has nothing to do with chickens, and up to eight of them are correct, which I find baffling.");
			}
			else if (Random == 24) {
				RandomlyDailyRiddles(786, "Sorry, I can't quite racal what I wanted for this one. Call it a sign of the times.");
			}
			else if (Random == 25) {
				RandomlyDailyRiddles(1420, "Don't blink.");
			}
			else if (Random == 26) {
				RandomlyDailyRiddles(970, "Ag! You woke me up. I dreamt of when I was a smaller, simpler goddess. Perhaps you could remind me of that time?");
			}
			else if (Random == 27) {
				RandomlyDailyRiddles(260, "Find me a shimmering thing of wonder - I don't care what, so long as it's ausome!");
			}
			else if (Random == 28) {
				RandomlyDailyRiddles(186, "Bessie's a good name for a cow. Put it together with the sea France, and you have the first step in getting me what I want...");
			}
			else if (Random == 29) {
				RandomlyDailyRiddles(780, "2623, you'll know what to do. But there are alternates.");
			}
			else if (Random == 30) {
				RandomlyDailyRiddles(298, "Actually, I can't think of anything. Therefore, the opposite must do.");
			}
			else if (Random == 31) {
				RandomlyDailyRiddles(926, "Steel yourself, for I grow weary (or is it hungry?) and must set this discussion aside.");
			}
			else if (Random == 32) {
				RandomlyDailyRiddles(688, "These are spooky. I know everybody has one, but getting it to me is the key.");
			}
			else if (Random == 33) {
				RandomlyDailyRiddles(1002, "Hercules never defeated one of these, though, flames are still involved.");
			}
			else if (Random == 34) {
				RandomlyDailyRiddles(1530, "Whenever I sea these, I remember to keep quiet.");
			}
			else if (Random == 35) {
				RandomlyDailyRiddles(194, "Get me my favorite topping, and remember - I'm a fun girl.");
			}
			else if (Random == 36) {
				RandomlyDailyRiddles(334, "What's your favorite color? I can't decide between pink and yellow, so you'll have to bring me both.");
			}
			else if (Random == 37) {
				RandomlyDailyRiddles(1896, "I have trouble remembering things, so get me something that never forgets.");
			}
			else if (Random == 38) {
				RandomlyDailyRiddles(436, "Get me something to paint-and don't bangle it up.");
			}
			else if (Random == 39) {
				RandomlyDailyRiddles(454, "I want something strange, deadly, and otherworldly - but nothing from Mercury, please.");
			}
			else if (Random == 40) {
				RandomlyDailyRiddles(988, "Take one down from its perch and bring it here - just make sure you don't sluice it!");
			}
			else if (Random == 41) {
				RandomlyDailyRiddles(1312, "What do Vikings, Lumberjacks and Shrubbery have in common?");
			}
			else if (Random == 42) {
				RandomlyDailyRiddles(382, "I hate to break it to you, but it's time to give me some space.");
			}
			else if (Random == 43) {
				RandomlyDailyRiddles(922, "Most of the things I want are a mystery, but this one is especially so.");
			}
			else if (Random == 44) {
				RandomlyDailyRiddles(664, "I had a plan to get something tasty, but it's full of holes..");
			}
		}
	}
}