#!/usr/bin/env sh

###########################################################
# Important paths
SCRIPT_DIR="$( cd "$(dirname "$0")" || exit 1; pwd -P )"
PROJECT_DIR="${SCRIPT_DIR}/.."

export SCRIPT_DIR
export PROJECT_DIR

###########################################################
# Travis CI folding

TRAVIS_LAST_FOLD=""

fold_start() {
	echo "travis_fold:start:$1"
	TRAVIS_LAST_FOLD="$1"
}

fold_end() {
	if [ "$TRAVIS_LAST_FOLD" = "" ]; then
		return
	fi
	echo "travis_fold:end:$TRAVIS_LAST_FOLD"
	TRAVIS_LAST_FOLD=""
}
