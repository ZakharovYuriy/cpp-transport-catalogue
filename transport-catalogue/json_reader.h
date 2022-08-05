#pragma once

#include <iostream>

#include "json.h"
#include "transport_catalogue.h"

namespace transport {
	namespace json {
		class Reader {
		public:
			void ReadDocumentInCatalogue(std::istream& i_stream, Catalogue& catalogue);
			void ResponseToRequests(std::ostream& output, Catalogue& catalogue);
			::transport::svg::detail::Settings& GetRenderSettings();

		private:
			std::vector<::transport::json::detail::Request> requests_;
			::transport::svg::detail::Settings render_settings_;

			void StopRequest(const::json::Dict&, Catalogue&);
			void BusRequest(const ::json::Dict&, Catalogue&);

			::json::Node ReadStopInfo(const json::detail::Request&, Catalogue&);
			::json::Node ReadBusInfo(const json::detail::Request&, Catalogue&);
			::json::Node ReadMapInfo(const json::detail::Request&, Catalogue&);
			::json::Node ReadRoutInfo(const json::detail::Request&, Catalogue&);

			::json::Node GraphVertexHandler(const std::vector<graph::EdgeId>& edge_nomber, Catalogue& catalogue);
			::json::Node AddStopItem(std::string_view stop_name, double time);
			::json::Node AddBussItem(std::string_view bus_name, int span_count, double time);

			void ReadBaseRequests(::json::Node& stops_and_buses, Catalogue& catalogue);
			void SetStatRequests(::json::Node& requests);
			void ReadRoutingSettings(::json::Node& settings, Catalogue& catalogue);
			::json::Document GetOutputDoc(Catalogue& catalogue);
			::transport::svg::detail::Settings ReadRenderSettings(::json::Node& requests);
		};
	}
}