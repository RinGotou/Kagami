/*
*	Suzu on SVM (C)2016-2017 DandelionDIMN
*	licensed under LGPL 2.1
*/
#include "SVM.h"

using namespace SVM;

//basic integer and double string 
inline bool isNumber(string src) {
	const regex Pattern(R"(\d+|\d+\.?\d*)");

	return std::regex_match(src, Pattern);
}

//boolean string
inline bool isBoolean(string src) {
	return (src == "true" || src == "false");
}

size_t SVM::FindTwinBracket(const string &src, size_t left) {
	const size_t SrcSize = src.size();
	size_t result, i;

	if (src[left] == '(') {
		i = SrcSize;
		--i;

		while (src[i] != ')' && i != left) {
			--i;
		}

		result = i;
	}
	else {
		result = left;
	}

	return result;
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

MsgBridge Token::InitTokenTree(string buf) {
	MsgBridge msg;
	const regex PatternA(R"([a-zA-Z_][a-zA-Z_0-9]*|==|<=|>=|&&|\|\||p{Punct})"),
		PatternStr(R"("(\"|\\|\n|\t|[^"])*")"),
		PatternDict(R"(\(([a-zA-Z0-9_()=<>|&]*)\))");
}

MsgBridge Token::ExecToken(int mode) {

}

Token &Token::getContent(size_t sub) {

}