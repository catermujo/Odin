@echo off
setlocal
if "%~1"=="" (
    echo Usage: build_windows.bat PATH_TO_ZLIB_1_2_12_SOURCE
    exit /b 1
)
set "ZLIB_SOURCE=%~f1"
if not exist "%~dp0build" mkdir "%~dp0build"
pushd "%~dp0build"
cl /nologo /c /O2 /MT /GL- /Z7 /Brepro /D_CRT_SECURE_NO_WARNINGS /I"%ZLIB_SOURCE%" "%ZLIB_SOURCE%\adler32.c" "%ZLIB_SOURCE%\compress.c" "%ZLIB_SOURCE%\crc32.c" "%ZLIB_SOURCE%\deflate.c" "%ZLIB_SOURCE%\gzclose.c" "%ZLIB_SOURCE%\gzlib.c" "%ZLIB_SOURCE%\gzread.c" "%ZLIB_SOURCE%\gzwrite.c" "%ZLIB_SOURCE%\infback.c" "%ZLIB_SOURCE%\inflate.c" "%ZLIB_SOURCE%\inftrees.c" "%ZLIB_SOURCE%\inffast.c" "%ZLIB_SOURCE%\trees.c" "%ZLIB_SOURCE%\uncompr.c" "%ZLIB_SOURCE%\zutil.c"
if errorlevel 1 exit /b 1
lib /nologo /Brepro /out:"%~dp0libz.lib" adler32.obj compress.obj crc32.obj deflate.obj gzclose.obj gzlib.obj gzread.obj gzwrite.obj infback.obj inflate.obj inftrees.obj inffast.obj trees.obj uncompr.obj zutil.obj
set "ZLIB_RESULT=%ERRORLEVEL%"
popd
exit /b %ZLIB_RESULT%
