#pragma once

#include <string>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport {
	namespace detail {
		double StringToDouble(const std::string& s);
		std::pair<bool, std::vector<std::string>> ReadBuses(std::string_view str, int64_t& pos);
		void ReadStopParameters(int64_t& pos, const std::string_view str, const std::string_view name, ::transport::Catalogue& transport);
		std::pair<std::string_view, std::string_view> ReadRequestBeginning(const std::string_view str, int64_t& pos);
	}

	namespace user_interaction {
		void ReadDataBase(std::istream& i_stream, ::transport::Catalogue& transport);
		void RequestToTheDatabase(std::istream& i_stream, std::ostream& o_stream, ::transport::Catalogue& transport);
	}
}