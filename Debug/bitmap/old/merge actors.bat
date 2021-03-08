@echo off
bmp32 merge actor_rgb.bmp actor_alpha.bmp
del actors.bmp
ren out32.bmp actors.bmp
