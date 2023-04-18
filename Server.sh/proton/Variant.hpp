#pragma once
#include "Packet.h"
#include "Vector.hpp"
#include <enet/dirent.h>

#include <variant>
#include <iostream>
#include <vector>
#include <type_traits>
#include <string_view>
#include <memory>

using VariantTypes = variant<int, float, uint32_t, string, Vector2, Vector3>;
class Variant {
private:
	vector<VariantTypes> variants_;
public:
	Variant(char const* function) : variants_{ string { function } } {}

	template<typename... Args>
	Variant& push(Args const&... args) {
		((variants_.emplace_back(args)), ...);
		return *this;
	}

	ENetPacket* pack(int const& netId = -1, int const& delay = 0) const {
		gamepacket_t packet(delay, netId);

		for (VariantTypes const& variant : variants_) {
			switch (variant.index()) {
			case 0: packet.Insert(get<0>(variant));
				break;
			case 1: packet.Insert(get<1>(variant));
				break;
			case 2: packet.Insert(get<2>(variant));
				break;
			case 3: packet.Insert(get<3>(variant));
				break;
			case 4: {
				Vector2 v = get<4>(variant);
				packet.Insert(v.get_x(), v.get_y());
			} break;
			case 5: {
				Vector3 v = get<5>(variant);
				packet.Insert(v.get_x(), v.get_y(), v.get_x());
			} break;
			default: break;
			}
		}
		return packet.get_packet();
	}
};