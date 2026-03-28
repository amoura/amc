@echo off

pushd ..\build
cl /wd4047 /wd4312 /nologo /Zi /WX /D_HAS_EXCEPTIONS=0 /GR- /fsanitize=address  /Fe:generate.exe ..\src\generate.c
generate ..\src\lex.h ..\src\ast.h
cl /wd4047 /wd4312 /nologo /Zi /WX /D_HAS_EXCEPTIONS=0 /GR- /fsanitize=address   /Fe:tst.exe ..\src\main.c
popd
