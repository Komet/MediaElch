#!/bin/bash

root="$(pwd)/scripts"
outDir="${root}/generated_media"

# Create an example video file.
echo "Not a valid video file" > "$root/fake_data/Demo.mov"

# Delete old data
rm -rf "${outDir}"

###########################################################
# Movies
###########################################################

movieInput="${root}/fake_data/movies_en.txt"
movieOutDir="${outDir}/movies"

printf "Creating fake movies...     "
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
		while IFS= read -r episode
		do
			if [ "$episode" != "" ]; then
				local seasonFile=$(basename ${season})
				local seasonDir="${showOutDir}/${seasonFile%.*}" # Name without file extension

				mkdir -p ${seasonDir};
				ln -s "${root}/fake_data/Demo.mov" "${seasonDir}/${episode}.mov"
			fi
		done < "$season"
	done
}

printf "Creating fake tv shows...   "
for show in "${root}/fake_data/tvshows_en/*"
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
printf "[Done]\n"
