#pragma once

#include <string>
#include "transport_catalogue.h"
#include "geo.h"
namespace transport {
	namespace detail {
		double StringToDouble(const std::string& s);
	}
	namespace user_interaction {
		void ReadDataBase(std::istream& i_stream, int number_of_requests_create, ::transport::Catalogue& transport);
		void RequestToTheDatabase(std::istream& i_stream, int number_of_requests, ::transport::Catalogue& transport);
	}
}