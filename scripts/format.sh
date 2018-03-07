#!/bin/bash

if [ ! -f MediaElch.pro ]; then
	echo ""
	echo "    Call \"./scripts/format.sh\" from MediaElch root directory!"
	echo ""
	exit 1
fi

# Format all files using clang-format
find . ! -path "*/quazip/*" -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i -style=file {} \;
