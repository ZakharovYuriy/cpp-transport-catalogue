# cpp-transport-catalogue

Read in other languages: [English](README.md), [Русский](README.RUS.md)

# Program Description
Transport Directory - supports graphical output, route search and calculation of travel time.<br>
First, requests to create a database are submitted for input, then requests to the database itself. <br>

When creating a database with the `-make_base` command, the result of building ** is serialized** and saved to the file specified in the `serialization_settings` parameter.<br>
Then, when the `-process_requests` command is called from the file specified in the `serialization_settings` parameter, the **deserialization** of the created database is performed and queries are executed.<br>

Using queries, you can find out: <br>
- which bus routes pass through the stop<br>
- what stops are there on the bus route<br>
- find out the shortest route between two stops<br>
A person can use several buses on the route. <br>
One bus can even be used several times if it makes a big detour in some sections and it is easier to cut off on another bus.<br>

# Assembly using Cmake
To build this project on linux you need:<br>
1)If you don't have Cmake installed, install Cmake<br>
2)For the application to work, you need to install **Protobuf**, here is the [instruction](Instruction.Protobuf.EN.md) || [инструкция](Instruction.Protobuf.RUS.md)<br>
2)If the "Debug" or "Release" folders are not created:<br>

```
mkdir ../Debug
mkdir ../Release
```
3)Go to the `transport-catalog` folder and run the command for Debug and/or Release conf:<br>
:exclamation: :exclamation: :exclamation:<br>
Change `/path/to/protobuf/package` to the path to the protobuf you installed in the previous steps<br>

<!-- /home/yuriy/Документы/YP/protobuf/package -->

```
cmake -E chdir ../Debug/ cmake -G "Unix Makefiles" ../transport-catalogue/ -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_PREFIX_PATH=/path/to/protobuf/package 
cmake -E chdir ../Release/ cmake -G "Unix Makefiles" ../transport-catalogue -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_PREFIX_PATH=/path/to/protobuf/package 
```
4)Go to "Debug" or "Release" folder and build:<br>

```
cmake --build .
```

5)To **Run** program - in the debug or release folder run:<br>

```
./transport_catalogue
```

# Using

### Before you start:
  0. Installation and configuration of all required components in the development environment to run the application
  1. The use case is shown in main.cpp and when calling `./transport_catalogue -help`
  2. Examples of input documents and answers to them are in the 'Examples` folder.

## Description of features:
### make_base program
The task of the make_base program is to build a database and serialize it into a file with the specified name.<br>

The input to the make_base program is JSON with the following keys:<br>
 - base_requests: Bus and Stop requests to create a database.<br>
- render_settings: rendering settings.<br>
 - serialization_settings: serialization settings.<br>
- routing_settings — routing settings, dictionary with two keys:<br>
   bus_wait_time — waiting time for the bus at the stop, in minutes.<br>
   bus_velocity — bus speed, in km/h.<br>

### process_requests program
The process_requests program should output JSON with responses to requests.<br>

The process_requests program receives a JSON file with the following keys as input:<br>
- stat_requests: Bus and Stop requests to the finished database.<br>
- serialization_settings: serialization settings.<br>
- Route — these are requests for building a route between two stops.<br>
In addition to the standard id and type properties, they contain two more:<br>
from — the stop where you need to start the route.<br>
to — stop where you need to finish the route.<br>
Both values are names of stops existing in the database. However, they may not belong to any bus route.<br>

# System requirements:
  1. C++17(STL)
  2. GCC (MinG w64) 11.2.0  
  
# Plans for completion:
1. Add UI
2. Replace the route search library
3. Add the ability to offer multiple paths to choose from

# Technology stack:
  1. Hash functions, **unordered_map** and **unordered_set**
  2. Trees
  3. **namespases**
  4. JSON
  5. Smart pointers **unique_ptr**, **shared_ptr** and **weak_ptr**
  6. Inheritance and polymorphism, abstract classes, interfaces
  7. Runtime polymorphism with std::variant, dynamic type conversion
  8. Immediately invoked lambda expression, **std::invoke**
  9. **mutable**
 10. Working with paths and streams: input/output, string, for working with files
 11. Regular expressions **std::regex**
 12. RAII - "Resource Acquisition is Initialization”
 13. **move**-semantics, Forwarding reference
 14. Dense packaging, Serialization, deserialization, Google Protocol Buffers - Protobuf
 15. Table of virtual methods

