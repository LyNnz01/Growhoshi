#pragma once
struct Items {
	uint16_t id = 0, count = 0;
};
struct Friends {
	string name = "";
	bool mute = false, block_trade = false;
	long long last_seen = 0;
};
struct PlayMods {
	int id = 0;
	long long time = 0;
	string user = "";
};

struct Player {
	// Clashparkour
	int Time_Remaining = 0, Time_Remaining_1 = 0;
	bool At_Clashparkour = false, Reach_Tier = false;
	// Wolf
	bool wolf_world = false, end_wolf = false;
	// DNA Proccesor
	int DNAInput = 0;
	int IDDNA1 = 0;
	int IDDNA2 = 0;
	int IDDNA3 = 0;
	bool update = false;
	uint8_t sprite = 0, wild = 6, golem = 6;
	bool adventure_begins = false;
	uint8_t pure_shadow = 0, grow_air_ballon = 0;
	bool MKW = false, MKP = false; // Minokawa Save
	// Warning System
	int Warning = 0;
	vector<string> Warning_Message{};
	vector<string> WM{};
	// Halloween
	int Darking_Sacrifice = 0, Task_Darking = 0, Task_Dark_Ticket = 0, Task_Gift_Growganoth = 0, Task_Mountain = 0;
	int Tomb_X51 = 8000, Tomb_X42_1 = 8000, Tomb_X27 = 8000, Tomb_X71 = 8000, Tomb_X72 = 8000, Tomb_X42_2 = 8000;
	long long Tomb_Time = 0, Tomb_Finish = 0;
	bool TombofGrowganoth = false;
	//
	int eq_a = 0, eq_a_1 = 0;
	bool eq_a_update = false;
	bool CanDB = false;
	bool xenonite_effect = false;
	// Ancestral
	int AncesID = 0, Upgradeto = 0, HowmuchSoulStone = 0, IDCeles = 0, JumlahCeles = 0, IDCrystalized = 0, JumlahCrystalized = 0;
	int DailyRiddles = 0;
	// World Time
	long long World_Timed = 0;
	bool WorldTimed = false;
	// Eye Hair Color
	uint32_t hair_color = 0xFFFFFFFF, eye_drop = 0xFFFFFFFF, eye_lenses = 0xFFFFFFFF;
	// Rift Cape
	bool fixrw = false;
	bool Time_Change = true;
	int Cycle_Time = 30;
	// Cape 1
	int Cape_R_0 = 147, Cape_G_0 = 56, Cape_B_0 = 143, Collar_R_0 = 147, Collar_G_0 = 56, Collar_B_0 = 143;
	bool Cape_Collar_0 = true, Closed_Cape_0 = false, Open_On_Move_0 = true, Aura_0 = true, Aura_1st_0 = false, Aura_2nd_0 = false, Aura_3rd_0 = true;
	// Cape 2
	int Cape_R_1 = 137, Cape_G_1 = 30, Cape_B_1 = 43, Collar_R_1 = 34, Collar_G_1 = 35, Collar_B_1 = 63;
	bool Cape_Collar_1 = true, Closed_Cape_1 = true, Open_On_Move_1 = false, Aura_1 = true, Aura_1st_1 = false, Aura_2nd_1 = true, Aura_3rd_1 = false;
	// Total
	int Cape_Value = 1952541691, Cape_Collor_0_Value = 2402849791, Cape_Collar_0_Value = 2402849791, Cape_Collor_1_Value = 723421695, Cape_Collar_1_Value = 1059267327;
	// End
	// Rift Wings
	bool Wing_Time_Change = true;
	int Wing_Cycle_Time = 30;
	// Wings 1
	int Wing_R_0 = 93, Wing_G_0 = 22, Wing_B_0 = 200, Wing_Metal_R_0 = 220, Wing_Metal_G_0 = 72, Wing_Metal_B_0 = 255;
	bool Open_Wing_0 = false, Closed_Wing_0 = false, Trail_On_0 = true, Stamp_Particle_0 = true, Trail_1st_0 = true, Trail_2nd_0 = false, Trail_3rd_0 = false, Material_1st_0 = true, Material_2nd_0 = false, Material_3rd_0 = false;
	// Wings 2
	int Wing_R_1 = 137, Wing_G_1 = 30, Wing_B_1 = 43, Wing_Metal_R_1 = 34, Wing_Metal_G_1 = 35, Wing_Metal_B_1 = 65;
	bool Open_Wing_1 = false, Closed_Wing_1 = false, Trail_On_1 = true, Stamp_Particle_1 = false, Trail_1st_1 = false, Trail_2nd_1 = true, Trail_3rd_1 = false, Material_1st_1 = false, Material_2nd_1 = true, Material_3rd_1 = false;
	// Total
	int Wing_Value = 104912, Wing_Color_0_Value = 3356909055, Wing_Metal_0_Value = 4282965247, Wing_Color_1_Value = 723421695, Wing_Metal_1_Value = 1059267327;
	// End
	// Infinity Crown
	bool Crown_Time_Change = true;
	int Crown_Cycle_Time = 15;
	// Crown 1
	int Base_R_0 = 255, Base_G_0 = 200, Base_B_0 = 37;
	int Gem_R_0 = 255, Gem_G_0 = 0, Gem_B_0 = 64;
	int Crystal_R_0 = 26, Crystal_G_0 = 45, Crystal_B_0 = 140;
	bool Crown_Floating_Effect_0 = false, Crown_Laser_Beam_0 = true, Crown_Crystals_0 = true, Crown_Rays_0 = true;
	// Crown 2
	int Base_R_1 = 255, Base_G_1 = 255, Base_B_1 = 255;
	int Gem_R_1 = 255, Gem_G_1 = 0, Gem_B_1 = 255;
	int Crystal_R_1 = 0, Crystal_G_1 = 45, Crystal_B_1 = 140;
	bool Crown_Floating_Effect_1 = true, Crown_Laser_Beam_1 = true, Crown_Crystals_1 = true, Crown_Rays_1 = true;
	// Total
	int Crown_Value = 1768716607;
	long long int Crown_Value_0_0 = 4294967295, Crown_Value_0_1 = 4278255615, Crown_Value_0_2 = 4190961919;
	long long int Crown_Value_1_0 = 633929727, Crown_Value_1_1 = 1073807359, Crown_Value_1_2 = 2351766271;
	// End
	// Crown of Season
	int Aura_Season = 2, Trail_Season = 2;
	// End
	// Banner Bandolier
	int Banner_Item = 0, CBanner_Item = 0, Banner_Flag = 0, CBanner_Flag = 0;
	// Magic Magnet
	int Magnet_Item = 0;
	// End
	int Healt = 100;
	int PunchHowmuch = 0;
	bool DrDes = false;
	int lastshirt = 0;
	int lastpants = 0;
	int lastfeet = 0;
	int lastface = 0;
	int lasthand = 0;
	int lastback = 0;
	int lastmask = 0;
	int lastnecklace = 0;
	int lastances = 0;
	int lasthair = 0;

