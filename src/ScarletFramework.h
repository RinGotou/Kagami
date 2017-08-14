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
	class Command;
	class Array;

	typedef MsgBridge(*funcPointer)(Command &, Array &);

	class MsgBridge {
	private:
		string buf;
		int code;
	public:
		string getBuf() const { return buf; }
		int getCode() const { return code; }

		string setBuf(const string src) { 
			this->buf = src;
			return buf;
		}
		
		int setCode(const int src) {
			this->code = src;
			return code;
		}
	};

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
	};

	class DataSet {
	private:
		string key;
	public:
		virtual DataCell getCell() const = 0;
		virtual size_t getSize() const = 0;

		virtual string getKey() const { return key; }
	};

	class Array : public DataSet{
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

	class Activity {
	private:
		string key;
		funcPointer dest;
	public:
		string getKey() const { return key; }

		MsgBridge operator()(Command &cmd, Array &dataSet) {
			return dest(cmd, dataSet);
		}
	};

	class Command {

	};
}
