#include "json.h"
#include <variant>
#include <optional>
#include <sstream>

using namespace std;

namespace json {

    bool operator==(const json::Node& left_node, const json::Node& right_node) {
        return left_node.GetValue()== right_node.GetValue();
    }
    bool operator!= (const json::Node& left_node, const json::Node& right_node) {
        return !(left_node == right_node);
    }

    bool operator== (const json::Document& left_node, const json::Document& right_node) {
        return left_node.GetRoot() == right_node.GetRoot();
    }
    bool operator!= (const json::Document& left_node, const json::Document& right_node) {
        return !(left_node==right_node);
    }

    namespace {
        struct SolutionPrinter {
            ostream& out;
            void operator()(std::nullptr_t) const {
                out<<"null"s;
            }
            void operator()(Array arr) const {
                out << '[';
                for (auto arr_itr = arr.begin(); arr_itr < arr.end(); ++arr_itr) {
                    std::visit(SolutionPrinter{ out }, arr_itr->GetValue());
                    if (arr_itr != arr.end() - 1) {
                        out << ',';
                    }
                }
                out <<']';
            }
            void operator()(Dict dict) const {
                out << '{';
                auto size = dict.size();
                auto counter = dict.size();
                counter = 0;
                for (const auto& node : dict) {
                    counter++;
                    out << '\"';
                    out << node.first;
                    out << '\"';
                    out << ':';
                    std::visit(SolutionPrinter{ out }, node.second.GetValue());
                    if (counter < size) {
                        out << ',';
                    }
                }
                out << '}';
            }
            void operator()(bool boolean) const {
                out << (boolean ? "true" : "false");
            }
            void operator()(int integer) const {
                out << integer;
            }
            void operator()(double double_type) const {
                out << double_type;
            }
            void operator()(std::string word) const {
                out << '\"';
                for (const char& w:word) {
                    if (w == '\n') {
                        out << "\\n";
                    }
                    else if (w == '\t') {
                        out << '\t';
                    }
                    else if (w == '\r') {
                        out << "\\r";
                    }
                    else if (w == '\"') {
                        out << "\\\"";
                    }
                    else if (w == '\\') {
                        out << "\\\\";
                    }
                    else {                            
                        out << w;
                    }
                }
                out<< '\"';
            }
        };

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result; 
            bool end_of_arr = false;
            for (char c; input >> c;) {
                if (c == ']') {
                    end_of_arr = true;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!end_of_arr) {
                throw ParsingError("Failed to read number from stream"s);
            }

            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;
            bool is_null = false;
            bool is_bool_true = false;
            bool is_bool_false = false;
            bool is_int = true;


            // ��������� � parsed_num ��������� ������ �� input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // ��������� ���� ��� ����� ���� � parsed_num �� input
            auto read_digits = [&is_int ,&is_null, &is_bool_true,&is_bool_false, &input, read_char] {
                if (!std::isdigit(input.peek())) {
                    is_int = false;
                    string null_w = "null"s;
                    string buff = "";
                    for (const auto& w : null_w) {
                        if (input.peek() == w) {
                            is_null = true;
                            read_char();
                        }
                        else {
                            is_null = false;
                            break;
                        }
                    }
                    string bool_true = "true"s;
                    string bool_false = "false"s;
                    for (const auto& w : bool_true) {
                        if (input.peek() == w) {
                            is_bool_true = true;
                            read_char();
                        }
                        else {
                            is_bool_true = false;
                            break;
                        }
                    }
                    
                    for (const auto& w : bool_false) {
                        if (input.peek() == w) {
                            is_bool_false = true;
                            read_char();
                        }
                        else {
                            is_bool_false = false;
                            break;
                        }
                    }
                    if (!(is_null || is_bool_false|| is_bool_true)) {
                        throw ParsingError("A digit is expected"s);
                    }
                    return;
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            

            if (input.peek() == '-') {
                read_char();
            }

            // ������ ����� ����� �����
            if (input.peek() == '0') {
                read_char();
                // ����� 0 � ������ � JSON �� ����� ���� ������ �����
            }
            else {
                read_digits();
            }
           
            // ������ ������� ����� �����
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // ������ ���������������� ����� �����
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // ������� ������� ������������� ������ � int
                    try {
                        return Node(move(std::stoi(parsed_num)));
                    }
                    catch (...) {
                        // � ������ �������, ��������, ��� ������������,
                        // ��� ���� ��������� ������������� ������ � double
                    }
                }
                else if (is_null) {
                    return Node(nullptr);
                }
                else if (is_bool_true) {
                    return Node(true);
                }
                else if (is_bool_false) {                   
                    return Node(false);
                }
                return Node(move(std::stod(parsed_num)));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }

        }

