#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

cd "${0%/*}/../.."

. ./.ci/utils.sh

export_project_information

cp ./build/MediaElch_win.zip "./build/MediaElch_win_${VERSION_NAME}.zip"

echo "Preparing bintray.json for win"

cat > "./.ci/bintray.json" << EOF
{
	"package": {
		"name": "MediaElch-win",
		"repo": "MediaElch",
		"subject": "komet",
		"website_url": "https://www.kvibes.de/mediaelch/",
		"vcs_url": "https://github.com/komet/MediaElch.git",
		"issue_tracker_url": "https://github.com/komet/MediaElch/issues",
		"licenses": ["LGPL-3.0"]
	},
	"version": {
		"name": "${VERSION_NAME}",
		"desc": "MediaElch version ${ME_VERSION} for Windows\nDate: ${DATE_DESC}\nGit Branch: ${GIT_BRANCH}\nGit Hash: ${GIT_HASH}",
		"released": "${RELEASE_DATE}",
		"gpgSign": false
	},
	"files":
	[
		{
			"includePattern": "$(pwd)/build/MediaElch_win_${VERSION_NAME}.zip",
			"uploadPattern": "MediaElch_win_${VERSION_NAME}.zip"
		}
	],
	"publish": true
}
EOF

jq "" ./.ci/bintray.json
