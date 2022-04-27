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
	::transport::user_interaction::ReadDataBase(cin,number_of_requests, transport);
	cin >> number_of_requests;
	::transport::user_interaction::RequestToTheDatabase(cin, number_of_requests, transport);
}