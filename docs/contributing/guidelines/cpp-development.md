# C++ Development Guidelines

High level list of development guidelines for MediaElch C++ development.

- C++14 must be supported.
  - Don't use ` [[nodiscard]]`, but the `ELCH_NODISCARD` marco from `Meta.h`.
  - Don't concatenate namespaces, but nest them.
- Non-UI classes should be in a `mediaelch::` namespace.
- `const` all the things!
- Avoid `auto` except where the type is repeated on the same line, e.g. in `*_cast<Type>(…)`.
