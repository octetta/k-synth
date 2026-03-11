#!/bin/bash
for i in dw8k-??; do
  ./ksynth -g $i.ks
  gnuplot W.gnuplot
  rm W.gnuplot
  mv W.png $i.png
done
