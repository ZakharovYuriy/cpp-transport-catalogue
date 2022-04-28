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
#include "transport_catalogue.h"

using namespace transport;
void Catalogue::SetStop(std::string&& stop, ::transport::detail::Coordinates&& coordinate, std::unordered_map<std::string_view, int> real_distances) {
	::transport::detail::Stop* stop_name;
		if (!name_of_stop_.count(stop)) {
			stops_.push_back(::transport::detail::Stop(std::move(stop), std::move(coordinate)));
			name_of_stop_[stops_.back().name_of_stop] = &stops_.back();
			stop_name = &stops_.back();
		}
		else {
			stop_name = name_of_stop_.at(stop);
			stop_name->coordinat.lat = coordinate.lat;
			stop_name->coordinat.lng = coordinate.lng;
		}
		if (!real_distances.empty()) {
			for (const auto [name,length] : real_distances) {
				if (name_of_stop_.count(name)) {
					lengths[{stop_name, name_of_stop_.at(name)}] = length;
				}
				else {
					stops_.push_back(::transport::detail::Stop(std::move(static_cast<std::string>(name))));
					name_of_stop_[stops_.back().name_of_stop] = &stops_.back();
					lengths[{stop_name, & stops_.back()}] = length;
				}
			}
		}
	}
	void Catalogue::SetBus(std::string bus_name, bool circular_route, std::vector<std::string>&& stops) {
		::transport::detail::Bus bus;
		bus.bus_nomber = bus_name;
		bus.is_circular = circular_route;

		busses_.push_back(bus);
		name_of_bus_[busses_.back().bus_nomber] = &busses_.back();

		for (auto& stop : stops) {
			auto stop_name = name_of_stop_.find(stop)->second;
			busses_.back().stops.push_back(stop_name);
			stop_and_busses[stop_name->name_of_stop].insert(busses_.back().bus_nomber);
		}

	}

	void Catalogue::GetStop() {

	}

	void Catalogue::GetBus() {

	}

	void Catalogue::GetBusInfo(std::string& bus_nomber) {
		if (name_of_bus_.count(bus_nomber)) {
			auto bus = *name_of_bus_[bus_nomber];
			int quantity_stops = bus.stops.size();
			double map_distance = 0;
			int real_distance = 0;
			auto stop_adr = ::transport::detail::Stop();
			::transport::detail::Stop* last_step = &stop_adr;
			for (auto stop : bus.stops) {
				if (last_step->name_of_stop.empty()) {
					last_step = stop;
					continue;
				}
				if (lengths.count({ last_step,stop })) {
					real_distance += lengths.at({ last_step,stop });
				}
				else {
					real_distance += lengths.at({ stop,last_step });
				}
				map_distance += ComputeDistance(last_step->coordinat, stop->coordinat);
				last_step = stop;
			}
			auto stop_adr_reverse = ::transport::detail::Stop();
			::transport::detail::Stop* last_step_reverse = &stop_adr_reverse;
			if (!bus.is_circular) {
				for (int iter = bus.stops.size() - 1; iter >= 0; --iter) {
					if (last_step_reverse->name_of_stop.empty()) {
						last_step_reverse = bus.stops[iter];
						continue;
					}
					if (lengths.count({ last_step_reverse,bus.stops[iter] })) {
						real_distance += lengths.at({ last_step_reverse,bus.stops[iter] });
					}
					else {
						real_distance += lengths.at({ bus.stops[iter],last_step_reverse });
					}
					map_distance += ComputeDistance(last_step_reverse->coordinat, bus.stops[iter]->coordinat);
					last_step_reverse = bus.stops[iter];
				}

				quantity_stops = 2 * quantity_stops - 1;
			}
			std::unordered_set<::transport::detail::Stop*> uniq_stops;
			for (auto stop : bus.stops) {
				uniq_stops.insert(stop);
			}


			::transport::user_interaction::ResultOutputBus(bus.bus_nomber, quantity_stops, uniq_stops.size(), real_distance, real_distance / map_distance);
		}
		else {
			::transport::user_interaction::BadResultBus(bus_nomber);
		}
	}

	void Catalogue::GetStopInfo(std::string& stop_name) {
		if (name_of_stop_.count(stop_name)) {
			if (stop_and_busses.count(stop_name)) {
				std::vector<std::string_view>vect_to_sort(stop_and_busses[stop_name].begin(), stop_and_busses[stop_name].end());
				std::sort(vect_to_sort.begin(), vect_to_sort.end());
				::transport::user_interaction::ResultOutputStop(stop_name, vect_to_sort);
			}
			else {
				::transport::user_interaction::BadResultNoBusses(stop_name);
			}
		}
		else {
			::transport::user_interaction::BadResultStop(stop_name);
		}
	}
