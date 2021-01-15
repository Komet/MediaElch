#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

tx status
tx pull -a -f

# Run it twice. Sometimes strings are updated on the
# second run.
lupdate -verbose -no-obsolete MediaElch.pro
lupdate -verbose -no-obsolete MediaElch.pro
lrelease data/i18n/*.ts

# Update source
tx push -s

git add data/i18n
git commit -m "[i18n] Update translations"
