#include "ScarletFramework.h"

using namespace SEngine;

namespace SEngine {
	size_t CurrentLine;


}

string Array::getCell(const size_t subscr) const {
	string result = STR_EMPTY;

	if (subscr >= set.size()) {
		MSG_SUB_OUT_OF_RANGE.Report(true);
	}
	else {
		result = set[subscr];
	}

	return result;
}

string Array::setCell(const size_t subscr, const string value) {
	string result = STR_EMPTY;

	if (subscr >= set.size()) {
		MSG_SUB_OUT_OF_RANGE.Report(true);
	}
	else {
		set[subscr] = value;
		result = set[subscr];
	}

	return result;
}

size_t Array::replace(const string src, const string dest) {
	size_t size = 0;
	for (auto &unit : set) {
		if (unit == dest) {
			unit == src;
			++size;
		}
	}

	return size;
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

size_t Dict::append(DataCell src) {
	for (const auto &unit : set) {
		if (unit.getKey() == src.getKey()) {
			MSG_CELL_CONFLICT.Report();
			return set.size();
		}
	}

	set.push_back(src);
	return set.size();
}

size_t Dict::eraseByKey(string dest) {
	vector<DataCell>::iterator t, destPtr;
	bool result = false;

	for (t = set.begin(); t != set.end(); t++){
		if (t->getKey() == dest) {
			destPtr = t;
			result = true;
		}
	}

	if (result == true) {
		set.erase(destPtr);
	}

	return set.size();
}

size_t Dict::eraseByValue(string dest) {
	vector<DataCell>::iterator t;
	vector<vector<DataCell>::iterator> destPtr;
	bool result = false;
	
	for (t = set.begin(); t != set.end(); t++) {
		if (t->getKey() == dest) {
			destPtr.push_back(t);
			result = true;
		}
	}

	if (result == true) {
		for (const auto &unit : destPtr) {
			set.erase(unit);
		}
	}

	return set.size();
}

size_t Dict::eraseBySub(size_t subscr) {
	if (subscr >= set.size()) {
		MSG_SUB_OUT_OF_RANGE.Report(true);
		return set.size();
	}

	vector<DataCell>::iterator t = set.begin();
	size_t i = 0; //loop

	while (i < subscr) {
		++i;
		++t;
	}

	set.erase(t);

	return set.size();
}

string Stack::append(string value) {
	set.push_back(value);
	return set.back();
}

string Stack::pop() {
	set.pop_back();
	return set.back();
}

