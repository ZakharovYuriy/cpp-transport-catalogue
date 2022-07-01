#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <map>
#include <vector>

#include "geo.h"
#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
using namespace std::string_literals;

namespace transport {
    namespace svg {
        Path::Path(::transport::detail::Bus* bus, const SphereProjector& project, const ::transport::svg::detail::PathSettings& settings){
            for (const auto& stop : bus->stops) {
                points_.push_back(project(stop->coordinat));
            }

            if (!bus->is_circular) {
                if (points_.size() > 1) {
                    std::deque<::svg::Point> reverce_points;
                    for (auto iter = points_.begin(); iter < points_.end() - 1; ++iter) {
                        reverce_points.push_front(*iter);
                    }
                    for (const auto& point : reverce_points) {
                        points_.push_back(point);
                    }
                }
            }

            for (const auto& point : points_) {
                polyline_.AddPoint(point);
            }
            polyline_.SetFillColor(settings.fill).SetStrokeColor(settings.stroke).SetStrokeLineCap(::svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND).SetStrokeWidth(settings.stroke_width);
        }

        void Path::Draw(::svg::ObjectContainer& container) const{
            container.Add(polyline_);
        }

        RouteNames::RouteNames(const std::string& name, const ::svg::Point& position, const ::transport::svg::detail::Settings& global_settings, const ::svg::Color& color) {           
            ::svg::Point offset = { global_settings.bus_label_offset[0],global_settings.bus_label_offset[1] };
            background_.SetPosition(position).SetOffset(offset).SetFontSize(global_settings.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetData(name);
            title_ = background_;
            background_.SetFillColor(global_settings.underlayer_color).SetStrokeColor(global_settings.underlayer_color).SetStrokeWidth(global_settings.underlayer_width)
                .SetStrokeLineCap(::svg::StrokeLineCap::ROUND).SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND);
            title_.SetFillColor(color);
        }

        void RouteNames::Draw(::svg::ObjectContainer& container) const {
            container.Add(background_);
            container.Add(title_);
        }

        StopNames::StopNames(const std::string& name, const ::svg::Point& position, const ::transport::svg::detail::Settings& global_settings) {
            ::svg::Point offset = { global_settings.stop_label_offset[0],global_settings.stop_label_offset[1] };
            background_.SetPosition(position).SetOffset(offset).SetFontSize(global_settings.stop_label_font_size).SetFontFamily("Verdana"s).SetData(name);
            title_ = background_;
            background_.SetFillColor(global_settings.underlayer_color).SetStrokeColor(global_settings.underlayer_color).SetStrokeWidth(global_settings.underlayer_width)
                .SetStrokeLineCap(::svg::StrokeLineCap::ROUND).SetStrokeLineJoin(::svg::StrokeLineJoin::ROUND);
            title_.SetFillColor("black");
        }

        void StopNames::Draw(::svg::ObjectContainer& container) const {
            container.Add(background_);
            container.Add(title_);
        }
        CirclesOnPath::CirclesOnPath(::svg::Point point, const ::transport::svg::detail::Settings& global_settings) {
                ::svg::Circle circle;
                circle.SetCenter(point).SetRadius(global_settings.stop_radius).SetFillColor("white"s);
                circle_=::std::move(circle);
        }
        void CirclesOnPath::Draw(::svg::ObjectContainer& container) const {
                container.Add(circle_);
        }

        void MapRender::DrowMap(Catalogue& catalogue, ::svg::ObjectContainer& target) {
            list_of_busses_ = catalogue.GetOrderedListOfBuses();
            const auto& stop_coordinates = catalogue.GetStopsCoordinatesFromListOfBuses();
            SphereProjector project(stop_coordinates.begin(), stop_coordinates.end(), global_settings_.width, global_settings_.height, global_settings_.padding);
            long unsigned int bus_number_in_array = 0;
            for (const auto& bus : list_of_busses_) {
                if (bus->stops.empty()) {
                    continue;
                }

                if (bus_number_in_array >= global_settings_.color_palette.size()) {
                    bus_number_in_array = 0;
                }
                DrowPath(project, bus_number_in_array, bus, target);
                ++bus_number_in_array;
            }
            bus_number_in_array = 0;
            ::std::map<::std::string, ::svg::Point > name_stops;
            for (const auto& bus : list_of_busses_) {
                if (bus->stops.empty()) {
                    continue;
                }
                if (bus_number_in_array >= global_settings_.color_palette.size()) {
                    bus_number_in_array = 0;
                }
                DrowRouteNames(project, bus_number_in_array, bus, target);
                ++bus_number_in_array;
            }
            int size_of_array_no_repetitions = 0;
            for (const auto& bus : list_of_busses_) {
                if (bus->stops.empty()) {
                    continue;
                }                
                size_of_array_no_repetitions = bus->stops.size() - 1;
                for (int i = 0; i <= size_of_array_no_repetitions; ++i) {
                    if (name_stops.count(bus->stops[i]->name_of_stop)==0) {
                        name_stops[(bus->stops[i]->name_of_stop)] = project(bus->stops[i]->coordinat);
                    }
                }                
            }
            DrowCircles(target, name_stops);
           
            for (const auto& bus : list_of_busses_) {
                if (bus->stops.empty()) {
                    continue;
                }
                size_of_array_no_repetitions = bus->stops.size() - 1;
                for (int i = 0; i <= size_of_array_no_repetitions; ++i) {
                    if (name_stops.count(bus->stops[i]->name_of_stop) == 0) {
                        name_stops[(bus->stops[i]->name_of_stop)] = project(bus->stops[i]->coordinat);
                    }
                }                
            }
            DrowStopNames(target, name_stops);
        }

        void MapRender::DrowPath(const SphereProjector& project, long unsigned int& bus_number_in_array, ::transport::detail::Bus* bus, ::svg::ObjectContainer& target) {
            ::transport::svg::detail::PathSettings path_settings;
            path_settings.stroke = global_settings_.color_palette[bus_number_in_array];

            path_settings.stroke_width = global_settings_.line_width;

            Path path(bus, project, path_settings);
            path.Draw(target);
        }

        void MapRender::DrowRouteNames(const SphereProjector& project, long unsigned int& bus_number_in_array, ::transport::detail::Bus* bus, ::svg::ObjectContainer& target) {
            auto stop_begin = *(bus->stops.begin());
            ::svg::Point position = project(stop_begin->coordinat);
            RouteNames route_names(bus->bus_number, position, global_settings_, global_settings_.color_palette[bus_number_in_array]);
            route_names.Draw(target);

            if (!bus->is_circular) {
                auto stop_end = *(bus->stops.end()-1);
                if (stop_end != stop_begin) {
                    ::svg::Point position = project(stop_end->coordinat);
                    RouteNames route_names(bus->bus_number, position, global_settings_, global_settings_.color_palette[bus_number_in_array]);
                    route_names.Draw(target);
                }
            }           
        }

        void MapRender::DrowCircles(::svg::ObjectContainer& target, ::std::map<::std::string, ::svg::Point>& name_stops) {
            for (const auto& [stop_name, point] : name_stops) {
                CirclesOnPath circle(point, global_settings_);
                circle.Draw(target);
            }
        }

        void MapRender::DrowStopNames(::svg::ObjectContainer& target, ::std::map<::std::string, ::svg::Point>& name_stops) {
            for (const auto& [stop_name, point] : name_stops) {
                StopNames stop_names(stop_name, point, global_settings_);
                stop_names.Draw(target);
            }
        }
    }
}