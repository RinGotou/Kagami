# Kagami

## What's this?
It's a simple and tiny experimental script language. The name of this project is from Japanese kanji "鏡".

## About repo branch
"master" is complete version of code in "testing" branch.IT'S NOT REAL STABLE BRANCH!

"x.x-stable" is the stable version of interpreter.

"testing" is current developing branch.

## Feature
[√] Dynamic and weak typing language with reflection and closure support

[√] Stack-based Backend Machine

[√] Based on C++14

## What does it looks like?
```
fn ReadFile()
    stream = instream('SomeWords.txt')
    while(stream.eof() != true)
        println(stream.get())
    end
    stream.close()
end

ReadFile()
```

## How to use these codes?
Strongly recommend to compile them in Visual Studio 2017 or later version.

If you're using CMake,please turn off all compiling options in common.h to make it happy.

SDL2 stuff is under consideration now,so I don't want you to turn off DISABLE_SDL macro until I think it's good enough to use.

You can find other notes in source code. Sorry for lacking of dev log.

## Help me?
You can post issues or create pull request.

## License
BSD-2-Clause
