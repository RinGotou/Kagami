/*
*	Suzu on SVM (C)2016-2017 DandelionDIMN
*	licensed under LGPL 2.1
*	
*	SVM.cpp - Processor Core Code.
*	TIP:You can find some comments in header files,not all comments here.
*/
#include "SVM.h"

using namespace SVM;

//basic integer and double string 
inline bool isNumber(string src) {
	return std::regex_match(src, PatternNum);
}

//boolean string
inline bool isBoolean(string src) {
	return std::regex_match(src, PatternBool);
}

size_t SVM::FindTwinBracket(const string &src, size_t left) {
	const size_t SrcSize = src.size();

	size_t result, i;
	size_t BracketA = 1, BracketB = 0;

	for (i = left; i < SrcSize; ++i) {
		if (BracketA == BracketB) {
			break;
		}
		if (src[i] == ')') {
			++BracketB;
			result = i;
		}
		if (src[i] == '(') {
			++BracketA;
		}
	}

	if (BracketA > BracketB) {
		result = left;
	}
}

vector<string> SpiltByComma(const string &src) {

}

string Token::GetTokenContentString(const string &src) {
	const size_t SrcSize = src.size();
	const size_t right = SrcSize - 1;
	size_t left = 0;
	string result;

	if (src.at(right) == ')') {
		while (src[left] != '(' && left < SrcSize) {
			++left;
		}

		if (left < SrcSize) {
			result = src.substr(left + 1, right - left);
		}
		else {
			result = STR_EMPTY;
		}
	}
	else {
		result = STR_EMPTY;
	}

	return result;
}

MsgBridge Token::InitTokenTree(const string &buf) {
	const size_t BufSize = buf.size();
	size_t i = 0;
	MsgBridge msg;
	vector<string> TokenBufPool;

	//Init Pool
	TokenBufPool.push_back(STR_EMPTY);
	string *BufPtr = &(TokenBufPool.back());

	//Init Identity Switches
	struct {
		char value;
		
	}LastChar;

	//lambda function 
	//SwitchPointer - refresh BufPtr to read latest unit in pool.
	//PushBack - Fill char to latest unit
	//PatternCheck - Check char with custom pattern
	auto SwitchPointer = [&BufPtr, &TokenBufPool]() {
		BufPtr = &(TokenBufPool.back());
	};

	auto PushBack = [&TokenBufPool](const char &unit) {
		return TokenBufPool.back().append(1, unit);
	};

	auto CharCheck = [&BufPtr](const regex &Pat) {
		return std::regex_match(*BufPtr, Pat);
	};

	//Walkup
	for (i = 0; i < BufSize; ++i) {
		PushBack(buf[i]);

		LastChar.value = buf[i];
	}
}

MsgBridge Token::ExecToken(int mode) {

}

Token &Token::getContent(size_t sub) {

}