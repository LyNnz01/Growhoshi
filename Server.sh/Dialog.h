#pragma once
#include <chrono>
#include <sstream>
#include <filesystem>
#include "proton/Variant.hpp"
#include "proton/Vector.hpp"

class DialogBuilder {
	string dialog = "set_default_color|";
public:
	string to_string() { return dialog; }

	DialogBuilder(string const& color = "`0") { dialog += color + "\n"; }

	DialogBuilder& raw(string const& dialogText) {
		dialog += dialogText;
		return *this;
	}
	DialogBuilder& set_custom_spacing(int const& x, int const& y) {
		dialog += format("\nset_custom_spacing|x:{};y:{}|", x, y);
		return *this;
	}
	DialogBuilder& add_break() {
		dialog += "\nadd_custom_break|";
		return *this;
	}
	DialogBuilder& add_item_picker(string const& name, string const& message) {
		dialog += format("\nadd_item_picker|{}|{}|{}|", name, message, "Choose an item from your inventory");
		return *this;
	}
	DialogBuilder& add_player_info(string const& name, int const& currentLevel, int const& currentExp, int const& expRequired) {
		dialog += format("\nadd_player_info|{}|{}|{}|{}|", name, currentLevel, currentExp, expRequired);
		return *this;
	}
	DialogBuilder& add_checkbox(bool const& checked, string const& name, string const& message) {
		dialog += format("\nadd_checkbox|{}|{}|{}|", name, message, static_cast<int>(checked));
		return *this;
	}
	DialogBuilder& add_selector_checkbox(int const& id, string const& name, initializer_list<string> messages, int const& indexChecked) {
		vector<string> buttons = messages;
		for (int i = 0; i < buttons.size(); i++) {
			dialog += format("\nadd_checkbox|{}_{}_{}|{}|{}|", name, i, id, buttons[i], static_cast<int>(i == indexChecked));
		}
		return *this;
	}
	DialogBuilder& add_smalltext(string const& message) {
		dialog += format("\nadd_smalltext|{}|", message);
		return *this;
	}
	DialogBuilder& end_list() {
		dialog += "\nadd_button_with_icon||END_LIST|noflags|0|0|";
		return *this;
	}
	DialogBuilder& add_dual_layer(bool const& big, bool const& iconLeft, int const& foreground, int const& background, double const& size, string message) {
		dialog += format("\nadd_dual_layer_icon_label|{}|{}|left|{}|{}|{}|{}|", big ? "big" : "small", message, background, foreground, size, static_cast<int>(!iconLeft));
		return *this;
	}
	DialogBuilder& add_text_input(int const& length, string const& name, string const& message, string const& defaultInput = "") {
		dialog += format("\nadd_text_input|{}|{}|{}|{}|", name, message, defaultInput, length);
		return *this;
	}
	DialogBuilder& add_seed_icon(string const& itemId) {
		dialog += format("\nadd_seed_color_icons|{}|", itemId);
		return *this;
	}
	DialogBuilder& add_static_icon_button(string const& name, int const& id, string const& message, string const& hoverNumber = "") {
		dialog += format("\nadd_button_with_icon|{}|{}|staticBlueFrame|{}|{}|", name, message, id, hoverNumber);
		return *this;
	}
	DialogBuilder& add_label_icon(bool const& big, int const& id, string const& message) {
		dialog += format("\nadd_label_with_icon|{}|{}|left|{}|", big ? "big" : "small", message, id);
		return *this;
	}
	DialogBuilder& add_icon_button(string const& btnName, string const& text, string const& option, int const& itemID, string const& unkVal = "") {
		dialog += format("\nadd_button_with_icon|{}|{}|{}|{}|{}|", btnName, text, option, itemID, unkVal);
		return *this;
	}
	DialogBuilder& add_kit_disabled_button(string const& btnName, string const& progress, int const& itemID) {
		dialog += format("\nadd_button_with_icon|{}|`4{}`|staticGreyFrame,no_padding_x,is_count_label,disabled|{}||", btnName, progress, itemID);
		return *this;
	}
	DialogBuilder& add_kit_claim_button(string const& btnName, string const& underText, int const& itemID) {
		dialog += format("\nadd_button_with_icon|{}|`2{}`|staticYellowFrame,no_padding_x,is_count_label|{}||", btnName, underText, itemID);
		return *this;
	}
	DialogBuilder& add_kit_claimed_button(string const& btnName, int const& itemID) {
		dialog += format("\nadd_button_with_icon|{}|`5CLAIMED`|staticBlueFrame,no_padding_x,is_count_label|{}||", btnName, itemID);
		return *this;
	}
	DialogBuilder& add_label_icon_button(bool const& big, string const& message, int const& id, string const& name) {
		dialog += format("\nadd_label_with_icon_button|{}|{}|left|{}|{}|", big ? "big" : "small", message, id, name);
		return *this;
	}
	DialogBuilder& add_spacer(bool const& big) {
		dialog += format("\nadd_spacer|{}|", big ? "big" : "small");
		return *this;
	}
	DialogBuilder& add_textbox(string const& message) {
		dialog += format("\nadd_textbox|{}|", message);
		return *this;
	}
	DialogBuilder& add_quick_exit() {
		dialog += "\nadd_quick_exit|";
		return *this;
	}
	DialogBuilder& start_custom_tabs() {
		dialog += "\nstart_custom_tabs|";
		return *this;
	}
	DialogBuilder& reset_placement_x() {
		dialog += "\nreset_placement_x|";
		return *this;
	}
	DialogBuilder& reset_placement_y() {
		dialog += "\nreset_placement_y|";
		return *this;
	}
	DialogBuilder& add_custom_margin(int const& x, int const& y) {
		dialog += format("\nadd_custom_margin|x:{};y:{}|", x, y);
		return *this;
	}
	DialogBuilder& add_player_picker(string const& name, string const& button) {
		dialog += format("\nadd_player_picker|{}|{}|", name, button);
		return *this;
	}
	DialogBuilder& add_input(int const& length, string const& name, string const& message, string const& defaultInput = "") {
		dialog += format("\nadd_text_input|{}|{}|{}|{}|", name, message, defaultInput, length);
		return *this;
	}
	DialogBuilder& end_dialog(string const& name, string const& cancel, string const& accept) {
		dialog += format("\nend_dialog|{}|{}|{}|", name, cancel, accept);
		return *this;
	}
	DialogBuilder& add_label(bool const& big, string const& message) {
		dialog += format("\nadd_label|{}|{}|left|", big ? "big" : "small", message);
		return *this;
	}
	DialogBuilder& add_button(string const& name, string const& button) {
		dialog += format("\nadd_button|{}|{}|noflags|0|0|", name, button);
		return *this;
	}
	DialogBuilder& add_small_font_button(string const& name, string const& button) {
		dialog += format("\nadd_small_font_button|{}|{}|noflags|0|0|", name, button);
		return *this;
	}
	DialogBuilder& add_disabled_button(string const& name, string const& button) {
		dialog += format("\nadd_button|{}|{}|off|0|0|", name, button);
		return *this;
	}
	DialogBuilder& add_small_font_disabled_button(string const& name, string const& button) {
		dialog += format("\nadd_small_font_button|{}|{}|off|0|0|", name, button);
		return *this;
	}
	DialogBuilder& add_custom_button(string const& name, string const& option) {
		/*
		* Some Example Option
		* (image:interface/large/btn_passtabs.rttex;image_size:228,92;frame:0,2;width:0.15;min_width:60)
		* (image:interface/large/gui_wrench_u_transform.rttex;image_size_x:400;image_size_y:260;width:0.19;)
		* (icon:13106;state:enabled;color:128,128,128,180;border:grey;preset:listitem;margin_rself:0,-0.5;anchor:barBG_0;top:-1.0;left:0.166667;display:inline_free;)
		* (image:interface/large/store_buttons/store_buttons37.rttex;image_size:256,160;frame:1,2;width:0.35;action:store;state:disabled;url:main/itemomonth2;)
		*/
		dialog += format("\nadd_custom_button|{}|{}|", name, option);
		return *this;
	}
	DialogBuilder& add_custom_label(string const& option1, string const& option2) {
		/*
		* Some Example Option
		* (add_custom_label|1|target:claim_p2p_2100;top:0.25;left:0.25;size:small)
		* (add_custom_label|`52100|target:claim_p2p_2100;top:0.62;)
		* (add_custom_label|BUY IN STORE|target:itemomonth2;top:0.85;left:0.5)
		*/
		dialog += format("\nadd_custom_label|{}|{}|", option1, option2);
		return *this;
	}
	DialogBuilder& add_custom_spacer(double const& x) {
		dialog += format("\nadd_custom_spacer|x:{}|", x);
		return *this;
	}
	DialogBuilder& add_custom_textbox(string const& text, string const& size) {
		dialog += format("\nadd_custom_textbox|{}|size:{}|", text, to_lower(size));
		return *this;
	}
	DialogBuilder& embed_data(bool const& pushFront, string const& embed, string const& data) {
		if (pushFront) dialog.insert(0, format("\nembed_data|{}|{}\n", embed, data));
		else dialog += format("\nembed_data|{}|{}", embed, data);
		return *this;
	}
	DialogBuilder& add_achieve_button(string const& achName, string const& achToGet, int const& achID, string const& unk) {
		dialog += format("\nadd_achieve_button|{}|{}|left|{}|{}|", achName, achToGet, achID, unk);
		return *this;
	}
};
class CommunityBuilder {
	string layout = "set_description_text|";
public:
	string to_string() { return layout; }
	CommunityBuilder(string const& description) { layout += format("{}|\n", description); }

