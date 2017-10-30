/*
*	Suzu on SVM (C)2016-2017 DandelionDIMN
*	licensed under LGPL 2.1
*
*	SVM.h Processor Dimension.
*/
#pragma once
#include "CommonIncludes.h"

namespace SVM {
	using std::cout;
	using std::cin;
	using std::endl;
	using std::flush;
	using std::string;
	using std::vector;
	using std::deque;
	using std::list;
	using std::regex;

	const string STR_EMPTY = "";
	const string STR_EOL = "\n";
	const string MSG_ILLEGAL_TOKEN = "__MSG_ILLEGAL_TOKEN";
	const size_t SSTOR_MAX_BUF_SIZE = 2000;
	//regular name/==/<=/>=/&&/||/symbol
	const regex PatternA(R"([a-zA-Z_][a-zA-Z_0-9]*|==|<=|>=|&&|\|\||p{Punct})");

	const regex PatternRegChar(R"([a-zA-Z_][a-zA-Z_0-9]*)");
	const regex PatternStr(R"("(\"|\\|\n|\t|[^"])*")");

	const regex PatternNum(R"(\d+\.?\d*)");
	const regex PatternInt(R"(\d+)");
	const regex PatternDouble(R"(\d+\.\d+)");
	const regex PatternBool(R"(\btrue\b|\bfalse\b)");
	
	class MsgBridge;
	class Token;

	typedef MsgBridge(*VMInterface)(Token &);
	typedef enum {
		TypeNull = -1,
		TypeInt = 0,
		TypeDouble,
		TypeArray,
		TypeBool,
		TypeDict,
		TypeSet,
		TypeStr,
		TypeRegChar,
		TypeOper
	} TypeEnum;


	//Message Bridge Class
	//For message code level intro,please visit Suzu's dev document.
	class MsgBridge {
	private:
		string buf;
		int code;
	public:
		string getBuf() const {
			return buf;
		}
		int getCode() const {
			return code;
		}
		MsgBridge() : buf(STR_EMPTY) {
			code = 0;
		}

		string setBuf(const string src) {
			this->buf = src;
			return buf;
		}

		int setCode(const int src) {
			this->code = src;
			return code;
		}

		//method to show message
		void Report(bool onError = false, string ExtraMsg = STR_EMPTY) const {
			if (onError) {
				cout << "Error occured:";
			}
			else {
				cout << "Event occured:";
			}

			cout << buf << endl
				<< ExtraMsg << endl;
		}

		MsgBridge(string buf, int code) {
			this->buf = buf;
			this->code = code;
		}
	};

	//Script storage will not analyse any words in script.
	//It will buffer strings and provide source code string to treebuilder.
	class ScriptStorage {
	private:
		std::ifstream ifs;
		size_t CurrentLine;
		deque<string> PrimevalString;

		//buffering main function
		size_t Build();

		ScriptStorage() {
			CurrentLine = -1;
		}
	public:
		//start buffering and provide current string
		string Read();

		bool getHealth() const{
			return ifs.good();
		}

		void Reset() {
			ifs.close();
			CurrentLine = -1;
			PrimevalString.clear();
			deque<string>(PrimevalString).swap(PrimevalString);
		}

		void Back() {
			if (CurrentLine > 0) {
				CurrentLine--;
			}
		}

		void BackToHead() {
				CurrentLine = 0;
		}

		ScriptStorage(string src) {
				ifs.open(src.c_str(), std::ios::in);
				CurrentLine = 0;
		}
	};

	//The basic word unit in Grammar Tree.
	//This is a weak type language,so it contains a identifer function
	//to judge word's type and return to treebuilder.
	class Token {
	private:
		string value;
	public:
		string getValue() const {
			return this->value;
		}

		string setValue(string value) {
			this->value = value;
			return this->value;
		}

		string operator()(string value) {
			this->value = value;
			return this->value;
		}

		bool compare(Token& src) const {
			return (src.value == this->value);
		}

		bool compare(string src) const {
			return (this->value == src);
		}

		TypeEnum getType(int Mode = 0) const;
	}

