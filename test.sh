#!/usr/bin/env bash

./gen $1 $1 >in.gen.txt

./fft <in.gen.txt >out.fft.txt 2> time.fft.txt &
pid_fft=$!

./karatsuba <in.gen.txt >out.karatsuba.txt 2> time.karatsuba.txt &
pid_karatsuba=$!

./naif      <in.gen.txt >out.naif.txt      2> time.naif.txt      &
pid_naif=$!

wait $pid_fft
echo "Time fft:       $(cat time.fft.txt)"

wait $pid_karatsuba
echo "Time karatsuba: $(cat time.karatsuba.txt)"

diff -q out.fft.txt  out.karatsuba.txt

wait $pid_naif
echo "Time naif:      $(cat time.naif.txt     )"

diff -q out.naif.txt out.fft.txt
diff -q out.naif.txt out.karatsuba.txt
