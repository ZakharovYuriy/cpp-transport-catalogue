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
    //using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node  final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        bool IsString()const;
        bool IsBool()const;
        bool IsArray()const;
        bool IsMap()const;
        bool IsInt()const;
        bool IsDouble()const;
        bool IsPureDouble()const;
        bool IsNull()const;

        const Value& GetValue() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;
        int AsInt() const;
        double AsDouble() const;
        const std::nullptr_t& AsNull() const;
        const std::string& AsString() const;
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

    bool operator== (const json::Node& left_node, const json::Node& right_node);
    bool operator!= (const json::Node& left_node, const json::Node& right_node);
    bool operator== (const json::Document& left_node, const json::Document& right_node);
    bool operator!= (const json::Document& left_node, const json::Document& right_node);
}
