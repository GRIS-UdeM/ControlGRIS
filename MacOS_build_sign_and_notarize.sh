#!/bin/bash

if [ $# -ne 1 ];then
	echo "You must provide the notarizing account's password as an argument."
	exit 1
fi

export LINE=":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"

export INSTALLER_SIGNATURE="Developer ID Installer: Gael Lane Lepine (62PMMWH49Z)"
export APP_SIGNATURE="Developer ID Application: Gael Lane Lepine (62PMMWH49Z)"
export TEAM_ID="62PMMWH49Z"
export NOTARIZE_USER="glanelepine@gmail.com"
export PASS="$1"
export IDENTIFIER="ca.umontreal.musique.gris.controlgris.pkg"

export PROJECT_PATH="`pwd`"
export PROJECT_FILE="$PROJECT_PATH/ControlGRIS.jucer"
export XCODE_PATH="$PROJECT_PATH/Builds/MacOSX"
export BIN_PATH="$XCODE_PATH/build/Release/"
export TEMP_PATH="$PROJECT_PATH/out"
export PLIST_PATH="$PROJECT_PATH/Application.plist"
export ZIP_PATH="$TEMP_PATH/plugins.zip"

Projucer="$PROJECT_PATH/submodules/StructGRIS/submodules/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer"

#==============================================================================
# get app version
export VERSION=`$Projucer --get-version "$PROJECT_FILE"`
echo -e "$LINE\nVersion is $VERSION"
export PKG_PATH="$TEMP_PATH/ControlGRIS_$VERSION.pkg"

#==============================================================================
function generate_project() {
	# NOTE : this creates the project file with unusable file permissions...
	echo "Generating project files..."
	$Projucer --resave "$PROJECT_FILE" || exit 1
	cd "$PROJECT_PATH"
}

#==============================================================================
function build() {
	echo -e "$LINE\nBuilding ControlGRIS $VERSION\n$LINE"
	cd "$XCODE_PATH"
	chmod -R 755 .
	xcodebuild -configuration Release || exit 1
}

#==============================================================================
function copy_to_temp() {
	echo -e "$LINE\nCopying non-aax plugins...\n$LINE"
	cd "$BIN_PATH" || exit 1
	rm -fr "$TEMP_PATH"
	mkdir "$TEMP_PATH"
	for filename in *.vst3; do
		cp -R -H "$filename" "$TEMP_PATH"
	done
	for filename in *.component; do
		cp -R -H "$filename" "$TEMP_PATH"
	done
}

#==============================================================================
function sign() {
	echo -e "$LINE\nSigning non-aax plugins...\n$LINE"
	cd "$TEMP_PATH"
	for filename in *; do
		codesign \
			--force \
			-s "$APP_SIGNATURE" \
			-v "$filename" \
			--options=runtime \
			--timestamp \
			|| exit 1
	done
}

#==============================================================================
function sign_aax() {
	echo -e "$LINE\nSigning aax plugin...\n$LINE"
	cd "$BIN_PATH"
	PLUGIN_NAME=`echo *.aaxplugin`
	IN_PATH="$TEMP_PATH/unsigned_$PLUGIN_NAME"
	OUT_PATH="$TEMP_PATH/$PLUGIN_NAME"
	cp -R -H "$PLUGIN_NAME" "$IN_PATH" || exit 1

	wraptool sign \
			 --verbose \
			 --signid "$APP_SIGNATURE" \
			 --account grisresearch \
			 --wcguid A4B35290-7C9C-11EB-8B4D-00505692C25A \
			 --in "$IN_PATH" \
			 --out "$OUT_PATH" \
			 --autoinstall on \
			 --dsigharden \
			 --extrasigningoptions "--timestamp --options runtime" \
			 || exit 1
	rm -fr "$IN_PATH" || exit 1
}

#==============================================================================
function build_tree() {
	echo -e "$LINE\nBuilding directory structure..."
	cd $TEMP_PATH
	BASE_PATH="Product/Library/Audio/Plug-Ins"
	AU_PATH="$BASE_PATH/Components"
	VST3_PATH="$BASE_PATH/VST3"
	AAX_PATH="Product/Library/Application Support/Avid/Audio/Plug-Ins"

	mkdir -p "$AU_PATH" || exit 1
	mkdir -p "$VST3_PATH" || exit 1
	mkdir -p "$AAX_PATH" || exit 1

	cp -R -H *.component "$AU_PATH" || exit 1
	cp -R -H *.vst3 "$VST3_PATH" || exit 1
	cp -R -H *.aaxplugin "$AAX_PATH" || exit 1
}

#==============================================================================
function package() {
	echo -e "$LINE\nBuilding package...\n$LINE"
	cd $TEMP_PATH

	pkgbuild    --root "Product" \
	            --install-location "/" \
	            --identifier "$IDENTIFIER" \
	            --version "$VERSION" \
	            --sign "$INSTALLER_SIGNATURE" \
	            --timestamp \
	            "$PKG_PATH" || exit 1
}

#==============================================================================
function notarize_pkg()
{
	echo -e "$LINE\nNotarizing pkg...\n$LINE"

	cd "$TEMP_PATH" || exit 1

	xcrun notarytool submit --apple-id "$NOTARIZE_USER" \
							--password "$PASS" \
							--team-id $TEAM_ID \
							"ControlGRIS_$VERSION.pkg" || exit 1

	wait_for_notarization
	xcrun stapler staple "$PKG_PATH" || exit 1
}

#==============================================================================
function get_last_request_uuid() {
	checkHistory=`xcrun notarytool history --apple-id "$NOTARIZE_USER" --password "$PASS" --team-id $TEAM_ID`
	echo "$checkHistory" | head -n 5 | tail -n 1 | cut -d' ' -f 6
}

#==============================================================================
function wait_for_notarization() {
	echo -e "$LINE\nChecking for notarization success...\n$LINE"
	echo "waiting a bit..."
	sleep 30
	WAITING=" In Progress"
	SUCCESS=" Accepted"
	uuid=`get_last_request_uuid`
	checkStatus="$WAITING"
	while [[ "$checkStatus" == "$WAITING" ]];do
		sleep 10
		checkHistory=`xcrun notarytool info "$uuid" --apple-id "$NOTARIZE_USER" --password "$PASS" --team-id $TEAM_ID`
		checkStatus=`echo "$checkHistory" | grep status | head -n 1 | cut -d: -f 2`
		echo "Status is \"$checkStatus\""
	done
	if [[ "$checkStatus" != "$SUCCESS" ]];then
		echo -e "Error : notarization was refused, see the report:\n"
		xcrun notarytool info "$uuid" --apple-id "$NOTARIZE_USER" --password "$PASS" --team-id $TEAM_ID
		xcrun notarytool log "$uuid" --apple-id "$NOTARIZE_USER" --password "$PASS" --team-id $TEAM_ID
		exit 1
	fi
}

build
copy_to_temp
sign
sign_aax
build_tree
package
notarize_pkg

echo -e "$LINE\nDone !\n$LINE"
