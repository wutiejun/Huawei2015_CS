#!/bin/bash 
# allin demo_1
make -C ../allin clean
make -j32 -C ../allin 
#make -C ../allin 
cp ../allin/game ./ 
