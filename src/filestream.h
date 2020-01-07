#pragma once
//#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

namespace kagami {
  using std::FILE;
  using std::fopen;
  using std::string;
  using std::wstring;
  using std::fputc;
  using std::fputwc;
  using std::fgetc;
  using std::fgetwc;

  string GetLine();
  wstring GetLineW();
  FILE *GetVMStdout(FILE *dest = nullptr);
  FILE *GetVMStdin(FILE *dest = nullptr);
  void CloseStream();

  class BasicStream {
  protected:
    bool eof_;
    FILE *fp_;

  public:
    virtual ~BasicStream() { if (fp_ != nullptr && fp_ != stdout) fclose(fp_); }
    BasicStream(FILE *fp) : eof_(false), fp_(fp) {}
    BasicStream(const char *path, const char *mode) :
      eof_(false), fp_(fopen(path, mode)) {}
    BasicStream(string path, string mode) :
      eof_(false), fp_(fopen(path.data(), mode.data())) {}
    BasicStream() : eof_(true), fp_(nullptr) {}

    void operator=(BasicStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(BasicStream &&rhs) {
      operator=(rhs);
    }

    bool Good() const { return fp_ != nullptr; }
    bool eof() const { return eof_; }
    FILE *_GetPtr() { return fp_; }
  };

  class InStream : public BasicStream {
  public:
    InStream(FILE *fp) : BasicStream(fp) {}
    InStream(const char *path) : BasicStream(path, "r") {}
    InStream(string path) : BasicStream(path.data(), "r") {}
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

  class InStreamW : public BasicStream {
  public:
    InStreamW(FILE *fp) : BasicStream(fp) {}
    InStreamW(const char *path) : BasicStream(path, "r") {}
    InStreamW(string path) : BasicStream(path.data(), "r") {}
    InStreamW(const InStreamW &) = delete;
    InStreamW(const InStreamW &&) = delete;
    InStreamW() : BasicStream() {}
    void operator=(InStreamW &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(InStreamW &&rhs) {
      operator=(rhs);
    }

    wstring GetLine();
  };

  class OutStream : public BasicStream {
  public:
    OutStream(FILE *fp) : BasicStream(fp) {}
    OutStream(const char *path, const char *mode) : BasicStream(path, mode) {}
    OutStream(string path, string mode) : BasicStream(path.data(), mode.data()) {}
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

  class OutStreamW : public BasicStream {
  public:
    OutStreamW(FILE *fp) : BasicStream(fp) {}
    OutStreamW(const char *path, const char *mode) : BasicStream(path, mode) {}
    OutStreamW(string path, string mode) : BasicStream(path.data(), mode.data()) {}
    OutStreamW(const OutStreamW &) = delete;
    OutStreamW(const OutStreamW &&) = delete;
    OutStreamW() : BasicStream() {}
    void operator=(OutStreamW &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(OutStreamW &&rhs) {
      operator=(rhs);
    }

    bool WriteLine(wstring str);
  };

  //class FileStreamEx : public BasicStream {

  //};

}

#define VM_STDOUT GetVMStdout()
#define VM_STDIN GetVMStdin()