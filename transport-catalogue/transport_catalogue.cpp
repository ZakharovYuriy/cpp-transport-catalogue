#include <algorithm>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"

using namespace ::transport;

void Catalogue::SetDistancesToStop(const std::string_view current_name, const std::unordered_map<std::string_view, int> real_distances) {
	if (!real_distances.empty()) {
		for (const auto& [name, length] : real_distances) {
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
			const auto &bus = *name_of_bus_[bus_number];
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

	std::pair<int, int> Catalogue::FindInVertexIdCounterOrСreate(::transport::detail::Stop* stop, ::transport::detail::Bus* bus) {
		if (Stop_and_vertexId_.count(stop)) {
			return { Stop_and_vertexId_.at(stop).waiting,
			Stop_and_vertexId_.at(stop).travel };
		}
		Stop_and_vertexId_[stop] = detail::VertexIdType{ vertex_counter_,++vertex_counter_ };
		vertexId_info[vertex_counter_ - 1] = { detail::Waiting, stop, bus->bus_number };
		vertexId_info[vertex_counter_] = { detail::Travel, stop, bus->bus_number };
		++vertex_counter_;

		return { Stop_and_vertexId_.at(stop).waiting,
			Stop_and_vertexId_.at(stop).travel };
	}

	void Catalogue::AddEdgeForCircularBus(transport::detail::Bus& bus) {
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
					const auto& first_stop = name_of_stop_.at(bus.stops[i]->name_of_stop);
					const auto& second_stop = name_of_stop_.at(bus.stops[nomber_of_second_st]->name_of_stop);

					int nomber_before_second_st = nomber_of_second_st - 1;
					if (nomber_of_second_st == 0) {
						nomber_before_second_st = iterations - 1;
					}
					const auto& begore_second_stop = name_of_stop_.at(bus.stops[nomber_before_second_st]->name_of_stop);

					const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrСreate(first_stop, &bus);
					const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrСreate(second_stop, &bus);

					++number_of_stops;
					if (lengths.count({ begore_second_stop,second_stop })) {
						distance += lengths.at({ begore_second_stop,second_stop });
					}
					else {
						distance += lengths.at({ second_stop,begore_second_stop });
					}

					//из стоп в тревел
					edge_range_info[{travel_first_st, waiting_second_st}].stations_passed = number_of_stops;
					edge_range_info[{travel_first_st, waiting_second_st}].bus_name = bus.bus_number;
					edge.from = travel_first_st;
					edge.to = waiting_second_st;
					edge.weight = (distance * 0.06) / routing_settings_.bus_velocity;
					graph_.AddEdge(edge);

					if (first_stop == second_stop) {
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
				}
			}
		}
	}

	void Catalogue::IterationAddingEdge(int from, int to, int& distance, int& number_of_stops, transport::detail::Bus& bus) {
		graph::Edge<double> edge;
		const auto& first_stop = name_of_stop_.at(bus.stops[from]->name_of_stop);
		const auto& second_stop = name_of_stop_.at(bus.stops[to]->name_of_stop);

		int nomber_begore_second_stop = to - 1;
		if (from>to) {
			nomber_begore_second_stop = to + 1;
		}
		const auto& begore_second_stop = name_of_stop_.at(bus.stops[nomber_begore_second_stop]->name_of_stop);

		const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrСreate(first_stop, &bus);
		const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrСreate(second_stop, &bus);

		++number_of_stops;
		if (lengths.count({ begore_second_stop,second_stop })) {
			distance += lengths.at({ begore_second_stop,second_stop });
		}
		else {
			distance += lengths.at({ second_stop,begore_second_stop });
		}

		//из стоп в тревел
		edge_range_info[{travel_first_st, waiting_second_st}].stations_passed= number_of_stops;
		edge_range_info[{travel_first_st, waiting_second_st}].bus_name = bus.bus_number;
		edge.from = travel_first_st;
		edge.to = waiting_second_st;
		edge.weight = (distance * 0.06) / routing_settings_.bus_velocity;
		graph_.AddEdge(edge);

		if (first_stop == second_stop) {
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
	}

	void Catalogue::AddEdgeForNotCircularBus(transport::detail::Bus& bus) {
		if (!bus.stops.empty()) {
			int iterations = bus.stops.size();
			for (int i = 0; i < iterations; ++i) {
				int distance = 0;
				int number_of_stops = 0;
				for (int n = i + 1; n < iterations; ++n) {
					IterationAddingEdge(i, n,number_of_stops, distance, bus);
				}
			}		
			for (int i = iterations-1; i >0; --i) {
				int distance = 0;
				int number_of_stops = 0;
				for (int n = i-1 ; n >= 0; --n) {
					IterationAddingEdge(i, n, number_of_stops, distance, bus);
				}
			}
		}
	}

	detail::EdgeInfo Catalogue::GetEdgeInfo(graph::VertexId from, graph::VertexId to) {
		return edge_range_info.at({ from,to });
	}
	void Catalogue::SetRoutingSettings(detail::RoutingSettings settings) {		
		routing_settings_ = settings;
	}
	detail::RoutingSettings Catalogue::GetRoutingSettings() {
		return routing_settings_;
	}

	void Catalogue::BuildGraph() {
		if (graph_.IsComplete()) {
			return;
		}

		graph_ = Graph(stops_.size()*2);

		for (auto& bus : busses_) {
			int stop_counter = 0;
			bool first_end_stop = true;
			//bool second_end_stop = true;
			auto stop_adr = ::transport::detail::Stop();
			::transport::detail::Stop* last_step = &stop_adr;

			for (const auto& stop : bus.stops) {
				++stop_counter;

				double distance = 0;

				if (last_step->name_of_stop.empty()) {
					last_step = stop;
					continue;
				}
				if (lengths.count({ last_step,stop })) {
					distance += lengths.at({ last_step,stop });
				}
				else {
					distance += lengths.at({ stop,last_step });
				}

				graph::Edge<double> edge;

				if (first_end_stop) {
					first_end_stop = false;
					if (!Stop_and_vertexId_.count(last_step)) {
						const auto& [waiting_first_st, travel_first_st] = FindInVertexIdCounterOrСreate(last_step, &bus);
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

				if (!Stop_and_vertexId_.count(stop)) {
					const auto& [waiting_second_st, travel_second_st] = FindInVertexIdCounterOrСreate(stop, &bus);
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
				AddEdgeForCircularBus(bus);
			}
			else {
				AddEdgeForNotCircularBus(bus);
			}

		}

		graph_.SetComplete();
	}

	Catalogue::Graph& Catalogue::GetGraph() {
		return graph_;
	}

	std::optional <std::pair<detail::VertexIdType, detail::VertexIdType>> Catalogue::Get_range_vertexId(const std::string& from, const std::string& to) {
		if (name_of_stop_.count(from) && name_of_stop_.count(to)) {
			if (Stop_and_vertexId_.count(name_of_stop_.at(from)) && Stop_and_vertexId_.count(name_of_stop_.at(to))) {
				detail::VertexIdType& vertID_from = Stop_and_vertexId_.at(name_of_stop_.at(from));
				detail::VertexIdType& vertID_to = Stop_and_vertexId_.at(name_of_stop_.at(to));
				return std::pair<detail::VertexIdType, detail::VertexIdType>{vertID_from, vertID_to};
			}
		}
		return {};
	}

	detail::VertexIdInfo Catalogue::Get_Stop_Info_By_VertexId(graph::VertexId verdex_id) {
		return vertexId_info.at(verdex_id);
	}