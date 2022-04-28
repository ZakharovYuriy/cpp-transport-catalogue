#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <string_view>
#include <charconv>
#include <array>
#include <sstream>

#include "transport_catalogue.h"
#include "geo.h"
#include "input_reader.h"

namespace transport {
    namespace detail {
        double StringToDouble(const std::string& s) {
            std::istringstream i(s);
            double x;
            if (!(i >> x))
                return 0;
            return x;
        }
    }

    void ReadBuses() {
    
    }

    void ReadStop(int64_t& space, int64_t& pos, int& stop_nomber,const int64_t pos_end, std::string_view& str, std::string_view& name,::transport::Catalogue& transport) {
        std::array<std::string, 100> names;
        std::array<int, 100> length;

        space = str.find(',', pos);
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
                length[stop_nomber] = std::stoi((static_cast<std::string>(str.substr(pos, space - pos))));
                pos = space + 4;
                space = str.find(',', pos);
                names[stop_nomber] = (space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
                ++stop_nomber;
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

        transport.SetStop(static_cast<std::string>(name), std::move(k), std::move(names), std::move(length), stop_nomber);
    }

    void ReadCoordinates() {

    }

    namespace user_interaction {
        void ReadDataBase(std::istream& i_stream,int number_of_requests_create, ::transport::Catalogue& transport) {
            std::string line = "";
            std::unordered_map<std::string, std::pair<bool, std::vector<std::string>>> name_of_bus;

            for (int number = 0; number <= number_of_requests_create; ++number) {
                //std::vector<int>                 
                int stop_nomber = 0;

                std::getline(i_stream, line);
                auto str = static_cast<std::string_view>(line);
                int64_t pos = 0;
                const int64_t pos_end = str.npos;

                int64_t space = str.find(' ', pos);
                auto mode = str.substr(pos, space - pos);
                pos = space + 1;

                space = str.find(':', pos);
                auto name = str.substr(pos, space - pos);
                pos = space + 2;

                if (mode == "Stop") {
                    ReadStop(space,pos,stop_nomber,pos_end,str,name,transport);
                }
                else if (mode == "Bus") {
                    std::vector<std::string> stops;

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
                    name_of_bus[static_cast<std::string>(name)] = { is_circular,stops };
                }
            }
            for (auto [bus, stops] : name_of_bus) {
                transport.SetBus(bus, stops.first, move(stops.second));
            }
        }

        void RequestToTheDatabase(std::istream& i_stream, int number_of_requests, ::transport::Catalogue& transport) {
            for (int number = 0; number <= number_of_requests; ++number) {
                auto [type, query] = ::transport::user_interaction::ReadRequests();
                if (type == "Bus") {
                    transport.GetBusInfo(query);
                }
                if (type == "Stop") {
                    transport.GetStopInfo(query);
                }
            }
        }
    }
}