#include "serialization.h"
#include "json_reader.h"
#include "router.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <iostream>
#include <vector>

namespace protobuf {
	void SerializeCatalogStops(const ::transport::Catalogue& catalog, transport_catalogue_serialize::Catalog& to_send_catalog) {
		for (const auto& stop : catalog.GetStops()) {
			transport_catalogue_serialize::Stop temp_stop;
			temp_stop.set_name_of_stop(stop.name_of_stop);

			transport_catalogue_serialize::Coordinat temp_coordinat;
			temp_coordinat.set_lat(stop.coordinat.lat);
			temp_coordinat.set_lng(stop.coordinat.lng);

			*temp_stop.mutable_coordinat() = temp_coordinat;

			*to_send_catalog.add_stops_list() = temp_stop;
		}
	}

	void SerializeCatalogBusses(const ::transport::Catalogue& catalog, transport_catalogue_serialize::Catalog& to_send_catalog) {
		for (const auto& bus : catalog.GetBusses()) {
			transport_catalogue_serialize::Bus temp_bus;
			temp_bus.set_bus_number(bus.bus_number);
			temp_bus.set_is_circular(bus.is_circular);
			for (const auto& stop : bus.stops) {
				*temp_bus.add_stops_in_bus() = stop->name_of_stop;
			}
			*to_send_catalog.add_busses_list() = temp_bus;
		}
	}

	void SerializeCatalogLength(const ::transport::Catalogue& catalog, transport_catalogue_serialize::Catalog& to_send_catalog) {
		for (const auto& [stations, length] : catalog.GetLengths()) {
			const auto& [st_from, st_to] = stations;
			transport_catalogue_serialize::Length temp_length;
			temp_length.set_stop_from(st_from->name_of_stop);
			temp_length.set_stop_to(st_to->name_of_stop);
			temp_length.set_length(length);
			*to_send_catalog.add_lengths_list() = temp_length;
		}
	}

	transport_catalogue_serialize::Catalog SerializeCatalog(const ::transport::Catalogue& catalog) {
		transport_catalogue_serialize::Catalog to_send_catalog;
		SerializeCatalogStops(catalog, to_send_catalog);
		SerializeCatalogBusses(catalog, to_send_catalog);
		SerializeCatalogLength(catalog, to_send_catalog);
		return to_send_catalog;
	}

	svg_serialize::RenderSettings SerializeSVG(const ::transport::json::Reader& reader) {
		svg_serialize::RenderSettings render_settings;
		::transport::svg::detail::Settings settings = (const_cast<::transport::json::Reader&>(reader)).RenderSettings();//изменения в RenderSettings вноситься не будут 

		render_settings.set_width(settings.width);
		render_settings.set_hight(settings.hight);
		render_settings.set_padding(settings.padding);
		render_settings.set_line_width(settings.line_width);
		render_settings.set_stop_radius(settings.stop_radius);
		render_settings.set_bus_label_font_size(settings.bus_label_font_size);
		render_settings.set_bus_label_offset_x(settings.bus_label_offset[0]);
		render_settings.set_bus_label_offset_y(settings.bus_label_offset[1]);
		render_settings.set_stop_label_font_size(settings.stop_label_font_size);
		render_settings.set_stop_label_offset_x(settings.stop_label_offset[0]);
		render_settings.set_stop_label_offset_y(settings.stop_label_offset[1]);
		render_settings.set_underlayer_color(settings.underlayer_color);
		render_settings.set_underlayer_width(settings.underlayer_width);
		for (const auto& color : settings.color_palette) {
			*render_settings.add_color_palette() = color;
		}

		return render_settings;
	}

	router_serialize::BuildGraph SerializeRouter(const ::transport::json::Reader& reader) {
		router_serialize::BuildGraph serializing_router;
		auto builded_router = reader.GetGraphBuilder();

		router_serialize::RoutingSettings ser_settings;
		ser_settings.set_bus_wait_time(builded_router.GetRoutingSettings().bus_wait_time);
		ser_settings.set_bus_velocity(builded_router.GetRoutingSettings().bus_velocity);

		*serializing_router.mutable_routing_settings() = ser_settings;

		const auto& router_internal_data = builded_router.GetRouter().GetRoutesInternalData();//внутренние данные роутера
		for (const auto& vect_of_data : router_internal_data) {
			router_serialize::RoutesInternalData ser_vector_of_internal_data;
			for (const auto& internal_data : vect_of_data) {
				router_serialize::InternalData ser_internal_data;//внутренний вектор
				router_serialize::OptionalInternalData opt_data;
				if (internal_data.has_value()) {					
					ser_internal_data.set_weight(internal_data.value().weight);

					if (internal_data.value().prev_edge.has_value()) {
						ser_internal_data.set_prev_edge(internal_data.value().prev_edge.value());
						ser_internal_data.set_has_data(true);
					}
					else {
						ser_internal_data.set_has_data(false);
					}

					*opt_data.mutable_optional_internal_data() = ser_internal_data;
				}
				*ser_vector_of_internal_data.add_internal_data() = opt_data;
			}
			*serializing_router.add_router_internal_data() = ser_vector_of_internal_data;//ВНЕШНИЙ ВЕКТОР
		}

		return serializing_router;
	}

