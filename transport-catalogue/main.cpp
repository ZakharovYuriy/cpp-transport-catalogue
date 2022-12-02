#include "log_duration.h"

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <filesystem>

#include "map_renderer.h"
#include "json_reader.h"
#include "serialization.h"
#include "transport_catalogue.h"

using namespace std;

void OpenBaseAnswerRequestFile(const string &create_base, const string &request,
		const string &output_path = "") {
	using filesystem::path;

	path base = create_base;
	path req = request;
	path p3 = output_path;
	path svg;
	path answer;
	if (output_path.empty()) {
		p3 = req;
		p3 = p3.remove_filename();
	}
	svg = p3 / path("output-result.svg");
	answer = p3 / path("answer.json");

	::transport::Catalogue transport;
	std::ifstream in_base(base);
	std::ofstream out_text;
	out_text.open(answer);

	std::ofstream out_svg;
	out_svg.open(svg);

	::transport::json::Reader reader;

	reader.ReadDocumentInCatalogue(in_base, transport);
	if (!request.empty()) {
		std::ifstream in_req(req);
		reader.ReadDocumentInCatalogue(in_req, transport);
	}
	reader.ResponseToRequests(out_text, transport);
	::svg::Document doc;

	::transport::svg::MapRender map_render(reader.RenderSettings());
	map_render.DrowMap(transport, doc);
	doc.Render(out_svg);
}

void STDIN_STDOUT() {
	::transport::Catalogue transport;
	::transport::json::Reader reader;
	reader.ReadDocumentInCatalogue(cin, transport);
	reader.ResponseToRequests(cout, transport);
}

void make_base(istream &in_json) {
	::transport::Catalogue transport;
	::transport::json::Reader reader;
	reader.ReadDocumentInCatalogue(in_json, transport);
	std::ofstream out(reader.GetSerializePath(), ios::out | ios::binary);
	protobuf::Serialize(transport, reader, out);
}

void process_requests(istream &in_json, ostream &out) {
	::transport::Catalogue transport;
	::transport::json::Reader reader;

	reader.ReadDocumentInCatalogue(in_json, transport);
	std::ifstream in(reader.GetSerializePath(), ios::binary);
	protobuf::DeSerialize(transport, reader, in);

	::transport::svg::MapRender map_render(reader.RenderSettings());

	reader.ResponseToRequests(out, transport);
}

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
	stream
			<< "Simple Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void PrintFrom(std::ostream &stream = std::cerr) {
	stream << "from\n"sv;
}

void PrintTo(std::ostream &stream = std::cerr) {
	stream << "to\n"sv;
}

void PrintHelp(std::ostream &stream = std::cerr) {
	string help =
			R"123(
-make_base         :at the entrance, you must submit a text 
                    in the form of a JSON document, with information 
                    for building a route map, as well as with the name 
                    of the file where the results of the 
                    construction will be saved.

-process_requests  :the input must be submitted in the form of a JSON 
                    document, with stat_requests requests, as well as 
                    with the file name, for deserialization 
                    of the database from it.

-base_and_request  :creates a base for building routes from the 
                    make_base file and responds to requests from the
                    process_requests file, if desired, you can specify
                    a folder to save the results, otherwise, 
                    saving will be performed in the same folder 
                    where the file with requests is stored

-in_std            :input to the console text in JSON format 
                    with commands for building a database 
                    and queries to it. The result of the work 
                    will be output to the console in JSON format

-test              :runs the test from the </Examples/TEST> folder
)123";
	stream << help;
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		const std::string_view mode(argv[i]);
		if (mode == "-help"sv) {
			// process requests here
			PrintHelp(cout);
		} else if (mode == "-make_base"sv) {
			// make base here
			make_base(cin);
		} else if (mode == "-process_requests"sv) {
			// process requests here
			process_requests(cin, cout);
		} else if (mode == "-base_and_request"sv) {
			if(argc>i+3){
				OpenBaseAnswerRequestFile(argv[i+1],argv[i+2],argv[i+3]);
				i+=3;
			}else{
				OpenBaseAnswerRequestFile(argv[i+1],argv[i+2]);
				i+=2;
			}
		} else if (mode == "-in_std"sv) {
			STDIN_STDOUT();
		}else if (mode == "-test"sv) {
			OpenBaseAnswerRequestFile("../Examples/TEST/make_base"s,
					"../Examples/TEST/process_requests"s,
					"../Examples/TEST/OUTPUT"s);
		} else {
			PrintUsage();
			return 1;
		}
	}
}

