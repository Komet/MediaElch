# Development Setup

__State__: Work-in-progress, last updated 2023-05-07

## Table of Contents

1. [Git](#git)
2. [CLion](#clion)

## Git

We have a list of commits that can safely be ignored when doing a `git blame`.
`.git-blame-ignore-revs` in MediaElch's project root contains a list of
commits that we ignore.  You can tell your local Git project to do the same with:

```sh
# See .git-blame-ignore-revs in project root
git config blame.ignoreRevsFile .git-blame-ignore-revs
```

Furthermore, since 2022-10-03, we try to use [Conventional Commits].
Because we sometimes forget what "commit types" there are, we have a
default commit message template, that has some tips.

```sh
git config commit.template scripts/etc/git-default-commit-message.txt
```

_History_: We did not have any convention, but I mostly used `[Type] Description`,
where `Type` was whatever I liked.

[Conventional Commits]: https://www.conventionalcommits.org/en/v1.0.0/


## CLion

If you use CLion (by Jetbrains), you may want to have [debugger renderers for Qt][clion-qt-debug].
Read the documentation for detailed setup steps.  This worked for me ob Ubuntu 22.04:

```sh
gdb --version
# Should be >= 12

cd "$HOME/Projects" # Use your projects paths 
git clone --depth=1 --single-branch --branch=master https://github.com/KDE/kdevelop.git
```

In `MediaElch/.gdbinit`, I added:

```gdb
python

import sys, os

p = '/path/to/kdevelop/plugins/gdb/printers/'

print(f".gdbinit Python: current working directory is {os.getcwd()}")
print(f".gdbinit Python: adding custom pretty-printers directory to the GDB path: {p}")

sys.path.insert(0, p)

end

source /path/to/kdevelop/plugins/gdb/printers/gdbinit
```

If the project specific `.gdbinit` doesn't work, see CLion's "[configuring debugger options][clion-debugger-options]".
I have adapted `~/.config/gdb/gdbinit` with:

```
add-auto-load-safe-path /path/to/MediaElch/.gdbinit
```

[clion-qt-debug]: https://www.jetbrains.com/help/clion/qt-tutorial.html#debug-renderers
[clion-debugger-options]: https://www.jetbrains.com/help/clion/configuring-debugger-options.html#gdb-startup