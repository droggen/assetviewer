@echo off

echo Packaging...

IF "%1"=="" GOTO HAVE_0

rd /q /s windows
del /q assetviewer-win.zip
md windows


set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC

%1\windeployqt.exe build-assetviewer-Desktop_Qt_5_12_3_MSVC2017_64bit-Release\release\assetviewer.exe --dir windows --force --compiler-runtime 

copy build-assetviewer-Desktop_Qt_5_12_3_MSVC2017_64bit-Release\release\assetviewer.exe windows

rem copy "%1\libstdc++-6.dll" windows
rem copy %1\libwinpthread-1.dll windows
rem copy %1\libgcc_s_dw2-1.dll windows


cd windows
zip -r ..\assetviewer-win.zip * -x *.git*
cd..

echo Packaging done

exit /b

:HAVE_0
echo Specify the path to qt. Example: %0 C:\Qt\5.8\mingw53_32\bin
pause

