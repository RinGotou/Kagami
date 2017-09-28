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

	return result;
}

bool ScriptStorage::Build(string src, bool ForceOverride = false) {

}