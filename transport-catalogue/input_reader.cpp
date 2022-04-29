#include <charconv>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace transport {
    namespace detail {
        double StringToDouble(const std::string& s) {
            std::istringstream i(s);
            double x;
            if (!(i >> x))
                return 0;
            return x;
        }

        std::pair<bool, std::vector<std::string>> ReadBuses(std::string_view str, int64_t& pos) {
            std::vector<std::string> stops;
            const int64_t pos_end = str.npos;
            int64_t space = str.find(',', pos);
            auto znak = std::find_if(str.begin() + pos, str.end(), [](auto word) { return word == '>' || word == '-'; });
            bool is_circular = false;
            if (*znak == '>') {
                is_circular = true;
            }
            --pos;
            while (true) {
                int64_t space = str.find(*znak, pos);
                stops.push_back(static_cast<std::string>(space == pos_end ? str.substr(pos + 1) : str.substr(pos + 1, space - 2 - pos)));
                if (space == pos_end) {
                    break;
                }
                else {
                    pos = space + 1;
                }
            }
            return { is_circular,stops };
        }

        void ReadStopParameters(int64_t& pos,const std::string_view str, const std::string_view name, ::transport::Catalogue& transport) {
            std::unordered_map<std::string_view, int> real_distances;
            const int64_t pos_end = str.npos;

            int64_t space = str.find(',', pos);
            auto lat = str.substr(pos, space - pos);
            pos = space + 2;

            auto lng = lat;
            space = str.find(',', pos);
            if (space == pos_end) {
                lng = str.substr(pos);
            }
            else {
                lng = str.substr(pos, space - pos);
                pos = space + 2;
                while (true) {
                    int64_t space = str.find(' ', pos);
                    int length = std::stoi((static_cast<std::string>(str.substr(pos, space - pos))));
                    pos = space + 4;
                    space = str.find(',', pos);
                    auto name = (space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));

                    real_distances[name] = length;
                    if (space == pos_end) {
                        break;
                    }
                    else {
                        pos = space + 2;
                    }
                }
            }
            ::transport::detail::Coordinates k;
            k.lat = detail::StringToDouble(static_cast<std::string>(lat));
            k.lng = detail::StringToDouble(static_cast<std::string>(lng));

            transport.SetStop(std::move(static_cast<std::string>(name)), std::move(k));
            transport.SetDistancesToStop(name, real_distances);
        }

        std::pair<std::string_view, std::string_view> ReadRequestBeginning(const std::string_view str, int64_t& pos){
            pos = 0;
            int64_t space = str.find(' ', pos);
            auto mode = str.substr(pos, space - pos);
            pos = space + 1;

            space = str.find(':', pos);
            auto name = str.substr(pos, space - pos);
            pos = space + 2;
            return { mode, name };
        }
    }

    namespace user_interaction {
        void ReadDataBase(std::istream& i_stream, ::transport::Catalogue& transport) {
            std::string line = "";
            std::unordered_map<std::string, std::pair<bool, std::vector<std::string>>> name_of_bus;
            std::getline(i_stream, line);
            int number_of_requests_create = std::stoi(line);
            for (int number = 0; number < number_of_requests_create; ++number) {
                int64_t pos = 0;
                int64_t space = 0;

                std::getline(i_stream, line);
                auto str = static_cast<std::string_view>(line);

                auto [mode, name] = detail::ReadRequestBeginning(str,pos);

                if (mode == "Stop") {
                    detail::ReadStopParameters(pos,str,name,transport);
                }
                else if (mode == "Bus") {
                    name_of_bus[static_cast<std::string>(name)] = detail::ReadBuses(str, pos);
                }
            }
            for (auto [bus, stops] : name_of_bus) {
                transport.SetBus(bus, stops.first, move(stops.second));
            }
        }

        void RequestToTheDatabase(std::istream& i_stream, std::ostream& o_stream, ::transport::Catalogue& transport) {
            std::string line = "";
            std::getline(i_stream, line);
            int number_of_requests = std::stoi(line);
            for (int number = 0; number < number_of_requests; ++number) {
                auto [type, query] = ::transport::user_interaction::ReadRequests(i_stream);
                if (type == "Bus") {
                    ::transport::user_interaction::ResultOutputBus(o_stream, transport.GetBusInfo(query));
                }
                if (type == "Stop") {
                    ::transport::user_interaction::ResultOutputStop(o_stream, transport.GetStopInfo(query));
                }
            }
        }
    }
}