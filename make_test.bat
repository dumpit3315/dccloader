@echo off
clang -DDCC_TESTING test.c dcc/dn_dcc_proto.c minilzo/minilzo.c -o dcc_test.exe