	CommunityBuilder& raw(string const& text) {
		layout += text;
		return *this;
	}
	CommunityBuilder& set_default_color(string const& color = "`o") {
		layout += format("set_default_color|{}", color);
		return *this;
	}
	CommunityBuilder& embed_data(bool const& pushFront, string const& embed, string const& data) {
		if (pushFront) layout.insert(0, format("\nembed_data|{}|{}\n", embed, data));
		else layout += format("\nembed_data|{}|{}", embed, data);
		return *this;
	}
	CommunityBuilder& add_textbox(string const& text) {
		layout += format("\nadd_textbox|{}|left|", text);
		return *this;
	}
	CommunityBuilder& add_button(string const& button, string const& btnName) {
		layout += format("\nadd_community_button|{}|{}|noflags|0|0|", button, btnName);
		return *this;
	}
	CommunityBuilder& add_ft_worldBtn(string const& worldName, string const& ownerName) {
		layout += format("\nadd_cmmnty_ft_wrld_bttn||`1{}`` by `5{}``|noflags|OPENWORLD|{}|0|0|", worldName, ownerName, worldName);
		return *this;
	}
	CommunityBuilder& add_wotd_worldBtn(int const& top, string const& worldName, string const& ownerName, string const& imagePath, int const& x, int const& y) {
		layout += format("\nadd_cmmnty_wotd_bttn|OPENWORLD|`w#{}`` {} by `#{}``|{}||{}|{}|{}|", top, worldName, ownerName, imagePath, x, y, worldName);
		return *this;
	}
	CommunityBuilder& hub_type(string const& CommunityHubType) {
		layout += format("\ncommunity_hub_type|{}", CommunityHubType);
		return *this;
	}
	CommunityBuilder& end_dialog(string const& dialogName, string const& cancel, string const& accept) {
		layout += format("\nend_dialog|{}|{}|{}|", dialogName, cancel, accept);
		return *this;
	}
	CommunityBuilder& add_default_tabs() {
		layout += (
			"\nadd_tab_button|community_featured_worlds|featured worlds|interface/large/btn_community_hub.rttex||0|0|0|0||"
			"\nadd_tab_button|community_worldoftheday|world of the day|interface/large/btn_community_hub.rttex||0|1|0|0||"
			"\nadd_tab_button|communityworlds_toprated|top rated|interface/large/btn_community_hub.rttex||0|4|0|0||"
			"\nadd_tab_button|communityworlds_toptoday|top today|interface/large/btn_community_hub.rttex||0|5|0|0||"
			"\nadd_tab_button|communityworlds_topoverall|topoverall|interface/large/btn_community_hub.rttex||0|6|0|0||"
			//"\nadd_tab_button|community_growtorials|community growtorials|interface/large/btn_community_hub.rttex||0|2|0|0||"
			//"\nadd_tab_button|community_influencervideos|influencer videos|interface/large/btn_community_hub.rttex||0|3|0|0||"
			);
		return *this;
	}
};
class ShopBuilder {
	string layout = "";
public:
	string to_string() { return layout; }
	ShopBuilder() { layout; }

