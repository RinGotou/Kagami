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
	using std::list;
	using std::regex;

	const string STR_EMPTY = "";
	const string MSG_ILLEGAL_TOKEN = "__MSG_ILLEGAL_TOKEN";
	const regex PatternA(R"([a-zA-Z_][a-zA-Z_0-9]*|==|<=|>=|&&|\|\||p{Punct})");
	const regex PatternRegChar(R"([a-zA-Z_][a-zA-Z_0-9])");
	const regex PatternOperator(R"([=<>+-*/\|])");
	const regex PatternStr(R"("(\"|\\|\n|\t|[^"])*")");
	const regex PatternDict(R"(\(([a-zA-Z0-9_()=<>|&]*)\))");
	const regex PatternNum(R"(\d+|\d+\.?\d*)");
	const regex PatternBool(R"(\btrue\b|\bfalse\b)");

	class MsgBridge;
	class Token;
	class Dict;

	typedef MsgBridge(*VMInterface)(Token &);

	//find right bracket symbol for the left one.
	//if there's nothing found,it will return the original left bracket value.
	size_t FindTwinBracket(const string &src, size_t left);


	//Message Bridge Class
	//For message code level intro,please visit Suzu's dev document.
	class MsgBridge {
	private:
		string buf;
		int code;
	public:
		string getBuf() const { return buf; }
		int getCode() const { return code; }
		MsgBridge() : buf(STR_EMPTY) { code = 0; }

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

	//Token Class.
	class Token {
	private:
		string key;
		vector<Token> TokenContent;
		size_t Subscript;
		bool HealthState;

		//get string inside the token's brackets.
		//automatically,no need to provide left bracket.
		string GetTokenContentString(const string &src);
	public:
		Token() : key(STR_EMPTY) { 
			Subscript = 0;
			HealthState = false;
		}
		string getKey() const { return key; }
		bool getHealthState() const { return HealthState; }
		bool setHealthState(const bool state) { HealthState = state; }
		MsgBridge InitTokenTree(const string &buf);
		MsgBridge ExecToken(int mode);
		Token &getContent(size_t sub);
	};

	//Common Parent of Memory Unit Types.
	//DO NOT use it directly.
	class MemUnit {
	protected:
		string key;
	public:
		string getKey() const { return key; }
	};
}