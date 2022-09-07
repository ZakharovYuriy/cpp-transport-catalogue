#pragma once

#include <map>
#include <unordered_map>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport {
	class GraphBuilder {
	private:
		using Graph = graph::DirectedWeightedGraph<double>;
	public:
		GraphBuilder() = default;
		GraphBuilder(const Catalogue& catalogue, int bus_wait_time,
			double bus_velocity) :routing_settings_({ bus_wait_time, bus_velocity }), catalogue_(catalogue) {
			router_ = graph::Router<double>();
			BuildGraph();
		};

		Graph& GetGraph();
		void AddEdgeForCircularRoute(transport::detail::Bus& bus);
		void IterationAddingEdge(int from, int to, int& distance, int& number_of_stops, transport::detail::Bus& bus);
		void AddEdgeForNotCircularRoute(transport::detail::Bus& bus);
		detail::EdgeInfo GetEdgeInfo(graph::VertexId from, graph::VertexId to);
		void SetRoutingSettings(detail::RoutingSettings settings);
		detail::RoutingSettings GetRoutingSettings();
		std::optional <detail::EdgeRange> Get_range_vertexId(const std::string& from, const std::string& to);
		detail::VertexIdInfo Get_Stop_Info_By_VertexId(graph::VertexId);

		void SetRouter(const graph::Router<double>& router) {
			router_ = router;
		}

		Graph* BuildGraph(const Catalogue& catalogue, detail::RoutingSettings settings) {
			CleareAll();
			catalogue_ = catalogue;
			SetRoutingSettings(settings);
			BuildGraph();
			return &graph_;
		}

		graph::Router<double>&  GetRouter() {
			return router_;
		}

	private:
		std::unordered_map<::transport::detail::Stop*, detail::VertexRange> Stop_and_vertexId_;
		std::unordered_map < graph::VertexId, detail::VertexIdInfo> vertexId_info;
		std::map < std::pair<int, int>, detail::EdgeInfo> edge_range_info;

		Graph graph_;
		graph::Router<double> router_;//

		size_t vertex_counter_ = 0;
		detail::RoutingSettings routing_settings_;//
		Catalogue catalogue_;

		void CleareAll() {
			Stop_and_vertexId_.clear();
			vertexId_info.clear();
			edge_range_info.clear();
			vertex_counter_ = 0;
			routing_settings_ = {0,0};
		}

		Graph& BuildGraph();
		detail::VertexRange FindInVertexIdCounterOr—reate(::transport::detail::Stop* stop, ::transport::detail::Bus* bus);
		
		void StopsAndDistanceCounter(int& number_of_stops, int& distance,
			::transport::detail::Stop* begore_second_stop, ::transport::detail::Stop* second_stop);
		void TransitionBetweenIdenticalStops(graph::VertexId travel_first_st, graph::VertexId travel_second_st, graph::VertexId waiting_first_st);
	};
}