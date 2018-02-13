#include "SEParser.h"

namespace tracking {
	vector<Messege> base;

	void Reset() {
		Util().CleanUpVector(base);
	}

	void log(Messege &msg, string res) {
		base.push_back(msg);
	}
}

namespace entry {
	deque<EntryProvider> base;
	deque<MemoryProvider> childbase;

	void Inject(EntryProvider &provider) {
		base.push_back(provider);
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

bool Util::ActivityStart(EntryProvider &provider, vector<string> container, vector<string> &elements,
	size_t top, Messege &msg) {
	bool rv = true;
	int code = kCodeSuccess;
	Messege temp(kStrNothing, kCodeSuccess);

	if (provider.Good()) {
		temp = provider.StartActivity(container);
		code = temp.GetCode();

		if (code < kCodeSuccess) {
			tracking::log(temp,"ActivityStart 1");
			msg = temp;
			rv = false;
		}
		else if (code == kCodeRedirect) {
			if (top == elements.size()) {
				elements.push_back(temp.GetValue());
			}
			else {
				elements[top] = temp.GetValue();
			}
		}
		else {
			if (top == elements.size()) {
				elements.push_back(kStrEmpty);
			}
			else {
				elements[top] = kStrEmpty;
			}
		}
	}
	else {
		msg.SetCode(kCodeIllegalCall).SetValue(kStrFatalError);
		tracking::log(msg, "ActivityStart 2");
		rv = false;
	}

	return rv;
}

void Util::PrintEvents() {
	using tracking::base;
	ofstream ofs("event.log",std::ios::trunc);
	size_t i = 0;
	if (ofs.good()) {

		if (base.empty()) {
			ofs << "No Events\n";
		}
		else{
			for (auto unit : base) {
				++i;
				ofs << "Count:" << i << "\n"
					<< "Code:" << unit.GetCode() << "\n";

				if (unit.GetValue() == kStrFatalError) {
					ofs << "Priority:Fatal\n";
				}
				else {
					ofs << "Priority:Warning\n";
				}

				if (unit.GetDetail() != kStrEmpty) {
					ofs << "Detail:" << unit.GetDetail() << "\n";
				}

				ofs << "-----------------------\n";
			}
		}

		ofs << "Event Output End\n";
	}

	ofs.close();
}

void Util::Cleanup() {

}

Messege ScriptProvider::Get() {
	using tracking::log;
	string currentstr;
	Messege result(kStrEmpty,kCodeSuccess);

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
	else {
		result.SetValue(kStrFatalError).SetCode(kCodeBadStream);

		log(result,"ScriptProvder::Get() 1");

		return result;
	}

	size_t size = pool.size();
	if (current < size) {
		result.SetValue(pool[current]);
		current++;
	}
	else if (current == size) {
		result.SetValue(kStrEOF);
	}
	else {
		result.SetValue(kStrFatalError).SetCode(kCodeOverflow);
		log(result, "ScriptProvder::Get() 2");
	}

	return result;
}

Chainloader &Chainloader::Build(string target) {
	using tracking::log;
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
		log(Messege(kStrWarning, kCodeIllegalArgs), "Chainloader::Build() 1");
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
	util.CleanUpVector(output);

	return *this;
}

Messege Chainloader::Execute() {
	Util util;
	EntryProvider provider;
	Messege result(kStrNothing, kCodeSuccess);
	Messege temp(kStrNothing, kCodeSuccess);
	size_t i, j, top;
	size_t size = raw.size();
	size_t forwardtype = kTypeNull;
	//size_t StopCode = 0;
	bool rv = true;
	bool commaexpression = false;
	bool directappend = false; //temp fix for string type

	vector<string> symbols;
	vector<size_t> tracer;
	vector<string> elements;
	vector<string> container;
	//string deposit;

	auto ErrorTracking = [&result](int code, string value, string detail) {
		tracking::log(Messege(value, code).SetDetail(detail), "Chainloader::Execute() ");
		result.SetCode(code).SetValue(value).SetDetail(detail);
	};

	auto GetPriority = [](string value) -> int {
		if (value == "+" || value == "-") return 1;
		//if (value == "**") return 3;
		return 2;
	};

	//TODO:Rebuild algorithm
	//-----------------!!WORKING!!-----------------------//
	for (i = 0; i < size; i++) {
		if (regex_match(raw[i], kPatternSymbol)) {
			if (raw[i] == "(") {
				if (forwardtype == kTypeSymbol || forwardtype == kTypeNull) {
					commaexpression = true;
				}
				tracer.push_back(i - 1);
				symbols.push_back(raw[i]);
			}
			else if (raw[i] == ")") {
				top = tracer.back();

				if (symbols.empty()) {
					ErrorTracking(kCodeIllegalSymbol, kStrFatalError, "can't find left bracket");
					break;
				}

				while (symbols.back() != "(") {
					util.CleanUpVector(container);
					provider = entry::Query(symbols.back());
					j = provider.GetRequiredCount();


					if (j > elements.size()) {
						ErrorTracking(kCodeIllegalArgs, kStrFatalError, "too few parameters");
						break;
					}
					else {
						while (j != 0) {
							container.push_back(elements.back());
							elements.pop_back();
							--j;
						}
					}

					rv = util.ActivityStart(provider, container, elements, elements.size(), result);
					if (rv == false) break;
					symbols.pop_back();

					j = 0;
				}

				util.CleanUpVector(container);
				while (elements.size() > top + 1) {
					container.push_back(elements.back());
					elements.pop_back();
				}
				if (commaexpression) {
					provider = entry::Query("__COMMAEXP");
				}
				else {
					provider = entry::Query(raw[top]);
				}

				rv = util.ActivityStart(provider, container, elements, top, result);
				symbols.pop_back();
				tracer.pop_back();
	
				if (rv == false) {
					break;
				}
					
				forwardtype = kTypeSymbol;
			}
			else if (raw[i] == ",") {
				if (i == raw.size() - 1) {
					ErrorTracking(kCodeIllegalSymbol, kStrWarning, "illegal comma location");
				}
				forwardtype = kTypeSymbol;
			}
			else if (raw[i] == "\"") {
				if (directappend) {
					elements.back().append(raw[i]);
				}
				else {
					elements.push_back(raw[i]);

				}
				directappend = !directappend;
			}
			else {
				symbols.push_back(raw[i]);
			}

			forwardtype = kTypeSymbol;
		}
		else {
			if (directappend) {
				elements.back().append(raw[i]);
			}
			else {
				elements.push_back(raw[i]);
			}

			forwardtype = kTypePreserved;
		}
	}

	//--------------------------------------------------------

	while (!symbols.empty()) {
		util.CleanUpVector(container);

		if (elements.empty()) {
			ErrorTracking(kCodeIllegalSymbol, kStrFatalError, "entries expected");
			break;
		}
		if (symbols.back() == "(" || symbols.back() == ")") {
			ErrorTracking(kCodeIllegalSymbol, kStrFatalError, "another bracket expected");
			break;
		}

		provider = entry::Query(symbols.back());
		j = provider.GetRequiredCount();

		if (j > elements.size()) {
			ErrorTracking(kCodeIllegalArgs, kStrFatalError, "more entries expected");
			break;
		}

		while (j != 0) {
			container.push_back(elements.back());
			elements.pop_back();
			--j;
		}

		rv = util.ActivityStart(provider, container, elements, elements.size(), result);

		if (rv == false) {
			break;
		}

		symbols.pop_back();

		j = 0;
	}
	//--------------------------------------------------------

	return result;
}

//private
int Chainloader::GetPriority(string target) {
	if (target == "+" || target == "-") return 1;
	if (target == "*" || target == "/" || target == "\\" || target == "mod") return 2;
	return 3;
}

Messege Chainloader::Start() {
	const size_t size = raw.size();

	Util util;
	EntryProvider provider;
	Messege result, tempresult;
	size_t i, j, k;

	int forwardtype = kTypeNull;
	bool entryresult = true;
	bool commaexp = false;
	bool directappend = false;

	vector<size_t> tracer; //seems we can work without this?
	vector<size_t> item;
	stack<string> symbol;
	vector<string> container0;

	i = 0;
	while (i < size) {
		//TODO:fuction is symbol too.
		if (regex_match(raw[i], kPatternSymbol)) {

		}
		else {

		}


		//step in
		++i;
	}

	return result;
}

Messege EntryProvider::StartActivity(vector<string> p) {
	Messege result;
	size_t size = p.size();

	if (size == requiredcount || requiredcount == kFlagAutoSize) {
		result = activity(p);
	}
	else {
		if (requiredcount == kFlagNotDefined) {
			result.SetCode(kCodeBrokenEngine).SetValue(kStrFatalError);
		}
	}

	return result;
}

Messege CommaExpression(vector<string> &res) {
	Messege result;
	if (res.empty()) {
		result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
	}
	else {
		result.SetCode(kCodeRedirect).SetValue(res.back());
	}

	return result;
}

#ifdef _DEV_
Messege EmptyCall(vector<string> &res) {
	Messege result;
	std::cout << "You just call me right?" << std::endl;
	return result;
}
#endif

void TotalInjection() {
	using namespace entry;

	Inject(EntryProvider("__COMMAEXP", CommaExpression, kFlagAutoSize));
#ifdef _DEV_
	Inject(EntryProvider("hi_suzu", EmptyCall, 1));
#endif
}

Messege Util::ScriptStart(string target) {
	Messege result;
	Messege temp;
	size_t i;
	size_t size;
	vector<Chainloader> loaders;
	Chainloader cache;

	if (target == kStrEmpty) {
		result.SetCode(kCodeIllegalArgs).SetValue(kStrFatalError);
		tracking::log(result, "Util::ScriptStart() 1");
	}
	else {
		TotalInjection();

		ScriptProvider sp(target);

		while (!sp.eof()) {
			temp = sp.Get();
			if (temp.GetCode() == kCodeSuccess) {
				cache.Reset().Build(temp.GetValue());
				loaders.push_back(cache);
			}
			else {
				result.SetCode(kCodeIllegalArgs).SetValue(kStrFatalError);
				tracking::log(result, "Util::ScriptStart() 2");
				break;
			}
		}
		
		if (!loaders.empty()) {
			size = loaders.size();

			for (i = 0; i < size; i++) {
				temp = loaders[i].Execute();

				if (temp.GetCode() != kCodeSuccess) {
					if (temp.GetValue() == kStrFatalError) break;
				}
			}
		}
	}

	return result;
}

