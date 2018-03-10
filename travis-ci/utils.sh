#!/usr/bin/env sh

# Binary dir for compilers, etc
BIN_DIR=$(dirname $(which g++))

###########################################################
# Print colors

RED='\033[0;31m'
ORANGE='\033[0;33m'
BLUE='\033[0;34m'
LIGHT_BLUE='\033[1;34m'
NC='\033[0m' # No Color

print_important()
{
	printf '%b' "${BLUE}${1}${NC}\n"
}

print_info() {
	printf '%b' "${LIGHT_BLUE}${1}${NC}\n"
}

print_warning() {
	printf '%b' "${ORANGE}${1}${NC}\n"
}

print_error() {
	printf '%b' "${RED}${1}${NC}\n"
}

###########################################################
# Travis CI folding

fold_start() {
	echo -e "travis_fold:start:$1"
}

fold_end() {
	echo -e "travis_fold:end:$1"
}
