#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'


cd "$(dirname "$0")/.."

echo ""
echo "SOURCES += \\"
for file in $(find src -type f -iname '*.cpp' | sort | grep -v 'cli/'); do
	echo "    ${file} \\"
done

echo ""
echo "HEADERS += Version.h \\"
for file in $(find src -type f -iname '*.h' | sort | grep -v 'cli/'); do
	echo "    ${file} \\"
done

echo ""
echo "FORMS += \\"
for file in $(find src -type f -iname '*.ui' | sort); do
	echo "    ${file} \\"
done