# Structure
### Description of the program files
 - transport_catalogue.h, transport_catalogue.cpp — class of the transport directory<br>
 - main.cpp — entry point.<br>
Two libraries were used in the work:<br>
 - graph.h — class implementing weighted oriented graph<br>
 - router.h — class implementing shortest path search in weighted oriented graph<br>
- domain.cpp ,domain.h - In this file, classes/structures that are part of the application domain and do not depend on the transport directory<br>
 - geo.cpp , geo.h - functions for working with geographical coordinates<br>
 - json.cpp ,json.h - developed simplified library for working with JSON<br>
 - json_builder.cpp , json_builder.h - necessary classes/structures to create an output file in JSON format<br>
 - json_reader.cpp , json_reader.h - necessary classes/structures for reading and processing the input file in JSON format<br>
 - log_duration.h - a file with a class that allows **profiling** <br>
 - map_renderer.cpp , map_renderer.h - code responsible for visualizing the route map in SVG format.<br>
 - serialization.cpp , serialization.h - necessary classes/structures for processing serialized files.<br>
 - svg.cpp ,svg.h - processing and storing SVG image parameters<br>
 - transport_router.cpp , transport_router.h - building routes<br>
 - transport_catalogue.proto, transport_router.protoF, svg.proto - proto files, for serialization<br>

# Input data format
Data in JSON object format. Its top-level structure:<br>

```
{
  "base_requests": [ ... ],
  "stat_requests": [ ... ]
} 
```
This is a dictionary containing keys:<br>
`base_requests` - is an array with a description of bus routes and stops,<br>
`stat_requests` - is an array with requests to the transport directory.<br>

## Description of the route database
The `base_requests` array contains two types of elements: routes and stops. They are listed in any order.<br>

### Example of a stop description:<br>

```
{
  "type": "Stop",
  "name": "Электросети",
  "latitude": 43.598701,
  "longitude": 39.730623,
  "road_distances": {
    "Улица Докучаева": 3000,
    "Улица Лизы Чайкиной": 4300
  }
} 
```
Description of the stop — dictionary with keys:<br>
- type — string equal to "Stop". Means that the dictionary describes the situation;<br>
- name — the name of the stop;<br>
- latitude and longitude — latitude and longitude of the stop — floating point numbers;<br>
- road_distances — a dictionary that specifies the road distance from this stop to neighboring ones. Each key in this dictionary is the name of a nearby stop, the value is an integer distance in meters.<br>
 
### Example of a bus route description:
```
{
  "type": "Bus",
  "name": "14",
  "stops": [
    "Улица Лизы Чайкиной",
    "Электросети",
    "Улица Докучаева",
    "Улица Лизы Чайкиной"
  ],
  "is_roundtrip": true
} 
```

Description of the bus route — dictionary with keys:<br>
- type — string "Bus". Means that the dictionary describes the bus route;<br>
- name — the name of the route;<br>
- stops — an array with the names of stops that the route passes through. For a circular route, the name of the last stop duplicates the name of the first one. For example: ["stop1", "stop2", "stop3", "stop1"];<br>
- is_roundtrip — value of the bool type. true if the route is circular.<br>

## Format of requests to the transport directory and responses to them
Requests are stored in the `stat_requests` array. In response to them, the program should output a JSON array of responses to stdout:<br>

```
[
  { response to the first request },
  { response to the second request },
  ...
  { response to the last request }
] 
```
Each query is a dictionary with the required keys `id` and `type`. They specify a unique numeric identifier of the request and its type. There may be other keys in the dictionary that are specific to a particular type of query.<br>
In the output JSON array, each `stat_requests` request must have a dictionary response with the mandatory `request_id` key. The key value must be equal to the `id` of the corresponding request. There are other keys in the dictionary that are specific to a particular type of response.<br>
The order of responses to requests in the output array must match the order of requests in the `stat_requests` array.<br>

### Getting route information
Request format:<br>

