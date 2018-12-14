# Data directory

The `data/` directory is contains project files that are not explicitly code.
For example, graphics and localization files are not code in the same sense
as the rest of the project, but are good candidates for inclusion in the
`data/` directory.

## Structure

```
.
├── desktop               # Files for linux desktops
├── fonts                 # TTF fonts
├── i18n                  # Translations for MediaElch pulled from transifex
│   ├── MediaElch_en.qm
│   ├── MediaElch_en.ts
│   ├── ...
├── img                   # Contains all images (e.g. flags and icons)
├── i18n.qrc              # Qt Resource file for translations
├── MediaElch.qrc         # Qt Resource file for images and fonts
└── README.md             # This file

```
