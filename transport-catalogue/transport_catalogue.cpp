#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"

using namespace ::transport;

void Catalogue::SetDistancesToStop(const std::string_view current_name, const std::unordered_map<std::string_view, int> real_distances) {
	if (!real_distances.empty()) {
		for (const auto [name, length] : real_distances) {
			if (name_of_stop_.count(name)) {
				lengths[{name_of_stop_.at(current_name), name_of_stop_.at(name)}] = length;
			}
			else {
				stops_.push_back(::transport::detail::Stop(std::move(static_cast<std::string>(name))));
				name_of_stop_[stops_.back().name_of_stop] = &stops_.back();
				lengths[{ name_of_stop_.at(current_name), & stops_.back()}] = length;
			}
		}
	}
}

void Catalogue::AddStop(const std::string& stop_n, const ::geo::Coordinates&& coordinate) {
	::transport::detail::Stop* stop_name;
		if (!name_of_stop_.count(stop_n)) {
			stops_.push_back(::transport::detail::Stop(std::move(stop_n), coordinate));
			name_of_stop_[stops_.back().name_of_stop] = &stops_.back();
			stop_name = &stops_.back();
		}
		else {
			stop_name = name_of_stop_.at(stop_n);
			stop_name->coordinat.lat = coordinate.lat;
			stop_name->coordinat.lng = coordinate.lng;
		}		
	}

//задает маршруты только при наличии всех остановок
	void Catalogue::AddBus(const std::string& bus_name, const bool circular_route, const std::vector<std::string>& stops) {
		::transport::detail::Bus bus;
		bus.bus_number = bus_name;
		bus.is_circular = circular_route;

		busses_.push_back(bus);
		name_of_bus_[busses_.back().bus_number] = &busses_.back();

		for (auto& stop : stops) {
			auto stop_name = name_of_stop_.find(stop)->second;
			busses_.back().stops.push_back(stop_name);
			stop_and_busses[stop_name->name_of_stop].insert(busses_.back().bus_number);
		}
	}

	detail::Stop Catalogue::GetStop(const std::string_view name) {
		return *name_of_stop_.at(name);
	}

	detail::Bus Catalogue::GetBus(const std::string_view number) {
		return *name_of_bus_.at(number);
	}

	Catalogue::Distance Catalogue::ComputeRealAndMapDistance(const ::transport::detail::Bus& bus) const {
		Distance distance;
		auto stop_adr = ::transport::detail::Stop();
		::transport::detail::Stop* last_step = &stop_adr;

		for (auto stop : bus.stops) {
			if (last_step->name_of_stop.empty()) {
				last_step = stop;
				continue;
			}

			if (lengths.count({ last_step,stop })) {
				distance.real_distance += lengths.at({ last_step,stop });
			}
			else {
				distance.real_distance += lengths.at({ stop,last_step });
			}
			distance.map_distance += ComputeDistance(last_step->coordinat, stop->coordinat);
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
					distance.real_distance += lengths.at({ last_step_reverse,bus.stops[iter] });
				}
				else {
					distance.real_distance += lengths.at({ bus.stops[iter],last_step_reverse });
				}

				distance.map_distance += ComputeDistance(last_step_reverse->coordinat, bus.stops[iter]->coordinat);
				last_step_reverse = bus.stops[iter];
			}
		}
		return distance;
	}

	detail::BusInfo Catalogue::GetBusInfo(const std::string& bus_number){
		if (name_of_bus_.count(bus_number)) {
			auto bus = *name_of_bus_[bus_number];
			int quantity_stops = bus.stops.size();
			
			if (!bus.is_circular) {
				quantity_stops = 2 * quantity_stops - 1;
			}

			std::unordered_set<::transport::detail::Stop*> uniq_stops;
			for (auto stop : bus.stops) {
				uniq_stops.insert(stop);
			}

			Catalogue::Distance dist = ComputeRealAndMapDistance(bus);
			return detail::BusInfo(bus, quantity_stops, uniq_stops.size(), dist.real_distance, static_cast<double>(dist.real_distance) / static_cast<double>(dist.map_distance));
		}
		else {
			return detail::BusInfo(bus_number);
		}
	}

	detail::StopInfo Catalogue::GetStopInfo(const std::string& stop_name){
		if (name_of_stop_.count(stop_name)) {
			if (stop_and_busses.count(stop_name)) {
				std::vector<std::string_view>vect_to_sort(stop_and_busses[stop_name].begin(), stop_and_busses[stop_name].end());
				std::sort(vect_to_sort.begin(), vect_to_sort.end());
				return detail::StopInfo(name_of_stop_.at(stop_name), vect_to_sort);
			}
			else {
				return detail::StopInfo(name_of_stop_.at(stop_name));
			}
		}
		else {
			return detail::StopInfo(stop_name);
		}
	}

	std::deque<::transport::detail::Bus*> Catalogue::GetOrderedListOfBuses() {
		std::deque<::transport::detail::Bus*> deq_to_sort;
		for (auto& bus:busses_) {
			deq_to_sort.push_front(&bus);
		}
		::std::sort(deq_to_sort.begin(), deq_to_sort.end(), [](::transport::detail::Bus* a, ::transport::detail::Bus* b) {
			return a->bus_number < b->bus_number;
			});
		return deq_to_sort;
	}

	std::deque<::transport::detail::Stop>& Catalogue::GetListOfStops() {
		return stops_;
	}

	std::deque<::geo::Coordinates> Catalogue::GetStopsCoordinatesFromListOfBuses() {
		std::deque<::geo::Coordinates> coordinates;
		for (auto& bus : busses_) {
			for (const auto& stop : bus.stops) {	
				coordinates.push_back(::std::find_if(stops_.begin(), stops_.end(), [stop](auto a) {return a.name_of_stop == stop->name_of_stop; })->coordinat);
			}
		}
		return coordinates;
	}
