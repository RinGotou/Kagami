#include "SEParser.h"

using namespace suzu;

int main(int argc, char **argv) {

	Util util;

	Messege result = util.ScriptStart(R"(C:\Apps\1.txt)");

	if (result.GetCode() == kCodeSuccess) {
		std::cout << "script finished." << std::endl;
	}
	else {
		std::cout << result.GetCode() << '\n' << result.GetValue() << std::endl;
	}

	util.PrintEvents();

	return 0;
}