	bool Cursed = false, In_Betting = false, Already_Bet = false;
	long long Cursed_Time = 0;
	int Offer = 0, Gems = 0, MySpin = 0, Bet_What = 0; // 0 = gems 1 = locks

	// Lquest
	int quest_step = 1;
	int quest_progress = 0;
	bool quest_active = false;
	string lastquest = "";
	string choosing_quest = "";
	//
	bool While_Respawning = false;
	string ip = "", lo = ""; //real ip
	long long int last_option = 0;
	int option_open = 0;
	vector<string> last_visited_worlds{}, worlds_owned{}, trade_history;
	vector<map<int, int>> trade_items{};
	string name_color = "`0";
	int id = 0, netID = 0, state = 0, trading_with = -1;
	bool trade_accept = false, accept_the_offer = false;
	int x = -1, y = -1;
	int gems = 0;
	int usedlocke = 0;
	long long int xp = 0;
	bool usedmegaphone = 0;
	map<int, int> ac_{};
	bool hit1 = false;
	int skin = 0x8295C3FF;
	int punched = 0;
	bool b_logs = false;
	int enter_game = 0;
	int lock = 0;
	int flagmay = 256;
	int lockeitem = 0;
	bool ghost = false, invis = false;
	int n = 0; //new player passed save
	string note = "";
	int totaltopup = 0;
	int supp = 0, hs = 0;
	int mod = 0, dev = 0, m_h = 0;
	int wls = 0;
	int csn = -1;
	long long int s_t = 0;
	string s_r = "";
	long long int b_t = 0, b_s = 0; // ban seconds
	//int last_infected = 0;
	string b_r = "", b_b = ""; // ban reason & banned by
	string m_r = "", m_b = "";
	int hair = 0, shirt = 0, pants = 0, feet = 0, face = 0, hand = 0, back = 0, mask = 0, necklace = 0, ances = 0; /*clothes*/
	string tankIDName = "", tankIDPass = "", requestedName = "", world = "", email = "", country = "", home_world = "", last_wrenched = "";
	string d_name = "";
	vector<Items> inv{};
	vector<Friends> friends{};
	vector<int> last_friends_selection{};
	vector<string> pending_friends{};
	vector<string> bans{}, logs{};
	string last_edit = "";
	string growmoji = "(wl)|ā|0&(oops)|ą|0&(sleep)|Ċ|0&(punch)|ċ|0&(bheart)|Ĕ|0&(grow)|Ė|0&(gems)|ė|0&(gtoken)|ę|0&(cry)|ĝ|0&(vend)|Ğ|0&(bunny)|ě|0&(cactus)|ğ|0&(pine)|Ĥ|0&(peace)|ģ|0&(terror)|ġ|0&(troll)|Ġ|0&(evil)|Ģ|0&(fireworks)|Ħ|0&(football)|ĥ|0&(alien)|ħ|0&(party)|Ĩ|0&(pizza)|ĩ|0&(clap)|Ī|0&(song)|ī|0&(ghost)|Ĭ|0&(nuke)|ĭ|0&(halo)|Į|0&(turkey)|į|0&(gift)|İ|0&(cake)|ı|0&(heartarrow)|Ĳ|0&(lucky)|ĳ|0&(shamrock)|Ĵ|0&(grin)|ĵ|0&(ill)|Ķ|0&(eyes)|ķ|0&(weary)|ĸ|0&(moyai)|ļ|0&(plead)|Ľ|0&";

