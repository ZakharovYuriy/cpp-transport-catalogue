#pragma once

#include <array>
#include <string>
#include <vector>

#include "geo.h"
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace transport {
	namespace detail {
		struct Stop {
			Stop() = default;
			Stop(const std::string&& name, const ::geo::Coordinates& coord) :name_of_stop{ name }, coordinat{ coord }{
			}
			Stop(const std::string&& name) :name_of_stop{ name } {
			}
			std::string name_of_stop = "";
			::geo::Coordinates coordinat = { 0,0 };
		};

		struct Bus {
			std::string bus_nomber = "";
			bool is_circular = false;
			std::vector <Stop*> stops;
		};

		struct BusInfo {
			BusInfo() = delete;
			BusInfo(const std::string& nomber) :bus_nomber(nomber) {
			}
			BusInfo(const Bus& bus, const int number_st, const int unic_st, const int real_dist, const double dist) :
				number_of_stops(number_st), unic_stops(unic_st), real_distance(real_dist), distance(dist) {
				not_empty = true;
				bus_nomber = bus.bus_nomber;
			}
			std::string bus_nomber;
			int number_of_stops = 0;
			int unic_stops = 0;
			int real_distance = 0;
			double distance = 0;
			bool not_empty = false;
		};

		struct StopInfo {
			StopInfo() = delete;
			StopInfo(std::string name) :stop_name(name) {
			}
			StopInfo(const Stop* stop) :stop_name(stop->name_of_stop), exist(true) {
			}
			StopInfo(const Stop* stop, std::vector<std::string_view> buses_input) :buses(buses_input) {
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

	namespace json {
		namespace detail {
			enum RequestType {
				Bus,
				Stop,
				Map,
				NoType
			};

			struct Request {
				Request() = default;
				Request(int identificator, std::string type_str) :id(identificator) {
					if (type_str == "Map") {
						type = RequestType::Map;
					}
					if (type_str == "Bus") {
						type = RequestType::Bus;
					}
					if (type_str == "Stop") {
						type = RequestType::Stop;
					}
				}
				int id=0;
				RequestType type= RequestType::NoType;
				std::string name="";
			};
		}
	}

	namespace svg {
		namespace detail {
			struct Settings {				
				Settings() = default;

				double width = 0;
				double height = 0;
				double padding = 0;
				double line_width = 0;
				double stop_radius = 0;
				unsigned int  bus_label_font_size = 0;
				std::array <double, 2>  bus_label_offset = {0,0};
				int stop_label_font_size = 0;
				std::array <double, 2>  stop_label_offset = { 0,0 };
				std::string underlayer_color = "none";
				double underlayer_width = 0;
				std::vector<std::string> color_palette={};
			};

			struct PathSettings {
				PathSettings() = default;

				std::string stroke = "none";
				std::string fill = "none";
				double stroke_width = 0; 
			};
		}
	}
}