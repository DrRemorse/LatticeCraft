@echo off
bmp32 merge bigtone24.bmp bigtone8.bmp
del bigtone.bmp
del .\default\bigtone.bmp
ren out32.bmp bigtone.bmp
copy bigtone.bmp .\default
