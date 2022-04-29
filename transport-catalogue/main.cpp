#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"


using namespace std;

int main() {
	int number_of_requests = 0;
	::transport::Catalogue transport;
	cin >> number_of_requests;
	::transport::user_interaction::ReadDataBase(cin,number_of_requests, transport);
	cin >> number_of_requests;
	::transport::user_interaction::RequestToTheDatabase(cout, number_of_requests, transport);
}