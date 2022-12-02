#include "transport_router.h"

#include "graph.h"
#include "domain.h"

namespace transport {

	void GraphBuilder::StopsAndDistanceCounter(int& number_of_stops, int& distance,
		::transport::detail::Stop* begore_second_stop, ::transport::detail::Stop* second_stop) {
		++number_of_stops;
		if (catalogue_.GetLengthsMap().count({ begore_second_stop,second_stop }) != 0) {
			distance += catalogue_.GetLengthsMap().at({ begore_second_stop,second_stop });
		}
		else {
			distance += catalogue_.GetLengthsMap().at({ second_stop,begore_second_stop });
		}
	}

	void GraphBuilder::TransitionBetweenIdenticalStops(graph::VertexId travel_first_st, graph::VertexId travel_second_st, graph::VertexId waiting_first_st) {
		graph::Edge<double> edge;
		//прямое направление
		edge.from = travel_first_st;
		edge.to = travel_second_st;
		edge.weight = routing_settings_.bus_wait_time;
		graph_.AddEdge(edge);
		//обратное направление
		edge.from = travel_second_st;
		edge.to = waiting_first_st;
		edge.weight = routing_settings_.bus_wait_time;
		graph_.AddEdge(edge);
	}

	void GraphBuilder::AddEdgeForCircularRoute(::transport::detail::Bus& bus) {
		if (!bus.stops.empty()) {
			int iterations = bus.stops.size();

			--iterations;
			for (int i = 0; i < iterations; ++i) {
				int distance = 0;
				int number_of_stops = 0;
				for (int n = 1; n < iterations; ++n) {
					int nomber_of_second_st = i + n;
					if (nomber_of_second_st == iterations) {
						nomber_of_second_st = nomber_of_second_st - iterations;
					}
					if (nomber_of_second_st > iterations) {
						continue;
					}

					graph::Edge<double> edge;
					const auto& first_stop = catalogue_.GetStopMap().at(bus.stops[i]->name_of_stop);
					const auto& second_stop = catalogue_.GetStopMap().at(bus.stops[nomber_of_second_st]->name_of_stop);

					int nomber_before_second_st = nomber_of_second_st - 1;
					if (nomber_of_second_st == 0) {
						nomber_before_second_st = iterations - 1;
					}
					const auto& begore_second_stop = catalogue_.GetStopMap().at(bus.stops[nomber_before_second_st]->name_of_stop);

					const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrCreate(first_stop, &bus);
					const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrCreate(second_stop, &bus);

					StopsAndDistanceCounter(number_of_stops, distance, begore_second_stop, second_stop);

					//из стоп в тревел
					edge_range_info[{travel_first_st, waiting_second_st}].stations_passed = number_of_stops;
					edge_range_info[{travel_first_st, waiting_second_st}].bus_name = bus.bus_number;
					edge.from = travel_first_st;
					edge.to = waiting_second_st;
					edge.weight = (distance * 0.06) / routing_settings_.bus_velocity;
					graph_.AddEdge(edge);

					if (first_stop == second_stop) {
						TransitionBetweenIdenticalStops(travel_first_st, travel_second_st, waiting_first_st);
					}
				}
			}
		}
	}

	void GraphBuilder::IterationAddingEdge(int from, int to, int& distance, int& number_of_stops, transport::detail::Bus& bus) {
		graph::Edge<double> edge;
		const auto& first_stop = catalogue_.GetStopMap().at(bus.stops[from]->name_of_stop);
		const auto& second_stop = catalogue_.GetStopMap().at(bus.stops[to]->name_of_stop);

		int nomber_begore_second_stop = to - 1;
		if (from > to) {
			nomber_begore_second_stop = to + 1;
		}
		const auto& begore_second_stop = catalogue_.GetStopMap().at(bus.stops[nomber_begore_second_stop]->name_of_stop);

		const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrCreate(first_stop, &bus);
		const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrCreate(second_stop, &bus);

		StopsAndDistanceCounter(number_of_stops, distance, begore_second_stop, second_stop);

		//из стоп в тревел
		edge_range_info[{travel_first_st, waiting_second_st}].stations_passed = number_of_stops;
		edge_range_info[{travel_first_st, waiting_second_st}].bus_name = bus.bus_number;
		edge.from = travel_first_st;
		edge.to = waiting_second_st;
		edge.weight = (distance * 0.06) / routing_settings_.bus_velocity;
		graph_.AddEdge(edge);

		if (first_stop == second_stop) {
			TransitionBetweenIdenticalStops(travel_first_st, travel_second_st, waiting_first_st);
		}
	}

	void GraphBuilder::AddEdgeForNotCircularRoute(transport::detail::Bus& bus) {
		if (!bus.stops.empty()) {
			int iterations = bus.stops.size();
			for (int i = 0; i < iterations; ++i) {
				int distance = 0;
				int number_of_stops = 0;
				for (int n = i + 1; n < iterations; ++n) {
					IterationAddingEdge(i, n, number_of_stops, distance, bus);
				}
			}
			for (int i = iterations - 1; i > 0; --i) {
				int distance = 0;
				int number_of_stops = 0;
				for (int n = i - 1; n >= 0; --n) {
					IterationAddingEdge(i, n, number_of_stops, distance, bus);
				}
			}
		}
	}

