#include "log_duration.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "map_renderer.h"

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport {
	namespace json {
		void Reader::ReadDocumentInCatalogue(std::istream& i_stream, Catalogue& catalogue) {
			using namespace std::string_literals;

			::json::Dict dict = ::json::Load(i_stream).GetRoot().AsMap();
			if (dict.count("base_requests"s)) {
				ReadBaseRequests(dict.at("base_requests"s), catalogue);
			}
			if (dict.count("stat_requests"s)) {
				SetStatRequests(dict.at("stat_requests"s));
			}
			if (dict.count("render_settings"s)) {
				render_settings_ = ReadRenderSettings(dict.at("render_settings"s));
			}
		}

		void Reader::ResponseToRequests(std::ostream& output, Catalogue& catalogue) {
			::json::Print(GetOutputDoc(catalogue), output);
		}

		::transport::svg::detail::Settings& Reader::GetRenderSettings() {
			return render_settings_;
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
			for (const auto& [name, distance] : map_stop_or_bus.at("road_distances"s).AsMap()) {
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
					const auto& map_stop_or_bus = stop_or_bus.AsMap();
					if (map_stop_or_bus.at("type"s).AsString() == "Stop"s) {
						StopRequest(map_stop_or_bus, catalogue);
					}
				}
				for (const auto& stop_or_bus : stops_and_buses.AsArray()) {
					const auto& map_stop_or_bus = stop_or_bus.AsMap();
					if (map_stop_or_bus.at("type"s).AsString() == "Bus"s) {
						BusRequest(map_stop_or_bus, catalogue);
					}
				}

			}
		}

		void Reader::SetStatRequests(::json::Node& requests) {
			using namespace std::string_literals;
			if (requests.IsArray()) {
				for (const auto& request : requests.AsArray()) {
					::json::Dict dict = request.AsMap();
					::transport::json::detail::Request
						req(dict.at("id"s).AsInt(), dict.at("type"s).AsString());

					if (dict.count("name"s)) {
						req.name = dict.at("name"s).AsString();
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
			::json::Dict answer;
			if (!request.name.empty() && catalogue.GetBusInfo(request.name).not_empty) {
				auto bus_info = catalogue.GetBusInfo(request.name);
				answer["curvature"s] = ::json::Node{ bus_info.distance };
				answer["request_id"s] = ::json::Node{ request.id };
				answer["route_length"s] = ::json::Node{ bus_info.real_distance };
				answer["stop_count"s] = ::json::Node{ bus_info.number_of_stops };
				answer["unique_stop_count"s] = ::json::Node{ bus_info.unic_stops };
			}
			else {
				answer["request_id"s] = ::json::Node{ request.id };
				answer["error_message"s] = ::json::Node{ "not found"s };
			}
			return ::json::Node{ answer };
		}
		::json::Node Reader::ReadStopInfo(const json::detail::Request& request, Catalogue& catalogue) {
			using namespace std::string_literals;
			::json::Dict answer;
			if (!request.name.empty() && catalogue.GetStopInfo(request.name).exist) {
				auto stop_info = catalogue.GetStopInfo(request.name);
				::json::Array buses;
				for (const auto& bus : stop_info.buses) {
					buses.push_back(::json::Node{ static_cast<std::string>(bus) });
				}
				answer["buses"s] = ::json::Node{ buses };
				answer["request_id"s] = ::json::Node{ request.id };
			}
			else {
				answer["request_id"s] = ::json::Node{ request.id };
				answer["error_message"s] = ::json::Node{ "not found"s };
			}
			return ::json::Node{ answer };
		}

		::json::Node Reader::ReadMapInfo(const json::detail::Request& request, Catalogue& catalogue) {
			using namespace std::string_literals;
			::json::Dict answer;

			::svg::Document doc;
			::transport::svg::MapRender map_render(render_settings_);
			map_render.DrowMap(catalogue, doc);
			std::ostringstream out;
			doc.Render(out);

			answer["map"s] = ::json::Node{ out.str() };
			answer["request_id"s] = ::json::Node{ request.id };

			return ::json::Node{ answer };
		}

		::json::Document Reader::GetOutputDoc(Catalogue& catalogue) {
			using namespace std::string_literals;
			::json::Array RequestsResponse;

			for (const auto& request : requests_) {
				if (request.type == ::transport::json::detail::RequestType::Bus) {
					RequestsResponse.push_back(ReadBusInfo(request, catalogue));
				}
				else if (request.type == ::transport::json::detail::RequestType::Stop) {
					RequestsResponse.push_back(ReadStopInfo(request, catalogue));
				}
				else if (request.type == ::transport::json::detail::RequestType::Map) {
					RequestsResponse.push_back(ReadMapInfo(request, catalogue));
				}
			}
			::json::Document doc(::json::Node{ RequestsResponse });
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
			if (requests.IsMap()) {
				const ::json::Dict& dict = requests.AsMap();
				::transport::svg::detail::Settings settings;

				settings.width = dict.at("width"s).AsDouble();
				settings.height = dict.at("height"s).AsDouble();

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