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

Chainloader Chainloader::build(string target) {
	Util util;
	vector<string> output;
	char binaryoptchar = NULL;
	size_t size = target.size();
	size_t i;
	string current = kStrEmpty;
	//headlock:blank characters at head of raw string,false - enable
	//allowblank:blank characters at string value and left/right of operating symbols
	bool headlock = false;
	bool allowblank = false;

	if (target == kStrEmpty) {
		resources::eventbase::base.push_back(
			Messege(kStrWarning, kCodeIllegalArgs));
		return *this;
	}

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

		if (target[i] == '"') allowblank = !allowblank;


		switch (target[i]) {
		case '(':
		case ',':
		case ')':
		case '"':
			output.push_back(current);
			output.push_back(string().append(1, target[i]));
			current = kStrEmpty;
			break;
		case '=':
		case '>':
		case '<':
			if (i + 1 < size && target[i + 1] == '=') {
				binaryoptchar = target[i];
				output.push_back(current);
				current = kStrEmpty;
				continue;
			}
			else if (binaryoptchar != NULL) {
				string binaryopt = { binaryoptchar, target[i] };
				if (util.GetDataType(binaryopt).GetCode() == kTypeSymbol) {
					output.push_back(binaryopt);
					binaryoptchar = NULL;
				}
			}
			else {
				output.push_back(current);
				output.push_back(string().append(1, target[i]));
				current = kStrEmpty;
			}
			break;
		case ' ':
		case '\t':
			if (allowblank) {
				current.append(1, target[i]);
			}
			else {
				if (i + 1 < size) {
					if (std::regex_match(string().append(1, target[i + 1]), kPatternSymbol)
						|| std::regex_match(string().append(1, target[i - 1]), kPatternSymbol)
						|| target[i - 1] == ' ' || target[i - 1] == '\t') {
						continue;
					}
				}
				else {
					continue;
				}
			}

			break;
		default:
			current.append(1, target[i]);
			break;
		}
	}

	raw = output;

	return *this;
}