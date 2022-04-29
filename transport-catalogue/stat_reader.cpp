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
        std::pair< std::string, std::string> ReadRequests(std::istream& i_stream) {
            std::vector <std::string> result_query;
            std::string line = "";
            std::getline(i_stream, line);
            auto str = static_cast<std::string_view>(line);
            int64_t pos = 0;

            int64_t space = str.find(' ', pos);
            auto type = str.substr(pos, space - pos);
            pos = space + 1;

            auto bus_nomber = str.substr(pos);
            return { static_cast<std::string>(type), static_cast<std::string>(bus_nomber) };
        }

        void ResultOutputBus(std::ostream& o_stream, const ::transport::detail::BusInfo& bus) {
            if (bus.not_empty) {
                o_stream << "Bus " << bus.bus_nomber << ": " << bus.number_of_stops << " stops on route, " << bus.unic_stops
                    << " unique stops, " << std::setprecision(6) << bus.real_distance << " route length, " << bus.distance << " curvature" << std::endl;
            }
            else {
                ::transport::user_interaction::BadResultBus(o_stream, bus.bus_nomber);
            }
        }

        void ResultOutputStop(std::ostream& o_stream, const detail::StopInfo& stop) {
            if (stop.exist) {
                if (!stop.buses.empty()) {
                    o_stream << "Stop " << stop.stop_name << ": buses ";
                    for (auto bus : stop.buses) {
                        o_stream << bus << " ";
                    }
                    o_stream << std::endl;
                }
                else {
                    BadResultNoBusses(o_stream, stop.stop_name);
                }
            }
            else {
                BadResultStop(o_stream, stop.stop_name);
            }
        }

        void BadResultBus(std::ostream& o_stream, const std::string_view name_bus) {
            o_stream << "Bus " << name_bus << ": " << "not found" << std::endl;
        }

        void BadResultStop(std::ostream& o_stream, const std::string_view name_stop) {
            o_stream << "Stop " << name_stop << ": " << "not found" << std::endl;
        }

        void BadResultNoBusses(std::ostream& o_stream, const std::string_view name_stop) {
            o_stream << "Stop " << name_stop << ": " << "no buses" << std::endl;
        }
    }
}
