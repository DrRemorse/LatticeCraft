@echo off
bmp32 merge diff32.bmp diff32a.bmp
del diffuse.bmp
del .\default\diffuse.bmp
ren out32.bmp diffuse.bmp
copy diffuse.bmp .\default
