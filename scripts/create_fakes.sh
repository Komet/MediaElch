#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

current="$(pwd)"
root="$(
	cd "$(dirname "$0")"
	pwd -P
)"
outDir="${root}/generated_media"

# Create example files.
echo "Not a valid video file" > "${root}/library/Demo.mov"
echo "Not a valid mp3 file" > "${root}/library/Demo.mp3"

# Delete old data
rm -rf "${outDir}"

###########################################################
# Movies
###########################################################

movieInput="${root}/library/movies_en.txt"
movieOutDir="${outDir}/movies"

printf "Creating fake movies...         "
while IFS= read -r line; do
	if [ "$line" != "" ]; then
		mkdir -p "${movieOutDir}/${line}"
		cp "${root}/library/Demo.mov" "${movieOutDir}/${line}/movie.mov"
	fi
done < "$movieInput"
printf "[Done]\n"

echo "Creating a fake BluRay structure"
printf "  for 'The Simpsons Movie'...   "
mkdir -p "${outDir}/movies/The_Simpsons_Movie_2007"
cd "${outDir}/movies/The_Simpsons_Movie_2007"
mkdir BDMV
cd BDMV
mkdir PLAYLIST
mkdir CLIPINF
mkdir -p STREAM/SSIF
mkdir AUXDATA
mkdir BACKUP
touch PLAYLIST/12345.mpls
touch CLIPINF/12345.clpi
touch STREAM/12345.m2ts
touch STREAM/SSIF/12345.ssif
touch AUXDATA/sound.bdmv
touch AUXDATA/12345.otf
touch BACKUP/index.bdmv
touch BACKUP/MovieObject.bdmv
touch BACKUP/12345.mpls
touch BACKUP/12345.clpi
touch index.bdmv
touch MovieObject.bdmv
cd "${current}"
printf "[Done]\n"

###########################################################
# TV Shows
###########################################################

showBaseOutDir="${outDir}/tvshows_en"

# $1: TV Show directory
create_fake_show() {
	local showInputDir=${1}
	local showName
	local showOutDir

	showName=$(basename $1)
	showOutDir="${showBaseOutDir}/${showName}"

	for season in "${showInputDir}"/*.txt; do
		local seasonFile
		local seasonDir # Name without file extension

		seasonFile=$(basename "${season}")
		seasonDir="${showOutDir}/${seasonFile%.*}"

		mkdir -p "${seasonDir}"

		while IFS= read -r episode; do
			if [ "$episode" != "" ]; then
				cp "${root}/library/Demo.mov" "${seasonDir}/${episode}.mov"
			fi
		done < "$season"
	done
}

printf "Creating fake tv shows...       "
for show in "${root}/library/tvshows_en/"*; do
	create_fake_show $show
done
printf "[Done]\n"

###########################################################
# Concerts
###########################################################

concertInput="${root}/library/concerts.txt"
concertOutDir="${outDir}/concerts"

printf "Creating fake concerts...     "
while IFS= read -r line; do
	if [ "$line" != "" ]; then
		mkdir -p "${concertOutDir}/${line}"
		cp "${root}/library/Demo.mov" "${concertOutDir}/${line}/concert.mov"
	fi
done < "$concertInput"

printf "  [Done]\n"

###########################################################
# Music
###########################################################

musicBaseOutDir="${outDir}/music"

# $1: Music directory
create_fake_music() {
	local aristInputDir=${1}
	local artistName
	local artistOutDir

	artistName=$(basename $1)
	artistOutDir="${musicBaseOutDir}/${artistName}"

	for album in "${aristInputDir}"/*.txt; do
		local albumFile
		local albumDir # Name without file extension

		albumFile=$(basename "${album}")
		albumDir="${artistOutDir}/${albumFile%.*}"

		mkdir -p "${albumDir}"

		while IFS= read -r song; do
			if [ "$song" != "" ]; then
				cp "${root}/library/Demo.mp3" "${albumDir}/${song}.mp3"
			fi
		done < "$album"
	done
}

printf "Creating fake music albums...   "
for artist in "${root}/library/music/"*; do
	create_fake_music $artist
done

printf "[Done]\n"

###########################################################
# Downloads
###########################################################

downloadInput="${root}/library/downloads.txt"
downloadOutDir="${outDir}/downloads"

mkdir -p "${downloadOutDir}"

printf "Creating fake downloads...    "
while IFS= read -r line; do
	if [ "$line" != "" ]; then
		cp "${root}/library/Demo.mov" "${downloadOutDir}/${line}"
	fi
done < "$downloadInput"

printf "  [Done]\n"
