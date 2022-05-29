#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node()=default;
        explicit   Node(Value val);
        explicit   Node(std::nullptr_t val);
        explicit   Node(int val);
        explicit   Node(Array val);
        explicit   Node(Dict val);
        explicit   Node(bool val);
        explicit   Node(double val);
        explicit   Node(std::string val);

        bool IsString()const;
        bool IsBool()const;
        bool IsArray()const;
        bool IsMap()const;
        bool IsInt()const;
        bool IsDouble()const;
        bool IsPureDouble()const;
        bool IsNull()const;

        Value GetValue()const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        const bool& AsBool() const;
        const int& AsInt() const;
        const double& AsDouble() const;
        const std::nullptr_t& AsNull() const;
        const std::string& AsString() const;

    private:
        Value number_;
        double number_double_=0.0;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    bool operator== (const json::Node& n1, const json::Node& n2);
    bool operator!= (const json::Node& n1, const json::Node& n2);
    bool operator== (const json::Document& n1, const json::Document& n2);
    bool operator!= (const json::Document& n1, const json::Document& n2);
}
