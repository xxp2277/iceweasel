@echo off&setlocal enabledelayedexpansion

set M_BITS=0
IF NOT DEFINED SOURCE_DIR SET SOURCE_DIR="%1"
IF not exist "!SOURCE_DIR!" mkdir "!SOURCE_DIR!"&echo SOURCE_DIR: !SOURCE_DIR!
IF NOT DEFINED BUID_DIR SET BUID_DIR=d:\works
IF not exist "!BUID_DIR!" mkdir "!BUID_DIR!"&echo BUID_DIR: !BUID_DIR!
@echo 
@echo GITHUB_WORKSPACE: %GITHUB_WORKSPACE%
@cd /d %GITHUB_WORKSPACE%
if exist mozconfig32 set M_BITS=32&call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32"
if exist mozconfig64 set M_BITS=64&call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64"
@echo
@echo .........................
@set PATH=%PATH%;!BUID_DIR!\mozillabuild\bin;!BUID_DIR!\mozillabuild\clang\bin
@echo PATH: [%PATH%]
@cd /d "!BUID_DIR!"
if "!M_BITS!" == "0" @echo mozconfig32 or mozconfig64 not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\clang" @echo clang not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\msys" @echo msys not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\nodejs" @echo nodejs not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\hg" @echo hg not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\rust" @echo rust not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\python" @echo python not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\python3" @echo python3 not exist&EXIT /B 4
if not exist "!BUID_DIR!\mozillabuild\msysdo.exe" @echo msysdo not exist&EXIT /B 4
@cd /d "!SOURCE_DIR!"
@git clone --depth=1 https://github.com/adonais/libportable.git libportable-src
@echo libportable-src:
@dir "!SOURCE_DIR!\libportable-src" /a
@del /q !BUID_DIR!\*.gz !BUID_DIR!\*.7z
@dir "!BUID_DIR!" /a
@cd /d "!SOURCE_DIR!\libportable-src"
nmake -f Makefile.msvc CC=clang-cl clean
nmake -f Makefile.msvc CC=clang-cl install

@cd /d %GITHUB_WORKSPACE%
@rd /s/q "!SOURCE_DIR!\libportable-src"
@echo
@dir %GITHUB_WORKSPACE% /a

::@echo .........................
::@echo msvc version:
::dir "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Redist\MSVC\14.25.28508\x64" /a
::@echo windows sdk dll:
::dir "c:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x64" /a
::@echo all environment:
::@set
