#pragma once
#include "analyzer.h"
 
namespace kagami {
  namespace trace {
    using Event = pair<string, Message>;

    template <class Tx>
    class OutStreamPrintPolicy {
    public:
      void Do(Tx *stream, string content) {
        *stream << content << '\n';
      }
    };

    template <>
    class OutStreamPrintPolicy<ostream> {
    public:
      void Do(ostream *stream, string content) {
        *stream << content << '\n';
        stream->flush();
      }
    };

    template <class Tx>
    class OutStreamInitPolicy {
    public:
      Tx *Get(string) {}
    };

    template <>
    class OutStreamInitPolicy<ostream> {
    public:
      ostream *Get(string) { return &std::cout; }
    };

    template <>
    class OutStreamInitPolicy<ofstream> {
    public:
      ofstream *Get(string path) { return new ofstream(path); }
    };

    template <class Tx>
    class OutStreamDestroyPolicy {
    public:
      void Do(Tx *tx) {}
    };

    template <>
    class OutStreamDestroyPolicy<ofstream> {
    public:
      void Do(ofstream *ofs) {
        ofs->close();
        delete ofs;
      }
    };

    class StreamPackage {
    public:
      virtual void Init(string) = 0;
      virtual void Print(string) = 0;
      virtual void Destroy() = 0;
      virtual bool Good() = 0;
      virtual ~StreamPackage() {}
    };

    template <class Tx>
    class OutStreamPackage :public StreamPackage {
    private:
      Tx *stream_;

    public:
      OutStreamPackage() : stream_(nullptr) {}

      void Init(string path) { 
        stream_ = OutStreamInitPolicy<Tx>().Get(path); 
      }

      void Print(string content) { 
        OutStreamPrintPolicy<Tx>().Do(stream_, content); 
      }

      void Destroy() {
        OutStreamDestroyPolicy<Tx>().Do(stream_);
      }

      bool Good() { 
        return stream_->good(); 
      }
    };

    using FileStreamPackage = OutStreamPackage<ofstream>;
    using StdoutPackage = OutStreamPackage<ostream>;

    class LoggerPolicy {
    protected:
      string GetTimeString() {
        auto now = time(nullptr);
        string nowtime(ctime(&now));
        nowtime.pop_back();
        return nowtime;
      }

      string MessageToString(Message msg, string time) {
        string str = "[" + time + "]";

        if (msg.GetLevel() != kStateNormal) {
          str.append("(Line:" + to_string(msg.GetIndex() + 1) + ")");
        }
        
        switch (msg.GetLevel()) {
        case kStateError:str.append("Error:"); break;
        case kStateWarning:str.append("Warning:"); break;
        case kStateNormal:str.append("Info:"); break;
        default:break;
        }

        str.append(msg.GetDetail());

        return str;
      }

      void PrintFileStreamError() {
        std::cout << "Logger Error:Can't create log in specific path, "
          << "Logger will redirect to stdout.\n";
      }

      void PrintLogHeader(StreamPackage &pkg, string path) {
        pkg.Print("[Script:" + path + "]");
      }

      void InitPackage(unique_ptr<StreamPackage> &pkg, string path) {
        if (path == "stdout") {
          pkg = make_unique<StdoutPackage>();
        }
        else {
          pkg = make_unique<FileStreamPackage>();
        }

        pkg->Init(path);

        if (!pkg->Good()) {
          PrintFileStreamError();
          pkg->Destroy();
          pkg.reset();
          pkg = make_unique<StdoutPackage>();
        }
      }

    public:
      virtual void Inject(string, string) = 0;
      virtual void Init() = 0;
      virtual void Add(Message) = 0;
      virtual void Final() = 0;
      virtual ~LoggerPolicy() {}
    };

    class RealTimePolicy :public LoggerPolicy {
    private:
      unique_ptr<StreamPackage> stream_package_;
      string path_;
      string script_path_;
    public:
      RealTimePolicy() {}

      void Inject(string path, string script_path) {
        path_ = path;
        script_path_ = script_path;
      }

      void Init() {
        InitPackage(stream_package_, path_);
        PrintLogHeader(*stream_package_, script_path_);
      }

      void Add(Message msg) {
        stream_package_->Print(MessageToString(msg, GetTimeString()));
      }

      void Final() {
        stream_package_->Destroy();
        stream_package_.release();
      }
    };

    class CachePolicy :public LoggerPolicy {
    private:
      vector<Event> cache_;
      string path_;
      string script_path_;

    public:
      CachePolicy() {}

      void Inject(string path, string script_path) {
        path_ = path;
        script_path_ = script_path;
      }

      void Init() {}

      void Add(Message msg) {
        cache_.emplace_back(Event(GetTimeString(), msg));
      }

      void Final() {
        unique_ptr<StreamPackage> stream_package;

        InitPackage(stream_package, path_);
        PrintLogHeader(*stream_package, script_path_);

        for (auto it = cache_.begin(); it != cache_.end(); ++it) {
          stream_package->Print(MessageToString(it->second, it->first));
        }

        stream_package->Destroy();
        stream_package.reset();
      }
    };

    void InitLogger(LoggerPolicy *policy);
    void AddEvent(Message msg);
    void AddEvent(string info);
  }
}

#if defined(_DEBUG_)
#define DEBUG_EVENT(MSG) trace::AddEvent(MSG)
#define ASSERT_MSG(COND, MSG)               \
  if (COND) { DEBUG_EVENT(MSG); }
#else
#define DEBUG_EVENT(MSG)
#define ASSERT_MSG(COND, MSG)
#endif

