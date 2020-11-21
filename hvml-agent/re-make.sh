#!/bin/bash
cp -f ../build/parser/src/libhvml_parser.so ./
cp -f ../build/interpreter/src/libhvml_interpreter.so ./

make clean -f Makefile.hvml_agent
make -f Makefile.hvml_agent
