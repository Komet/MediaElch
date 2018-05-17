#!/bin/bash

root="$(pwd)/scripts"
outDir="${root}/generated_media"

# Create example files.
echo "Not a valid video file" > "$root/fake_data/Demo.mov"
echo "Not a valid mp3 file" > "$root/fake_data/Demo.mp3"

# Delete old data
rm -rf "${outDir}"

###########################################################
# Movies
###########################################################

movieInput="${root}/fake_data/movies_en.txt"
movieOutDir="${outDir}/movies"

printf "Creating fake movies...         "
while IFS= read -r line
do
	if [ "$line" != "" ]; then
		mkdir -p "${movieOutDir}/${line}";
		ln -s "${root}/fake_data/Demo.mov" "${movieOutDir}/${line}/movie.mov"
	fi
done < "$movieInput"
printf "[Done]\n"


###########################################################
# TV Shows
###########################################################

showBaseOutDir="${outDir}/tvshows_en"

# $1: TV Show directory
create_fake_show() {
	local showInputDir=${1}
	local showName=$(basename $1)
	local showOutDir="${showBaseOutDir}/${showName}"

	for season in ${showInputDir}/*.txt
	do
		local seasonFile=$(basename "${season}")
		local seasonDir="${showOutDir}/${seasonFile%.*}" # Name without file extension
		mkdir -p "${seasonDir}";

		while IFS= read -r episode
		do
			if [ "$episode" != "" ]; then
				ln -s "${root}/fake_data/Demo.mov" "${seasonDir}/${episode}.mov"
			fi
		done < "$season"
	done
}

printf "Creating fake tv shows...       "
for show in "${root}/fake_data/tvshows_en/"*
do
	create_fake_show $show
done
printf "[Done]\n"


###########################################################
# Concerts
###########################################################

concertInput="${root}/fake_data/concerts.txt"
concertOutDir="${outDir}/concerts"

printf "Creating fake concerts...     "
while IFS= read -r line
do
	if [ "$line" != "" ]; then
		mkdir -p "${concertOutDir}/${line}";
		ln -s "${root}/fake_data/Demo.mov" "${concertOutDir}/${line}/concert.mov"
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
	local artistName=$(basename $1)
	local artistOutDir="${musicBaseOutDir}/${artistName}"

	for album in ${aristInputDir}/*.txt
	do
		local albumFile=$(basename "${album}")
		local albumDir="${artistOutDir}/${albumFile%.*}" # Name without file extension
		mkdir -p "${albumDir}";

		while IFS= read -r song
		do
			if [ "$song" != "" ]; then
				ln -s "${root}/fake_data/Demo.mp3" "${albumDir}/${song}.mp3"
			fi
		done < "$album"
	done
}

printf "Creating fake music albums...   "
for artist in "${root}/fake_data/music/"*
do
	create_fake_music $artist
done

printf "[Done]\n"
