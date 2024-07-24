#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

{ time ./karatsuba <in.gen.txt >out.karatsuba.txt; } 2> /tmp/time_karatsuba.txt &
pid_karatsuba=$!

{ time ./naif <in.gen.txt >out.naif.txt; } 2> /tmp/time_naif.txt &
pid_naif=$!

wait $pid_karatsuba
real_time_karatsuba=$(grep real /tmp/time_karatsuba.txt | awk '{print $2}')
echo "Time karatsuba: $real_time_karatsuba"

wait $pid_naif
real_time_naif=$(grep real /tmp/time_naif.txt | awk '{print $2}')
echo "Time naif:      $real_time_naif"

diff -q out.naif.txt out.karatsuba.txt
