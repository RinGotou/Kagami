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
	class Call;
	class Array;

	const string STR_EMPTY = "";
	const string STR_TRUE = "true";
	const string STR_FALSE = "false";

	typedef MsgBridge(*funcPointer)(Call &, Array &);

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

		int Report(bool onError = false,string ExtraMsg = STR_EMPTY) const {
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

		MsgBridge(MsgBridge &src) {
			this->buf = src.buf;
			this->code = src.code;
		}
	};

	const MsgBridge MSG_SUB_OUT_OF_RANGE("Subscript out of range", 1);
	const MsgBridge MSG_KEY_NOT_MATCH("Key is not matching any token", 2);

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

	class DataSet {
	private:
		string key;
	public:
		virtual DataCell getCell() const = 0;
		virtual size_t getSize() const = 0;
		virtual string getKey() const { return key; }
	};

	class Array : public DataSet {
	private:
		vector<string> set;
		size_t ArraySize;
	public:
		size_t getSize() const { return ArraySize; }
		Array() { ArraySize = 0; }
		Array(vector<string> &);
		Array(string);
		string getCell(const size_t) const;

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
	};

	class Stack : public DataSet {

	};

	class Activity {
	private:
		string key;
		funcPointer dest;
	public:
		string getKey() const { return key; }

		MsgBridge operator()(Call &call, Array &dataSet) {
			return dest(call, dataSet);
		}
	};

	class Call {
		string key;

	};
}
