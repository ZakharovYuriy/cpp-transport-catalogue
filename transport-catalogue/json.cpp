#include "json.h"
#include <variant>
#include <optional>
#include <sstream>

using namespace std;

namespace json {

    bool operator== (const json::Node& n1, const json::Node& n2) {
        if (n1.IsString() && n2.IsString()) {
            if (n1.AsString() == n2.AsString())
            {
                return true;
            }
        }
        else if (n1.IsBool() && n2.IsBool()) {
            if (n1.AsBool() == n2.AsBool())
            {
                return true;
            }
        }
        else if (n1.IsArray() && n2.IsArray()) {
            for (long unsigned int i = 0; i < n1.AsArray().size(); ++i) {
                if (!(n1.AsArray().at(i) == n2.AsArray().at(i)))
                {
                    return false;
                }
            }
            return true;
        }
        else if (n1.IsMap() && n2.IsMap()) {
            for (const auto& element : n1.AsMap()) {
                if (n2.AsMap().at(element.first) != element.second) {
                    return false;
                }
            }
            return true;
        }
        else if (n1.IsInt() && n2.IsInt()) {
            if (n1.AsInt() == n2.AsInt())
            {
                return true;
            }
        }
        else if (n1.IsPureDouble() && n2.IsPureDouble()) {
            if (n1.AsDouble() == n2.AsDouble())
            {
                return true;
            }
        }
        else if (n1.IsNull() && n2.IsNull()) {
            if (n1.AsNull() == n2.AsNull())
            {
                return true;
            }
        }
        return false;
    }
    bool operator!= (const json::Node& n1, const json::Node& n2) {
        return !(n1 == n2);
    }

    bool operator== (const json::Document& n1, const json::Document& n2) {
        return n1.GetRoot() == n2.GetRoot();
    }
    bool operator!= (const json::Document& n1, const json::Document& n2) {
        return !(n1==n2);
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


            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&is_int ,&is_null, &is_bool_true,&is_bool_false, &input, read_char] {
                if (!std::isdigit(input.peek())) {
                    is_int = false;
                    string null_w = "null"s;
                    string buff="";
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

            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в начале в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }
           
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(move(std::stoi(parsed_num)));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
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
            for (char w= input.get(); !input.eof(); w = input.get()) {
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

    Node::Node(Value val)
        : number_(move(val)) {
    }
    Node::Node(std::string val)
        : number_(move(val)) {
    }
    Node::Node(Array val)
        : number_(move(val)) {
    }
    Node::Node(Dict val)
        : number_(move(val)) {
    }
    Node::Node(int val)
        : number_(move(val)), number_double_(move(static_cast<double>(val))){
    }
    Node::Node(double val)
        : number_(move(val)) {
    }
    Node::Node(bool val)
        : number_(move(val)) {
    }
    Node::Node(std::nullptr_t val)
        : number_(move(val)) {
    }

    Value Node::GetValue()const {
        return number_;
    }

    bool Node::IsString()const {
        return std::holds_alternative<std::string>(number_);
    }
    bool Node::IsBool()const {
        return std::holds_alternative<bool>(number_);
    }
    bool Node::IsArray()const {
        return std::holds_alternative<Array>(number_);
    }
    bool Node::IsMap()const {
        return std::holds_alternative<Dict>(number_);
    }
    bool Node::IsInt()const {
        return std::holds_alternative<int>(number_);
    }
    bool Node::IsDouble()const {
        return std::holds_alternative<double>(number_)|| std::holds_alternative<int>(number_);
    }
    bool Node:: IsPureDouble()const{
        return std::holds_alternative<double>(number_);
    }
    bool Node::IsNull()const {
        return std::holds_alternative<std::nullptr_t>(number_);
    }

    const Array& Node::AsArray() const {
        if (!std::holds_alternative<Array>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<Array>(&number_));
    }
    const Dict& Node::AsMap() const {
        if (!std::holds_alternative<Dict>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<Dict>(&number_));
    }
    const int& Node::AsInt() const {
        if (!std::holds_alternative<int>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<int>(&number_));
    }
    const bool& Node::AsBool() const {
        if (!std::holds_alternative<bool>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<bool>(&number_));
    }
    const double& Node::AsDouble() const {
        if (!(std::holds_alternative<double>(number_) || std::holds_alternative<int>(number_))) {
            throw logic_error("incorrect tipe");
        }

        if (std::get_if<double>(&number_)) {
            return *(std::get_if<double>(&number_));
        }
        else {
            return number_double_;
        }
    }
    const std::nullptr_t& Node::AsNull() const {
        if (!std::holds_alternative< std::nullptr_t>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<std::nullptr_t>(&number_));
    }
    const string& Node::AsString() const {
        if (!std::holds_alternative<std::string>(number_)) {
            throw logic_error("incorrect tipe");
        }
        return *(std::get_if<std::string>(&number_));
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
