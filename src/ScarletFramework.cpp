#include "ScarletFramework.h"

using namespace SEngine;

Array::Array(vector<string> &src) {
	set = src;
	ArraySize = set.size();
}

Array::Array(string unit) {
	set.push_back(unit);
	ArraySize = set.size();
}

string Array::getCell(const size_t key) const {

}