        Node LoadString(istream& input) {
            string result;
            bool symv = false;
            bool end_of_string = false;
            for (char w = input.get(); !input.eof(); w = input.get()) {
                if (symv) {                   
                    if (w == 'n') {
                        result += "\n";
                    }
                    if (w == 't') {
                        result += '\t';
                    }
                    if (w == 'r') {
                        result += "\r";
                    }
                    if (w == '\"') {
                        result += '\"';
                    }
                    if (w == '\\') {
                        result += "\\";
                    }
                    symv = false;
                }
                else {
                    if (w == '\\') {
                        symv = true;
                    }
                    else {
                        if (w == '"') {
                            end_of_string = true;
                            break;
                        }
                        else {
                            result += w;
                        }
                    }
                }
            }
            if (!end_of_string) {
                throw ParsingError("Failed to convert string"s);
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            bool end_of_dict = false;
            for (char c; input >> c ;) {
                if (c == '}') {
                    end_of_dict = true;
                    break;
                }

                if (c == ',') {
                    input >> c;
                }             

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (!end_of_dict) {
                throw ParsingError("Failed to read number from stream"s);
            }
            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace


    const Node::Value& Node::GetValue()const {
        return *this;
    }

    bool Node::IsString()const {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsBool()const {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsArray()const {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsMap()const {
        return std::holds_alternative<Dict>(*this);
    }
    bool Node::IsInt()const {
        return std::holds_alternative<int>(*this);
    }
    bool Node:: IsPureDouble()const{
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsDouble()const {
        return IsInt() || IsPureDouble();;
    }  
    bool Node::IsNull()const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    const Array& Node::AsArray() const {
        using namespace std::literals;
        if (!IsArray()) {
            throw std::logic_error("Not an array"s);
        }

        return std::get<Array>(*this);
    }
    const Dict& Node::AsMap() const {
        using namespace std::literals;
        if (!IsMap()) {
            throw std::logic_error("Not a dict"s);
        }

        return std::get<Dict>(*this);
    }
    int Node::AsInt() const {
        using namespace std::literals;
        if (!IsInt()) {
            throw std::logic_error("Not an int"s);
        }
        return std::get<int>(*this);
    }
    bool Node::AsBool() const {
        using namespace std::literals;
        if (!IsBool()) {
            throw std::logic_error("Not a bool"s);
        }

        return std::get<bool>(*this);
    }
    double Node::AsDouble() const {
        using namespace std::literals;
        if (!IsDouble()) {
            throw std::logic_error("Not a double"s);
        }
        return IsPureDouble() ? std::get<double>(*this) : AsInt();
    }
    const std::nullptr_t& Node::AsNull() const {
        if (!IsNull()) {
            throw logic_error("Not a nullptr_t");
        }
        return std::get<std::nullptr_t>(*this);
    }
    const string& Node::AsString() const {
        using namespace std::literals;
        if (!IsString()) {
            throw std::logic_error("Not a string"s);
        }

        return std::get<std::string>(*this);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(SolutionPrinter{ output }, doc.GetRoot().GetValue());
    }
}  // namespace json