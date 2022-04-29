#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

namespace transport {
	namespace user_interaction {
		std::pair< std::string, std::string> ReadRequests();
		void ResultOutputBus(const ::transport::detail::BusInfo& bus);
		void ResultOutputStop(const detail::StopInfo& stop);
		void BadResultBus(std::string_view name_bus);
		void BadResultStop(std::string_view name_bus);
		void BadResultNoBusses(std::string_view name_bus);
	}
}
