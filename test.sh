#!/usr/bin/env bash

./gen 100000 100000 >in_gen.txt

{ time ./naif <in_gen.txt >out.naif.txt; } 2> /tmp/time.txt
real_time=$(grep real /tmp/time.txt | awk '{print $2}')
echo "Time naif:      $real_time"

{ time ./karatsuba <in_gen.txt >out.karatsuba.txt; } 2> /tmp/time.txt
real_time=$(grep real /tmp/time.txt | awk '{print $2}')
echo "Time karatsuba: $real_time"

diff -q out.naif.txt out.karatsuba.txt
