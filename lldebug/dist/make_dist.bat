rem 
rem リリースファイルを作るためのバッチファイルです。
rem 

set MAJOR_VER=0
set MINOR_VER=2
set UPDATE_VER=0

set DIRNAME=lldebug_%MAJOR_VER%_%MINOR_VER%_%UPDATE_VER%
set ARCNAME=%DIRNAME%.zip
set OUTDIR=..\tags\%DIRNAME%
set OUTARC=..\tags\%ARCNAME%

cd ..
rmdir /Q /S %OUTDIR%
mkdir %OUTDIR%

rem 
rem 実行ファイルを集めたフォルダを作成します。
rem 
mkdir %OUTDIR%\bin_win32
call path-vc8
msbuild VisualC8\lldebug_libs.sln /p:Configuration="Release" /t:Build
msbuild VisualC8\lldebug.sln /p:Configuration="Release" /t:Build
copy VisualC8\release\lldebug_frame.exe %OUTDIR%\bin_win32
copy VisualC8\release\lldebug.lib %OUTDIR%\bin_win32
copy VisualC8\release\lua_debug.exe %OUTDIR%\bin_win32
copy VisualC8\release\boost_system.lib %OUTDIR%\bin_win32

rem 
rem 必要なファイルすべてをコピーします。
rem 
xcopy . %OUTDIR%\ /EXCLUDE:dist\exclude.txt /S

rem
rem ファイルをアーカイブします
rem
pushd ..\tags
del %ARCNAME%
e:\common\lha32\lha32.exe a -r -x %ARCNAME% %DIRNAME%\
popd

pause
