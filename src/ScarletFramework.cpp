#include "ScarletFramework.h"

using namespace SEngine;

namespace SEngine {
	size_t CurrentLine;


}

Array::Array(vector<string> &src) {
	set = src;
	ArraySize = set.size();
}

Array::Array(string unit) {
	set.push_back(unit);
	ArraySize = set.size();
}

string Array::getCell(const size_t subscr) const {
	string result = STR_EMPTY;

	if (subscr >= ArraySize) {
		MSG_SUB_OUT_OF_RANGE.Report(true);
	}
	else {
		result = set[subscr];
	}

	return result;
}

DataCell Dict::getCell(const size_t subscr) const {
	DataCell result;

	if (subscr >= set.size()) {
		MSG_SUB_OUT_OF_RANGE.Report(true);
		result = *(new DataCell());
	}
	else {
		result = set[subscr];
	}

	return result;
}

DataCell Dict::getCell(const string cellKey) const {
	DataCell result = *(new DataCell());

	for (const auto &unit : set) {
		if (unit.getKey() == cellKey) {
			result = unit;
			break;
		}
	}

	return result;
}