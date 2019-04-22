#include "common.h"

namespace kagami {
  class BasicStream {
  protected:
    bool eof_;
    FILE *fp_;

  public:
    virtual ~BasicStream() { fclose(fp_); }
    BasicStream(const char *path, const char *mode) :
      eof_(false), fp_(fopen(path, mode)) {}
    BasicStream(string path, string mode) :
      eof_(false), fp_(fopen(path.c_str(), mode.c_str())) {}
    BasicStream() : eof_(true), fp_(nullptr) {}
    void operator=(BasicStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(BasicStream &&rhs) {
      operator=(rhs);
    }

    bool Good() const { return fp_ != nullptr; }
    bool eof() const { return eof_; }
  };

  class InStream : public BasicStream {
  public:
    InStream(const char *path) : BasicStream(path, "r") {}
    InStream(string path) : BasicStream(path.c_str(), "r") {}
    InStream(const InStream &) = delete;
    InStream(const InStream &&) = delete;
    InStream() : BasicStream() {}
    void operator=(InStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(InStream &&rhs) {
      operator=(rhs);
    }

    string GetLine();
  };

  class OutStream : public BasicStream {
  public:
    OutStream(const char *path, const char *mode) : BasicStream(path, mode) {}
    OutStream(string path, string mode) : BasicStream(path.c_str(), mode.c_str()) {}
    OutStream(const OutStream &) = delete;
    OutStream(const OutStream &&) = delete;
    OutStream() : BasicStream() {}
    void operator=(OutStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(OutStream &&rhs) {
      operator=(rhs);
    }

    bool WriteLine(string str);
  };

  //class FileStreamEx : public BasicStream {

  //};
}