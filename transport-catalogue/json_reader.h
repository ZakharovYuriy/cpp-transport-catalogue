#pragma once

#include <iostream>

#include "json.h"
#include "transport_catalogue.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

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

			void ReadBaseRequests(::json::Node& stops_and_buses, Catalogue& catalogue);
			void SetStatRequests(::json::Node& requests);
			::json::Document GetOutputDoc(Catalogue& catalogue);
			::transport::svg::detail::Settings ReadRenderSettings(::json::Node& requests);
		};
	}
}