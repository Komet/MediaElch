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

###########################################################
# Gather information for packaging MediaElch such as version, git
# hash and date.
export_project_information() {
	if [ ! -f MediaElch.pro ]; then
		echo "[export_project_information] Must be called from MediaElch's root directory!"
		exit 1
	fi

	if [ -n "${TRAVIS_BRANCH-}" ]; then
		GIT_BRANCH="${TRAVIS_BRANCH}"
	else
		GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
	fi

	echo "  GIT_BRANCH = ${GIT_BRANCH}"
	export GIT_BRANCH

	GIT_DATE=$(git --git-dir=".git" show --no-patch --pretty="%ci")
	echo "  GIT_DATE = ${GIT_DATE}"
	export GIT_DATE;

	RELEASE_DATE=$(date -u +"%Y-%m-%dT%H:%M:%S%z" --date="${GIT_DATE}")
	echo "  RELEASE_DATE = ${RELEASE_DATE}"
	export RELEASE_DATE;

	ME_VERSION=$(sed -ne 's/.*AppVersionFullStr[^"]*"\(.*\)";.*/\1/p' Version.h)
	echo "  ME_VERSION = ${ME_VERSION}"
	export ME_VERSION;

	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	echo "  GIT_HASH = ${GIT_HASH}"
	export GIT_HASH;

	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	echo "  DATE_HASH = ${DATE_HASH}"
	export DATE_HASH;

	DATE_DESC=$(date -u +"%Y-%m-%d %H:%M")
	echo "  DATE_DESC = ${DATE_DESC}"
	export DATE_DESC;

	VERSION_NAME="${ME_VERSION}_${DATE_HASH}_git-${GIT_BRANCH}-${GIT_HASH}"
	echo "  VERSION_NAME = ${VERSION_NAME}"
	export VERSION_NAME;
}
