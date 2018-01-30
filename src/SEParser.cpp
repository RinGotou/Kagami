#include "SEParser.h"

using namespace suzu;

namespace resources {
	namespace eventbase {
		deque<Messege> base;

		void Reset() {
			Util().CleanupDeque(base);
		}
	}

	namespace functions {
		vector<FunctionNode> registry;

		bool Init();
	}
}

string InputSource::GetString() {
	string currentstr, result;

	auto IsBlankStr = [] (string target) -> bool { 
		if (target == kStrEmpty || target.size() == 0) return true;
		for (const auto unit : target) {
			if (unit != ' ' || unit != '\n' || unit != '\t' || unit != '\r') {
				return false;
			}
		}
		return true;
	};

	if (pool.empty() && IsStreamReady()) {
		while (!(stream.eof())) {
			std::getline(stream, currentstr);
			if (IsBlankStr(currentstr) != true) {
				pool.push_back(currentstr);
			}
		}
		stream.close();
	}

	size_t size = pool.size();
	if (current < size) {
		result = pool.at(current);
		current++;
	}
	else if (current == size) {
		result = kStrEOF;
	}
	else {
		result = kStrFatalError;
		resources::eventbase::base.push_back(Messege(kStrFatalError, kCodeOverflow));
		//TODO:Something else?
	}

	return result;
}

Messege Chainloader::build(string target) {
	Util util;
	stack<string> operations;
	vector<string> output;
	size_t size = target.size();
	size_t type;
	size_t i;
	string current = kStrEmpty;
	bool headlock = false;
	

	if (target == kStrEmpty) return Messege(kStrWarning, kCodeIllegalArgs);


	//--------------!!WORKING!!---------------------//
	for (i = 0; i < size; i++) {
		if (headlock == false && std::regex_match(string().append(1, target[i]),
			kPatternBlank)) {
			continue;
		}
		else if (headlock == false && std::regex_match(string().append(1, target[i]),
			kPatternBlank) == false) {
			headlock == true;
		}


		if (!(current.empty())) {
			//TODO:identify
		}
		else if (current.empty()) {
			current.append(1, target[i]);
		}
	}
}