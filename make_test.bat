@echo off
clang -DDCC_TESTING -DHAVE_MINILZO=1 -DHAVE_LZ4=1 -D_CRT_SECURE_NO_WARNINGS=1 test/test_rle_compress.c dcc/dn_dcc_proto.c minilzo/minilzo.c lz4/lz4_fs.c plat/wdog.c -o dcc_test_rle.exe