#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

namespace transport {
	namespace user_interaction {
		std::pair< std::string, std::string> ReadRequests(std::istream& i_stream);
		void ResultOutputBus(std::ostream& o_stream, const ::transport::detail::BusInfo& bus);
		void ResultOutputStop(std::ostream& o_stream, const detail::StopInfo& stop);
		void BadResultBus(std::ostream& o_stream, const std::string_view name_bus);
		void BadResultStop(std::ostream& o_stream, const std::string_view name_bus);
		void BadResultNoBusses(std::ostream& o_stream, const std::string_view name_bus);
	}
}
