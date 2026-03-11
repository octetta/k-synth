#!/bin/bash
for i in dw8k-??.ks; do
  if [ -e $i ] ; then
    ./ksynth -g -i -f $i
    b=$(basename $i .ks)
    gnuplot W.gnuplot
    rm W.gnuplot
    [ -e W.png ] && mv W.png $b.png
    [ -e W.i16 ] && mv W.i16 $b.i16
    [ -e W.f32 ] && mv W.f32 $b.f32
  else
    printf "no file %s\n" $i.ks
  fi
done
