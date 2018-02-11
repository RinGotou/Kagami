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
	using std::regex_match;

	const string kStrEmpty = "";
	const string kStrFatalError = "__FATAL";
	const string kStrWarning = "__WARNING";
	const string kStrEOF = "__EOF";
	const string kStrPass = "__PASS";
	const string kStrNothing = "__NOTHING";
	const string kStrRedirect = "__*";

	const string kstrDefine = "def";
	const string kStrVar = "var";
	const string kStrReturn = "return";
	const string kStrFor = "for";
	const string kStrWhile = "while";

	const int kCodeRedirect = 2;
	const int kCodeStandby = 1;
	const int kCodeNothing = 1;
	const int kCodeSuccess = 0;
	const int kCodeBrokenEngine = -1;
	const int kCodeOverflow = -2;
	const int kCodeIllegalArgs = -3;
	const int kCodeIllegalCall = -4;
	const int kCodeIllegalSymbol = -5;

	const size_t kTypeFunction = 0;
	const size_t kTypeString = 1;
	const size_t kTypeInteger = 2;
	const size_t KTypeDouble = 3;
	const size_t kTypeBoolean = 4;
	const size_t kTypeSymbol = 5;
	const size_t kTypeNull = 100;

	const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
	const regex kPatternString(R"("(\"|\\|\n|\t|[^"])*")");
	const regex kPatternNumber(R"(\d+\.?\d*)");
	const regex kPatternInteger(R"(\d+)");
	const regex kPatternDouble(R"(\d+\.\d+)");
	const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
	const regex kPatternSymbol(R"(==|<=|>=|&&|\|\||[[:Punct:]]|len)");
	const regex kPatternBlank(R"([[:blank:]])");

	class Messege;
	class EntryProvider;
	typedef Messege (*Activity)(vector<string> &);

	class Messege {
	private:
		string value;
		int code;
	public:
		Messege() {
			value = kStrEmpty;
			code = kCodeStandby;
		}

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

		Messege GetDataType(string target);
		bool ActivityStart(EntryProvider &provider, vector<string> container,
			vector<string> &raw, size_t top, Messege &msg);
	};



	class ScriptProvider {
	private:
		std::ifstream stream;
		size_t current;
		vector<string> pool;

		ScriptProvider() {}

		bool IsStreamReady() const {
			return (stream.is_open() && stream.good());
		}
	public:
		ScriptProvider(string target) {
			stream.open(target.c_str(), std::ios::in);
			current = 0;
		}

		ScriptProvider(char *target) {
			stream.open(target, std::ios::in);
			current = 0;
		}

		~ScriptProvider() {
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

	class Chainloader {
	private:
		vector<string> raw;
	public:
		Chainloader() {}
		Chainloader Build(vector<string> raw) {
			this->raw = raw;
			return *this;
		}

		Chainloader Build(string target);
		Messege Execute();
	};

	class EntryProvider {
	private:
		string name;
		Activity activity;
		int requiredcount;
	public:
		EntryProvider() : name(kStrNothing), activity(nullptr) {
			requiredcount = -1;
		}
		EntryProvider(string n, Activity a, int r = 0) : name(n){
			requiredcount = r;
		}


		string GetName() const {
			return this->name;
		}

		Activity GetActivity() const {
			return this->activity;
		}

		int GetRequiredCount() const {
			return this->requiredcount;
		}

		bool Good() const {
			return (activity != nullptr && requiredcount != -1);
		}

		bool operator==(EntryProvider &target) {
			return (target.name == this->name &&
				target.activity == this->activity &&
				target.requiredcount == this->requiredcount);
		}

		Messege StartActivity(vector<string> p);
	};

	//TODO:JSON Mini Parser
	//--------------!!WORKING!!-----------------//
	class JSONProvider {
	private:

	public:

	};

	void TotalInjection();
}

#endif // !_SE_PARSER_

