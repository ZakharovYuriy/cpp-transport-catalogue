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
//#include "stat_reader.h"
#include "geo.h"

namespace transport {
	namespace detail {
		struct Stop {
			Stop() = default;
			Stop(const std::string&& name, const ::transport::detail::Coordinates& coord) :name_of_stop{ name }, coordinat{ coord }{
			}
			Stop(const std::string&& name) :name_of_stop{ name } {
			}
			std::string name_of_stop="";
			::transport::detail::Coordinates coordinat;
		};

		struct Bus {
			std::string bus_nomber="";
			bool is_circular=false;
			std::vector <Stop*> stops;
		};

		struct BusInfo {
			BusInfo() = delete;
			BusInfo(std::string& nomber) :bus_nomber(nomber) {
			}
			BusInfo(const Bus& bus, const int number_st, const int unic_st, const int real_dist, const double dist) :
				 number_of_stops(number_st), unic_stops(unic_st), real_distance(real_dist), distance(dist) {
				not_empty = true;
				bus_nomber = bus.bus_nomber;
			}
			std::string bus_nomber;
			int number_of_stops=0;
			int unic_stops=0;
			int real_distance=0; 
			double distance=0;
			bool not_empty = false;
		};

		struct StopInfo {
			StopInfo() = delete;
			StopInfo(std::string name):stop_name(name){
			}
			StopInfo(const Stop* stop) :stop_name(stop->name_of_stop), exist(true){
			}
			StopInfo(const Stop* stop, std::vector<std::string_view> buses_input) :buses(buses_input){
				exist = true;
				stop_name = stop->name_of_stop;
			}
			std::string stop_name;
			std::vector<std::string_view> buses;
			bool exist = false;
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
	}

	class Catalogue {
	public:
		void SetDistancesToStop(const std::string_view name, const std::unordered_map<std::string_view, int> real_distances);
		void SetStop(std::string&& stop, const ::transport::detail::Coordinates&& coordinate);
		void SetBus(const std::string& bus_name, const bool circular_route, const std::vector<std::string>& stops);
		detail::Stop GetStop(const std::string_view name);
		detail::Bus GetBus(const std::string_view nomber);
		detail::BusInfo GetBusInfo(std::string& bus_nomber);
		detail::StopInfo GetStopInfo(std::string& stop_name);

	private:		
		std::pair<int, double>ComputeRealAndMapDistance(const ::transport::detail::Bus& bus);
		std::deque<::transport::detail::Stop>stops_;
		std::deque<::transport::detail::Bus>busses_;
		std::unordered_map<std::string_view, ::transport::detail::Stop*> name_of_stop_;
		std::unordered_map<std::string_view, ::transport::detail::Bus*> name_of_bus_;
		std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_and_busses;
		std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, int, ::transport::detail::StopHasher > lengths;
	};

}
