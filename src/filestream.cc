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
}