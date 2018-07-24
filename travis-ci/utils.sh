#!/usr/bin/env sh

###########################################################
# Important paths

export BIN_DIR=$(dirname $(which g++))
export SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
export PROJECT_DIR="${SCRIPT_DIR}/.."

###########################################################
# Travis CI folding

TRAVIS_LAST_FOLD=""

fold_start() {
	echo -e "travis_fold:start:$1"
	TRAVIS_LAST_FOLD="$1"
}

fold_end() {
	if [ "$TRAVIS_LAST_FOLD" == "" ]; then
		return
	fi
	echo -e "travis_fold:end:$TRAVIS_LAST_FOLD"
	TRAVIS_LAST_FOLD=""
}
