#!/bin/bash

#==============================================================================
export USAGE="usage: MacOS_build_sign_and_notarize --pass <dev-id-password> [--dev]"
export PASS=""
export DEV_BUILD=""
export BINARY_NAME=""

#==============================================================================
# Parse args

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    --pass)
    PASS="$2"
    shift # past argument
    shift # past value
    ;;
    --dev)
	DEV_BUILD=true
	BINARY_NAME="ControlGRIS2_dev"
	shift
	;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if [[ "$PASS" == "" ]];then
	echo "Missing param --pass"
	echo "$USAGE"
	exit 1
elif [[ "$DEV_BUILD" == "" ]];then
    DEV_BUILD=false
    BINARY_NAME="ControlGRIS2"
fi

export LINE=":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"

export INSTALLER_SIGNATURE="Developer ID Installer: Gael Lane Lepine (62PMMWH49Z)"
export APP_SIGNATURE="Developer ID Application: Gael Lane Lepine (62PMMWH49Z)"
export TEAM_ID="62PMMWH49Z"
export NOTARIZE_USER="glanelepine@gmail.com"
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
function prepare_for_dev_build() {
    if [[ "$DEV_BUILD" == true ]];then
        cd $PROJECT_PATH
        echo -e "$LINE\nPrepare project file for a dev plugin build\n$LINE"

        sed -i '' 's|pluginName="ControlGRIS2"|pluginName="ControlGRIS2_dev"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|pluginDesc="ControlGRIS2"|pluginDesc="ControlGRIS2_dev"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|pluginCode="Xzz2"|pluginCode="Xzb2"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|targetName="ControlGRIS2"|targetName="ControlGRIS2_dev"|g' ./ControlGRIS.jucer || exit 1

        sed -i '' 's|Applications/GRIS/ControlGRIS2.app|Applications/GRIS/ControlGRIS2_dev.app|g' ./ControlGRIS.plist || exit 1

        $Projucer --resave "$PROJECT_FILE" || exit 1
    fi
}

#==============================================================================
function reset_dev_build() {
    if [[ "$DEV_BUILD" == true ]];then
        cd $PROJECT_PATH
        echo -e "$LINE\nReset project file after a dev plugin build\n$LINE"

        sed -i '' 's|pluginName="ControlGRIS2_dev"|pluginName="ControlGRIS2"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|pluginDesc="ControlGRIS2_dev"|pluginDesc="ControlGRIS2"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|pluginCode="Xzb2"|pluginCode="Xzz2"|g' ./ControlGRIS.jucer || exit 1
        sed -i '' 's|targetName="ControlGRIS2_dev"|targetName="ControlGRIS2"|g' ./ControlGRIS.jucer || exit 1

        sed -i '' 's|Applications/GRIS/ControlGRIS2_dev.app|Applications/GRIS/ControlGRIS2.app|g' ./ControlGRIS.plist || exit 1

        $Projucer --resave "$PROJECT_FILE" || exit 1
    fi
}

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
	echo -e "$LINE\nCopying non-aax plugins and standalone...\n$LINE"
	cd "$BIN_PATH" || exit 1
	rm -fr "$TEMP_PATH"
	mkdir "$TEMP_PATH"
	for filename in *.vst3; do
		cp -R -H "$filename" "$TEMP_PATH"
	done
	for filename in *.component; do
		cp -R -H "$filename" "$TEMP_PATH"
	done
	cp -R -H "$BINARY_NAME.app" "$TEMP_PATH"
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
	STANDALONE="Product/Applications/GRIS"

	mkdir -p "$AU_PATH" || exit 1
	mkdir -p "$VST3_PATH" || exit 1
	mkdir -p "$AAX_PATH" || exit 1
	mkdir -p "$STANDALONE" || exit 1

	cp -R -H *.component "$AU_PATH" || exit 1
	cp -R -H *.vst3 "$VST3_PATH" || exit 1
	cp -R -H *.aaxplugin "$AAX_PATH" || exit 1
	cp -R -H $BINARY_NAME.app "$STANDALONE" || exit 1
}

#==============================================================================
function package() {
	echo -e "$LINE\nBuilding package...\n$LINE"
	cd $TEMP_PATH

	pkgbuild    --root "Product" \
	            --install-location "/" \
	            --identifier "$IDENTIFIER" \
	            --version "$VERSION" \
				--component-plist "../ControlGRIS.plist" \
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

function rename_dev_pkg() {
    if [[ "$DEV_BUILD" == true ]];then
        cd "$PROJECT_PATH/out"
        COMMIT_NUM=`git rev-parse --short HEAD`
        mv "ControlGRIS_${VERSION}.pkg" "ControlGRIS_${VERSION}_dev_${COMMIT_NUM}.pkg"
    fi
}

prepare_for_dev_build
build
copy_to_temp
sign
sign_aax
build_tree
package
notarize_pkg
reset_dev_build
rename_dev_pkg

echo -e "$LINE\nDone !\n$LINE"
