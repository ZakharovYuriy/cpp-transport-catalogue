#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <algorithm>
#include <deque>
#include <array>
#include "stat_reader.h"
#include "geo.h"

namespace transport {
	class Catalogue {
	public:
		void SetStop(std::string&& stop, ::transport::detail::Coordinates&& coordinate, std::array<std::string, 100>&& names, std::array<int, 100>&& length, int stops_quantity);
		void SetBus(std::string bus_name, bool circular_route, std::vector<std::string>&& stops);
		void GetStop();
		void GetBus();
		void GetBusInfo(std::string& bus_nomber);
		void GetStopInfo(std::string& stop_name);

	private:
		struct Stop {
			Stop() {}
			Stop(const std::string&& name, const ::transport::detail::Coordinates& coord) :name_of_stop{ name }, coordinat{ coord }{
			}
			Stop(const std::string&& name) :name_of_stop{ name } {
			}
			std::string name_of_stop;
			::transport::detail::Coordinates coordinat;
		};
		struct Bus {
			std::string bus_nomber;
			bool is_circular;
			std::vector <Stop*> stops;
		};

		struct StopHasher {
			size_t operator() (const std::pair<Stop*, Stop*> stop_place) const {
				size_t first_stop = d_hasher_(stop_place.first);
				size_t second_stop = d_hasher_(stop_place.second);

				return first_stop + second_stop * 37;
			}

		private:
			std::hash<const void*>d_hasher_;
		};

		std::deque<Stop>stops_;
		std::deque<Bus>busses_;
		std::unordered_map<std::string_view, Stop*> name_of_stop_;
		std::unordered_map<std::string_view, Bus*> name_of_bus_;
		std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_and_busses;
		std::unordered_map < std::pair<Stop*, Stop*>, int, StopHasher > lengths;
	};
}