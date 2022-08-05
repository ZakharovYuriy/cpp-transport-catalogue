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
#include "graph.h"
#include "domain.h"



namespace transport {
	class Catalogue {
	private:
		using Graph = graph::DirectedWeightedGraph<double>;
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

		void BuildGraph();
		Graph& GetGraph();
		void AddEdgeForCircularBus(transport::detail::Bus& bus);
		void IterationAddingEdge(int from, int to, int& distance, int& number_of_stops, transport::detail::Bus& bus);
		void AddEdgeForNotCircularBus(transport::detail::Bus& bus);
		detail::EdgeInfo GetEdgeInfo(graph::VertexId from, graph::VertexId to);
		void SetRoutingSettings(detail::RoutingSettings settings);
		detail::RoutingSettings GetRoutingSettings();
		std::optional <std::pair<detail::VertexIdType, detail::VertexIdType>> Get_range_vertexId(const std::string& from, const std::string& to);
		detail::VertexIdInfo Get_Stop_Info_By_VertexId(graph::VertexId);

		struct Distance {
			double map_distance = 0;
			int real_distance = 0;
		};

	private:		
		Distance ComputeRealAndMapDistance(const ::transport::detail::Bus& bus) const;
		std::deque<::transport::detail::Stop> stops_;
		std::deque<::transport::detail::Bus> busses_;
		std::unordered_map<std::string_view, ::transport::detail::Stop*> name_of_stop_;
		std::unordered_map<std::string_view, ::transport::detail::Bus*> name_of_bus_;
		std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_and_busses;
		std::unordered_map < std::pair<::transport::detail::Stop*, ::transport::detail::Stop*>, int, ::transport::detail::StopHasher > lengths;
		
		std::unordered_map<::transport::detail::Stop*, detail::VertexIdType> Stop_and_vertexId_;
		std::unordered_map < graph::VertexId, detail::VertexIdInfo> vertexId_info;
		std::map < std::pair<int,int>, detail::EdgeInfo> edge_range_info;

		Graph graph_;
		size_t vertex_counter_ = 0;
		detail::RoutingSettings routing_settings_;	

		std::pair<int, int> FindInVertexIdCounterOr—reate(::transport::detail::Stop* stop, ::transport::detail::Bus* bus);
	
	};

}
