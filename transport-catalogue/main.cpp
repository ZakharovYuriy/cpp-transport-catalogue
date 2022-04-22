#include <iostream>
#include <string>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
	int number_of_requests = 0;
	::transport::Catalogue transport;
	cin >> number_of_requests;
	::transport::user_interaction::ReadDataBase(number_of_requests, transport);
	cin >> number_of_requests;
	for (int number = 0; number <=number_of_requests; ++number) {
		auto [type, query] = ::transport::user_interaction::ReadRequests();
		if (type=="Bus") {
			transport.GetBusInfo(query);
		}
		if (type == "Stop") {
			transport.GetStopInfo(query);
		}
	}
}