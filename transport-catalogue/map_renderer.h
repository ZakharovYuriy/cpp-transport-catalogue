#pragma once

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <deque>
#include <vector>

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace transport {
    namespace svg {
        inline const double EPSILON = 1e-6;
        inline bool IsZero(double value) {
             return std::abs(value) < EPSILON;
         }

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding);

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            ::svg::Point operator()(geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };

        class Path final: public ::svg::Drawable {
        public:
            Path(::transport::detail::Bus* bus, const SphereProjector& project, const ::transport::svg::detail::PathSettings& settings) ;
            void Draw(::svg::ObjectContainer& container) const override;

        private:
            ::svg::Polyline polyline_;
            std::vector<::svg::Point> points_;
        };

        class RouteNames final : public ::svg::Drawable {
        public:
            RouteNames(const std::string& name, const ::svg::Point& position, const ::transport::svg::detail::Settings& global_settings, const ::svg::Color& color);
            void Draw(::svg::ObjectContainer& container) const override;

        private:
            ::svg::Text background_, title_;
        };

        class StopNames final : public ::svg::Drawable {
        public:
            StopNames(const std::string& name, const ::svg::Point& position, const ::transport::svg::detail::Settings& global_settings);
            void Draw(::svg::ObjectContainer& container) const override;

        private:
            ::svg::Text background_, title_;
        };

        class CirclesOnPath final : public ::svg::Drawable {
        public:
            CirclesOnPath(::svg::Point point, const ::transport::svg::detail::Settings& global_settings);
            void Draw(::svg::ObjectContainer& container) const override;
        private:
            ::svg::Circle circle_;
        };

        class MapRender {
        public: 
            MapRender() = default;
            MapRender(::transport::svg::detail::Settings global_settings) :global_settings_(global_settings) {
            };

            void DrowMap(Catalogue& catalogue, ::svg::ObjectContainer& target);

        private:
            ::transport::svg::detail::Settings global_settings_;
            std::deque<::transport::detail::Bus*> list_of_busses_;

            void DrowPath(const SphereProjector& project, long unsigned int& bus_nomber_in_array, ::transport::detail::Bus* bus, ::svg::ObjectContainer& target);
            void DrowRouteNames(const SphereProjector& project, long unsigned int& bus_nomber_in_array, ::transport::detail::Bus* bus, ::svg::ObjectContainer& target);
            void DrowCircles(::svg::ObjectContainer& target, ::std::map<::std::string, ::svg::Point>& name_stops);
            void DrowStopNames(::svg::ObjectContainer& target, ::std::map<::std::string, ::svg::Point>& name_stops);
        };

        
    }
}

namespace transport {
    namespace svg {
        template <typename PointInputIt>
        SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding): padding_(padding){
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }
    }
}

