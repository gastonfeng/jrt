@echo off
set DOS16M =AUTO
\c-51\bin\c-51 %1 -ms -L -I\c-51\include\ -xDFT2
if NOT EXIST %1.r03 goto end:
\c-51\bin\xlink -c8051 -FAOMF8051 %1 %2 %3 %4 \c-51\lib\cl8051s -f LNK -l %1.map -o %1.hex -x -n
if NOT EXIST %1.hex goto end:
oh %1.hex to %1.bin
:end
