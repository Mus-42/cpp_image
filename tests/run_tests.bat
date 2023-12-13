@echo off

call "bin/bmp2bmp" -i tests/images/i0.bmp -o tests/out/i0_1bpp.bmp -bpp 1
call "bin/bmp2bmp" -i tests/images/i0.bmp -o tests/out/i0_4bpp.bmp -bpp 4
call "bin/bmp2bmp" -i tests/images/i0.bmp -o tests/out/i0_8bpp.bmp -bpp 8
call "bin/bmp2bmp" -i tests/images/i0.bmp -o tests/out/i0_16bpp.bmp -bpp 16
call "bin/bmp2bmp" -i tests/images/i0.bmp -o tests/out/i0_32bpp.bmp -bpp 32

call "bin/bmp_info" -i tests/out/i0_1bpp.bmp
call "bin/bmp_info" -i tests/out/i0_4bpp.bmp
call "bin/bmp_info" -i tests/out/i0_8bpp.bmp
call "bin/bmp_info" -i tests/out/i0_16bpp.bmp
call "bin/bmp_info" -i tests/out/i0_32bpp.bmp

call "bin/gen_example_bmps"

call "bin/bmp_info" -i tests/out/e0_1bpp.bmp
call "bin/bmp_info" -i tests/out/e0_4bpp.bmp
call "bin/bmp_info" -i tests/out/e0_8bpp.bmp
call "bin/bmp_info" -i tests/out/e0_16bpp.bmp
call "bin/bmp_info" -i tests/out/e0_32bpp.bmp