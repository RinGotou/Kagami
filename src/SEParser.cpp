#include "SEParser.h"

using namespace suzu;

namespace resources {
	namespace tracking {
		deque<Messege> base;

		void Reset() {
			Util().CleanupDeque(base);
		}
	}

	namespace entry {
		deque<EntryProvider> base;

		void Init() {
			//
		}

		EntryProvider Query(string target) {
			EntryProvider result;
			for (auto &unit : base) {
				if (unit.GetName() == target) {
					result = unit;
				}
			}

			return result;
		}
	}
}

Messege Util::GetDataType(string target) {
	using std::regex_match;
	//default error messege
	Messege result(kStrRedirect, kCodeIllegalArgs);

	auto match = [&](const regex &pat) -> bool {
		return regex_match(target, pat);
	};

	if (match(kPatternFunction)) {
		result.SetCode(kTypeFunction);
	}
	else if (match(kPatternString)) {
		result.SetCode(kTypeString);
	}
	else if (match(kPatternBoolean)) {
		result.SetCode(kTypeBoolean);
	}
	else if (match(kPatternInteger)) {
		result.SetCode(kTypeInteger);
	}
	else if (match(kPatternDouble)) {
		result.SetCode(KTypeDouble);
	}
	else if (match(kPatternSymbol)) {
		result.SetCode(kTypeSymbol);
	}

	return result;
}

string ScriptProvider::GetString() {
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
		resources::tracking::base.push_back(Messege(kStrFatalError, kCodeOverflow));
		//TODO:Something else?
	}

	return result;
}

