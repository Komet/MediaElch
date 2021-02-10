#!/usr/bin/env bash

source "$(dirname "${BASH_SOURCE[0]}")/../scripts/utils.sh"
source "$(dirname "${BASH_SOURCE[0]}")/third_party_versions.sh"

check_mediaelch_dir_or_exit() {
	if [ ! -f MediaElch.pro ]; then
		echo "[export_project_information] Must be called from MediaElch's root directory!"
		exit 1
	fi
}

print_system_info_unix() {
	echo ""
	print_important "System Information:"
	echo "  Architecture:    ${OS_REV}"
	echo "  Machine:         ${OS_MACH}"
	echo "  Concurrent Jobs: ${JOBS}"
	echo ""
}

require_command() {
	if command -v "$1" > /dev/null 2>&1; then
		# shellcheck disable=SC2059
		printf "  ${GREEN}✔${NC} $1 installed\n"
	else
		# shellcheck disable=SC2059
		printf "  ${RED}✘${NC} $1 not installed.\n"
		export HAS_MISSING=1
	fi
}

# Gather information for packaging MediaElch such as version, git
# hash and date.
gather_project_and_system_details() {
	check_mediaelch_dir_or_exit

	# Git Details
	if [ -n "${TRAVIS_BRANCH-}" ]; then
		GIT_BRANCH="${TRAVIS_BRANCH}"
	else
		GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
	fi

	GIT_VERSION_FULL=$(git describe --abbrev=12 | sed -e 's/-g.*$// ; s/^v//') # Format: 2.4.3-123
	GIT_VERSION=${GIT_VERSION_FULL//-/.}                                       # Format: 2.4.3
	GIT_REVISION=${GIT_VERSION_FULL//*-/}                                      # Format: 123
	GIT_DATE=$(git --git-dir=".git" show --no-patch --pretty="%ci")
	if [ -z ${OS_NAME-} ] || [ ${OS_NAME-} = "Linux" ]; then
		GIT_DATE_FORMATTED=$(date -u +"%Y-%m-%dT%H:%M:%S%z" --date="${GIT_DATE}")
	elif [ ${OS_NAME-} = "Darwin" ]; then
		GIT_DATE_FORMATTED=$(date -ujf "%Y-%m-%d %H:%M:%S %z" "${GIT_DATE}" "+%Y-%m-%dT%H:%M:%S%z")
	fi

	ME_VERSION=$(sed -ne 's/.*AppVersionFullStr[^"]*"\(.*\)";.*/\1/p' Version.h) # Format: 2.4.3-dev
	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	DATE_DESC=$(date -u +"%Y-%m-%d %H:%M")
	ME_VERSION_NAME="${ME_VERSION}_${DATE_HASH}_git-${GIT_BRANCH}-${GIT_HASH}"

	if [[ -z "$GIT_REVISION" ]] || [[ "$GIT_VERSION" == "$GIT_REVISION" ]]; then
		GIT_REVISION="1" # May be empty or equal to the current tag
	fi

	print_important "Information used for packaging:"
	echo "  OS Name                  = ${OS_NAME}"
	echo "  Git Branch               = ${GIT_BRANCH}"
	echo "  Git Version              = ${GIT_VERSION}"
	echo "  Git Revision             = ${GIT_REVISION}"
	echo "  Git Date                 = ${GIT_DATE}"
	echo "  Git Date (formatted)     = ${GIT_DATE_FORMATTED}"
	echo "  Git Hash                 = ${GIT_HASH}"
	echo "  Date Hash                = ${DATE_HASH}"
	echo "  Date Description         = ${DATE_DESC}"
	echo "  MediaElch Version        = ${ME_VERSION}"
	echo "  Version Name (Long)      = ${ME_VERSION_NAME}"

	if [[ "$GIT_VERSION" != "$ME_VERSION" ]]; then
		echo ""
		print_warning "Git version and MediaElch version do not match!"
		print_warning "Add a new Git tag using:"
		print_warning "  git tag -a v1.1.0 -m \"Version 1.1.0\""
		echo ""
		print_warning "Will still continue"
	fi
}
