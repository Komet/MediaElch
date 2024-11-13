# MediaElch Design Document

__State__: *Work In Progress* as of 2024-11-13

This document describes how MediaElch's source code is structured.


## Table of Contents

 - What is this?
 - Directory Structure
 - Separation of UI and Non-UI
 - Separate by Purpose, not Model


## Directory Structure

MediaElch's C++ source files are located in directory `src/`.
The following listing describes the purpose of each subdirectory.

```
src
  cli           Command line interface. Not used by QMake.
  data          Data Structures such as Movie TvShow, etc.  Only contains
                structures, should not contain SQL, XML or UI logic.
  database      Everything database related, i.e. MediaElch's internal
                cache database, settings, etc.
  export        Exporting MediaElch's data to e.g. CSV or HTML.
                Does not include media center exports/imports.
  file_search   MediaElch's core logic for loading files such as movies
                from disk.
  globals       _Old_. Should be removed in the future.
                In the past, contained globals such as IDs or Managers.
                However, that meant that everything depended on every Qt
                module, because the Globals depended on UI, XML, SQL, etc.
  import        Import related stuff.  _Needs cleanup_.
  log           Central log infrastructure.  Log files, log patterns, etc.
  media         Everything related to (media) files, e.g. filesystem types
                or MediaInfo file structures.
  media_center  Kodi related exporter/importer.  Similar to export/import,
                but separated because of its size.
  model         Qt models for all data/ structures.  Used by UI.
  network       Download and network managers, status codes, etc.
  renamer       Sources for renaming files on disk.
  scrapers      Scrapers for meta data, images and more.
  settings      Structures for settings, also includes import/exporter
                for settings.
  ui            MediaElch's UI.  Sub-directories are structured similar
                to src/.
  utils         Small utilities that don't warrant a custom directory.
                For example time/math utilities, Qt adapters, etc.
  workers       _May change_.  Currently contains interfaces for "workers",
                i.e. jobs that can be run in the background asynchronously.
```


## Separation of UI and Non-UI

We aim to separate MediaElch's user interface from the internal logic.
The latter includes the scraping logic, file searcher and more.

By separating both, we can easier test MediaElch without instantiating a GUI.

_Note:_ At the moment, UI and Core are tightly coupled.


## Separate by Purpose, not Model

Don't put anything that contains `Movie` into a folder `movie/`.
Instead, separate by purpose or feature, for example, don't do:

```
movie/
    Movie.cpp
    MovieDatabase.cpp
    MovieExporter.cpp
    MovieWidget.cpp
```

instead do:

```
core/Movie.cpp
database/MovieDatabase.cpp
export/MovieExporter.cpp
ui/MovieWidget.cpp
```

Background being: A folder should represent a module, that can be wrapped
in a library.  If we put everything movie-related into a folder `movie/`,
we would either have a huge library `movie.so` that links against `Qt::Core`,
`Qt::Widgets`, `Qt::Sql`, etc., or multiple libraries inside the same directory.

This is personal taste.  This also makes incorrect dependencies more visible,
e.g. from model to UI.  By having separate libraries (in different directories),
the dependency becomes more obvious due to the added library-link.

There is probably a better term than "purpose" and "model".
A quick Google search tells me it's ["Horizontal" vs. "Vertical"][0].


[0]: https://www.slideshare.net/ChristianHujer/vertical-vs-horizontal-software-architecture
