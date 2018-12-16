#!/usr/bin/env bash

set -e          # Exit on errors
set -o pipefail # Unveils hidden failures

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
PROJECT_DIR="$( cd "${SCRIPT_DIR}/.." ; pwd -P )"

cd $PROJECT_DIR

tx status
tx pull -a -f

# Run it twice. Sometimes strings are updated on the
# second run.
lupdate -verbose -no-obsolete MediaElch.pro
lupdate -verbose -no-obsolete MediaElch.pro
lrelease MediaElch.pro

# Update source
tx push -s

git add data/i18n
git commit -m "[i18n] Update translations"
