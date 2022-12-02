#include <algorithm>
#include <variant>
#include <vector>
#include <string>

#include "json_builder.h"
#include "json.h"
namespace json{
	namespace detail {
		ArrayItemContext KeyContext::StartArray() {
			builder_->StartArray();
			return ArrayItemContext(builder_);
		}
		DictItemContext KeyContext::Value(Node val) {
			builder_->Value(val);
			return DictItemContext(builder_);
		}
		DictItemContext KeyContext::StartDict() {
			builder_->StartDict();
			return DictItemContext(builder_);
		}


		KeyContext DictItemContext::Key(std::string key) {
			builder_->Key(key);
			return detail::KeyContext(builder_);
		}
		Builder& DictItemContext::EndDict() {
			return builder_->EndDict();
		}


		DictItemContext ArrayItemContext::StartDict() {
			builder_->StartDict();
			return DictItemContext(builder_);			
		}
		ArrayItemContext ArrayItemContext::StartArray() {
			builder_->StartArray();
			return ArrayItemContext(builder_);
		}
		Builder& ArrayItemContext::EndArray() {
			return builder_->EndArray();
		}

		ArrayItemContext& ArrayItemContext::Value(Node val) {
			builder_->Value(val);
			return *this;
		}
	}

	bool Builder::CheckObjectCompleted() {
		if (nodes_stack_.size() == 1) {
			if (!(nodes_stack_.top().IsArray() &&
				  nodes_stack_.top().AsArray().empty())) {

				if (!(nodes_stack_.top().AsDict().empty() && 
						nodes_stack_.top().AsDict().empty())) {

					return true;
				}
			}
		}
		return false;
	}

	bool Builder::WrongValuePlace() {
		if (main_nodes_stack_.empty()) {
			return false;
		}

		if (!main_nodes_stack_.back()->IsArray()) {
			if (!main_nodes_stack_.back()->IsDict()) {
				return true;
			}
			else {
				if (!last_key_is_empty_) {
					return true;
				}
			}
		}
		return false;
	}

	detail::KeyContext Builder::Key(std::string key) {
		if (double_key_check> depth_of_dicts|| last_key_is_empty_) {
			throw std::logic_error("key error");
		}
		else {
			last_key_is_empty_ = true;
			++double_key_check;
			if (CheckObjectCompleted()) {
				throw std::logic_error("the object is completed");
			}
			key_.push(key);
		}
		return detail::KeyContext(this);
	}

	Builder& Builder::Value(Node val) {
		if (double_key_check > depth_of_dicts) {
			--double_key_check;
		}

		if (CheckObjectCompleted()) {
			throw std::logic_error("the object is completed");
		}

		if (WrongValuePlace()){
			throw std::logic_error("wrong value place");
		}

		last_key_is_empty_ = false;

		nodes_stack_.push(val);
		return *this;
	}

	void Builder::StartObject(const Node&& obj) {
		if (CheckObjectCompleted()) {
			throw std::logic_error("the object is completed");
		}

		if (WrongValuePlace()) {
			throw std::logic_error("wrong value place");
		}

		last_key_is_empty_ = false;

		nodes_stack_.push(move(obj));
		main_nodes_stack_.push_back(&nodes_stack_.top());
	}

	detail::DictItemContext Builder::StartDict() {
		//открываем новый словарь- увеличивем глубину
		++depth_of_dicts;
		
		StartObject(move(Dict()));
		
		return detail::DictItemContext(this);
	}

	detail::ArrayItemContext Builder::StartArray() {
		StartObject(move(Array()));

		return detail::ArrayItemContext(this);
	}

	Builder& Builder::EndDict() {		
		if (CheckObjectCompleted()) {
			throw std::logic_error("the object is completed");
		}
		//проверка на неверное закрытие
		if (main_nodes_stack_.back()->IsArray()) {
			throw std::logic_error("wrong end of array");
			return*this;
		}
		//закрываем новый словарь- уменьшаем глубину
		--depth_of_dicts;
		double_key_check= depth_of_dicts;

		
		Dict dict;
		if (last_key_is_empty_) {
			double_key_check = depth_of_dicts;
			dict[key_.top()] = nullptr;
			key_.pop();
		}
		while (!nodes_stack_.empty()) {			
			if (main_nodes_stack_.back() == &(nodes_stack_.top())) {
				main_nodes_stack_.pop_back();
				nodes_stack_.pop();
				nodes_stack_.push(move(dict));
				break;
			}
			dict[key_.top()] = nodes_stack_.top();
			key_.pop();
			nodes_stack_.pop();
		}
		return *this;
	}

	Builder& Builder::EndArray() {
		
		if (CheckObjectCompleted()) {
			throw std::logic_error("the object is completed");
		}
		//проверка на неверное закрытие
		if (main_nodes_stack_.back()->IsDict()) {
			throw std::logic_error("wrong end of array");
			return *this;
		}	
		Array result;

		while (!nodes_stack_.empty()) {
			if (main_nodes_stack_.back() == &(nodes_stack_.top())) {
				main_nodes_stack_.pop_back();
				nodes_stack_.pop();
				reverse(result.begin(), result.end());
				nodes_stack_.push(move(result));
				break;
			}
			result.push_back(nodes_stack_.top());
			nodes_stack_.pop();
		}

		return *this;
	}

	Node Builder::Build() {
		if (nodes_stack_.empty()) {
			throw std::logic_error("the object is not finished");
		}
		else if (nodes_stack_.size()>1) {
			throw std::logic_error("incomplete array or dictionary");
		}
		else {
			root_ = nodes_stack_.top();			
		}	
		return root_;	
	}
}
