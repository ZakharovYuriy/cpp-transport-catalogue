#pragma once

#include <algorithm>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "geo.h"
#include "domain.h"


namespace transport {
	class Catalogue {
	public:
		void SetDistancesToStop(const std::string_view name, const std::unordered_map<std::string_view, int> real_distances);
		void AddStop(const std::string& stop_name, const ::geo::Coordinates&& coordinate);
		void AddBus(const std::string& bus_name, const bool circular_route, const std::vector<std::string>& stops);
		detail::Stop GetStop(const std::string_view name);
		detail::Bus GetBus(const std::string_view number);
		detail::BusInfo GetBusInfo(const std::string& bus_number);
		detail::StopInfo GetStopInfo(const std::string& stop_name);
		std::deque<::transport::detail::Bus*> GetOrderedListOfBuses();
		std::deque<::transport::detail::Stop>& GetListOfStops();
		std::deque<::geo::Coordinates> GetStopsCoordinatesFromListOfBuses();	

		const std::unordered_map<std::string_view, ::transport::detail::Stop*>& GetStopMap() const;
		std::deque<::transport::detail::Bus>& GetBuses();
		const std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, 
			int, ::transport::detail::StopHasher >& GetLengthsMap() const;
		size_t GetNumberOfStops() const;

		std::deque<::transport::detail::Stop> GetStops() const;
		std::deque<::transport::detail::Bus> GetBusses() const;
		std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, int, ::transport::detail::StopHasher > GetLengths() const;

		//void SetStops(std::deque<::transport::detail::Stop> stops);
		//void SetBusses(std::deque<::transport::detail::Bus> busses);
		//void SetLengths(std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, int, ::transport::detail::StopHasher > lengths);

	private:		
		detail::Distance ComputeRealAndMapDistance(const ::transport::detail::Bus& bus) const;
		std::deque<::transport::detail::Stop> stops_;
		std::deque<::transport::detail::Bus> busses_;
		std::unordered_map<std::string_view, ::transport::detail::Stop*> name_of_stop_;
		std::unordered_map<std::string_view, ::transport::detail::Bus*> name_of_bus_;
		std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_and_busses;
		std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, int, ::transport::detail::StopHasher > lengths_;			
	};

}
