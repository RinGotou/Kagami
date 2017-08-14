#pragma once
#include "ScarletIncludes.h"

namespace SEngine {
	using std::cin;
	using std::cout;
	using std::endl;
	using std::flush;
	using std::string;
	using std::vector;
	using std::copy;

	class MsgBridge;
	class DataCell;
	class Token;
	class Dict;

	const string STR_EMPTY = "";
	const string STR_TRUE = "true";
	const string STR_FALSE = "false";

	typedef MsgBridge(*funcPointer)(Token &, Dict &);

	class CodeUnit {
	protected:
		string key;
	public:
		string getKey() const { return key; }
	};

	class DataSet {
	private:
		string key;
	public:
		virtual size_t getSize() const = 0;
		string getKey() const { return key; }
	};

	class MsgBridge {
	private:
		string buf;
		int code;
	public:
		string getBuf() const { return buf; }
		int getCode() const { return code; }
		MsgBridge() {}

		string setBuf(const string src) { 
			this->buf = src;
			return buf;
		}
		
		int setCode(const int src) {
			this->code = src;
			return code;
		}

		void Report(bool onError = false,string ExtraMsg = STR_EMPTY) const {
			if (onError) {
				cout << "Error occured:";
			}
			else {
				cout << "Event occured:";
			}

			cout << buf << endl
				 << ExtraMsg << endl;
		}

		MsgBridge(string bSrc, int cSrc) {
			this->buf = bSrc;
			this->code = cSrc;
		}
	};

	const MsgBridge MSG_SUB_OUT_OF_RANGE("Subscript out of range", 1);
	const MsgBridge MSG_KEY_NOT_MATCH("Key is not matching any token", 2);
	const MsgBridge MSG_CELL_CONFLICT("DataCell is already existed", 3);
	const MsgBridge MSG_TOO_MANY_PARAM("Too many parameters", 4);

	class DataCell {
	private:
		string key, value;
	public:
		string getKey() const { return key; }
		string getValue() const { return value; }

		string setKey(const string src) {
			this->key = src;
			return key;
		}

		string setValue(const string src) {
			this->value = src;
			return value;
		}

		DataCell(string kSrc, string vSrc) {
			this->key = kSrc;
			this->value = vSrc;
		}

		DataCell() {
			this->key = STR_EMPTY;
			this->value = STR_EMPTY;
		}
	};


	class Array : public DataSet {
	protected:
		vector<string> set;
	public:
		size_t getSize() const { return set.size(); }
		Array() {}
		Array(vector<string> &src) { set = src; }
		Array(string unit) { set.push_back(unit); }
		string getCell(const size_t) const;
		string setCell(const size_t, const string);
		size_t replace(const string, const string);
	};

	class Dict : public DataSet {
	private:
		vector<DataCell> set;
	public:
		size_t getSize() const { return set.size(); }
		Dict() {}
		Dict::Dict(vector<DataCell> &src) { set = src; }
		Dict(DataCell unit) { set.push_back(unit); }
		DataCell getCell(const size_t subscr) const;
		DataCell getCell(const string cellKey) const;
		size_t append(DataCell);
		size_t pop();
		size_t eraseByKey(string);
		size_t eraseByValue(string);
		size_t eraseBySub(size_t);
	};

	class Stack : public Array {
	public:
		string append(string);
		string pop();
	};

	class EnumSet : public Stack {
	private:
		vector<string> set;
		string setCell(const size_t, const string) {}
		string replace(const string) {}
	};

	//for resizable parameter dict, ParamCount should be -1.

	class Activity : public CodeUnit {
	private:
		funcPointer dest;
		size_t ParamCount;
	public:
		MsgBridge operator()(Token &, Dict &);

		Activity(string kSrc, funcPointer pSrc, size_t cSrc = -1) {
			this->key = kSrc;
			this->dest = pSrc;
			this->ParamCount = cSrc;
		}

		Activity() {
			dest = nullptr;
			ParamCount = -1;
		}
	};

	class Token : public CodeUnit {
	private:
		void *destPtr;
	public:
		template<class AnyType>
		MsgBridge Chainloader(int);

		Token(string kSrc) { this->key = kSrc; }
	};

	class Function : public CodeUnit {
	private:

	};

	class CodeBlock {
	private:
		vector<Token> Origins;
	public:
		MsgBridge Chainloader(int);
	};
}
