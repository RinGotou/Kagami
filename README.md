# Kagami

## Hint
This project contains many terrible design.

## What's this?
It's a simple and tiny script language.The name of this project is from Japanese kanji "鏡".

## What does it looks like?

```
fn ReadFile()
    stream = instream('SomeWords.txt')
    while(stream.eof() != true)
        print(stream.get())
    end
    stream.close()
end

if(__name__ == '__main__')
    ReadFile()
end
```

## Feature
[√] Dynamic and weak typing language with reflection support

[√] Stack-based Backend Machine

[√] GC based on referenced count(base on C++ STL)

[√] Fully based on C++11

## How to use these codes?
Strongly recommend to compile them in Visual Studio 2010 or later version.You can also compile by using g++/clang++ with -std=c++11 option.

If you're using CMake,please turn off all compiling options in common.h to make it happy.

SDL2 stuff is under consideration now,so I don't want you to turn off _DISABLE_SDL_ until I think it's good enough to use.

You can find other notes in source code. Sorry for lacking of dev log.

## Help me?
You can post issues or create pull request.

## License
BSD-2-Clause