	void Serialize(const ::transport::Catalogue& catalog, const ::transport::json::Reader& reader, std::ostream& output) {	
		transport_catalogue_serialize::Data result_serialize;

		*result_serialize.mutable_catalog_settings() = SerializeCatalog(catalog);
		*result_serialize.mutable_svg_settings() = SerializeSVG(reader);
		*result_serialize.mutable_builded_router() = SerializeRouter(reader);

		result_serialize.SerializeToOstream(&output);
	}

	void DeSerializeCatalogStops(const transport_catalogue_serialize::Catalog& received_catalog, ::transport::Catalogue& new_catalog) {
		for (const auto& stop : received_catalog.stops_list()) {
			new_catalog.AddStop(stop.name_of_stop(), { stop.coordinat().lat(),stop.coordinat().lng() });
		}
	}

	void DeSerializeCatalogBusses(const transport_catalogue_serialize::Catalog& received_catalog, ::transport::Catalogue& new_catalog) {
		for (const auto& bus : received_catalog.busses_list()) {
			std::vector<std::string> stops;
			for (const auto& stop : bus.stops_in_bus()) {
				stops.push_back(stop);
			}
			new_catalog.AddBus(bus.bus_number(), bus.is_circular(), stops);
		}
	}
	
	void DeSerializeCatalogLength(const transport_catalogue_serialize::Catalog& received_catalog, ::transport::Catalogue& new_catalog) {
		for (const auto& length : received_catalog.lengths_list()) {
			new_catalog.SetDistancesToStop(length.stop_from(), { {length.stop_to(),length.length()} });
		}
	}

	void DeSerializeCatalog(const transport_catalogue_serialize::Catalog& received_catalog, ::transport::Catalogue& new_catalog) {
		DeSerializeCatalogStops(received_catalog, new_catalog);		
		DeSerializeCatalogBusses(received_catalog, new_catalog);		
		DeSerializeCatalogLength(received_catalog, new_catalog);
	}

	void DeSerializeSVG(const svg_serialize::RenderSettings& render_settings, ::transport::json::Reader& reader) {
		auto& settings = reader.RenderSettings();
		settings.width = render_settings.width();
		settings.hight = render_settings.hight();
		settings.padding = render_settings.padding();
		settings.line_width = render_settings.line_width();
		settings.stop_radius = render_settings.stop_radius();
		settings.bus_label_font_size = render_settings.bus_label_font_size();
		settings.bus_label_offset[0] = render_settings.bus_label_offset_x();
		settings.bus_label_offset[1] = render_settings.bus_label_offset_y();
		settings.stop_label_font_size = render_settings.stop_label_font_size();
		settings.stop_label_offset[0] = render_settings.stop_label_offset_x();
		settings.stop_label_offset[1] = render_settings.stop_label_offset_y();
		settings.underlayer_color = render_settings.underlayer_color();
		settings.underlayer_width = render_settings.underlayer_width();
		for (const auto& color : render_settings.color_palette()) {
			settings.color_palette.push_back(color);
		}
	}

	void DeSerializeRouter(const router_serialize::BuildGraph& receive_router, const ::transport::Catalogue& catalog, ::transport::json::Reader& reader) {
		auto& builder = reader.GetForChangeGraphBuilder();

		::transport::detail::RoutingSettings settings;
		settings.bus_velocity = receive_router.routing_settings().bus_velocity();
		settings.bus_wait_time = receive_router.routing_settings().bus_wait_time();

		std::vector < std::vector <std::optional<graph::SimpleInternalData>>> routes_internal_data;

		for (const auto& ser_routes_internal_data : receive_router.router_internal_data()) {
			std::vector <std::optional<graph::SimpleInternalData>> buff;

			for (const auto& ser_internal_data : ser_routes_internal_data.internal_data()) {
				graph::SimpleInternalData data;
				if(ser_internal_data.has_optional_internal_data()){	
					const auto& int_data = ser_internal_data.optional_internal_data();
					data.weight = int_data.weight();
					if (int_data.has_data()) {
						data.prev_edge = ser_internal_data.optional_internal_data().prev_edge();
					}					 
				}
				buff.push_back(data);
			}
			routes_internal_data.push_back(buff);
		}

		graph::Router<double> regain_router (builder.BuildGraph(catalog, settings), routes_internal_data);

		builder.SetRouter(regain_router);
	}

	void DeSerialize(::transport::Catalogue& new_catalog, ::transport::json::Reader& reader, std::istream& input) {
		transport_catalogue_serialize::Data received_data;
		received_data.ParseFromIstream(&input);

		DeSerializeCatalog(received_data.catalog_settings(), new_catalog);
		DeSerializeSVG(received_data.svg_settings(), reader);
		DeSerializeRouter(received_data.builded_router(), new_catalog, reader);
	}
}