#pragma once

#include <string>
#include "transport_catalogue.h"
#include "geo.h"

namespace transport {
	namespace detail {
		double StringToDouble(const std::string& s);
		std::pair<bool, std::vector<std::string>> ReadBuses(std::string_view str, int64_t& pos);
		void ReadStopParameters(int64_t& pos, const std::string_view str, std::string_view name, ::transport::Catalogue& transport);
		std::pair<std::string_view, std::string_view> ReadRequestBeginning(std::string_view str, int64_t& pos);
	}

	namespace user_interaction {
		void ReadDataBase(std::istream& i_stream, int number_of_requests_create, ::transport::Catalogue& transport);
		void RequestToTheDatabase(std::ostream& o_stream, int number_of_requests, ::transport::Catalogue& transport);
	}
}