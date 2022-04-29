#include <iostream>
#include <string>
#include <fstream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"


using namespace std;

void TestFileStream() {
	int number_of_requests = 0;
	::transport::Catalogue transport;
	std::ifstream in("C:\\Games\\test.txt"); // �������� ���� ��� ������
	std::ofstream out;          // ����� ��� ������
	out.open("C:\\Games\\TestOutput2.txt"); // �������� ���� ��� ������

	::transport::user_interaction::ReadDataBase(in, transport);
	::transport::user_interaction::RequestToTheDatabase(in, out, transport);
}

int main() {
	int number_of_requests = 0;
	::transport::Catalogue transport;
	::transport::user_interaction::ReadDataBase(cin, transport);
	::transport::user_interaction::RequestToTheDatabase(cin, cout, transport);
	TestFileStream();
}