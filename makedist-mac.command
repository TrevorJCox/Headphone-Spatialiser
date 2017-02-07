#! /bin/sh

BASEDIR=$(dirname $0)

cd $BASEDIR

rm -R /Library/Audio/Plug-Ins/Components/SpatialPodcast.component
rm -R /Library/Audio/Plug-Ins/VST/SpatialPodcast.vst
rm -R /Library/Application\ Support/Avid/Audio/Plug-Ins/SpatialPodcast.aaxplugin

xcodebuild -project ./Builds/MacOSX/SpatialPodcast.xcodeproj -target "SpatialPodcast (All)" -configuration Release 2> ./build-mac.log

echo "code sign AAX binary"
/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool sign --verbose --account X --wcguid X --signid "Developer ID Application: X" --in /Library/Application\ Support/Avid/Audio/Plug-Ins/SpatialPodcast.aaxplugin --out /Library/Application\ Support/Avid/Audio/Plug-Ins/SpatialPodcast.aaxplugin
# 

#---------------------------------------------------------------------------------------------------------
# zip

echo "copying binaries..."
echo ""
sudo mv /Users/oli/Library/Audio/Plug-Ins/Components/SpatialPodcast.component /Library/Audio/Plug-Ins/Components/SpatialPodcast.component
sudo mv /Users/oli/Library/Audio/Plug-Ins/VST/SpatialPodcast.vst /Library/Audio/Plug-Ins/VST/SpatialPodcast.vst

echo "building installer"
echo ""
packagesbuild installer/SpatialPodcast.pkgproj

#---------------------------------------------------------------------------------------------------------

echo "done"