	ShopBuilder& raw(string const& text) {
		layout += text;
		return *this;
	}
	ShopBuilder& set_description(string const& text) {
		layout += format("set_description_text|{}", text);
		return *this;
	}
	ShopBuilder& enable_tabs(bool const& enable) {
		layout += format("\nenable_tabs|{}", enable ? "1" : "0");
		return *this;
	}
	ShopBuilder& add_tabs(string const& btn, string const& name, string const& iconPath, int const& x, int const& y) {
		layout += format("\nadd_tab_button|{}|{}|{}||{}|{}|0|0||||-1|-1|||0|0|", btn, name, iconPath, x, y);
		return *this;
	}
	ShopBuilder& add_banner(string const& imagePath, int const& x, int const& y) {
		layout += format("\nadd_banner|{}|{}|{}|", imagePath, x, y);
		return *this;
	}
	ShopBuilder& add_big_banner(string const& imagePath, string const& text, int const& x, int const& y) {
		layout += format("\nadd_big_banner|{}|{}|{}|{}|", imagePath, x, y, text);
		return *this;
	}
	ShopBuilder& add_gems_item(string const& btn, string const& itemName, string const& store_buttons, string const& description, int const& x, int const& y, int const& price) {
		layout += format("\nadd_button|{}|`o{}``|interface/large/store_buttons/{}.rttex|`2You Get:`` 1 {}.<CR><CR>`5Description:`` {}``|{}|{}|{}|0|||-1|-1||-1|-1||1||||||0|0|", btn, itemName, store_buttons, itemName, description, x, y, price);
		return *this;
	}
	ShopBuilder& add_token_items(string const& btn, string const& itemName, string const& store_buttons, string const& description, int const& x, int const& y, int const& price) {
		layout += format("\nadd_button|{}|`o{}``|interface/large/store_buttons/{}.rttex|`2You Get:`` 1 {}.<CR><CR>`5Description:`` {}``|{}|{}|-{}|0|||-1|-1||-1|-1||1||||||0|0|", btn, itemName, store_buttons, itemName, description, x, y, price);
		return *this;
	}
	ShopBuilder& add_cash_item(string const& btn, string const& itemName, string const& store_buttons, string const& description, int const& x, int const& y) {
		layout += format("\nadd_button|{}|{}|interface/large/store_buttons/{}.rttex|https://discord.gg/Growhoshi|{}|{}|0||||-1|-1||-1|-1|`2You Get:`` 1 {}.<CR><CR>`5Description:`` {}``|1||||||0|0|", btn, itemName, store_buttons, x, y, itemName, description);
		return *this;
	}
};
// 2FA
/*
inline void OnSuperMain(ENetPeer* peer, string error = "") {
	// Verified Account 2FA Check
	if (pInfo(peer)->isVerified and pInfo(peer)->verifiedIP == pInfo(peer)->ip or pInfo(peer)->verifiedMAC == pInfo(peer)->mac or pInfo(peer)->verifiedRID == pInfo(peer)->rid) pInfo(peer)->canEnter = true;
	// Verified Account 2FA Invalid Verified DeviceID & Address
	if (pInfo(peer)->isVerified and pInfo(peer)->canEnter == false) {
		DialogBuilder dialog("`o");
		// MALES BAT LANJUTNYA ANJING
	}
	else pInfo(peer)->canEnter = true;
	if (pInfo(peer)->canEnter) {
		pInfo(peer)->isIn = true;
		gamepacket_t p;
		p.Insert("OnSuperMainStartAcceptLogonHrdxs47254722215a");
		p.Insert(item_hash);
		p.Insert("www.growtopia1.com");
		p.Insert("cache/");
		p.Insert("cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster");
		string pay_status = (pInfo(peer)->supp >= 1 ? "1" : "0");
		p.Insert("proto=185|choosemusic=audio/mp3/PineappleDisco2022.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=60094386|clash_active=0|dropeerlavacheck_faster=1|isPayingUser=" + pay_status + "|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|");
		//p.Insert("370236174");
		p.CreatePacket(peer);
	}
}
*/
// DIALOG HANDLER
inline void SendBirth_Dialog(ENetPeer* peer, string error, bool confirm = false) {
	if (confirm) {
		gamepacket_t p;
		p.Insert("OnDialogRequest");
		p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wChange your HoshiID``|left|1280|\nadd_textbox|" + error + "|left|\nadd_smalltext|This will change your HoshiID `4permanently``. You can't change it again for 30 days.<CR>Your `wBirth Certificate ``will be consumed if you click `5Change It``.<CR>Choose an appropriate name or `6we will change it for you!``|left|\nadd_spacer|small|\nend_dialog|confirm_hid|Cancel|Change it!|");
		p.CreatePacket(peer);
	}
	else {
		gamepacket_t p;
		p.Insert("OnDialogRequest");
		p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wChange your HoshiID``|left|1280|" + (not error.empty() ? "\nadd_textbox|" + error : "") + "|left|\nadd_smalltext|This will change your HoshiID `4permanently``. You can't change it again for 30 days.<CR>Your `wBirth Certificate ``will be consumed if you click `5Change It``.<CR>Choose an appropriate name or `6we will change it for you!``|left|\nadd_textbox|Enter your new name:|left|\nadd_text_input|hoshi_id|||32|\nadd_spacer|small|\nend_dialog|change_hid|Cancel|Change it!|");
		p.CreatePacket(peer);
	}
}
inline void SendDialogOnlineStatus(ENetPeer* peer) {
	bool C0, C1, C2;
	switch (pInfo(peer)->p_status) {
	case 0: {
		C0 = true, C1 = false, C2 = false;
		break;
	}
	case 1: {
		C0 = false, C1 = true, C2 = false;
		break;
	}
	case 2: {
		C0 = false, C1 = false, C2 = true;
		break;
	}
	default: break;
	}
	DialogBuilder status("`o");
	status.add_label_icon(true, 1366, "`wOnline Status``")
		.add_spacer(false)
		.add_checkbox(C0, "checkbox_status_online", "Online")
		.add_checkbox(C1, "checkbox_status_busy", "Busy")
		.add_checkbox(C2, "checkbox_status_away", "Away")
		.add_button("set_online_status", "Ok")
		.end_dialog("set_online_status", "", "")
		.add_quick_exit();
	enet_peer_send(peer, 0, Variant{ "OnDialogRequest" }.push(status.to_string()).pack());
}
inline void SendDialogPersonalizeProfile(ENetPeer* peer) {
	time_t s__;
	s__ = time(NULL);
	int days_ = int(s__) / (60 * 60 * 24);
	double hours_ = (double)((s__ - pInfo(peer)->playtime) + pInfo(peer)->seconds) / 3600;
	string num_text = to_string(hours_), status = "";
	string rounded = num_text.substr(0, num_text.find(".") + 3);
	DialogBuilder profile("`o");
	profile.add_label_icon(true, 12436, "`wPersonalize Wrench Menu``")
		.add_spacer(false)
		.add_label(false, "`oSelect what you want to display to the other players when they view your info in the wrench menu.``")
		.add_spacer(false)
		.add_checkbox(pInfo(peer)->Show_Achievements, "checkbox_show_achievements", "Display Selected Achievements?")
		.add_custom_margin(0, -20.85)
		.add_achieve_button("", "", 100, "0")
		.add_custom_margin(75, -70.85)
		.add_achieve_button("", "", 100, "1")
		.add_custom_margin(75, -70.85)
		.add_achieve_button("", "", 100, "2")
		.add_custom_margin(75, -70.85)
		.add_achieve_button("", "", 100, "3")
		.add_custom_margin(75, -70.85)
		.add_spacer(false)
		.reset_placement_x()
		.add_spacer(true)
		.add_checkbox(pInfo(peer)->Show_OwnedWorlds, "checkbox_show_ownworld_count", "Display account playtimes?")
		.add_custom_textbox("`1Account Playtime:`` " + rounded + " hours", "small")
		.add_spacer(false)
		.add_checkbox(pInfo(peer)->Show_AccountCreated, "checkbox_show_account_created", "Display days since account creation?")
		.add_custom_textbox("`1Account Age: `` " + to_string(days_ - pInfo(peer)->account_created) + " days", "small")
		.add_spacer(false);
	if (pInfo(peer)->home_world == "") {
		profile.add_label(false, "`sDisplay home world?``")
			.add_custom_textbox("No homeworld set", "small")
			.add_custom_textbox("Use '/sethome' to assign the current world as your home world.", "small")
			.add_spacer(false);
	}
	else {
		profile.add_checkbox(pInfo(peer)->Show_HomeWorld, "checkbox_show_homeworld", "Display home world?")
			.add_disabled_button("visit_home_world_" + pInfo(peer)->home_world, "`$Visit " + pInfo(peer)->home_world + "``")
			.add_spacer(false);
	}
	profile.add_button("preview", "Preview")
		.add_spacer(false)
		.add_button("save", "Save Changes")
		.add_button("discard", "Discard Changes")
		.end_dialog("personalize_profile", "", "")
		.add_quick_exit();
	enet_peer_send(peer, 0, Variant{ "OnDialogRequest" }.push(profile.to_string()).pack());
}
inline void SendKitDialog(ENetPeer* peer) {
	if (pInfo(peer)->startedKit) {
		DialogBuilder dialog("`o");
		dialog.add_player_info("`wGrowhoshi Kits``", pInfo(peer)->lvlKit, pInfo(peer)->xpKit, 125 * ((pInfo(peer)->lvlKit * pInfo(peer)->lvlKit) + 2))
			.add_textbox("Kits are `2unlocked ``by reaching the required kit levels:")
			.add_spacer(false);
		if (not pInfo(peer)->kit1 and pInfo(peer)->lvlKit >= 5) {
			dialog.add_kit_claim_button("info_kit1", to_string(pInfo(peer)->lvlKit) + "/5", 5140);
		}
		else if (pInfo(peer)->kit1) dialog.add_kit_claimed_button("claimed_kit1", 5140);
		else dialog.add_kit_disabled_button("info_kit1", to_string(pInfo(peer)->lvlKit) + "/5", 5140);
		//kit1
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit2 and pInfo(peer)->lvlKit >= 10) {
			dialog.add_kit_claim_button("info_kit2", to_string(pInfo(peer)->lvlKit) + "/10", 10004);
		}
		else if (pInfo(peer)->kit2) dialog.add_kit_claimed_button("claimed_kit2", 10004);
		else dialog.add_kit_disabled_button("info_kit2", to_string(pInfo(peer)->lvlKit) + "/10", 10004);
		//kit2
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit3 and pInfo(peer)->lvlKit >= 15) {
			dialog.add_kit_claim_button("info_kit3", to_string(pInfo(peer)->lvlKit) + "/15", 394);
		}
		else if (pInfo(peer)->kit3) dialog.add_kit_claimed_button("claimed_kit3", 394);
		else dialog.add_kit_disabled_button("info_kit3", to_string(pInfo(peer)->lvlKit) + "/15", 394);
		//kit3
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit4 and pInfo(peer)->lvlKit >= 20) {
			dialog.add_kit_claim_button("info_kit4", to_string(pInfo(peer)->lvlKit) + "/20", 9538);
		}
		else if (pInfo(peer)->kit4) dialog.add_kit_claimed_button("claimed_kit4", 9538);
		else dialog.add_kit_disabled_button("info_kit4", to_string(pInfo(peer)->lvlKit) + "/20", 9538);
		//kit4
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit5 and pInfo(peer)->lvlKit >= 30) {
			dialog.add_kit_claim_button("info_kit5", to_string(pInfo(peer)->lvlKit) + "/30", 9474);
		}
		else if (pInfo(peer)->kit5) dialog.add_kit_claimed_button("claimed_kit5", 9474);
		else dialog.add_kit_disabled_button("info_kit5", to_string(pInfo(peer)->lvlKit) + "/30", 9474);
		//kit5
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit6 and pInfo(peer)->lvlKit >= 40) {
			dialog.add_kit_claim_button("info_kit6", to_string(pInfo(peer)->lvlKit) + "/40", 7074);
		}
		else if (pInfo(peer)->kit6) dialog.add_kit_claimed_button("claimed_kit6", 7074);
		else dialog.add_kit_disabled_button("info_kit6", to_string(pInfo(peer)->lvlKit) + "/40", 7074);
		//kit6
		dialog.add_custom_margin(25, 0);
		if (not pInfo(peer)->kit7 and pInfo(peer)->lvlKit >= 50) {
			dialog.add_kit_claim_button("info_kit7", to_string(pInfo(peer)->lvlKit) + "/50", 6128);
		}
		else if (pInfo(peer)->kit7) dialog.add_kit_claimed_button("claimed_kit7", 6128);
		else dialog.add_kit_disabled_button("info_kit7", to_string(pInfo(peer)->lvlKit) + "/50", 6128);
		//kit7
		dialog.end_list().add_spacer(false)
			.add_smalltext("`4NOTE``: `wKit XP ``is entirely separatte from regular XP...")
			.add_smalltext("To get kit XP (not via gems like regular XP), you can do one of the following activities:")
			.add_smalltext("- Smash blocks (use `wEggs Benedict ``to boost like regular XP)<CR>- Harvest trees<CR>- Harvest providers<CR>- Activity such as Geiger Hunting, Fishing, and Surgery")
			.add_quick_exit().end_dialog("kit_dialog", "Thanks for the info!", "");
		enet_peer_send(peer, 0, Variant{ "OnDialogRequest" }.push(dialog.to_string()).pack());
	}
}
inline void SendInfoKitDialog(ENetPeer* peer, int caseType) {
	DialogBuilder dialog("`o");
	switch (caseType) {
	case 1: {
		dialog.add_label_icon(true, 5140, "Mystery Box Kit")
			.add_spacer(false)
			.add_textbox("This kit contains one item: `wx5 Mystery Box``")
			.add_spacer(false)
			.add_textbox("This kit requires Kit level `55 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 5) dialog.add_spacer(false).add_button("claim_kit1", "Claim");
		break;
	}
	case 2: {
		dialog.add_label_icon(true, 10004, "Gacha Kit")
			.add_spacer(false)
			.add_textbox("This kit contains: `wx3 Super Summer Surprise, x10 Summer Surprise, x1 SuperFirework, x1 Beach Blast``")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `510 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 10) dialog.add_spacer(false).add_button("claim_kit2", "Claim");
		break;
	}
	case 3: {
		dialog.add_label_icon(true, 394, "Free Supporter Kit")
			.add_spacer(false)
			.add_textbox("This kit, when redeemed gives you `2Supporter ``permission, including a `wRecycle ``button and much more! Should you already have Supporter, you will get `w100.000 Gems ``instead.")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `515 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 15) dialog.add_spacer(false).add_button("claim_kit3", "Claim");
		break;
	}
	case 4: {
		dialog.add_label_icon(true, 9538, "Wooden Booty Chest Kit")
			.add_spacer(false)
			.add_textbox("This kit contains one item: `wx3 Wooden Booty Chest``")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `520 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 20) dialog.add_spacer(false).add_button("claim_kit4", "Claim");
		break;
	}
	case 5: {
		dialog.add_label_icon(true, 9474, "Free Super Supporter Kit")
			.add_spacer(false)
			.add_textbox("This kit, when redeemed gives you `5Super Supporter ``permission, including a `wRecycle & /warp ``and much more! Should you already have `5Super Supporter``, you will get `w250.000 Gems ``instead.")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `530 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 30) dialog.add_spacer(false).add_button("claim_kit5", "Claim");
		break;
	}
	case 6: {
		dialog.add_label_icon(true, 7074, "Free Title Kit")
			.add_spacer(false)
			.add_textbox("This kit, when redeemed gives you `wStartopia Title``, open options to apply this title! Should you already have `wStartopia Title``, you will get `w100.000 Gems ``instead.")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `540 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 40) dialog.add_spacer(false).add_button("claim_kit6", "Claim");
		break;
	}
	case 7: {
		dialog.add_label_icon(true, 6128, "Free Extra Skin Kit")
			.add_spacer(false)
			.add_textbox("This kit, when redeemed gives you `wExtra Skin``, open options to apply this skin! Should you already have `wExtra Skin``, you will get `w200.000 Gems ``instead.")
			.add_spacer(false)
			.add_textbox("This kit required Kit level `550 ``to redeem.");
		if (pInfo(peer)->lvlKit >= 50) dialog.add_spacer(false).add_button("claim_kit7", "Claim");
		break;
	}
	default: {
		break;
	}
	}
	dialog.end_dialog("info_kit_dialog", "Exit", "Back").add_quick_exit();
	enet_peer_send(peer, 0, Variant{ "OnDialogRequest" }.push(dialog.to_string()).pack());
}
inline void backpack_show(ENetPeer* peer) {
	string item_log = "\nadd_spacer|small|\ntext_scaling_string|aaaaaaaaaaaaaaa|";
	for (int i_ = 0; i_ < pInfo(peer)->bp.size(); i_++) item_log += "\nadd_button_with_icon|" + to_string(i_) + "|" + items[pInfo(peer)->bp[i_].first].name + "|frame|" + to_string(pInfo(peer)->bp[i_].first) + "|" + to_string(pInfo(peer)->bp[i_].second) + "|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wBackpack - Level " + to_string(pInfo(peer)->b_l) + "``|left|6292|\nadd_textbox|`w" + to_string(pInfo(peer)->bp.size()) + "/" + to_string(pInfo(peer)->b_l * 10) + "`` items stored.|left|" + (pInfo(peer)->bp.size() >= 1 ? item_log + "\nadd_button_with_icon||END_LIST|noflags|0||" : "") + "\nadd_spacer|small|" + (pInfo(peer)->b_l * 10 < 250 ? "\nadd_button|backpack_upgrade|`2Upgrade for " + setGems((1000 * ((pInfo(peer)->b_l * pInfo(peer)->b_l) + 25)) * 2) + " gems``|noflags|0|0|" : "") + "" + (pInfo(peer)->bp.size() >= pInfo(peer)->b_l * 10 ? "" : "\nadd_item_picker|itemid|`wDeposit Item``|Choose an item to store|") + "\nend_dialog|backpack_menu|Exit||\nadd_quick_exit|");
	p.CreatePacket(peer);
}
inline void glory_show(ENetPeer* peer) {
	if (pInfo(peer)->glo >= 1) {
		string prizes = "";
		for (int i = 1; i < 10; i++) {
			if (std::find(pInfo(peer)->glo_p.begin(), pInfo(peer)->glo_p.end(), i) != pInfo(peer)->glo_p.end()) prizes += "\nadd_smalltext|`r#" + to_string(i + 1) + ": Level " + to_string(i * 10) + " claimed.``|left|\nadd_label_with_icon|small|`r" + setGems(i * 100000) + " Gems``|left|9438|\nadd_spacer|small|";
			else if (pInfo(peer)->level >= i * 10)prizes += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Level " + to_string(i * 10) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w" + setGems(i * 100000) + " Gems``|left|9438|\nadd_small_font_button|claimreward" + to_string(i) + "|Claim Reward|noflags|0|0|";
			else if (pInfo(peer)->level < i * 10) prizes += "\nadd_smalltext|`s#" + to_string(i + 1) + ": Reach Level " + to_string(i * 10) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s" + setGems(i * 100000) + " Gems``|left|9438|";
		}
		gamepacket_t p;
		p.Insert("OnDialogRequest");
		p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wRoad to Glory``|left|9436|\nadd_smalltext|Claim your Road to Glory Rewards Here|left|\nadd_spacer|small|\nadd_progress_bar|`wYour Level: " + to_string(pInfo(peer)->level) + "``|small|LVL 1|" + to_string(pInfo(peer)->level) + "|90|LVL 90|\nadd_spacer|small|\nadd_smalltext|`r#1: Road to Glory Pack instant reward claimed:``|left|\nadd_label_with_icon|small|`r100,000 Gems``|left|9438|\nadd_spacer|small|" + prizes + "\nadd_spacer|small|\nend_dialog|roadtoglory|Close||\nadd_quick_exit|\n");
		p.CreatePacket(peer);
	}
}
inline void farmer_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 98, 228, 1746, 1778, 1830, 5078, 1966, 6948, 6946, 4956 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		bool ok = (i == 9 ? true : false);
		if (find(pInfo(peer)->t_p.begin(), pInfo(peer)->t_p.end(), lvl += 5) != pInfo(peer)->t_p.end()) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Farmer Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 228 || list[i] == 1746 || list[i] == 1778 ? "200" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`r(Unlocked) Unique Farmer's Flag and skin! - Show off your mastery with a flag beside your name``|left|7064|" : "") + "\nadd_spacer|small|";
		}
		else if (pInfo(peer)->t_lvl >= lvl) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Farmer Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 228 || list[i] == 1746 || list[i] == 1778 ? "200" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`w(Unlocked) Unique Farmer's Flag and skin! - Show off your mastery with a flag beside your name``|left|7064|" : "") + "\nadd_small_font_button|t_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		}
		else if (pInfo(peer)->t_lvl < lvl) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Farmer Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 228 || list[i] == 1746 || list[i] == 1778 ? "200" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`s(Locked) Unique Farmer's Flag and skin! - Show off your mastery with a flag beside your name``|left|7064|" : "");
		}
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|Farmer Rewards|left|7064|\nadd_spacer|small|\nadd_textbox|`9Harvest your trees and receive awesome Farmer Rewards everyday by leveling up``|left|" + prize + "\nadd_player_info|`oFarmer``|" + to_string(pInfo(peer)->t_lvl) + "|" + to_string(pInfo(peer)->t_xp) + "|" + to_string(100 * ((pInfo(peer)->t_lvl * pInfo(peer)->t_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void builder_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 6896, 5212, 6948, 1068, 1966, 1836, 5080, 10754, 1874, 6946 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		bool ok = (i == 9 ? true : false);
		if (find(pInfo(peer)->bb_p.begin(), pInfo(peer)->bb_p.end(), lvl += 5) != pInfo(peer)->bb_p.end()) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Builder Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 5212 ? "1" : "") + "" + (list[i] == 1068 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`r(Unlocked) Unique Builder's Flag and skin! - Show off your mastery with a flag beside your name``|left|7070|" : "") + "\nadd_spacer|small|";
		}
		else if (pInfo(peer)->bb_lvl >= lvl) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Builder Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 5212 ? "1" : "") + "" + (list[i] == 1068 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`w(Unlocked) Unique Builder's Flag and skin! - Show off your mastery with a flag beside your name``|left|7070|" : "") + "\nadd_small_font_button|b_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		}
		else if (pInfo(peer)->bb_lvl < lvl) {
			prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Builder Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 5212 ? "1" : "") + "" + (list[i] == 1068 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|" + (ok ? "\nadd_label_with_icon|small|`s(Locked) Unique Builder's Flag and skin! - Show off your mastery with a flag beside your name``|left|7070|" : "");
		}
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|Builder Rewards|left|7070|\nadd_spacer|small|\nadd_textbox|`9Keep breaking blocks and achieve awesome rewards everyday by leveling up``|left|" + prize + "\nadd_player_info|`oBuilder``|" + to_string(pInfo(peer)->bb_lvl) + "|" + to_string(pInfo(peer)->bb_xp) + "|" + to_string(100 * ((pInfo(peer)->bb_lvl * pInfo(peer)->bb_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void provider_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 1008,1044,872,10450,870,5084,876,6950,6952,1674 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		if (find(pInfo(peer)->p_p.begin(), pInfo(peer)->p_p.end(), lvl += 5) != pInfo(peer)->p_p.end()) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Provider Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 1008 ? "5" : list[i] == 1044 ? "50" : list[i] == 872 ? "200" : list[i] == 10450 ? "3" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_spacer|small|";
		else if (pInfo(peer)->p_lvl >= lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Provider Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 1008 ? "5" : list[i] == 1044 ? "50" : list[i] == 872 ? "200" : list[i] == 10450 ? "3" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_small_font_button|p_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		else if (pInfo(peer)->p_lvl < lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Provider Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 1008 ? "5" : list[i] == 1044 ? "50" : list[i] == 872 ? "200" : list[i] == 10450 ? "3" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|";
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|Provider Rewards|left|866|\nadd_spacer|small|\nadd_textbox|`9A provider is a non-solid foreground block which can be harvested periodically (time depends on the provider) for an item. Harvest your providers and earn awesome rewards by leveling up!``|left|" + prize + "\nadd_player_info|`oProvider``|" + to_string(pInfo(peer)->p_lvl) + "|" + to_string(pInfo(peer)->p_xp) + "|" + to_string(75 * ((pInfo(peer)->p_lvl * pInfo(peer)->p_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void geiger_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 4654,262,826,828,9712,3146,2266,5072,5070,9716 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		if (find(pInfo(peer)->g_p.begin(), pInfo(peer)->g_p.end(), lvl += 5) != pInfo(peer)->g_p.end()) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Geiger Hunting Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 262 || list[i] == 826 || list[i] == 828 ? "50" : list[i] == 3146 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_spacer|small|";
		else if (pInfo(peer)->g_lvl >= lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Geiger Hunting Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 262 || list[i] == 826 || list[i] == 828 ? "50" : list[i] == 3146 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_small_font_button|g_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		else if (pInfo(peer)->g_lvl < lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Geiger Hunting Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 262 || list[i] == 826 || list[i] == 828 ? "50" : list[i] == 3146 ? "10" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|";
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|small|Geiger Hunting Rewards|left|2204|\nadd_spacer|small|\nadd_textbox|`9Once the player stands for a few moments on the source of radioactivity, the item they found will be added into their inventory. Hunt the items with your geiger and claim your prizes for leveling up!``|left|" + prize + "\nadd_player_info|`oGeiger Hunting``|" + to_string(pInfo(peer)->g_lvl) + "|" + to_string(pInfo(peer)->g_xp) + "|" + to_string(5 * ((pInfo(peer)->g_lvl * pInfo(peer)->g_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void fishing_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 3010, 3018, 3020, 3044, 5740, 3042, 3098, 3100, 3040, 10262 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		if (find(pInfo(peer)->ff_p.begin(), pInfo(peer)->ff_p.end(), lvl += 5) != pInfo(peer)->ff_p.end()) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Fishing Hunting Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 3018 ? "200" : list[i] == 3020 || list[i] == 3098 ? "25" : list[i] == 3044 || list[i] == 3218 ? "50" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_spacer|small|";
		else if (pInfo(peer)->ff_lvl >= lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Fishing Hunting Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 3018 ? "200" : list[i] == 3020 || list[i] == 3098 ? "25" : list[i] == 3044 || list[i] == 3218 ? "50" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_small_font_button|f_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		else if (pInfo(peer)->ff_lvl < lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Fishing Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 3018 ? "200" : list[i] == 3020 || list[i] == 3098 ? "25" : list[i] == 3044 || list[i] == 3218 ? "50" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|";
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|Fishing Rewards|left|7072|\nadd_spacer|small|\nadd_textbox|`9In Growtopia, fishing can be done by using bait on water while holding a Fishing Rod. The result from fishing can either be Fish or some random items that are obtainable through fishing.``|left|" + prize + "\nadd_player_info|`oFishing``|" + to_string(pInfo(peer)->ff_lvl) + "|" + to_string(pInfo(peer)->ff_xp) + "|" + to_string(5 * ((pInfo(peer)->ff_lvl * pInfo(peer)->ff_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void surgery_reward_show(ENetPeer* peer) {
	string prize = "";
	vector<int> list{ 6900, 6982, 2002, 6212, 3172, 9068, 6912, 10836, 3130, 8284 };
	for (int i = 0, lvl = 0; i < list.size(); i++) {
		if (find(pInfo(peer)->surg_p.begin(), pInfo(peer)->surg_p.end(), lvl += 5) != pInfo(peer)->surg_p.end()) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Surgeon Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`r(Unlocked) " + (list[i] == 6212 ? "50" : list[i] == 3172 || list[i] == 6912 ? "25" : list[i] == 10836 ? "100" : list[i] == 2002 ? "1" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_spacer|small|";
		else if (pInfo(peer)->s_lvl >= lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Surgeon Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`w(Unlocked) " + (list[i] == 6212 ? "50" : list[i] == 3172 || list[i] == 6912 ? "25" : list[i] == 10836 ? "100" : list[i] == 2002 ? "1" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|\nadd_small_font_button|s_claimreward" + to_string(i + 1) + "|Claim Reward|noflags|0|0|";
		else if (pInfo(peer)->s_lvl < lvl) prize += "\nadd_smalltext|`w#" + to_string(i + 1) + ": Reach Surgeon Level " + to_string(lvl) + " to claim this reward:``|left|\nadd_label_with_icon|small|`s(Locked) " + (list[i] == 6212 ? "50" : list[i] == 3172 || list[i] == 6912 ? "25" : list[i] == 10836 ? "100" : list[i] == 2002 ? "1" : "") + " " + items[list[i]].name + "``|left|" + to_string(list[i]) + "|";
	}
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|Surgeon Rewards|left|7068|\nadd_spacer|small|\nadd_textbox|`9Surgery is a feature in Growtopia that allows players to perform various types of surgeries on other players using various tools. It was first introduced in the Growtopia General Hospital update on November 21, 2013.`` `2Surgery XP is only added if the surgery was performed on a real pacient.``|left|" + prize + "\nadd_player_info|`oSurgeon``|" + to_string(pInfo(peer)->s_lvl) + "|" + to_string(pInfo(peer)->s_xp) + "|" + to_string(10 * ((pInfo(peer)->s_lvl * pInfo(peer)->s_lvl) + 2)) + "|\nadd_spacer|small|\nend_dialog|worlds_list||Back|\nadd_quick_exit|\n");
	p.CreatePacket(peer);
}
inline void SendDNAProcessor(ENetPeer* peer, int x, int y, bool Erorr, bool Analisa, bool RemoveDNA, int IDDNARemove, bool MasukinDNA, bool Invalid) {
	string Dialog;
	Dialog += "set_default_color|`o\nadd_label_with_icon|big|`wDNA Processor``|left|4080|";
	Dialog += "\nadd_spacer|small|\nembed_data|tilex|" + to_string(x) + "\nembed_data|tiley|" + to_string(y);
	if (Erorr) {
		Dialog += "\nadd_textbox|`4ERROR:`` That's not a DNA sample.|left|\nadd_spacer|small|";
	}
	if (Analisa) {
		Dialog += "\nadd_textbox|`2DNA analyzed.``|left|\nadd_spacer|small|";
	}
	if (RemoveDNA) {
		Dialog += "\nadd_textbox|`2" + items[IDDNARemove].name + " removed.``|left|\nadd_spacer|small|";
	}
	if (MasukinDNA) {
		if (pInfo(peer)->DNAInput == 1) {
			Dialog += "\nadd_smalltext|[`2DNA sequence 33% complete``]|left|";
		}
		else if (pInfo(peer)->DNAInput == 2) {
			Dialog += "\nadd_smalltext|[`2DNA sequence 66% complete``]|left|";
		}
		else if (pInfo(peer)->DNAInput == 3) {
			Dialog += "\nadd_smalltext|[`2DNA sequence 100% complete``]|left|";
		}
	}
	if (pInfo(peer)->IDDNA1 != 0) {
		Dialog += "\nadd_button_with_icon|remove0|`9" + items[pInfo(peer)->IDDNA1].name + "``|noflags|" + to_string(pInfo(peer)->IDDNA1) + "||";
	}
	if (pInfo(peer)->IDDNA2 != 0) {
		Dialog += "\nadd_button_with_icon|remove1|`9" + items[pInfo(peer)->IDDNA2].name + "``|noflags|" + to_string(pInfo(peer)->IDDNA2) + "||";
	}
	if (pInfo(peer)->IDDNA3 != 0) {
		Dialog += "\nadd_button_with_icon|remove2|`9" + items[pInfo(peer)->IDDNA3].name + "``|noflags|" + to_string(pInfo(peer)->IDDNA3) + "||";
	}
	if (pInfo(peer)->DNAInput > 0) {
		Dialog += "\nadd_button_with_icon||END_LIST|noflags|0||";
	}
	if (pInfo(peer)->DNAInput == 3) {
		if (Invalid) {
			Dialog += "\nadd_smalltext|`4DNA SEQUENCE INVALID``|left|";
		}
		else {
			Dialog += "\nadd_button|combine|`2Combine DNA sequence``|noflags|0|0|";
		}
	}
	else {
		if (pInfo(peer)->DNAInput == 0) {
			Dialog += "\nadd_textbox|DNA Processing ready. Please insert DNA strand.|left|";
		}
		Dialog += "\nadd_item_picker|choose|Insert DNA Here|Select a DNA Strand|";
	}
	Dialog += "\nend_dialog|dnaproc|Exit||";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(Dialog);
	p.CreatePacket(peer);
}
inline void SendManualMusicalWings(ENetPeer* peer) {
	int volume = pInfo(peer)->m_volume;
	string note = pInfo(peer)->m_note;
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert("set_default_color|`o\nadd_label_with_icon|big|`wMusical Wings``|left|10182|\n"
		"add_spacer|small|\n"
		"add_textbox|This wing will play up to 16 music notes. Each note is triggered as you pass over a block.|left|\n"
		"add_textbox|In the `2Volume `obox, enter a volume level for these notes, from 1-100. 100 is the normal volume of music notes.|left|\n"
		"add_textbox|In the `2Notes `obox, enter up to 16 music notes to play. For each note, you enter 2 symbols:|left|\n"
		"add_smalltext|-The note to play, `2A to G`0, as in normal music notation. Lowercase for lower octave, uppercase for higher.|\n"
		"add_smalltext|Spaces are optional, but sure make it easier to read.|\n"
		"add_smalltext|- Last, a `2#`o for a sharp note, a - for a natural note, or a `2b `ofor a flat note.|\n"
		"add_smalltext|Spaces are optional, but sure make it easier to read.|\n"
		"add_text_input|volume|Volume|" + to_string(volume) + "|3|\n"
		"add_text_input|text|Notes|" + note + "|50|\n"
		"add_spacer|small|\nadd_button|resoterdefault|Restore to Default|noflags|0|0|\n"
		"end_dialog|pianowings|Cancel|Update| ");
	p.CreatePacket(peer);
}
inline void SendDialogRiftCape(ENetPeer* peer, bool Instructions, string error = "") {
	string dialog;
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wRift Cape``|left|10424|\nadd_spacer|small|";
	dialog += "\nadd_text_input|text_input_time_cycle|Time Dilation Cycle Time:|" + to_string(pInfo(peer)->Cycle_Time) + "|5|";
	dialog += pInfo(peer)->Time_Change == true ? "\nadd_checkbox|checkbox_time_cycle|Time Dilation On / Off|1" : "\nadd_checkbox|checkbox_time_cycle|Time Dilation On / Off|0";
	dialog += error != "" ? "\nadd_textbox|`4" + error + "``|left|\nadd_spacer|small|" : "";
	dialog += Instructions ? "\nadd_textbox|This cape has several special functions!|left|\nadd_spacer|small|\nadd_textbox|To set the color for the cape and collar you need to enter an RGB (Red, Blue, Green) value. To separate the individual values, you need to use a comma.|left|\nadd_spacer|small|\nadd_textbox|Set the Time Dilation Cycle Time to define how often the cape will change between the two Cape Styles. Cycle time is in seconds; maximum number of seconds allowed is: 86400 seconds (24 hours).|left|" : "\nadd_button|button_manual|Instructions|noflags|0|0|";
	dialog += "\nadd_spacer|small|\nadd_label|big|Cape Style 1|left\nadd_spacer|small|\nadd_label|small|Cape Color:|left";
	dialog += "\nadd_text_input|text_input_cape_color0|Cape - R,G,B:|" + to_string(pInfo(peer)->Cape_R_0) + "," + to_string(pInfo(peer)->Cape_G_0) + "," + to_string(pInfo(peer)->Cape_B_0) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Cape_Collar_0 == true ? "\nadd_checkbox|checkbox_cape_collar0|Cape Collar On / Off|1" : "\nadd_checkbox|checkbox_cape_collar0|Cape Collar On / Off|0";
	dialog += "\nadd_text_input|text_input_collar_color0|Collar - R,G,B:|" + to_string(pInfo(peer)->Collar_R_0) + "," + to_string(pInfo(peer)->Collar_G_0) + "," + to_string(pInfo(peer)->Collar_B_0) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Closed_Cape_0 == true ? "\nadd_checkbox|checkbox_closed_cape0|Closed Cape|1" : "\nadd_checkbox|checkbox_closed_cape0|Closed Cape|0";
	dialog += pInfo(peer)->Open_On_Move_0 == true ? "\nadd_checkbox|checkbox_open_on_move0|Open Cape on Movement|1" : "\nadd_checkbox|checkbox_open_on_move0|Open Cape on Movement|0";
	dialog += pInfo(peer)->Aura_0 == true ? "\nadd_checkbox|checkbox_aura0|Aura On / Off|1" : "\nadd_checkbox|checkbox_aura0|Aura On / Off|0";
	dialog += pInfo(peer)->Aura_1st_0 == true ? "\nadd_checkbox|checkbox_aura_1st0|      Portal Aura|1" : "\nadd_checkbox|checkbox_aura_1st0|      Portal Aura|0";
	dialog += pInfo(peer)->Aura_2nd_0 == true ? "\nadd_checkbox|checkbox_aura_2nd0|      Starfield Aura|1" : "\nadd_checkbox|checkbox_aura_2nd0|      Starfield Aura|0";
	dialog += pInfo(peer)->Aura_3rd_0 == true ? "\nadd_checkbox|checkbox_aura_3rd0|      Electrical Aura|1" : "\nadd_checkbox|checkbox_aura_3rd0|      Electrical Aura|0";
	dialog += "\nadd_label|big|Cape Style 2|left\nadd_spacer|small|\nadd_label|small|Cape Color:|left";
	dialog += "\nadd_text_input|text_input_cape_color1|Cape - R,G,B:|" + to_string(pInfo(peer)->Cape_R_1) + "," + to_string(pInfo(peer)->Cape_G_1) + "," + to_string(pInfo(peer)->Cape_B_1) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Cape_Collar_1 == true ? "\nadd_checkbox|checkbox_cape_collar1|Cape Collar On / Off|1" : "\nadd_checkbox|checkbox_cape_collar1|Cape Collar On / Off|0";
	dialog += "\nadd_text_input|text_input_collar_color1|Collar - R,G,B:|" + to_string(pInfo(peer)->Collar_R_1) + "," + to_string(pInfo(peer)->Collar_B_1) + "," + to_string(pInfo(peer)->Collar_B_1) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Closed_Cape_1 == true ? "\nadd_checkbox|checkbox_closed_cape1|Closed Cape|1" : "\nadd_checkbox|checkbox_closed_cape1|Closed Cape|0";
	dialog += pInfo(peer)->Open_On_Move_1 == true ? "\nadd_checkbox|checkbox_open_on_move1|Open Cape on Movement|1" : "\nadd_checkbox|checkbox_open_on_move1|Open Cape on Movement|0";
	dialog += pInfo(peer)->Aura_1 == true ? "\nadd_checkbox|checkbox_aura1|Aura On / Off|1" : "\nadd_checkbox|checkbox_aura1|Aura On / Off|0";
	dialog += pInfo(peer)->Aura_1st_1 == true ? "\nadd_checkbox|checkbox_aura_1st1|      Portal Aura|1" : "\nadd_checkbox|checkbox_aura_1st1|      Portal Aura|0";
	dialog += pInfo(peer)->Aura_2nd_1 == true ? "\nadd_checkbox|checkbox_aura_2nd1|      Starfield Aura|1" : "\nadd_checkbox|checkbox_aura_2nd1|      Starfield Aura|0";
	dialog += pInfo(peer)->Aura_3rd_1 == true ? "\nadd_checkbox|checkbox_aura_3rd1|      Electrical Aura|1" : "\nadd_checkbox|checkbox_aura_3rd1|      Electrical Aura|0";
	dialog += "\nadd_spacer|small|\nadd_button|restore_default|Restore to Default|noflags|0|0|";
	dialog += "\nend_dialog|dialog_rift_cape|Cancel|Update|\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}
inline void SendCernuousEdit(ENetPeer* peer) {
	string dialog;
	string C0, C1, C2, C3, C4, P0, P1, P2, P3, P4;
	switch (pInfo(peer)->Aura_Season) {
	case 0: {
		C0 = "1", C1 = "0", C2 = "0", C3 = "0", C4 = "0";
		break;
	}
	case 1: {
		C0 = "0", C1 = "1", C2 = "0", C3 = "0", C4 = "0";
		break;
	}
	case 2: {
		C0 = "0", C1 = "0", C2 = "1", C3 = "0", C4 = "0";
		break;
	}
	case 3: {
		C0 = "0", C1 = "0", C2 = "0", C3 = "1", C4 = "0";
		break;
	}
	case 4: {
		C0 = "0", C1 = "0", C2 = "0", C3 = "0", C4 = "1";
		break;
	}
	default:
		break;
	}
	switch (pInfo(peer)->Trail_Season) {
	case 0: {
		P0 = "1", P1 = "0", P2 = "0", P3 = "0", P4 = "0";
		break;
	}
	case 1: {
		P0 = "0", P1 = "1", P2 = "0", P3 = "0", P4 = "0";
		break;
	}
	case 2: {
		P0 = "0", P1 = "0", P2 = "1", P3 = "0", P4 = "0";
		break;
	}
	case 3: {
		P0 = "0", P1 = "0", P2 = "0", P3 = "1", P4 = "0";
		break;
	}
	case 4: {
		P0 = "0", P1 = "0", P2 = "0", P3 = "0", P4 = "1";
		break;
	}
	default:
		break;
	}
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wCrown of Seasons``|left|10674|\nadd_spacer|small|\nadd_textbox|You can choose seperate Auras and Trails for this item. Each one represents a different season. Mix and match to get your perfect look.|left|\nadd_spacer|small|";
	dialog += "\nadd_label|big|Aura Season|left\nadd_spacer|small|";
	dialog += "\nadd_checkbox|checkbox_none0|None|" + C0;
	dialog += "\nadd_checkbox|checkbox_spring0|Spring|" + C1;
	dialog += "\nadd_checkbox|checkbox_summer0|Summer|" + C2;
	dialog += "\nadd_checkbox|checkbox_autumn0|Autumn|" + C3;
	dialog += "\nadd_checkbox|checkbox_winter0|Winter|" + C4;
	dialog += "\nadd_spacer|small|";
	dialog += "\nadd_label|big|Trail Season|left\nadd_spacer|small|";
	dialog += "\nadd_checkbox|checkbox_none1|None|" + P0;
	dialog += "\nadd_checkbox|checkbox_spring1|Spring|" + P1;
	dialog += "\nadd_checkbox|checkbox_summer1|Summer|" + P2;
	dialog += "\nadd_checkbox|checkbox_autumn1|Autumn|" + P3;
	dialog += "\nadd_checkbox|checkbox_winter1|Winter|" + P4;
	dialog += "\nadd_spacer|small|";
	dialog += "\nadd_button|restore_default|Restore to Default|noflags|0|0|";
	dialog += "\nend_dialog|dialog_cernuous_mask|Cancel|Update|\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}
inline void SendBannerBandolier2(ENetPeer* peer) {
	string dialog;
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|11748|\nadd_spacer|small|";
	dialog += "\nadd_textbox|Create a stylish banner by choosing an item from yout inventory.|left|\nadd_spacer|small|";
	dialog += pInfo(peer)->CBanner_Item != 0 ? "\nadd_label_with_icon|small|`w" + items[pInfo(peer)->CBanner_Item].name + "``|left|" + to_string(pInfo(peer)->CBanner_Item) + "|\nadd_spacer|small|" : "";
	dialog += "\nadd_item_picker|banneritem|`wChange Banner Item``|Choose the banner item!|";
	dialog += "\nadd_textbox|Pick a pattern for your banner.|left|\nadd_spacer|small|";
	if (pInfo(peer)->CBanner_Flag == 0) dialog += "\nadd_label_with_icon_button|big|Harlequin|left|5838|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->CBanner_Flag == 1) dialog += "\nadd_label_with_icon_button|big|Slant|left|5844|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->CBanner_Flag == 2) dialog += "\nadd_label_with_icon_button|big|Stripe|left|5848|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->CBanner_Flag == 3) dialog += "\nadd_label_with_icon_button|big|Panel|left|5846|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->CBanner_Flag == 4) dialog += "\nadd_label_with_icon_button|big|Cross|left|5842|patternpicker|\nadd_spacer|small|";
	dialog += "\nadd_button|reset|Reset|noflags|0|0|";
	dialog += "\nend_dialog|bannerbandolier|Cancel|Update|";
	dialog += "\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}
inline void SendBannerBandolier(ENetPeer* peer) {
	string dialog;
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wBanner Bandolier``|left|11748|\nadd_spacer|small|";
	dialog += "\nadd_textbox|Create a stylish banner by choosing an item from yout inventory.|left|\nadd_spacer|small|";
	dialog += pInfo(peer)->Banner_Item != 0 ? "\nadd_label_with_icon|small|`w" + items[pInfo(peer)->Banner_Item].name + "``|left|" + to_string(pInfo(peer)->Banner_Item) + "|\nadd_spacer|small|" : "";
	dialog += "\nadd_item_picker|banneritem|`wChange Banner Item``|Choose the banner item!|";
	dialog += "\nadd_textbox|Pick a pattern for your banner.|left|\nadd_spacer|small|";
	if (pInfo(peer)->Banner_Flag == 0) dialog += "\nadd_label_with_icon_button|big|Harlequin|left|5838|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->Banner_Flag == 1) dialog += "\nadd_label_with_icon_button|big|Slant|left|5844|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->Banner_Flag == 2) dialog += "\nadd_label_with_icon_button|big|Stripe|left|5848|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->Banner_Flag == 3) dialog += "\nadd_label_with_icon_button|big|Panel|left|5846|patternpicker|\nadd_spacer|small|";
	else if (pInfo(peer)->Banner_Flag == 4) dialog += "\nadd_label_with_icon_button|big|Cross|left|5842|patternpicker|\nadd_spacer|small|";
	dialog += "\nadd_button|reset|Reset|noflags|0|0|";
	dialog += "\nend_dialog|bannerbandolier|Cancel|Update|";
	dialog += "\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}
inline void SendDialogInfinityCrown(ENetPeer* peer, bool Instructions = false, string error = "") {
	string dialog;
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wInfinity Crown``|left|12958|\nadd_spacer|small|";
	dialog += error != "" ? "\nadd_textbox|`4" + error + "``|left|\nadd_spacer|small|" : "";
	dialog += Instructions ? "\nadd_textbox|This Crown has several special functions!|left|\nadd_spacer|small|\nadd_textbox|Set the Time Dilation Cycle Time to define how often the Crown will change between the two Crown Styles. Cycle time is in seconds; maximum number of seconds allowed is: 86400 seconds (24 hours).|left|\nadd_spacer|small|\nadd_textbox|To set the colors for the styles you need to enter RGB(Red, Green, Blue) values. To separate the individual values, you need to use a comma.|left|" : "\nadd_button|button_manual|Instructions|noflags|0|0|";
	dialog += "\nadd_text_input|text_input_time_cycle|Time Dilation Cycle Time:|" + to_string(pInfo(peer)->Crown_Cycle_Time) + "|5|\nadd_checkbox|checkbox_time_cycle|Time Dilation On / Off|" + string(pInfo(peer)->Crown_Time_Change ? "1" : "0") + "\nadd_spacer|small|\nadd_spacer|small|";
	dialog += "\nadd_label|big|Crown Style 1|left\nadd_spacer|small|\nadd_checkbox|checkbox_floating0|Floating Effect|" + string(pInfo(peer)->Crown_Floating_Effect_0 ? "1" : "0") + "\nadd_spacer|small|\nadd_label|small|Crown Base:|left\nadd_text_input|text_input_base_color0|Color - R,G,B:|" + to_string(pInfo(peer)->Base_R_0) + "," + to_string(pInfo(peer)->Base_G_0) + "," + to_string(pInfo(peer)->Base_B_0) + "|11|\nadd_spacer|small|\nadd_label|small|Crown Gem:|left\nadd_text_input|text_input_gem_color0|Color - R,G,B:|" + to_string(pInfo(peer)->Gem_R_0) + "," + to_string(pInfo(peer)->Gem_G_0) + "," + to_string(pInfo(peer)->Gem_B_0) + "|11|\nadd_checkbox|checkbox_laser_beam0|Laser Beam On/Off|" + string(pInfo(peer)->Crown_Laser_Beam_0 ? "1" : "0") + "\nadd_spacer|small|\nadd_label|small|Crown Crystal/Rays:|left\nadd_text_input|text_input_crystal_color0|Color - R,G,B:|" + to_string(pInfo(peer)->Crystal_R_0) + "," + to_string(pInfo(peer)->Crystal_G_1) + "," + to_string(pInfo(peer)->Crystal_B_1) + "|11|\nadd_checkbox|checkbox_crystals0| Crystals On/Off|" + string(pInfo(peer)->Crown_Crystals_0 ? "1" : "0") + "\nadd_checkbox|checkbox_rays0| Rays On/Off|" + string(pInfo(peer)->Crown_Rays_0 ? "1" : "0") + "\nadd_spacer|small|";
	dialog += "\nadd_label|big|Crown Style 2|left\nadd_spacer|small|\nadd_checkbox|checkbox_floating1|Floating Effect|" + string(pInfo(peer)->Crown_Floating_Effect_1 ? "1" : "0") + "\nadd_spacer|small|\nadd_label|small|Crown Base:|left\nadd_text_input|text_input_base_color1|Color - R,G,B:|" + to_string(pInfo(peer)->Base_R_1) + "," + to_string(pInfo(peer)->Base_G_1) + "," + to_string(pInfo(peer)->Base_B_1) + "|11|\nadd_spacer|small|\nadd_label|small|Crown Gem:|left\nadd_text_input|text_input_gem_color1|Color - R,G,B:|" + to_string(pInfo(peer)->Gem_R_1) + "," + to_string(pInfo(peer)->Gem_G_1) + "," + to_string(pInfo(peer)->Gem_B_1) + "|11|\nadd_checkbox|checkbox_laser_beam1|Laser Beam On/Off|" + string(pInfo(peer)->Crown_Laser_Beam_1 ? "1" : "0") + "\nadd_spacer|small|\nadd_label|small|Crown Crystal/Rays:|left\nadd_text_input|text_input_crystal_color1|Color - R,G,B:|" + to_string(pInfo(peer)->Crystal_R_1) + "," + to_string(pInfo(peer)->Crystal_G_1) + "," + to_string(pInfo(peer)->Crystal_B_1) + "|11|\nadd_checkbox|checkbox_crystals1| Crystals On/Off|" + string(pInfo(peer)->Crown_Crystals_1 ? "1" : "0") + "\nadd_checkbox|checkbox_rays1| Rays On/Off|" + string(pInfo(peer)->Crown_Rays_1 ? "1" : "0") + "\nadd_spacer|small|";
	dialog += "\nadd_button|restore_default|Restore to Default|noflags|0|0|\nend_dialog|dialog_infinity_crown|Cancel|Update|\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}
inline void SendDialogRiftWings(ENetPeer* peer, bool Instructions, string error = "") {
	string dialog;
	dialog += "set_default_color|`o\nadd_label_with_icon|big|`wRift Wings``|left|11478|\nadd_spacer|small|";
	dialog += "\nadd_text_input|text_input_time_cycle|Time Dilation Cycle Time:|" + to_string(pInfo(peer)->Wing_Cycle_Time) + "|5|";
	dialog += pInfo(peer)->Wing_Time_Change == true ? "\nadd_checkbox|checkbox_time_cycle|Time Dilation On / Off|1" : "\nadd_checkbox|checkbox_time_cycle|Time Dilation On / Off|0";
	dialog += error != "" ? "\nadd_textbox|`4" + error + "``|left|\nadd_spacer|small|" : "";
	dialog += Instructions ? "\nadd_textbox|This wings has several special functions!|left|\nadd_spacer|small|\nadd_textbox|To set the color for the wings and wings metal you need to enter an RGB (Red, Blue, Green) value. To separate the individual values, you need to use a comma.|left|\nadd_spacer|small|\nadd_textbox|Set the Time Dilation Cycle Time to define how often the wings will change between the two Wings Styles. Cycle time is in seconds; maximum number of seconds allowed is: 86400 seconds (24 hours).|left|" : "\nadd_button|button_manual|Instructions|noflags|0|0|";
	dialog += "\nadd_spacer|small|\nadd_label|big|Wings Style 1|left\nadd_spacer|small|\nadd_label|small|Wings Color:|left";
	dialog += "\nadd_text_input|text_input_wings_color0|Wings - R,G,B:|" + to_string(pInfo(peer)->Wing_R_0) + "," + to_string(pInfo(peer)->Wing_G_0) + "," + to_string(pInfo(peer)->Wing_B_0) + "|11|\nadd_spacer|small|";
	dialog += "\nadd_label|small|Wings Metal Color:|left\nadd_text_input|text_input_metal_color0|Metal Color - R,G,B:|" + to_string(pInfo(peer)->Wing_Metal_R_0) + "," + to_string(pInfo(peer)->Wing_Metal_G_0) + "," + to_string(pInfo(peer)->Wing_Metal_B_0) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Open_Wing_0 == true ? "\nadd_checkbox|checkbox_open_wings0|Always Open Wings|1" : "\nadd_checkbox|checkbox_open_wings0|Always Open Wings|0";
	dialog += pInfo(peer)->Closed_Wing_0 == true ? "\nadd_checkbox|checkbox_closed_wings0|Always Closed Wings|1" : "\nadd_checkbox|checkbox_closed_wings0|Always Closed Wings|0";
	dialog += "\nadd_spacer|small|";
	dialog += pInfo(peer)->Stamp_Particle_0 == true ? "\nadd_checkbox|checkbox_stamp_particle0|Stamp Particle|1" : "\nadd_checkbox|checkbox_stamp_particle0|Stamp Particle|0";
	dialog += "\nadd_spacer|small|";
	dialog += pInfo(peer)->Trail_On_0 == true ? "\nadd_checkbox|checkbox_trail0|Trail On / Off|1" : "\nadd_checkbox|checkbox_trail0|Trail On / Off|0";
	dialog += pInfo(peer)->Trail_1st_0 == true ? "\nadd_checkbox|checkbox_trail_1st0|      Portal Aura|1" : "\nadd_checkbox|checkbox_trail_1st0|      Portal Aura|0";
	dialog += pInfo(peer)->Trail_2nd_0 == true ? "\nadd_checkbox|checkbox_trail_2nd0|      Starfield Aura|1" : "\nadd_checkbox|checkbox_trail_2nd0|      Starfield Aura|0";
	dialog += pInfo(peer)->Trail_3rd_0 == true ? "\nadd_checkbox|checkbox_trail_3rd0|      Electrical Aura|1" : "\nadd_checkbox|checkbox_trail_3rd0|      Electrical Aura|0";
	dialog += "\nadd_spacer|small|\nadd_label|small|Wings Material:|left";
	dialog += pInfo(peer)->Material_1st_0 == true ? "\nadd_checkbox|checkbox_material_1st0|      Feathers|" : "\nadd_checkbox|checkbox_material_1st0|      Feathers|0";
	dialog += pInfo(peer)->Material_2nd_0 == true ? "\nadd_checkbox|checkbox_material_2nd0|      Blades|1" : "\nadd_checkbox|checkbox_material_2nd0|      Blades|0";
	dialog += pInfo(peer)->Material_3rd_0 == true ? "\nadd_checkbox|checkbox_material_3rd0|      Scales|1" : "\nadd_checkbox|checkbox_material_3rd0|      Scales|0";
	dialog += "\nadd_spacer|small|\nadd_label|big|Wings Style 2|left\nadd_spacer|small|\nadd_label|small|Wings Color:|left";
	dialog += "\nadd_text_input|text_input_wings_color1|Wings - R,G,B:|" + to_string(pInfo(peer)->Wing_R_1) + "," + to_string(pInfo(peer)->Wing_G_1) + "," + to_string(pInfo(peer)->Wing_B_1) + "|11|\nadd_spacer|small|";
	dialog += "\nadd_label|small|Wings Metal Color:|left\nadd_text_input|text_input_metal_color1|Metal Color - R,G,B:|" + to_string(pInfo(peer)->Wing_Metal_R_1) + "," + to_string(pInfo(peer)->Wing_Metal_G_1) + "," + to_string(pInfo(peer)->Wing_Metal_B_1) + "|11|\nadd_spacer|small|";
	dialog += pInfo(peer)->Open_Wing_1 == true ? "\nadd_checkbox|checkbox_open_wings1|Always Open Wings|1" : "\nadd_checkbox|checkbox_open_wings1|Always Open Wings|0";
	dialog += pInfo(peer)->Closed_Wing_1 == true ? "\nadd_checkbox|checkbox_closed_wings1|Always Closed Wings|1" : "\nadd_checkbox|checkbox_closed_wings1|Always Closed Wings|0";
	dialog += "\nadd_spacer|small|";
	dialog += pInfo(peer)->Stamp_Particle_1 == true ? "\nadd_checkbox|checkbox_stamp_particle1|Stamp Particle|1" : "\nadd_checkbox|checkbox_stamp_particle1|Stamp Particle|0";
	dialog += "\nadd_spacer|small|";
	dialog += pInfo(peer)->Trail_On_1 == true ? "\nadd_checkbox|checkbox_trail1|Trail On / Off|1" : "\nadd_checkbox|checkbox_trail1|Trail On / Off|0";
	dialog += pInfo(peer)->Trail_1st_1 == true ? "\nadd_checkbox|checkbox_trail_1st1|      Portal Aura|1" : "\nadd_checkbox|checkbox_trail_1st1|      Portal Aura|0";
	dialog += pInfo(peer)->Trail_2nd_1 == true ? "\nadd_checkbox|checkbox_trail_2nd1|      Starfield Aura|1" : "\nadd_checkbox|checkbox_trail_2nd1|      Starfield Aura|0";
	dialog += pInfo(peer)->Trail_3rd_1 == true ? "\nadd_checkbox|checkbox_trail_3rd1|      Electrical Aura|1" : "\nadd_checkbox|checkbox_trail_3rd1|      Electrical Aura|0";
	dialog += "\nadd_spacer|small|\nadd_label|small|Wings Material:|left";
	dialog += pInfo(peer)->Material_1st_1 == true ? "\nadd_checkbox|checkbox_material_1st1|      Feathers|" : "\nadd_checkbox|checkbox_material_1st1|      Feathers|0";
	dialog += pInfo(peer)->Material_2nd_1 == true ? "\nadd_checkbox|checkbox_material_2nd1|      Blades|1" : "\nadd_checkbox|checkbox_material_2nd1|      Blades|0";
	dialog += pInfo(peer)->Material_3rd_1 == true ? "\nadd_checkbox|checkbox_material_3rd1|      Scales|1" : "\nadd_checkbox|checkbox_material_3rd1|      Scales|0";
	dialog += "\nadd_spacer|small|\nadd_button|restore_default|Restore to Default|noflags|0|0|";
	dialog += "\nend_dialog|dialog_rift_wings|Cancel|Update|\nadd_quick_exit|";
	gamepacket_t p;
	p.Insert("OnDialogRequest");
	p.Insert(dialog);
	p.CreatePacket(peer);
}