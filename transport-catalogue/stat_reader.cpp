#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <string_view>
#include <charconv>
#include "transport_catalogue.h"
#include "stat_reader.h"

#include "geo.h"

namespace transport {
    namespace user_interaction {
        std::pair< std::string, std::string> ReadRequests() {
            std::vector <std::string> result_query;
            std::string line = "";
            std::getline(std::cin, line);
            auto str = static_cast<std::string_view>(line);
            int64_t pos = 0;

            int64_t space = str.find(' ', pos);
            auto type = str.substr(pos, space - pos);
            pos = space + 1;

            auto bus_nomber = str.substr(pos);
            return { static_cast<std::string>(type), static_cast<std::string>(bus_nomber) };
        }
        void ResultOutputBus(std::string_view name_bus, int number_of_stops, int unic_stops, int real_distance, double distance) {
            std::cout << "Bus " << name_bus << ": " << number_of_stops << " stops on route, " << unic_stops
                << " unique stops, " << std::setprecision(6) << real_distance << " route length, " << distance << " curvature" << std::endl;
        }
        void ResultOutputStop(std::string_view name_bus, std::vector<std::string_view> buses) {
            std::cout << "Stop " << name_bus << ": buses ";
            for (auto bus : buses) {
                std::cout << bus << " ";
            }
            std::cout << std::endl;
        }

        void BadResultBus(std::string_view name_bus) {
            std::cout << "Bus " << name_bus << ": " << "not found" << std::endl;
        }
        void BadResultStop(std::string_view name_stop) {
            std::cout << "Stop " << name_stop << ": " << "not found" << std::endl;
        }
        void BadResultNoBusses(std::string_view name_stop) {
            std::cout << "Stop " << name_stop << ": " << "no buses" << std::endl;
        }
    }
}
