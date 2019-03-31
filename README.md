# Kagami Project

Just another wild flower.

## What's this?
It's a project of experimental script language. The name of this project is from Japanese kanji "鏡"(かがみ).

## Design
I want to create a simple language for easier simple visual novel developing. Kagami is design for logic 
implementation, not CPU intensive job.   

This project contains a language interpreter, "如月"(きさらぎ, kisaragi). It's not a pure stack-based IR virtual 
machine.

## About repo branch
"master" is complete version of code in "testing" branch.IT'S NOT REAL STABLE BRANCH!

"x.x-stable" is the stable version of interpreter.

"testing" is current developing branch.

## How to use these codes?
Strongly recommend to compile them in Visual Studio 2017 or later version.

If you're using CMake,please turn off all compiling options in common.h to make it happy.

SDL2 stuff is under consideration now,so I don't want you to turn off DISABLE_SDL macro until I think it's good enough to use.

You can find other notes in source code. Sorry for lacking of dev log.

## Help me?
You can post issues or create pull request.

## License
BSD-2-Clause
