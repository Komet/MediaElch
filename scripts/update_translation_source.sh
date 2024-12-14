#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

tx status
tx pull -a -f

# This is specific to the current maintainer's system
export PATH="$HOME/Qt/6.8.1/gcc_64/bin/:$PATH"

# Run it twice. Sometimes strings are updated on the
# second run.
lupdate -verbose -no-obsolete -locations relative MediaElch.pro
lupdate -verbose -no-obsolete -locations relative MediaElch.pro

# Update source
tx push -s

lupdate -verbose -no-obsolete -locations relative MediaElch.pro

git add data/i18n
git commit -m "chore(i18n): Update translations"
