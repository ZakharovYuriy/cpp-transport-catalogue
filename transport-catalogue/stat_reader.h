#pragma once
#include <string>
#include <vector>
#include <string_view>

namespace transport {
	namespace user_interaction {
		std::pair< std::string, std::string> ReadRequests();
		void ResultOutputBus(std::string_view name_bus, int number_of_stops, int unic_stops, int real_distance, double distance);
		void ResultOutputStop(std::string_view name_bus, std::vector<std::string_view> buses);
		void BadResultBus(std::string_view name_bus);
		void BadResultStop(std::string_view name_bus);
		void BadResultNoBusses(std::string_view name_bus);
	}
}