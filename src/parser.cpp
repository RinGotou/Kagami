#include "parser.h"

using namespace Suzu;

namespace Tracking {
	vector<Message> base;

	void log(Message msg) {
		base.push_back(msg);
	}

	bool IsEmpty() {
		return base.empty();
	}
}

namespace Entry {
	deque<EntryProvider> base;
	deque<MemoryProvider> childbase;

	void Inject(EntryProvider provider) {
		base.push_back(provider);
	}

	EntryProvider Query(string target) {
		EntryProvider result;
		for (auto &unit : base) {
			if (unit.GetName() == target && unit.GetPriority() == 1) {
				result = unit;
			}
		}

		return result;
	}

	Message Order(string name, vector<string> &res) {
		Message result(kStrFatalError, kCodeIllegalCall, "Entry Not Found");
		for (auto &unit : base) {
			if (unit.GetName() == name) {
				result = unit.StartActivity(res);
			}
		}

		return result;
	}
}

namespace Suzu {
	Message Util::GetDataType(string target) {
		using std::regex_match;
		//default error Message
		Message result(kStrRedirect, kCodeIllegalArgs);

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
		size_t top, Message &msg) {
		bool rv = true;
		int code = kCodeSuccess;
		Message temp;

		if (provider.Good()) {
			temp = provider.StartActivity(container);
			code = temp.GetCode();

			if (code < kCodeSuccess) {
				Tracking::log(temp.SetDetail("ActivityStart 1"));
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
			Tracking::log(msg.SetCode(kCodeIllegalCall)
				.SetValue(kStrFatalError).
				SetDetail("ActivityStart 2"));
			rv = false;
		}

		return rv;
	}

	void Util::PrintEvents() {
		using namespace Tracking;

		ofstream ofs("event.log", std::ios::trunc);
		size_t i = 0;
		if (ofs.good()) {

			if (base.empty()) {
				ofs << "No Events\n";
			}
			else {
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
		//nothing to dispose here
	}

	Message ScriptProvider::Get() {
		using Tracking::log;
		string currentstr;
		Message result(kStrEmpty, kCodeSuccess);

		auto IsBlankStr = [](string target) -> bool {
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
			log(result.SetValue(kStrFatalError)
				.SetCode(kCodeBadStream)
				.SetDetail("ScriptProvder::Get() 1"));

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
			log(result.SetValue(kStrFatalError)
				.SetCode(kCodeOverflow)
				.SetDetail("ScriptProvder::Get() 2"));
		}

		return result;
	}

	Chainloader &Chainloader::Build(string target) {
		using Tracking::log;
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
			log(Message(kStrWarning, kCodeIllegalArgs).SetDetail("Chainloader::Build() 1"));
			return *this;
		}

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
			case '{':
			case '}':
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

	//private
	int Chainloader::GetPriority(string target) const {
		if (target == "+" || target == "-") return 1;
		if (target == "*" || target == "/" || target == "\\" || target == "mod") return 2;
		return 3;
	}

	Message Chainloader::Start() {
		const size_t size = raw.size();

		Util util;
		EntryProvider provider;
		Message result, tempresult;
		size_t i, j, k;
		bool entryresult = true;
		bool commaexp = false;
		bool directappend = false;
		size_t forwardtype = kTypeNull;
		deque<string> item;
		deque<string> symbol;
		vector<string> container0;

		for (i = 0; i < size; ++i) {
			if (regex_match(raw[i], kPatternSymbol)) {
				if (raw[i] == ",") {

				}
				if (raw[i] == "(") {

				}
				if (raw[i] == ")") {

				}
			}
			else if (regex_match(raw[i], kPatternFunction)) {

			}
			else {

			}
		}

		util.CleanUpVector(container0);
		util.CleanUpDeque(item);
		util.CleanUpDeque(symbol);

		return result;
	}

	Message EntryProvider::StartActivity(vector<string> p) {
		Message result;
		size_t size = p.size();

		if (size == requiredcount || requiredcount == kFlagAutoSize) {
			result = activity(p);
		}
		else {
			if (requiredcount == kFlagNotDefined) {
				Tracking::log(result.SetCode(kCodeBrokenEntry)
					.SetValue(kStrFatalError)
					.SetDetail(string("Illegal Entry - ")
						.append(this->name)));
			}
			else {
				Tracking::log(result.SetCode(kCodeIllegalArgs)
					.SetValue(kStrFatalError)
					.SetDetail(string("Parameter count doesn't match - ")
						.append(this->name)));
			}
		}

		return result;
	}

	Message CommaExpression(vector<string> &res) {
		Message result;
		if (res.empty()) {
			result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
		}
		else {
			result.SetCode(kCodeRedirect).SetValue(res.back());
		}

		return result;
	}

	Message MemoryQuery(vector<string> &res) {
		using namespace Entry;
		Message result;
		string temp;
		size_t begin = childbase.size() - 1;
		size_t i = 0;

		if (childbase.empty()) {
			Tracking::log(result.SetValue(kStrWarning)
				.SetCode(kCodeIllegalArgs)
				.SetDetail("MemoryQuery() 1"));
		}
		else {
			if (childbase.size() == 1 || res[1] == kArgOnce) {
				temp = childbase.back().query(res[0]);
			}
			else if (childbase.size() > 1 && res[1] != kArgOnce) {
				for (i = begin; i >= 0; --i) {
					temp = childbase[i].query(res[0]);
					if (temp != kStrNull) break;
				}
			}

			if (temp != kStrNull) {
				result.SetCode(kCodeSuccess).SetValue(kStrSuccess).SetDetail(temp);
			}
			else {
				Tracking::log(result.SetCode(kCodeIllegalCall)
					.SetValue(kStrFatalError)
					.SetDetail("MemoryQuery() 2"));
			}
		}

		return result;
	}

	Message Calculate(vector<string> &res) {
		Message result;

		//pending

		return result;
	}


	void TotalInjection() {
		using namespace Entry;

		//set root memory provider
		childbase.push_back(MemoryProvider());
		childbase.back().SetParent(&(childbase.back()));

		//inject basic Entry provider
		Inject(EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
		Inject(EntryProvider("memquery", MemoryQuery, 2, kFlagCoreEntry));
		Inject(EntryProvider("claculat", Calculate, kFlagAutoSize, kFlagCoreEntry));

	}

	Message Util::ScriptStart(string target) {
		Message result;
		Message temp;
		size_t i;
		size_t size;
		vector<Chainloader> loaders;
		Chainloader cache;

		if (target == kStrEmpty) {
			Tracking::log(result.SetCode(kCodeIllegalArgs)
				.SetValue(kStrFatalError)
				.SetDetail("Util::ScriptStart() 1"));
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
					Tracking::log(result.SetCode(kCodeIllegalArgs)
						.SetValue(kStrFatalError)
						.SetDetail("Util::ScriptStart() 2"));
					break;
				}
			}

			if (!loaders.empty()) {
				size = loaders.size();

				for (i = 0; i < size; i++) {
					temp = loaders[i].Start();

					if (temp.GetCode() != kCodeSuccess) {
						if (temp.GetValue() == kStrFatalError) break;
					}
				}
			}
		}

		return result;
	}
}