```
{
  "id": 12345678,
  "type": "Bus",
  "name": "14"
} 
```
The type key has the value “Bus". It can be used to determine that this is a request for route information.
The name key specifies the name of the route for which the application should output statistical information.<br>
### The answer to this request should be given in the form of a dictionary:

```
{
  "curvature": 2.18604,
  "request_id": 12345678,
  "route_length": 9300,
  "stop_count": 4,
  "unique_stop_count": 3
} 
```
Dictionary Keys:<br>
- curvature — the tortuosity of the route. It is equal to the ratio of the length of the road distance of the route to the length of the geographical distance. A number of type double;<br>
- request_id — must be equal to the id of the corresponding Bus request. Integer;<br>
- route_length — the length of the road distance of the route in meters, integer;<br>
- stop_count — the number of stops on the route;<br>
- unique_stop_count — the number of unique stops on the route.<br>
For example, on a circular route with stops A, B, C, A there are four stops. Three of them are unique.<br>
There are five stops on the ring route with stops A, B and C (A, B, C, B, A). Three of them are unique.<br>
<br>
If there is no route with the specified name in the directory, the answer will be as follows:<br>

```
{
  "request_id": 12345678,
  "error_message": "not found"
} 
```
### Getting information about a stop
Request format:<br>

```
{
  "id": 12345,
  "type": "Stop",
  "name": "Улица Докучаева"
} 
```
The name key sets the name of the stop.<br>
### Response to the request:

```
{
  "buses": [
      "14", "22к"
  ],
  "request_id": 12345
} 
```

The value of the response keys:
 - buses - is an array of names of routes that pass through this stop. The names are sorted in lexicographic order.<br>
- request_id - is an integer equal to the id of the corresponding Stop request.<br>
If there is no stop with the given name in the directory, the response to the request should be as follows:<br>

```
{
  "request_id": 12345,
  "error_message": "not found"
} 
```

## Structure of the render_settings dictionary:
```
{
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
} 
```
 - width and height — the width and height of the image in pixels. A real number in the range from 0 to 100000.<br>
- padding — the padding of the edges of the map from the borders of the SVG document. A real number no less then 0 and less than min(width, height)/2.<br>
- line_width — the thickness of the lines that draw bus routes. A real number in the range from 0 to 100000.<br>
- stop_radius — radius of circles that indicate stops. A real number in the range from 0 to 100000.<br>
 - bus_label_font_size — the size of the text used to write the names of bus routes. An integer in the range from 0 to 100000.<br>
 - bus_label_offset — offset of the route name label relative to the coordinates of the final stop on the map. An array of two elements of the double type. Sets the values of the dx and dy properties of the <text> SVG element. Array elements are numbers in the range from -100000 to 100000.<br>
- stop_label_font_size — the size of the text that displays the names of stops. An integer in the range from 0 to 100000.<br>
 - stop_label_offset — offset of the name of the stop relative to its coordinates on the map. An array of two elements of the double type. Sets the values of the dx and dy properties of the <text> SVG element. Numbers in the range from -100000 to 100000.<br>
- underlayer_color — the color of the background under the names of stops and routes. The color storage format will be below.
underlayer_width — the thickness of the substrate under the names of stops and routes. Sets the value of the stroke-width attribute of the <text> element. A real number in the range from 0 to 100000.<br>
- color_palette — color palette. A non-empty array.<br>
 <br>
The color can be specified in one of the following formats:<br>
 - in the form of a string, for example, "red" or "black";<br>
 - in an array of three integers in the range [0, 255]. They define the r, g and b components of the color in svg::Rgb format. The color [255, 16, 12] should be output in SVG as rgb(255,16,12);<br>
 - in an array of four elements: three integers in the range from [0, 255] and one real number in the range from [0.0, 1.0]. They set the components red, green, blue and opacity colors of the svg::Rgba format. The color specified as [255, 200, 23, 0.85], should be output in SVG as rgba(255,200,23,0.85).<br>


### In the state_request array, the request to get an image has the following form:
```
{
  "type": "Map",
  "id": 11111
} 
```
### The response to this request is given in the form of a dictionary with the keys request_id and map:

```
{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
  "request_id": 11111
} 
```
The map key is a string with a map image in SVG format.

