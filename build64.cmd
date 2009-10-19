@set ARCH=x86_64
@set PATH=%PATH%;D:\MinGW-W64-%ARCH%\bin
@set PREFIX=%ARCH%-w64-mingw32-
%PREFIX%windres -i notepad.rc --input-format=rc -o notepad.res -O coff
%PREFIX%gcc -Wall -O2 -std=c99 notepad.c notepad.res -o %ARCH%/notepad.exe -mwindows -s
%PREFIX%gcc -Wall -O2 -std=c99 notepad.c notepad.res -o %ARCH%/wait_for_exit/notepad.exe -mwindows -s -DWAIT_FOR_EXIT=1
