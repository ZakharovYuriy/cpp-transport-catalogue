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
    ::transport::svg::MapRender map_render(reader.GetRenderSettings());
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

int main() {
    //TestSVG();
    //TestFromSTDIN_STDOUT();
    TestFromFile();
}