Chainloader Chainloader::Build(string target) {
	Util util;
	vector<string> output;
	char binaryoptchar = NULL;
	size_t size = target.size();
	size_t i;
	string current = kStrEmpty;
	//headlock:blank characters at head of raw string,false - enable
	//allowblank:blank characters at string value and left/right of operating symbols
	//not only blanks(some special character included)
	bool headlock = false;
	bool allowblank = false;

	if (target == kStrEmpty) {
		resources::tracking::base.push_back(
			Messege(kStrWarning, kCodeIllegalArgs));
		return *this;
	}

	//--------------!!DEBUGGING!!---------------------//
	for (i = 0; i < size; i++) {
		if (headlock == false && std::regex_match(string().append(1, target[i]),
			kPatternBlank)) {
			continue;
		}
		else if (headlock == false && std::regex_match(string().append(1, target[i]),
			kPatternBlank) == false) {
			headlock = true;
		}

		if (target[i] == '"') {
			if (allowblank && target[i - 1] != '\\' && i - 1 >= 0) {
				allowblank = !allowblank;
			}
			else if (!allowblank) {
				allowblank = !allowblank;
			}
		}

		switch (target[i]) {
		case '(':
		case ',':
		case ')':
			if (allowblank) {
				current.append(1, target[i]);
			}
			else {
				if (current != kStrEmpty) output.push_back(current);
				output.push_back(string().append(1, target[i]));
				current = kStrEmpty;
			}
			break;
		case '"':
			if (allowblank && target[i - 1] == '\\' && i - 1 >= 0) {
				current.append(1, target[i]);
			}
			else {
				if (current != kStrEmpty) output.push_back(current);
				output.push_back(string().append(1, target[i]));
				current = kStrEmpty;
			}
			break;
		case '=':
		case '>':
		case '<':
			if (allowblank) {
				current.append(1, target[i]);
			}
			else {
				if (i + 1 < size && target[i + 1] == '=') {
					binaryoptchar = target[i];
					if (current != kStrEmpty) output.push_back(current);
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
					if (current != kStrEmpty) output.push_back(current);
					output.push_back(string().append(1, target[i]));
					current = kStrEmpty;
				}
			}
			break;
		case ' ':
		case '\t':
			if (allowblank) {
				current.append(1, target[i]);
			}
			else if ((current == kStrVar || current == kstrDefine || current == kStrReturn)
				&& output.empty() == true) {
				if (i + 1 < size && target[i + 1] != ' ' && target[i + 1] != '\t') {
					output.push_back(current);
					current = kStrEmpty;
				}
				continue;
			}
			else {
				if ((std::regex_match(string().append(1, target[i + 1]), kPatternSymbol)
					|| std::regex_match(string().append(1, target[i - 1]), kPatternSymbol)
					|| target[i - 1] == ' ' || target[i - 1] == '\t')
					&& i + 1 < size) {
					continue;
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
	util.CleanupVector(output);

	return *this;
}

Messege Chainloader::Execute() {
	using namespace resources;

	Util util;
	EntryProvider provider;
	Messege result(kStrNothing, kCodeSuccess);
	Messege temp(kStrNothing, kCodeSuccess);
	size_t i, j;
	size_t size = raw.size();
	size_t forwardtype = kTypeNull;
	size_t top;
	bool rv;

	stack<string> symbols;
	stack<size_t> tracer;
	vector<string> elements;
	vector<string> container;

	auto ErrorTracking = [&result](int code, string value) {
		tracking::base.push_back(Messege(kStrFatalError, code));
		result.SetCode(code).SetValue(kStrFatalError);
	};

	auto ActivityStart = [&](EntryProvider &provider) -> bool {
		bool rv = true;
		if (provider.Good()) {
			temp = provider.StartActivity(container);

			if (temp.GetCode() != kCodeSuccess) {
				tracking::base.push_back(temp);
				result = temp;
				rv = false;
			}
			else if (temp.GetCode() == kCodeRedirect) {
				raw[top] = temp.GetValue();
			}
			else {
				raw[top] = kStrEmpty;
			}
		}
		else {
			ErrorTracking(kCodeIllegalCall, kStrFatalError);
			rv = false;
		}

		return rv;
	};

	//TODO:analyzing and execute
	//-----------------!!WORKING!!-----------------------//
	for (i = 0; i < size; i++) {
		if (regex_match(raw[i], kPatternSymbol)) {
			if (raw[i] == "(") {
				if (forwardtype == kTypeSymbol) {
					//TODO:brackets that not belong to any function
				}
				else {
					tracer.push(i);
					symbols.push(raw[i]);
				}
			}
			else if (raw[i] == ")") {
				//TODO:some expression inside the call?
				//TODO:last choice is creating temp chainloader
				//these codes may need to modify
				top = tracer.top();
				util.CleanupVector(container);

				while (elements.size() > top + 1) {
					container.push_back(elements.back());
					elements.pop_back();
				}
				provider = entry::Query(raw[top]);
				rv = ActivityStart(provider);
				if (rv == false) break;
			}
			else if (raw[i] == ",") {
				if (i = raw.size() - 1) {
					ErrorTracking(kCodeIllegalSymbol, kStrWarning);
				}
				forwardtype = kTypeSymbol;
				continue;
			}
			else {
				//
			}

			forwardtype = kTypeSymbol;
		}
		else {
			forwardtype = util.GetDataType(raw[i]).GetCode();
			if (forwardtype == kCodeIllegalArgs) {
				result.SetCode(kCodeIllegalArgs).SetValue(kStrFatalError);
				tracking::base.push_back(result);
				break;
			}
			elements.push_back(raw[i]);
		}

		while (!symbols.empty()) {
			util.CleanupVector(container);

			if (!symbols.empty() && elements.empty()) {
				ErrorTracking(kCodeIllegalSymbol, kStrFatalError);
				break;
			}
			if (symbols.top() == "(" || symbols.top() == ")") {
				ErrorTracking(kCodeIllegalSymbol, kStrFatalError);
				break;
			}

			provider = entry::Query(symbols.top());
			j = provider.GetRequiredCount();

			if (j > elements.size() - 1) {
				ErrorTracking(kCodeIllegalArgs, kStrFatalError);
				break;
			}

			while (j != 0) {
				container.push_back(elements.back());
				elements.pop_back();
			}
			
			rv = ActivityStart(provider);
			if (rv == false) break;
		}
	}

	return result;
}

Messege EntryProvider::StartActivity(vector<string> p) {
	Messege result;

	size_t size = p.size();
	if (size != requiredcount) {
		if (requiredcount = -1) {
			result.SetCode(kCodeBrokenEngine).SetValue(kStrFatalError);
		}
		else {
			result.SetCode(kCodeIllegalArgs).SetValue(kStrFatalError);
		}
	}
	else {
		result = activity(p);
	}

	return result;
}