#include "filestream.h"

namespace kagami {
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

  bool OutStream::WriteLine(string str) {
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

  bool OutStreamW::WriteLine(wstring str) {
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

  string GetLine() {
    string result;
    int buf = 0;
    while (buf != EOF) {
      buf = getchar();
      if (buf == '\n') break;
      result.append(1, buf);
    }

    return result;
  }

  wstring GetLineW() {
    wstring result;
    wint_t buf = 0;
    while (buf != WEOF) {
      buf = getwchar();
      if (buf == L'\n') break;
      result.append(1, buf);
    }

    return result;
  }
}