echo on

SET project_dir="%cd%"

echo Building OpenKJ-Songbook-Agent branch %APPVEYOR_REPO_BRANCH%

echo Set up environment...
set PATH=%QT%\bin\;C:\Qt\Tools\QtCreator\bin\;C:\Qt\QtIFW2.0.1\bin\;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %PLATFORM%

mkdir "%project_dir%\output"

7z e "%project_dir%\ssl.zip" -o"%project_dir%\output"

mkdir "%project_dir%\cscrt
7z e "%project_dir%\appveyor\cscrt.7z" -p"%cscrt_pass%" -o"%project_dir%\cscrt"

echo Building OpenKJ-Songbook-Agent...
qmake CONFIG-=debug CONFIG+=release
nmake

echo Packaging...
cd %project_dir%\build\windows\msvc\%LONGARCH%\release\
dir
windeployqt release\SongbookAgent.exe
echo Signing OpenKJ SongbookAgent binary
signtool sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /f "%project_dir%\cscrt\cscrt.pfx" /p "%pfx_pass%" release\SongbookAgent.exe

echo Copying project files for archival...
copy "%project_dir%\README.md" "release\README.md"
copy "%project_dir%\LICENSE" "release\LICENSE.txt"


echo Copying files for installer...
robocopy release\ "%project_dir%\output" /E /np
del "%project_dir%\output\*.obj"
del "%project_dir%\output\*.cpp"
del "%project_dir%\output\*.h"


echo Creating installer...
cd %project_dir%\installer\windows\%LONGARCH%\
dir
rem binarycreator.exe --offline-only -c config\config.xml -p packages OpenKJ-Tools-%SBAVERSION%-windows-%LONGARCH%-installer.exe
"C:\Program Files (x86)\Inno Setup 5\iscc.exe" "%project_dir%\appveyor\openkj-songbook-agent_%LONGARCH%.iss" /O"%project_dir%/"
move "%project_dir%\openkj-songbook-agent.exe" "%project_dir%\openkj-songbook-agent-%SBAVERSION%-%BITS%-setup.exe"
echo Signing installer...
signtool sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /f "%project_dir%\cscrt\cscrt.pfx" /p "%pfx_pass%" "%project_dir%\openkj-songbook-agent-%SBAVERSION%-%BITS%-setup.exe
