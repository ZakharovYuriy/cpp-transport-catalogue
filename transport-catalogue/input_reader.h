#pragma once

#include <string>
#include "transport_catalogue.h"
#include "geo.h"
namespace transport {
	namespace detail {
		double StringToDouble(const std::string& s);
	}
	namespace user_interaction {
		void ReadDataBase(int number_of_requests_creature, ::transport::Catalogue& transport);
	}
}