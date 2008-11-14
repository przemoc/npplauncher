@set PATH=C:\MinGW\bin;%PATH%
windres -i notepad.rc --input-format=rc -o notepad.res -O coff
mingw32-gcc -Wall -O2 notepad.cpp notepad.res -o notepad.exe -mwindows -s -DWAIT_FOR_EXIT=1