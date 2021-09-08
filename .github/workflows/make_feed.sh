#!/bin/bash
# Retrieves the latest release output to generate a QiToolchain XML feed.
set -e
echo "Getting packages from current release at ${GITHUB_API_URL}/repos/${GITHUB_REPOSITORY}/releases/latest"
ASSETS_URL=$(curl --silent "${GITHUB_API_URL}/repos/${GITHUB_REPOSITORY}/releases/latest" |
    jq -r '.assets_url')
PACKAGES_URLS=$(curl --silent ${ASSETS_URL} |
    jq '.[] | .browser_download_url' |
    grep .zip | xargs -L 1 echo) # echo removes the extra quotes

echo "Making a toolchain feed."
echo "<toolchain>" > feed.xml
echo "Adding sub-toolchain from ${FEED}"
echo "  <feed url=\"${FEED}\" />" >> feed.xml
echo "Adding links for the following packages:" ${PACKAGES_URLS}
for URL in $PACKAGES_URLS; do
    ZIP=package.zip
    wget -q $URL -O $ZIP;
    NAME=$(unzip -p $ZIP package.xml | xmllint --xpath "string(/package/@name)" -)
    VERSION=$(unzip -p $ZIP package.xml | xmllint --xpath "string(/package/@version)" -)
    echo "  <package name=\"${NAME}\" version=\"${VERSION}\" url=\"${URL}\" />" >> feed.xml
    rm $ZIP
done
echo "</toolchain>" >> feed.xml

realpath feed.xml
