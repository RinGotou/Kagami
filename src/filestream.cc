#include "filestream.h"

namespace kagami {
  FILE *GetVMStdout(FILE *dest) {
    static FILE *vm_stdout = stdout;
    if (dest != nullptr) {
      vm_stdout = dest;
    }

    return vm_stdout;
  }

  FILE *GetVMStdin(FILE *dest) {
    static FILE *vm_stdin = stdin;
    if (dest != nullptr) {
      vm_stdin = dest;
    }

    return vm_stdin;
  }

  void CloseStream() {
    if (GetVMStdin() != stdin) fclose(GetVMStdin());
    if (GetVMStdout() != stdout) fclose(GetVMStdout());
  }

  string InStream::GetLine() {
    if (fp_ == nullptr || eof_) return string();

    int buf = 0;
    string result;

    while (buf != EOF) {
      buf = fgetc(fp_);

      if (buf == EOF) {
        eof_ = true;
        continue;
      }

      if (buf == '\n') {
        break;
      }
      else {
        result.append(1, static_cast<char>(buf));
      }
    }

    return result;
  }

  char InStream::Get() {
    auto buf = fgetc(fp_);
    if (buf == EOF) {
      eof_ = true; return '\0';
    }
    return buf;
  }

  wchar_t InStreamW::Get() {
    auto buf = fgetwc(fp_);
    if (buf == WEOF) {
      eof_ = true; return L'\0';
    }

    return buf;
  }

  wstring InStreamW::GetLine() {
    if (fp_ == nullptr || eof_) return wstring();

    wint_t buf = 0;
    wstring result;

    while (buf != WEOF) {
      buf = fgetwc(fp_);

      if (buf == WEOF) {
        eof_ = true;
        continue;
      }

      if (buf == L'\n') {
        break;
      }
      else {
        result.append(1, static_cast<wchar_t>(buf));
      }
    }

    return result;
  }

  string _ProcessingOutStreamArgument(bool append, bool binary) {
    string result;
    result.append(append ? "a" : "w");
    if (binary)result.append("b");
    return result;
  }

  bool OutStream::Write(string str) {
    if (fp_ == nullptr) return false;
    auto it = str.begin();
    auto end = str.end();
    int flag = 0;
    for (; it != end; ++it) {
      flag = fputc(*it, fp_);
      if (flag == EOF) break;
    }

    return flag != EOF;
  }

  bool OutStream::Write(char chr) {
    if (fp_ == nullptr) return false;
    return fputc(chr, fp_) != EOF;
  }

  bool OutStreamW::Write(wstring str) {
    if (fp_ == nullptr) return false;
    auto it = str.begin();
    auto end = str.end();
    wint_t flag = 0;
    for (; it != end; ++it) {
      flag = fputwc(*it, fp_);
      if (flag == WEOF) break;
    }

    return flag != WEOF;
  }

  bool OutStreamW::Write(wchar_t wchr) {
    if (fp_ == nullptr) return false;
    return fputc(wchr, fp_) != WEOF;
  }

  string GetLine() {
    string result;
    int buf = 0;
    while (buf != EOF) {
      buf = fgetc(GetVMStdin());
      if (buf == '\n' || buf == EOF) break;
      result.append(1, buf);
    }

    return result;
  }

  wstring GetLineW() {
    wstring result;
    wint_t buf = 0;
    while (buf != WEOF) {
      buf = fgetwc(GetVMStdin());
      if (buf == L'\n' || buf == WEOF) break;
      result.append(1, buf);
    }

    return result;
  }
}