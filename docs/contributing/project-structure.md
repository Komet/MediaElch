# MediaElch Design Document

**Work In Progress**

## Table of Contents

 - What is this?


## What is this?

This document describes important design decisions that
were set in the development process of MediaElch.

## Separation of UI and logic

We aim to separate MediaElch's user interface from the internal logic.
The latter includes the scraping logic, file searcher and more.

By separating both, we can easier test MediaElch without instantiating a GUI.

_Note:_ At the moment, UI and Core are tightly coupled.
