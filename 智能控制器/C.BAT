echo off
C:\c-51\bin\c-51 %1.c -mt -o %1.r03 -l %1.lst
c:\c-51\bin\xlink %1.r03 c:\c-51\lib\cl8051t.r03 -f LNK.xcl -l %1.m51 -c8051 -xsme -o %1.hex
oh %1.r03 to %1.bin
