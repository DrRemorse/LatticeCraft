@echo off
bmp32 merge cloudnorm.bmp cloudalpha.bmp
del clouds.bmp
del .\default\clouds.bmp
ren out32.bmp clouds.bmp
copy clouds.bmp .\default
