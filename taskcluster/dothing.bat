@echo off&setlocal enabledelayedexpansion

if not defined SOURCE_DIR set SOURCE_DIR="%1"
if not exist "!SOURCE_DIR!" mkdir "!SOURCE_DIR!"&echo SOURCE_DIR: !SOURCE_DIR!
if not defined BUID_DIR set BUID_DIR=d:\works
if not exist "!BUID_DIR!" mkdir "!BUID_DIR!"&echo BUID_DIR: !BUID_DIR!

if not defined MY_BITS @echo mozconfig32 or mozconfig64 not exist.&EXIT /B 4
if not defined LIBPORTABLE_PATH @echo Build libportable need this path.&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\clang" @echo clang not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\msys" @echo msys not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\nodejs" @echo nodejs not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\hg" @echo hg not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\rust" @echo rust not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\python" @echo python not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\python3" @echo python3 not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\msysdo.exe" @echo msysdo not exist&EXIT /B 4

@if "%MY_BITS%" == "win32" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32"
@if "%MY_BITS%" == "win64" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64"

@echo ##########################
@set PATH=!BUID_DIR!\mozillabuild\bin;!BUID_DIR!\mozillabuild\clang\bin;%PATH%
@echo PATH: [%PATH%]
@echo ##########################

@echo ##########################
@echo List toolchain
cd /d "%BUID_DIR%\mozillabuild"
dir /a
@echo ##########################

@cd /d "!SOURCE_DIR!"
@git clone --depth=1 https://github.com/adonais/libportable.git libportable-src
@echo libportable-src:
@dir "!SOURCE_DIR!\libportable-src" /a
@del /q !BUID_DIR!\*.gz !BUID_DIR!\*.7z
@dir "!BUID_DIR!" /a
@cd /d "!SOURCE_DIR!\libportable-src"
nmake -f Makefile.msvc CC=clang-cl clean
nmake -f Makefile.msvc CC=clang-cl install
@if not "%errorlevel%" == "0" @echo compile libportable failed.&EXIT /B %errorlevel%

@cd /d "!SOURCE_DIR!"
@git clone --depth=1 https://github.com/adonais/upcheck.git upcheck-src
@cd /d "!SOURCE_DIR!\upcheck-src"
nmake clean
nmake
@if not "%errorlevel%" == "0" @echo compile upcheck failed.EXIT /B %errorlevel%
@if exist "%LIBPORTABLE_PATH% copy /y "Release\upcheck.exe" "!LIBPORTABLE_PATH!\bin"

@echo ##########################
@cd /d %GITHUB_WORKSPACE%
@rd /s/q "!SOURCE_DIR!\libportable-src"
@echo GITHUB_WORKSPACE: [%GITHUB_WORKSPACE%]
@dir %GITHUB_WORKSPACE% /a
@echo ##########################

::@echo .........................
::@echo msvc version:
::dir "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Redist\MSVC\14.25.28508\x64" /a
::@echo windows sdk dll:
::dir "c:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x64" /a
::@echo all environment:
::@set
