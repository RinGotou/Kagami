//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#pragma once
#include "parser.h"

namespace kagami {
  namespace entry {
    using std::shared_ptr;
    typedef StrMap *(*Attachment)(void);
    std::wstring s2ws(const std::string& s);

    class Methods {
    private:
      vector<string> vec;
    public:
      Methods(string target) { vec = Kit().BuildStringVector(target); }
      Methods &Set(string target) { vec = Kit().BuildStringVector(target); return *this; }
      vector<string> Get() const { return vec; }
    };

#if defined(_WIN32)
    class Instance : public pair<string, HINSTANCE> {
    private:
      bool health;
      StrMap link_map;
    public:
      Instance() { health = false; }
      bool Load(string name, HINSTANCE h);
      bool GetHealth() const { return health; }
      StrMap GetMap() const { return link_map; }
      MemoryDeleter getDeleter() { return (MemoryDeleter)GetProcAddress(this->second, "FreeMemory"); }
    };
#else

#endif
  }
}



