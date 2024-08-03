# Module System

__Status__: Concept, work in progress  
__Last Updated on__: 2024-07-30

Provide a module system for MediaElch where components are more decoupled from
another than it currently is.

This document uses "Settings" as an example, where decoupling is needed.

## Current State

I want to have a better module system, or just any at all.

This starts with settings.  Currently, settings are centralized in one big
class "Settings".  That file stores all keys and provides getters and setters.
That makes it impossible to mock and test any widget at all, because everything
relies on this central settings instance (which is a singleton of course).
This central class is a bottleneck.  It knows everything about everything in
MediaElch and needs to be linked against all modules, including QtWidgets.

The only way to test it at all is to provide a way to replace the singleton,
which is doable, but annoying.

## Proposal

Inspiration by https://github.com/musescore/MuseScore/wiki/Settings-and-Configuration
But we _do_ rely on Qt, still!

Each module/component has a central class `<Name>Module`, for example
`CsvExportModule`.  This module is set up in our Manager, which connects all
components and calls each module's initializer.  The module's initializer sets
up itself, including settings and UI.

For settings, we still provide a central `Settings` class, but it doesn't
provide any getter or setters, only generic get-value and set-value functions,
which can be mocked. It acts as a basic key-value store. 
The module registers all its settings via its configuration class, which is
a typed interface and should be used throughout the module, i.e. there must
not be any raw access to settings.
By having the modules register their settings, we get a list of all settings
at runtime.

The gist is: the module registers everything that is needed for it to work.
