# Development Setup

__State__: Work-in-progress, last updated 2022-12-03


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