	int i_11818_1 = 0, i_11818_2 = 0;
	int8_t random_fossil = rand() % 3 + 4;
	bool Double_Jump = false, High_Jump = false, Fireproof = false, Punch_Power = false, Punch_Range = false, Speedy = false, Build_Range = false, Speedy_In_Water = false, Punch_Damage = false;
	int opc = 0;
	int cc = 0;
	bool bb = false;
	int vip = 0, rb = 0;
	int gp = 0, gd = 0, glo = 0;
	int w_w = 0, w_d = 0;
	int mds = 0;
	int offergems = 0;
	int confirm_reset = 0;
	bool show_location_ = true, show_friend_notifications_ = true, mlgeff = false;
	int level = 1, firesputout = 0, carnivalgameswon = 0;
	long long playtime = 0;
	long long int account_created = 0, seconds = 0;
	string rid = "", mac = "", meta = "", vid = "", platformid = "", wk = "", gameVersion = "";
	map<string, int> achievements{};
	string lastmsg = "", lastmsgworld = "";
	int gtwl = 0;
	int c_x = 0, c_y = 0;
	int lavaeffect = 0;
	int fires = 0;
	//cooldowns
	long long int i240 = 0, i756 = 0, i758 = 0;
	bool tmod = 0;
	//billboard
	int b_i = 0, b_a = 0, b_w = 0, b_p = 0;
	//
	int same_input = 0, not_same = 0;
	string last_input_text = "";
	long long int last_inf = 0, last_text_input = 0, last_spam_detection = 0, last_consumable = 0, last_world_enter = 0, last_kickall = 0, last_fish_catch = 0, respawn_time = 0, hand_torch = 0, punch_time = 0, lava_time = 0, world_time = 0, geiger_time = 0, valentine_time = 0, remind_time = 0, warp_time = 0, name_time = 0, address_change = 0;
	int dds = 0;
	int hack_ = 0;
	bool cancel_enter = false;
	int lastwrenchx = 0, lastwrenchy = 0, lastwrenchb = 0, lastchoosenitem = 0, lastchoosennr = 0;
	bool invalid_data = false;
	int pps = 0;
	unsigned int onlineID = 0;
	long long lastres = 0;
	int rescheat = 0;
	long long lpps = 0;
	long long int punch_count = 0;
	int received = 0;
	int geiger_ = 0;
	int fishing_used = 0, f_x = 0, f_y = 0, move_warning = 0, f_xy = 0, punch_warning = 0, fish_seconds = 0;
	vector<int> glo_p, lvl_p;
	int geiger_x = -1, geiger_y = -1;
	int t_xp = 0, bb_xp = 0, g_xp = 0, p_xp = 0, t_lvl = 0, bb_lvl = 0, g_lvl = 0, p_lvl = 0, ff_xp = 0, ff_lvl = 0, s_lvl = 0, s_xp = 0;
	vector<int> t_p, bb_p, p_p, g_p, ff_p, surg_p;
	bool saved_on_close = false;
	int booty_broken = 0;
	int dd = 0;
	bool AlreadyDailyQ = false;
	vector<PlayMods> playmods{};
	int b_l = 1;
	vector<pair<int, int>> bp;
	int choosenitem = 0;
	int promo = 0, flagset = 0;
	int radio = 0;
	int tk = 0;
	int b_ra = 0, b_lvl = 1;
	int magnetron_id = 0;
	int magnetron_x = 0;
	int magnetron_y = 0;
	vector<string> gr;
	long long int save_time = 0;
	vector<vector<long long>> completed_blarneys{
		{1, 0},
		{2, 0},
		{3, 0},
		{4, 0},
		{5, 0},
		{6, 0},
		{7, 0},
		{8, 0},
		{9, 0},
	};
	bool block_trade = false;
	int p_x = 0;
	int p_y = 0;
	int guild_id = 0;
	uint32_t pending_guild = 0;
	int is_legend = false;
	int legend = false;
	int pps2 = 0;
	long long lpps2 = 0;
	int pps23 = 0;
	long long lpps23 = 0;
	int superdev = 0;
	int exitwarn = 0;
	int last_exit = 0;
	int roleIcon = 6;
	int roleSkin = 6;

