//#define DEBUG
//#define DEBUG_DURATION
#ifdef DEBUG_DURATION
    #include "log_duration.h"
#endif

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>

#include "map_renderer.h"
#include "json_reader.h"
#include "serialization.h"
#include "transport_catalogue.h"

using namespace std;

void TestSVG() {
    ::transport::Catalogue transport;
    std::ifstream in("C:\\Games\\TestInput6.txt"); // окрываем файл для чтения
    std::ofstream out;          // поток для записи
    out.open("C:\\Games\\TestOutput6.svg"); // окрываем файл для записи
   
    ::transport::json::Reader reader;
    {
#ifdef DEBUG_DURATION
    LOG_DURATION("readDoc"s);
#endif
        reader.ReadDocumentInCatalogue(in, transport);
    }

    {
#ifdef DEBUG_DURATION
    LOG_DURATION("Drow"s);
#endif
    ::svg::Document doc;
    ::transport::svg::MapRender map_render(reader.RenderSettings());
    map_render.DrowMap(transport, doc);
    doc.Render(out);
    doc.Render(cout);
    }
}
void TestFromFile() {
    ::transport::Catalogue transport;
    std::ifstream in("C:\\Games\\TestInput4.txt"); // окрываем файл для чтения
    std::ofstream out;          // поток для записи
    out.open("C:\\Games\\TestOutput4.txt"); // окрываем файл для записи

    ::transport::json::Reader reader;

    reader.ReadDocumentInCatalogue(in, transport);
    reader.ResponseToRequests(out, transport);

}
void TestFromSTDIN_STDOUT() {
    ::transport::Catalogue transport;
    ::transport::json::Reader reader;
    reader.ReadDocumentInCatalogue(cin, transport);
    reader.ResponseToRequests(cout, transport);
}




void make_base(istream& in_json) {
    ::transport::Catalogue transport;
    ::transport::json::Reader reader;
    reader.ReadDocumentInCatalogue(in_json, transport);
    std::ofstream out(reader.GetSerializePath(), ios::out | ios::binary);
    protobuf::Serialize(transport, reader, out);
}

void process_requests(istream& in_json, ostream& out) {
    ::transport::Catalogue transport;
    ::transport::json::Reader reader;

    reader.ReadDocumentInCatalogue(in_json, transport);
    std::ifstream in(reader.GetSerializePath(), ios::binary);
    protobuf::DeSerialize(transport, reader, in);

    ::transport::svg::MapRender map_render(reader.RenderSettings());

    reader.ResponseToRequests(out, transport);
}




int main() {
    //TestSVG();
    //TestFromSTDIN_STDOUT();
    //TestFromFile();
    std::ifstream in("c:\\Users\\Z\\source\\repos\\B5T8L1\\build\\s14_3_opentest_4_make_base.json"); // окрываем файл для чтения
    std::ifstream in2("c:\\Users\\Z\\source\\repos\\B5T8L1\\build\\s14_3_opentest_4_process_requests.json"); // окрываем файл для чтения
    std::ofstream out;          // поток для записи
    out.open("c:\\Users\\Z\\source\\repos\\B5T8L1\\build\\TestOutput.txt"); // окрываем файл для записи

    make_base(in);
    process_requests(in2, cout);
}

/*using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        // make base here
        make_base(cin);

    }
    else if (mode == "process_requests"sv) {
        // process requests here
        process_requests(cin, cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}
*/