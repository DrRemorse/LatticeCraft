@echo off
bmp32 merge tone24.bmp tone8.bmp
del tonemap.bmp
del .\default\tonemap.bmp
ren out32.bmp tonemap.bmp
copy tonemap.bmp .\default