	vector<int> available_surg_items;
	// Stats variables
	bool sounded = false;
	bool labworked = false;
	bool fixed = false;
	bool fixable = false;
	bool flu = false;
	int pulse = 40;
	int site = 0;
	int sleep = 0;
	int dirt = 0;
	int broken = 0;
	int shattered = 0;
	int incisions = 0;
	int bleeding = 0;
	int incneeded = 0;
	int heart = 0;
	double temp = 98.6;
	double fever = 0.0;
	string pretext = "";
	string fixtext = "";
	string postext = "";
	string scantext = "";
	string tooltext = "The patient is prepped for surgery.";
	string endtext = "";
	int failchance = 0;
	bool s = true;
	bool antibs = false;

	int spongUsed = 0;
	int scalpUsed = 0;
	int stitcUsed = 0;
	int antibUsed = 0;
	int antisUsed = 0;
	int ultraUsed = 0;
	int labkiUsed = 0;
	int anestUsed = 0;
	int defibUsed = 0;
	int splinUsed = 0;
	int pinsUsed = 0;
	int clampUsed = 0;
	int transUsed = 0;

	int surgery_skill = 0;
	int surgery_done = 0;
	int su_8552_1 = 0;
	int su_8552_2 = 0;
	bool surgery_started = false;
	int started_type = 0;
	string surgery_world = "";
	string surged_person = "";
	string surged_display = "";
	int surgery_type = -1;
	string surgery_name = "";
	bool hospital_bed = false;
	int egg = 0;

	bool mercy = false;
	bool drtitle = false;
	bool drt = false;
	bool lvl125 = false;
	int donor = false;
	bool spotlight = false;
	int master = false;
	int have_master = false;
	int have_donor = false;
	/*
	int grow4good_day = 0;
	int grow4good_rarity = 0;
	int grow4good_total_rarity = 0;
	int grow4good_wl = 0;
	int grow4good_total_wl = 0;
	bool grow4good_donate_gems = false;
	int grow4good_gems = 0;
	bool grow4good_purchase_waving = false;
	int grow4good_surgery = 0;
	int grow4good_fish = 0;
	int grow4good_points = 0;
	int grow4good_claim_prize = 0;
	int grow4good_claimed_prize = 0;
	int grow4good_break = 0;
	int grow4good_place = 0;
	int grow4good_trade = 0;
	int grow4good_sb = 0;

	int donate_count = 0;
	*/
	int last_infected = 0;
	int pinata_day = 0;
	bool pinata_prize = false;
	bool pinata_claimed = false;
	bool honors = false;
	string game_version = "";
	double gver = 0.0;
	bool pc_player = false;
	bool bypass = false, update_player = false, bypass2 = false;
	int new_packets = 0;
	int ausd = 0;
	long long last_open_packet = 0;
	int packet_to_much = 0;
	bool in_enter_game = false;
	bool isIn = false;
	bool isUpdating = false;
	string uidtoken = "";
	string gid = "";
};
#define pInfo(peer) ((Player*)(peer->data))
struct PlayerMoving {
	int32_t netID, effect_flags_check;
	int packetType, characterState, plantingTree, punchX, punchY, secondnetID;
	float x, y, XSpeed, YSpeed;
};
struct FishMoving {
	int8_t packetType = 31, stopped_fishing;
	int32_t netID, x, y;
};

