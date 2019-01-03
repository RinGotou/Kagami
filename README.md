# Kagami

## Hint
This is old and crispy implementation. I will not add any new feature and stuff to this verison any more.

[Nagatsuki](https://github.com/suzunakamura/nagatsuki) will be the new main implementation.

## What's this?
It's a simple and tiny script language.The name of this project is from Japanese kanji "鏡（かがみ）".

## What does it looks like?

```
def ReadFile()
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

There's no any documents for this language and interpreter for now(because of development process and etc.).

I pick some of my favorite syntax from Python and Visual Basic .NET, and try to make Kagami easier to read and use. 

## Feature
[√] Dynamic and weak typing language with reflection support

[√] GC based on referenced count

[√] Fully based on C++11 and STL

[√] C Function support(partial and experimental)

[×] Class support/SDL2 support


## How to use these codes?
Strongly recommend to compile them in Visual Studio 2010 or later version.You can also compile by using g++/clang++ with -std=c++11 option.

If you're using CMake,please turn off all compiling options in common.h to make it happy.

SDL2 stuff is under consideration now,so I don't want you to turn off _DISABLE_SDL_ until I think it's good enough to use.

You can find other notes in source code. Sorry for lacking of dev log.

## Help me?
You can post issues or create pull request.

## License
This application is licensed under BSD-2-clause, just do anything you like without worrying.
