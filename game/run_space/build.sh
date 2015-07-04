#!/bin/bash
make -C ../demo_3 clean
make -j32 -C ../demo_3 
#make -C ../demo_3 
cp ../demo_3/game ./ 
