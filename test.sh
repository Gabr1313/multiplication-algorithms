#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

./karatsuba <in.gen.txt >out.karatsuba.txt 2> time.karatsuba.txt &
pid_karatsuba=$!

./naif      <in.gen.txt >out.naif.txt      2> time.naif.txt      &
pid_naif=$!

wait $pid_karatsuba
echo "Time karatsuba: $(cat time.karatsuba.txt)"

wait $pid_naif
echo "Time naif:      $(cat time.naif.txt     )"

diff -q out.naif.txt out.karatsuba.txt
