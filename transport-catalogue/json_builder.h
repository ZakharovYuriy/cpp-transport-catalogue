#pragma once
#include <stack>
#include <vector>
#include "json.h"

namespace json {

    class Builder;

    namespace detail {
        class DictItemContext;
        class ArrayItemContext;

        class KeyContext {
        public:
            KeyContext(Builder* builder) :builder_(builder) {
            }
            ArrayItemContext StartArray();
            DictItemContext Value(Node);
            DictItemContext StartDict();
        protected:
            Builder* builder_;
        };

        class DictItemContext {
        public:
            DictItemContext(Builder* builder):builder_(builder){
            }
            detail::KeyContext Key(std::string key);
            Builder& EndDict();
        protected:
            Builder* builder_;
        };
        
        class ArrayItemContext {
        public:
            ArrayItemContext(Builder* builder) :builder_(builder) {
            }
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            Builder& EndArray();
            ArrayItemContext& Value(Node);
        protected:
            Builder* builder_;
        };
    }

    class Builder {
    public:
        detail::KeyContext Key(std::string);
        Builder& Value(Node);
        detail::DictItemContext StartDict();
        detail::ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:
        std::stack <std::string> key_; 
        bool begin_arr_=false;
        bool begin_daict_ = false;
        Node root_;
        std::stack<Node> nodes_stack_;
        std::vector<Node*> main_nodes_stack_;

        void StartObject(const Node&&);
        
        int depth_of_dicts = 0;//текущее количество вложенных словарей
        int double_key_check = 0;//количество открытых ключей без значения
        bool last_key_is_empty_ = false;//содержит ли последний ключ значение
        bool CheckObjectCompleted();
        bool WrongValuePlace();
    };
}
