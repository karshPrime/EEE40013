@echo off
set filename=codememory

asm32 %filename%
convert %filename%.mot
copy %filename%.lst ..\Simulate\CodeMemory.lst
copy %filename%.mif ..\Simulate\CodeMemory.mif
copy %filename%.lst ..\Synthesis\CodeMemory.lst
copy %filename%.mif ..\Synthesis\CodeMemory.mif
pause
