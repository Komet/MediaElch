#!/usr/bin/env sh

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
