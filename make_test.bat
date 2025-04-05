@echo off
clang -DDCC_TESTING test.c dcc/dn_dcc_proto.c minilzo/minilzo.c lz4/lz4_fs.c -o dcc_test.exe