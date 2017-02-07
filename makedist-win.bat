echo off


call "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

msbuild .\Builds\VisualStudio2015\SpatialPodcast.sln /p:configuration=release /p:platform=win32 /nologo /verbosity:minimal /fileLogger /m /flp:logfile=build-win.log;errorsonly 
msbuild .\Builds\VisualStudio2015\SpatialPodcast.sln /p:configuration=release /p:platform=x64 /nologo /verbosity:minimal /fileLogger /m /flp:logfile=build-win.log;errorsonly;append

echo ------------------------------------------------------------------
echo Code sign aax binary...
wraptool sign --verbose --account X --wcguid X --keyfile .\..\..\..\Certificates\aax2.p12 --in .\Builds\VisualStudio2015\Release\SpatialPodcast.aaxplugin\Contents\Win32\SpatialPodcast.aaxplugin --out .\Builds\VisualStudio2015\Release\SpatialPodcast.aaxplugin\Contents\Win32\SpatialPodcast.aaxplugin
wraptool sign --verbose --account X --wcguid X --keyfile .\..\..\..\Certificates\aax2.p12 --in .\Builds\VisualStudio2015\x64\Release\SpatialPodcast.aaxplugin\Contents\x64\SpatialPodcast.aaxplugin --out .\Builds\VisualStudio2015\x64\Release\SpatialPodcast.aaxplugin\Contents\x64\SpatialPodcast.aaxplugin
REM - Make Installer (InnoSetup)

echo ------------------------------------------------------------------
echo Making Installer ...
"%ProgramFiles(x86)%\Inno Setup 5\iscc" /cc ".\installer\SpatialPodcast.iss"

echo ------------------------------------------------------------------
echo Printing log file to console...

type build-win.log

pause