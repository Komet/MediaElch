# Introduction to MediaElch Development

You want to develop for MediaElch? Make it better? That's great! 

This document will introduce you to MediaElch's development processes. We will
start on a very basic level that may seem too amateur-ish to you.

*So, is this document even necessary or useful? Shouldn't developers know this stuff
already?*  
Well, when I first started, I missed introductory documents on most open source projects.
I googled for hours and hours how to do certain things, what best practices there are,
how the open-source community works and still didn't find what I was looking for.
That's why I wrote this document: to help newcomers and those who want to become developers.

If you're familiar with the open-source world, C++/Qt projects and maintaining
GUI applications, then you may get bored soon. I hope following steps will help
some of you. Any feedback is welcome. :-)


## What programming language does MediaElch use? Why Qt/C++?
MediaElch is written in C++ and uses the [Qt][qt] framework.
Qt is one of the most popular GUI frameworks and works on Windows, macOS and Linux.
This makes it the best choice for MediaElch as well.

Today, official Python bindings are available so that may be a choice for new projects but
when MediaElch was first published only un-official bindings existed.

Furthermore C++ is an awesome language, though you may think otherwise :-)


## How is this project structured? Why are there so many directories?
Please refer to [`packaging.md`](packaging.md).


## How do I know which dependencies are required?
TODO


## How do I build MediaElch on X?
Please refer to https://mediaelch.github.io/mediaelch-doc/contributing/build/index.html

The build instructions in the user documentation are first steps
you should take when developing for MediaElch. They use the QMake build
system and are short and easy to understand. We will talk about more advanced
build options later.


## How do I know that my changes don't break anything?
MediaElch continues to improve its codebase but from time to time features break or
bugs are introduced. We try to avoid that by testing all of our codebase.

Every pull request is build on all major platforms and is tested.
That includes unit and integration tests (see [`test/README.md`](../../test/README.md)).


## What's the CI setup?
TODO

...

[qt]: https://www.qt.io/
