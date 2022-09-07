//#include "log_duration.h"

#include <iostream>
#include <optional>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "map_renderer.h"

#include "json.h"
#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport {
	namespace json {
		void Reader::ReadDocumentInCatalogue(std::istream& i_stream, Catalogue& catalogue) {
			using namespace std::string_literals;

			::json::Dict dict = ::json::Load(i_stream).GetRoot().AsDict();
			if (dict.count("base_requests"s)) {
				ReadBaseRequests(dict.at("base_requests"s), catalogue);
			}
			if (dict.count("stat_requests"s)) {
				SetStatRequests(dict.at("stat_requests"s));
			}
			if (dict.count("render_settings"s)) {
				render_settings_ = ReadRenderSettings(dict.at("render_settings"s));
			}
			if (dict.count("routing_settings"s)) {
				ReadRoutingSettings(dict.at("routing_settings"s), catalogue);
			}
			if (dict.count("serialization_settings"s)) {
				ReadSerializationSettings(dict.at("serialization_settings"s));
			}
		}

		void Reader::ResponseToRequests(std::ostream& output, Catalogue& catalogue) {
			::json::Print(GetOutputDoc(catalogue), output);
		}

		::transport::svg::detail::Settings& Reader::RenderSettings() {
			return render_settings_;
		}

		std::filesystem::path Reader::GetSerializePath() {
			return serialize_path_;
		}

		GraphBuilder  Reader::GetGraphBuilder() const {
			return builded_graph_;
		}

		GraphBuilder&  Reader::GetForChangeGraphBuilder() {
			return builded_graph_;
		}

		void Reader::StopRequest(const ::json::Dict& map_stop_or_bus, Catalogue& catalogue) {
			using namespace std::string_literals;
			::geo::Coordinates coordinat;
			coordinat.lat = map_stop_or_bus.at("latitude"s).AsDouble();
			coordinat.lng = map_stop_or_bus.at("longitude"s).AsDouble();
			catalogue.AddStop(
				map_stop_or_bus.at("name"s).AsString(),
				std::move(coordinat)
			);

			std::unordered_map<std::string_view, int> real_distances;
			for (const auto& [name, distance] : map_stop_or_bus.at("road_distances"s).AsDict()) {
				real_distances[name] = distance.AsInt();
			}
			catalogue.SetDistancesToStop(map_stop_or_bus.at("name"s).AsString(), real_distances);
		}

		void Reader::BusRequest(const ::json::Dict& map_stop_or_bus, Catalogue& catalogue) {
			using namespace std::string_literals;
			std::vector<std::string> stops;
			for (const auto& stop : map_stop_or_bus.at("stops"s).AsArray()) {
				stops.push_back(stop.AsString());
			}
			catalogue.AddBus(
				map_stop_or_bus.at("name"s).AsString(),
				map_stop_or_bus.at("is_roundtrip"s).AsBool(),
				move(stops)
			);
		}

		void Reader::ReadBaseRequests(::json::Node& stops_and_buses, Catalogue& catalogue) {
			using namespace std::string_literals;
			if (!stops_and_buses.AsArray().empty()) {
				for (const auto& stop_or_bus : stops_and_buses.AsArray()) {
					const auto& map_stop_or_bus = stop_or_bus.AsDict();
					if (map_stop_or_bus.at("type"s).AsString() == "Stop"s) {
						StopRequest(map_stop_or_bus, catalogue);
					}
				}
				for (const auto& stop_or_bus : stops_and_buses.AsArray()) {
					const auto& map_stop_or_bus = stop_or_bus.AsDict();
					if (map_stop_or_bus.at("type"s).AsString() == "Bus"s) {
						BusRequest(map_stop_or_bus, catalogue);
					}
				}
				if (routing_settings_.has_set) {
					if (!builded_graph_.GetGraph().IsComplete()) {
						builded_graph_ = GraphBuilder(catalogue, routing_settings_.bus_wait_time, routing_settings_.bus_velocity);
						builded_graph_.SetRouter(graph::Router<double>(&builded_graph_.GetGraph()));
					}				
				}
			}
		}

		void Reader::ReadRoutingSettings(::json::Node& settings, Catalogue& catalogue) {
			using namespace std::string_literals;
			if (!settings.AsDict().empty()) {
				const auto& map_settings = settings.AsDict();
				routing_settings_.bus_velocity = map_settings.at("bus_velocity"s).AsDouble();
				routing_settings_.bus_wait_time = map_settings.at("bus_wait_time"s).AsInt();
				routing_settings_.has_set = true;
				if (!builded_graph_.GetGraph().IsComplete()) {
					builded_graph_ = GraphBuilder(catalogue, routing_settings_.bus_wait_time, routing_settings_.bus_velocity);
					builded_graph_.SetRouter(graph::Router<double>(&builded_graph_.GetGraph()));
				}
			}
		}

		void Reader::ReadSerializationSettings(::json::Node& settings) {
			using namespace std::string_literals;
			if (!settings.AsDict().empty()) {
				const auto& serialization_settings = settings.AsDict();
				serialize_path_ = std::filesystem::path(serialization_settings.at("file"s).AsString());
			}
		}

		void Reader::SetStatRequests(::json::Node& requests) {
			using namespace std::string_literals;
			if (requests.IsArray()) {
				for (const auto& request : requests.AsArray()) {
					::json::Dict dict = request.AsDict();
					::transport::json::detail::Request
						req(dict.at("id"s).AsInt(), dict.at("type"s).AsString());

					if (dict.count("name"s)) {
						req.name = dict.at("name"s).AsString();
					}
					if (dict.count("from"s)) {
						json::detail::RouteRange route_range(dict.at("from"s).AsString(),dict.at("to"s).AsString());
						req.route_range = route_range;
					}
					requests_.push_back(req);
				}
			}
			else {
				throw std::invalid_argument("Wrong SetStatRequests type"s);
			}
		}

		::json::Node Reader::ReadBusInfo(const json::detail::Request& request, Catalogue& catalogue) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartDict();
			if (request.name.has_value() && catalogue.GetBusInfo(request.name.value()).not_empty) {
				auto bus_info = catalogue.GetBusInfo(request.name.value());
				builder.Key("curvature"s).Value(bus_info.distance).
					Key("request_id"s).Value(request.id).
					Key("route_length"s).Value(bus_info.real_distance).
					Key("stop_count"s).Value(bus_info.number_of_stops).
					Key("unique_stop_count"s).Value(bus_info.unic_stops);
			}
			else {
				builder.Key("request_id"s).Value(request.id).
					Key("error_message"s).Value("not found"s);
			}
			builder.EndDict();
			return builder.Build();
		}
		::json::Node Reader::ReadStopInfo(const json::detail::Request& request, Catalogue& catalogue) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartDict();
			if (request.name.has_value() && catalogue.GetStopInfo(request.name.value()).exist) {
				auto stop_info = catalogue.GetStopInfo(request.name.value());
				auto buses = ::json::Builder{};
				buses.StartArray();
					for (const auto& bus : stop_info.buses) {
						buses.Value(static_cast<std::string>(bus));
					}
				buses.EndArray();

				builder.Key("buses"s).Value(buses.Build()).
					Key("request_id"s).Value(request.id);
			}
			else {
				builder.Key("request_id"s).Value(request.id).
					Key("error_message"s).Value("not found"s);
			}
			builder.EndDict();
			return builder.Build();
		}

		::json::Node Reader::ReadMapInfo(const json::detail::Request& request, Catalogue& catalogue) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};

			::svg::Document doc;
			::transport::svg::MapRender map_render(render_settings_);
			map_render.DrowMap(catalogue, doc);
			std::ostringstream out;
			doc.Render(out);

			builder.StartDict().
				Key("map"s).Value(out.str()).
				Key("request_id"s).Value(request.id).
			EndDict();

			return builder.Build();
		}

		::json::Node Reader::AddStopItem(std::string_view stop_name,double time) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartDict().
				Key("stop_name"s).Value(static_cast<std::string>(stop_name)).
				Key("time"s).Value(time).
				Key("type"s).Value("Wait"s).
			EndDict();
			return builder.Build();
		}

		::json::Node Reader::AddBussItem(std::string_view bus_name, int span_count, double time) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartDict().
				Key("bus"s).Value(static_cast<std::string>(bus_name)).
				Key("span_count"s).Value(span_count).
				Key("time"s).Value(time).
				Key("type"s).Value("Bus"s).
			EndDict();
			return builder.Build();
		}

		::json::Node Reader::GraphVertexHandler(const std::vector<graph::EdgeId>& edge_nomber) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartArray();

			for (const auto edgeId : edge_nomber) {
				const auto& edge = builded_graph_.GetGraph().GetEdge(edgeId);
				const auto& vertex_info_from = builded_graph_.Get_Stop_Info_By_VertexId(edge.from);

				if (vertex_info_from.vertex_type == ::transport::detail::VertexType::Waiting) {
					builder.Value(AddStopItem(vertex_info_from.stop_pointer->name_of_stop, edge.weight));
				}
				else {
					const auto& edge_info = builded_graph_.GetEdgeInfo(edge.from, edge.to);
					builder.Value(AddBussItem(edge_info.bus_name, edge_info.stations_passed, edge.weight));
				}
			}

			builder.EndArray();
			return builder.Build();
		}

		::json::Node Reader::ReadRoutInfo(const json::detail::Request& request) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartDict();

			if (builded_graph_.Get_range_vertexId(request.route_range.value().from,
				request.route_range.value().to).has_value()) {

				auto [vertID_from, vertID_to] = builded_graph_.Get_range_vertexId(request.route_range.value().from,
					request.route_range.value().to).value();

				auto rout = builded_graph_.GetRouter().BuildRoute(vertID_from.waiting, vertID_to.waiting);

				if (rout.has_value()) {
					builder.Key("request_id"s).Value(request.id)
						.Key("total_time"s).Value(rout.value().weight);
					builder.Key("items"s).Value(GraphVertexHandler(rout.value().edges));
				}
				else {
					builder.Key("error_message"s).Value("not found"s)
						.Key("request_id"s).Value(request.id);
				}
			}
			else {
				builder.Key("error_message"s).Value("not found"s)
					.Key("request_id"s).Value(request.id);
			}

			builder.EndDict();
			return builder.Build();
		}

		::json::Document Reader::GetOutputDoc(Catalogue& catalogue) {
			using namespace std::string_literals;
			auto builder = ::json::Builder{};
			builder.StartArray();

			for (const auto& request : requests_) {
				if (request.type == ::transport::json::detail::RequestType::Bus) {
					builder.Value(ReadBusInfo(request, catalogue));
				}
				else if (request.type == ::transport::json::detail::RequestType::Stop) {
					builder.Value(ReadStopInfo(request, catalogue));
				}
				else if (request.type == ::transport::json::detail::RequestType::Map) {
					builder.Value(ReadMapInfo(request, catalogue));
				}
				else if (request.type == ::transport::json::detail::RequestType::Route) {
					builder.Value(ReadRoutInfo(request));
				}
			}
			builder.EndArray();
			::json::Document doc(builder.Build());
			return doc;
		}

		std::string ColorToStr(const ::json::Node colors) {
			std::string color_str = "";
			if (colors.IsString()) {
				color_str = colors.AsString();
			}
			else if (colors.IsArray()) {
				using namespace std::string_literals;
				if (colors.AsArray().size() == 3) {
					color_str = "rgb("s;
					for (int8_t i = 0; i < 2; i++) {
						color_str += std::to_string(colors.AsArray()[i].AsInt());
						color_str += ',';
					}
					color_str += std::to_string(colors.AsArray()[2].AsInt());
					color_str += ')';
				}
				else {
					color_str = "rgba("s;
					for (int8_t i = 0; i < 3; i++) {
						color_str += std::to_string(colors.AsArray()[i].AsInt());
						color_str += ',';
					}
					std::ostringstream strs;
					strs << colors.AsArray()[3].AsDouble();
					color_str += strs.str();
					color_str += ')';
				}
			}
			return color_str;
		}

		::transport::svg::detail::Settings Reader::ReadRenderSettings(::json::Node& requests) {
			using namespace std::string_literals;
			if (requests.IsDict()) {
				const ::json::Dict& dict = requests.AsDict();
				::transport::svg::detail::Settings settings;

				settings.width = dict.at("width"s).AsDouble();
				settings.hight = dict.at("height"s).AsDouble();

				settings.padding = dict.at("padding"s).AsDouble();

				settings.line_width = dict.at("line_width"s).AsDouble();
				settings.stop_radius = dict.at("stop_radius"s).AsDouble();

				settings.bus_label_font_size = dict.at("bus_label_font_size"s).AsInt();
				settings.bus_label_offset = {
					dict.at("bus_label_offset"s).AsArray()[0].AsDouble(),
					dict.at("bus_label_offset"s).AsArray()[1].AsDouble()
				};

				settings.stop_label_font_size = dict.at("stop_label_font_size"s).AsInt();
				settings.stop_label_offset = {
					dict.at("stop_label_offset"s).AsArray()[0].AsDouble(),
					dict.at("stop_label_offset"s).AsArray()[1].AsDouble()
				};

				settings.underlayer_color = ColorToStr(dict.at("underlayer_color"s));
				settings.underlayer_width = dict.at("underlayer_width"s).AsDouble();

				for (auto const& color : dict.at("color_palette"s).AsArray()) {
					settings.color_palette.push_back(ColorToStr(color));
				}
				return settings;
			}
			else {
				throw std::invalid_argument("Wrong SetStatRequests type"s);
			}
		}

	}
}