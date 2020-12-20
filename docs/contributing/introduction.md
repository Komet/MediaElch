# Introduction to MediaElch Development

You want to develop for MediaElch? Make it better? That's great! 

This document will introduce you to MediaElch's development processes. We will
start on a very basic level that may seem too amateur-ish to you.

*So, is this document even necessary or useful? Shouldn't developers know this stuff
already?*  
Well, when I first started, most open source projects that I was interested in had no
introductory documents.
I googled for hours and hours how to do certain tasks, what best practices there are,
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
Please refer to [`project-structure.md`](project-structure.md).


## How do you deploy MediaElch for all major operating systems?
Please refer to [`packaging.md`](packaging.md).


## How do I know which dependencies are required?
That is indeed an issue. The way I do this may not be the best way but is certainly easy:
By trial and error.

Seriously. I set up a new virtual machine (e.g. Ubuntu 18.04) and tried to build MediaElch.
Everytime I got a build error, I installed a new dependency and wrote it down.
And while I was at it, I created build instructions for different platforms. You may think that
you know all of your dependencies and don't have to do this but is really helpful for new users.

If you want to ensure that even with newer versions of your software all dependencies are written
down somewhere, you can create docker images. I've done that for MediaElch as well, see
[our Dockerfiles](../../.ci/docker/README.md)


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
Please refer to [`continous-integration.md`](continous-integration.md).


## When do you require a more recent Qt release?
From time to time we require a more recent Qt release.  For example
MediaElch v2.8 requires 5.6 and no longer supports Qt 5.5.  There may be new
features that make the development a lot easier. In the case of Qt 5.6 this
was related to network requests.  Previously we had to handle redirections
(`3xx` responses) manually.  Qt 5.6 does this automatically which made my
life a lot easier.

On the downside we want to support Ubuntu LTS versions. As of 2020-12-01,
Ubuntu 16.04 is still supported by Ubuntu but no longer by MediaElch's PPA.
But please try to support all LTS versions of Ubuntu if possible.

[qt]: https://www.qt.io/
