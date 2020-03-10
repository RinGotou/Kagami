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

  enum class SeekingMode {
    kSeekingBegin = SEEK_SET, 
    kSeekingCurrent = SEEK_CUR, 
    kSeekingEnd = SEEK_END
  };

  class BasicStream {
  protected:
    bool eof_;
    FILE *fp_;

  public:
    virtual ~BasicStream() { if (fp_ != nullptr && fp_ != stdout) fclose(fp_); }
    BasicStream(FILE *fp) : eof_(false), fp_(fp) {}
    BasicStream(string path, string mode) :
      eof_(false), fp_(fopen(path.data(), mode.data())) {}
    BasicStream() : eof_(true), fp_(nullptr) {}

    void operator=(BasicStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(BasicStream &&rhs) noexcept {
      operator=(rhs);
    }

    bool Good() const { return fp_ != nullptr; }
    bool eof() const { return eof_; }
    FILE *_GetPtr() { return fp_; }
    auto GetPosition() { return std::ftell(fp_); }
    bool SetPosition(long offset, SeekingMode mode) {
      return (std::fseek(fp_, offset, static_cast<int>(mode)));
    }
  };

  class InStream : public BasicStream {
  public:
    InStream(FILE *fp) : BasicStream(fp) {}
    InStream(string path, bool binary = false) :
      BasicStream(path.data(), binary ? "rb" : "r") {}
    InStream(const InStream &) = delete;
    InStream(const InStream &&) = delete;
    InStream() : BasicStream() {}

    void operator=(InStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(InStream &&rhs) noexcept {
      operator=(rhs);
    }

    string GetLine();
    char Get();
  };

  class InStreamW : public BasicStream {
  public:
    InStreamW(FILE *fp) : BasicStream(fp) {}
    InStreamW(string path, bool binary = false) :
      BasicStream(path.data(), binary ? "rb" : "r") {}
    InStreamW(const InStreamW &) = delete;
    InStreamW(const InStreamW &&) = delete;
    InStreamW() : BasicStream() {}
    void operator=(InStreamW &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(InStreamW &&rhs) noexcept {
      operator=(rhs);
    }

    wstring GetLine();
    wchar_t Get();
  };

  string _ProcessingOutStreamArgument(bool append, bool binary);

  class OutStream : public BasicStream {
  public:
    OutStream(FILE *fp) : BasicStream(fp) {}
    OutStream(const OutStream &) = delete;
    OutStream(const OutStream &&) = delete;
    OutStream() : BasicStream() {}
    OutStream(string path, bool append, bool binary) :
      BasicStream(path, _ProcessingOutStreamArgument(append, binary)) {}

    void operator=(OutStream &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(OutStream &&rhs) noexcept {
      operator=(rhs);
    }

    bool Write(string str);
    bool Write(char chr);
  };

  class OutStreamW : public BasicStream {
  public:
    OutStreamW(FILE *fp) : BasicStream(fp) {}
    OutStreamW(const OutStreamW &) = delete;
    OutStreamW(const OutStreamW &&) = delete;
    OutStreamW() : BasicStream() {}
    OutStreamW(string path, bool append, bool binary) :
      BasicStream(path, _ProcessingOutStreamArgument(append, binary)) {}

    void operator=(OutStreamW &rhs) {
      std::swap(eof_, rhs.eof_); std::swap(fp_, rhs.fp_);
    }
    void operator=(OutStreamW &&rhs) noexcept {
      operator=(rhs);
    }

    bool Write(wstring str);
    bool Write(wchar_t wchr);
  };
}

#define VM_STDOUT GetVMStdout()
#define VM_STDIN GetVMStdin()