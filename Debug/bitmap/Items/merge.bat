@echo off
bmp32 merge items24.bmp items8.bmp
del items32.bmp
del ..\default\items32.bmp
ren out32.bmp items32.bmp
copy items32.bmp ..\default
