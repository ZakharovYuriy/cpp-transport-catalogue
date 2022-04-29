#include <charconv>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geo.h"
#include "stat_reader.h"


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

        void ResultOutputBus(const ::transport::detail::BusInfo& bus) {
            if (bus.not_empty) {
                std::cout << "Bus " << bus.bus_nomber << ": " << bus.number_of_stops << " stops on route, " << bus.unic_stops
                    << " unique stops, " << std::setprecision(6) << bus.real_distance << " route length, " << bus.distance << " curvature" << std::endl;
            }
            else {
                ::transport::user_interaction::BadResultBus(bus.bus_nomber);
            }
        }

        void ResultOutputStop(const detail::StopInfo& stop) {
            if (stop.exist) {
                if (!stop.buses.empty()) {
                    std::cout << "Stop " << stop.stop_name << ": buses ";
                    for (auto bus : stop.buses) {
                        std::cout << bus << " ";
                    }
                    std::cout << std::endl;
                }
                else {
                    BadResultNoBusses(stop.stop_name);
                }
            }
            else {
                BadResultStop(stop.stop_name);
            }
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