BYTE* packPlayerMoving(PlayerMoving* dataStruct, int size_ = 56) {
	BYTE* data = new BYTE[size_];
	memset(data, 0, size_);
	memcpy(data + 0, &dataStruct->packetType, 4);
	memcpy(data + 4, &dataStruct->netID, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 20, &dataStruct->plantingTree, 4);
	memcpy(data + 24, &dataStruct->x, 4);
	memcpy(data + 28, &dataStruct->y, 4);
	memcpy(data + 32, &dataStruct->XSpeed, 4);
	memcpy(data + 36, &dataStruct->YSpeed, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	return data;
}
BYTE* packFishMoving(FishMoving* dataStruct, int size_ = 56) {
	BYTE* data = new BYTE[size_];
	memset(data, 0, size_);
	memcpy(data + 0, &dataStruct->packetType, 4);
	memcpy(data + 3, &dataStruct->stopped_fishing, 4);
	memcpy(data + 4, &dataStruct->netID, 4);
	memcpy(data + 44, &dataStruct->x, 4);
	memcpy(data + 48, &dataStruct->y, 4);
	return data;
}
void send_raw(ENetPeer* peer, int a1, void* packetData, int packetDataSize, int packetFlag) {
	ENetPacket* p;
	if (peer) {
		if (a1 == 4 && *((BYTE*)packetData + 12) & 8) {
			p = enet_packet_create(0, packetDataSize + *((DWORD*)packetData + 13) + 5, packetFlag);
			int four = 4;
			memcpy(p->data, &four, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
			memcpy((char*)p->data + packetDataSize + 4, 0, *((DWORD*)packetData + 13));
		}
		else {
			p = enet_packet_create(0, packetDataSize + 5, packetFlag);
			memcpy(p->data, &a1, 4);
			memcpy((char*)p->data + 4, packetData, packetDataSize);
		}
		enet_peer_send(peer, 0, p);
	}
}
void send_inventory(ENetPeer* peer) {
	const int inv_size = (int)pInfo(peer)->inv.size(), packetLen = 66 + (inv_size * 4) + 4;
	BYTE* d_ = new BYTE[packetLen];
	int MessageType = 0x4, PacketType = 0x9, NetID = -1, CharState = 0x8;
	memset(d_, 0, packetLen);
	memcpy(d_, &MessageType, 1);
	memcpy(d_ + 4, &PacketType, 4);
	memcpy(d_ + 8, &NetID, 4);
	memcpy(d_ + 16, &CharState, 4);
	int endianInvVal = _byteswap_ulong(inv_size);
	memcpy(d_ + 66 - 4, &endianInvVal, 4);
	endianInvVal = _byteswap_ulong(inv_size - 1);
	memcpy(d_ + 66 - 8, &endianInvVal, 4);
	vector<vector<uint32_t>> send_later{}; // fix mod items invis
	int val = 0;
	for (int i_ = 0; i_ < inv_size; i_++) {
		uint16_t item_id = pInfo(peer)->inv[i_].id;
		uint16_t item_count = pInfo(peer)->inv[i_].count;
		if (items[item_id].properties & Property_Mod || item_id == 7782 || item_id == 9500 || item_id == 9490 || item_id == 9492 || item_id == 9490 || item_id == 9492 || item_id == 9516 || item_id == 9540) {
			send_later.push_back({ item_id, item_count });
		}
		else {
			int val = 0;
			val |= item_id; // cia id
			val |= item_count << 16; // cia count
			val &= 0x00FFFFFF;
			val |= 0x00 << 24;
			memcpy(d_ + (i_ * 4) + 66, &val, 4);
		}
	}
	ENetPacket* const p_ = enet_packet_create(d_, packetLen, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, p_);
	delete[] d_;
	for (vector<uint32_t> a_ : send_later) {
		uint32_t item_id = a_[0];
		uint32_t item_count = a_[1];
		if (item_id == 0 or item_count == 0) continue;
		PlayerMoving data_{};
		data_.packetType = 13, data_.plantingTree = item_id;
		BYTE* raw = packPlayerMoving(&data_);
		raw[3] = item_count;
		send_raw(peer, 4, raw, 56, ENET_PACKET_FLAG_RELIABLE);
		delete[]raw;
	}
}