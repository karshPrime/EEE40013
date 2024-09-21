#!/bin/bash

HOME_DIR="$(dirname $(readlink -f $0))"
echo "Working in ${HOME_DIR}"
cd ${HOME_DIR}

FILE_NAME=codememory

./Asm32 ${FILE_NAME}
./Convert ${FILE_NAME}.mot
cp ${FILE_NAME}.lst ../Simulate/codememory.lst
cp ${FILE_NAME}.mif ../Simulate/codememory.mif
cp ${FILE_NAME}.lst ../Synthesis/codememory.lst
cp ${FILE_NAME}.mif ../Synthesis/codememory.mif

