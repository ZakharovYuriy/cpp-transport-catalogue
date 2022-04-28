#pragma once

#include <string>
#include <vector>
#include <string_view>

#include "transport_catalogue.h"

namespace transport {
	namespace user_interaction {
		std::pair< std::string, std::string> ReadRequests();
		void ResultOutputBus(const ::transport::detail::BusInfo& bus);
		void ResultOutputStop(std::string_view name_bus, std::vector<std::string_view> buses);
		void BadResultBus(std::string_view name_bus);
		void BadResultStop(std::string_view name_bus);
		void BadResultNoBusses(std::string_view name_bus);
	}
}
