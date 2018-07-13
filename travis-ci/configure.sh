#!/usr/bin/env bash

# Exit on errors
set -e

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

# Load utils (paths, color output, folding, etc.)
. "${SCRIPT_DIR}/utils.sh"
. "${SCRIPT_DIR}/../scripts/utils.sh"
if [ -f "${SCRIPT_DIR}/defs.sh" ]; then
	. "${SCRIPT_DIR}/defs.sh"
fi

pushd "${PROJECT_DIR}" > /dev/null
mkdir build && cd $_

print_important "Configuring for building for ${QT} on ${OS_NAME}"

if [ $(lc "${OS_NAME}") = "linux" ]; then

	if [ "${QT}" = "qtWin" ]; then
		PATH="$PATH:${MXEDIR}/usr/bin"
		$MXEDIR/usr/bin/${MXETARGET}-qmake-qt5 --version
		$MXEDIR/usr/bin/${MXETARGET}-qmake-qt5 ../MediaElch.pro CONFIG+=release

	else
		PATH="/opt/${QT}/bin:$PATH"
		qmake --version
		qmake ../MediaElch.pro CONFIG+=release PREFIX=/usr
	fi

elif [ "${OS_NAME}" = "Darwin" ]; then
	PATH="/usr/local/opt/qt/bin:$PATH"
	qmake --version
	qmake ../MediaElch.pro CONFIG+=release

else
	print_error "Unknown operating system: ${OS_NAME}"
	exit 1
fi

print_info "Successfully configured build"
popd > /dev/null