	detail::EdgeInfo GraphBuilder::GetEdgeInfo(graph::VertexId from, graph::VertexId to) {
		return edge_range_info.at({ from,to });
	}
	void GraphBuilder::SetRoutingSettings(detail::RoutingSettings settings) {
		routing_settings_ = settings;
	}
	detail::RoutingSettings GraphBuilder::GetRoutingSettings() {
		return routing_settings_;
	}

	GraphBuilder::Graph& GraphBuilder::BuildGraph() {
		if (graph_.IsComplete()) {
			return graph_;
		}

		graph_ = Graph(catalogue_.GetNumberOfStops() * 2);

		for (auto& bus : catalogue_.GetBuses()) {
			int stop_counter = 0;
			bool first_end_stop = true;
			auto stop_adr = ::transport::detail::Stop();
			::transport::detail::Stop* last_step = &stop_adr;

			for (const auto& stop : bus.stops) {
				
				int  distance = 0;

				if (last_step->name_of_stop.empty()) {
					last_step = stop;
					continue;
				}

				StopsAndDistanceCounter(stop_counter, distance, last_step, stop);

				graph::Edge<double> edge;

				if (first_end_stop) {
					first_end_stop = false;
					if (!Stop_and_vertexId_.count(last_step) != 0) {
						const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrCreate(last_step, &bus);
						//создание ребра для первой остановки
						//прямое направление
						first_end_stop = false;
						edge.from = waiting_first_st;
						edge.to = travel_first_st;
						edge.weight = routing_settings_.bus_wait_time;
						graph_.AddEdge(edge);
						//обратное направление
						edge.from = travel_first_st;
						edge.to = waiting_first_st;
						edge.weight = 0;
						graph_.AddEdge(edge);
					}
				}

				if (!Stop_and_vertexId_.count(stop) != 0) {
					const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrCreate(stop, &bus);
					if (stop_counter == bus.stops.size()) {
						if (!bus.is_circular)
						{//создание ребра промежуточных остановок
							//прямое направление
							edge.from = waiting_second_st;
							edge.to = travel_second_st;
							edge.weight = routing_settings_.bus_wait_time;
							graph_.AddEdge(edge);
							//обратное направление
							edge.from = travel_second_st;
							edge.to = waiting_second_st;
							edge.weight = 0;
							graph_.AddEdge(edge);
						}
					}
					else {//создание ребра промежуточных остановок
						//прямое направление
						edge.from = waiting_second_st;
						edge.to = travel_second_st;
						edge.weight = routing_settings_.bus_wait_time;
						graph_.AddEdge(edge);
						//обратное направление
						edge.from = travel_second_st;
						edge.to = waiting_second_st;
						edge.weight = 0;
						graph_.AddEdge(edge);
					}
				}
				last_step = stop;
			}

			//создание ребер между остановками
			if (bus.is_circular) {
				AddEdgeForCircularRoute(bus);
			}
			else {
				AddEdgeForNotCircularRoute(bus);
			}

		}

		graph_.SetComplete();

		return graph_;
	}

	GraphBuilder::Graph& GraphBuilder::GetGraph() {
		return graph_;
	}

	std::optional <detail::EdgeRange> GraphBuilder::Get_range_vertexId(const std::string& from, const std::string& to) {
		if ((catalogue_.GetStopMap().count(from) != 0) && (catalogue_.GetStopMap().count(to) != 0)) {
			if ((Stop_and_vertexId_.count(catalogue_.GetStopMap().at(from)) != 0) && (Stop_and_vertexId_.count(catalogue_.GetStopMap().at(to)) != 0)) {
				detail::EdgeRange result;
				result.from = Stop_and_vertexId_.at(catalogue_.GetStopMap().at(from));
				result.to = Stop_and_vertexId_.at(catalogue_.GetStopMap().at(to));
				return result;
			}
		}
		return {};
	}

	detail::VertexIdInfo GraphBuilder::Get_Stop_Info_By_VertexId(graph::VertexId verdex_id) {
		return vertexId_info.at(verdex_id);
	}

	detail::VertexRange GraphBuilder::FindInVertexIdCounterOrCreate(::transport::detail::Stop* stop, ::transport::detail::Bus* bus) {
		if (Stop_and_vertexId_.count(stop) != 0) {
			return { Stop_and_vertexId_.at(stop).waiting,
					Stop_and_vertexId_.at(stop).travel };
		}
		Stop_and_vertexId_[stop] = detail::VertexRange{ vertex_counter_,++vertex_counter_ };
		vertexId_info[vertex_counter_ - 1] = { detail::Waiting, stop, bus->bus_number };
		vertexId_info[vertex_counter_] = { detail::Travel, stop, bus->bus_number };
		++vertex_counter_;

		return { Stop_and_vertexId_.at(stop).waiting,
			Stop_and_vertexId_.at(stop).travel };
	}

}
