#!/bin/bash

HOME_DIR="$(dirname $(readlink -f $0))"
echo "Working in ${HOME_DIR}"
cd ${HOME_DIR}

FILE_NAME=codememory

rm ${FILE_NAME}.coe
rm ${FILE_NAME}.mot
rm ${FILE_NAME}.lst ../Simulate/codememory.lst
rm ${FILE_NAME}.mif ../Simulate/codememory.mif
rm ${FILE_NAME}.lst ../Synthesis/codememory.lst
rm ${FILE_NAME}.mif ../Synthesis/codememory.mif

