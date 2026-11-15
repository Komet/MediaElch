# C++ Development Guidelines

High level list of architecture guidelines for MediaElch development.

- No UI dependencies in non-UI code
  - No `QWidget`, input fields, etc. in domain logic
  - Background: Older versions had non-UI code depend on values in certain form fields.
