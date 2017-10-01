/*
*	Suzu on SVM (C)2016-2017 DandelionDIMN
*	licensed under LGPL 2.1
*	
*	SVM.cpp - Processor Core Code.
*	TIP:You can find some comments in header files,not all comments here.
*/
#include "SVM.h"

using namespace SVM;

size_t ScriptStorage::Build() {
	size_t CurrentSize = 0;
	string StringCache = STR_EMPTY;
	MsgBridge msg;

	if (ifs.good()) {
		while (ifs.eof() != true && CurrentSize < SSTOR_MAX_BUF_SIZE) {
			std::getline(ifs, StringCache);
			PrimevalString.push_back(StringCache);
			CurrentSize++;
		}

		if (ifs.eof()) {
			ifs.close();
			CurrentSize = 0;
		}
	}
	else {
		CurrentSize = 0;
	}


	return CurrentSize;
}

string ScriptStorage::Read() {
	string result = STR_EMPTY;
	size_t EOS = PrimevalString.size() - 1;

	if (CurrentLine <= EOS) {
		result = PrimevalString.at(CurrentLine);
		CurrentLine++;
	}
	else {
		if (ifs.eof() != true) {
			Build();
			EOS = PrimevalString.size() - 1;
			if (CurrentLine <= EOS) {
				result = PrimevalString.at(CurrentLine);
				CurrentLine++;
			}
		}
		else {
			result = STR_EMPTY;
		}
	}

	return result;
}

TypeEnum SVM::Token::getType(int Mode) const {
	//PENDING
	using std::regex_match;
	if (regex_match(value, PatternRegChar)) {
		//TODO:Finding Registered Token(function,variable,etc.)


	}
	if (regex_match(value, PatternNum)) {

	}
	if (regex_match(value, PatternBool)) {

	}
	if (regex_match(value, PatternStr)) {

	}

	return TypeStr;
}