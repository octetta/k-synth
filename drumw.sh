#!/bin/bash
for i in drums-*.ks; do
  ./ksynth -g $i
  gnuplot W.gnuplot
  rm W.gnuplot
  b=$(basename $i .ks)
  mv W.png $b.png
done
