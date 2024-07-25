#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

time_karatsuba=$( { ./karatsuba <in.gen.txt 2>&1 1>out.karatsuba.txt; } &
                   pid_karatsuba=$! )

time_naif=$(      { ./naif      <in.gen.txt 2>&1 1>out.naif.txt;      } &
                   pid_naif=$!      )

wait $pid_karatsuba
echo "Time karatsuba: " $time_karatsuba
wait $pid_naif
echo "Time naif:      " $time_naif

diff -q out.naif.txt out.karatsuba.txt
