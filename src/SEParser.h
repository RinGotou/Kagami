#pragma once
#ifndef _SE_PARSER_
#define _SE_PARSER_

#include "SEIncludes.h"

namespace suzu {
	using std::ifstream;
	using std::ofstream;
	using std::string;
	using std::vector;
	using std::stack;
	using std::deque;
	using std::regex;

	const string kStrEmpty = "";
	const string kStrFatalError = "__FATAL";
	const string kStrWarning = "__WARNING";
	const string kStrEOF = "__EOF";
	const string kStrPass = "__PASS";
	const string kStrNothing = "__NOTHING";
	const string kStrRedirect = "__*";
	const string kstrDefine = "def";
	const string kStrVar = "var";

	const int kCodeSuccess = 0;
	const int kCodeStandby = 1;
	const int kCodeOverflow = -1;
	const int kCodeIllegalArgs = -2;

	const size_t kTypeFunction = 0;
	const size_t kTypeString = 1;
	const size_t kTypeInteger = 2;
	const size_t KTypeDouble = 3;
	const size_t kTypeBoolean = 4;
	const size_t kTypeSymbol = 5;

	const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
	const regex kPatternString(R"("(\"|\\|\n|\t|[^"])*")");
	const regex kPatternNumber(R"(\d+\.?\d*)");
	const regex kPatternInteger(R"(\d+)");
	const regex kPatternDouble(R"(\d+\.\d+)");
	const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
	const regex kPatternSymbol(R"(==|<=|>=|&&|\|\||[[:Punct:]]|len)");
	const regex kPatternBlank(R"([[:blank:]])");
	class Token;

	//preserve for function pointer

	class Messege {
	private:
		string value;
		int code;
	public:
		Messege() : value(kStrEmpty), code(kCodeStandby) {}

		Messege(string value, int code) {
			this->value = value;
			this->code = code;
		}

		Messege SetValue(const string &value) {
			this->value = value;
			return *this;
		}

		string GetValue() const {
			return this->value;
		}

		Messege SetCode(const int &code) {
			this->code = code;
			return *this;
		}

		int GetCode() const {
			return this->code;
		}
	};

	class Util {
	public:
		template <class Type>
		void CleanupVector(vector<Type> &target) {
			target.clear();
			vector<Type>(target).swap(target);
		}

		template <class Type>
		void CleanupStack(stack<Type> &target) {
			target.clear();
			stack<Type>(target).swap(target);
		}

		template <class Type>
		void CleanupDeque(deque<Type> &target) {
			target.clear();
			deque<Type>(target).swap(target);
		}

		Messege GetDataType(string target) {
			using std::regex_match;
			//default error messege
			Messege result(kStrRedirect, kCodeIllegalArgs);

			auto match = [&] (const regex &pat) -> bool {
				return regex_match(target, pat);
			};

			if (match(kPatternFunction)) {
				result.SetCode(kTypeFunction);
			}
			else if (match(kPatternString)) {
				result.SetCode(kTypeString);
			}
			else if (match(kPatternBoolean)) {
				result.SetCode(kTypeBoolean);
			}
			else if (match(kPatternInteger)) {
				result.SetCode(kTypeInteger);
			}
			else if (match(kPatternDouble)) {
				result.SetCode(KTypeDouble);
			}
			else if (match(kPatternSymbol)) {
				result.SetCode(kTypeSymbol);
			}
			
			return result;
		}
	};



	class InputSource {
	private:
		std::ifstream stream;
		size_t current;
		vector<string> pool;

		InputSource() {}

		bool IsStreamReady() const {
			return (stream.is_open() && stream.good());
		}
	public:
		InputSource(string target) {
			stream.open(target.c_str(), std::ios::in);
			current = 0;
		}

		InputSource(char *target) {
			stream.open(target, std::ios::in);
			current = 0;
		}

		~InputSource() {
			stream.close();
			Util().CleanupVector(pool);
		}

		bool IsPoolReady() const {
			return !(pool.empty());
		}

		size_t WalkBack(size_t step = 1) {
			if (step > current) {
				current = 0;
			}
			else {
				current -= step;
			}

			return current;
		}

		void ResetReader() {
			current = 0;
		}

		void ResetPool() {
			Util().CleanupVector(pool);
		}

		string GetString();
	};

	class Token {
	private:
		string leftitem;
		string rightitem;
		string operation;
		Token *lefttoken;
		Token *righttoken;
	public:
		Token() {
			leftitem = kStrNothing;
			rightitem = kStrNothing;
			operation = kStrPass;
			lefttoken = nullptr;
			righttoken = nullptr;
		}

		string SetOperation(string &operation) {
			this->operation = operation;
		}

		string GetOperation() const {
			return this->operation;
		}

		bool SetLeftItem(string leftitem, Token *lefttoken = nullptr) {
			if (leftitem == kStrRedirect) {
				this->leftitem = kStrEmpty;
				this->lefttoken = lefttoken;
			}
			else {
				this->leftitem = leftitem;
				this->lefttoken = nullptr;
			}
		}

		bool SetRightItem(string rightitem, Token *righttoken = nullptr) {
			if (rightitem == kStrRedirect) {
				this->rightitem = kStrEmpty;
				this->righttoken = lefttoken;
			}
			else {
				this->rightitem = leftitem;
				this->righttoken = nullptr;
			}
		}

		bool IsLeaf() const {
			bool result;
			result = (leftitem == kStrNothing && rightitem == kStrNothing)
				|| ((leftitem == kStrRedirect && lefttoken == nullptr) &&
					(rightitem == kStrRedirect && righttoken == nullptr));
		}

	};

	class Chainloader {
	private:
		vector<string> raw;
	public:
		Chainloader() {}

		Chainloader build(string target);

		Messege execute();
	};

	class FunctionNode {
	private:
		string name;

	public:
	};
}

#endif // !_SE_PARSER_

