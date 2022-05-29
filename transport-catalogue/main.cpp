//#include "log_duration.h"

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>

#include "map_renderer.h"
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std;

void TestSVG() {
    ::transport::Catalogue transport;
    std::ifstream in("C:\\Games\\TestInput2.txt"); // окрываем файл для чтения
    std::ofstream out;          // поток для записи
    out.open("C:\\Games\\TestOutput2.svg"); // окрываем файл для записи
   
    ::transport::json::Reader reader;
    //{LOG_DURATION("readDoc"s);
        reader.ReadDocumentInCatalogue(in, transport);
    //}

    //{LOG_DURATION("Drow"s);
    ::svg::Document doc;
    ::transport::svg::MapRender map_render(reader.GetRenderSettings());
    map_render.DrowMap(transport, doc);
    doc.Render(out);
    doc.Render(cout);
    // }
}
void TestJSON() {
    ::transport::Catalogue transport;
    std::ifstream in("C:\\Games\\TestInput2.txt"); // окрываем файл для чтения
    std::ofstream out;          // поток для записи
    out.open("C:\\Games\\TestOutput3.txt"); // окрываем файл для записи

    ::transport::json::Reader reader;

    reader.ReadDocumentInCatalogue(in, transport);
    reader.ResponseToRequests(out, transport);

}
void JSONoutput() {
    ::transport::Catalogue transport;
    ::transport::json::Reader reader;
    reader.ReadDocumentInCatalogue(cin, transport);
    reader.ResponseToRequests(cout, transport);
}

int main() {
    TestSVG();
    TestJSON();
    JSONoutput();
}