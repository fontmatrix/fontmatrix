#!/bin/sh

# Update TS files without PRO file and waiting for WRAP_TS macro in CMake 
ls ../*.h > lupdate.files
ls ../*.cpp >> lupdate.files
ls ../*.ui >> lupdate.files
for i in *.ts; do lupdate `cat lupdate.files ` -ts "$i" ; done
