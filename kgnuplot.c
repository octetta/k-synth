#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ksynth.h"

/*
 * k_gnuplot(x, name, path)
 *
 * Writes a gnuplot script to `path` that produces useful graphs of vector `x`.
 * The script writes a PNG to <path>.png when run.
 *
 * For short vectors (N <= 64):   stem plot — individual sample values
 * For wavetable-length vectors:  three-panel plot:
 *   1. Waveform (time domain)
 *   2. Spectrum (magnitude of first 64 harmonics via DFT)
 *   3. Phase wheel (scatter of phase vs amplitude, useful for visualising
 *      asymmetry and DC offset)
 *
 * Usage:
 *   k_gnuplot(myvar, "W", "/tmp/wave.gp");
 *   system("gnuplot /tmp/wave.gp");
 */

/* Simple DFT — magnitude of first `nbins` harmonics */
static void dft_mag(const double *x, int n, double *mag, int nbins) {
    for (int k = 0; k < nbins; k++) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; i++) {
            double angle = 2.0 * 3.14159265358979323846 * k * i / n;
            re += x[i] * cos(angle);
            im += x[i] * sin(angle);
        }
        mag[k] = sqrt(re*re + im*im) / n;
    }
}

void k_gnuplot(K x, const char *name, const char *path) {
    if (!x || x->n < 1 || !path) return;
    if (name == NULL) name = "x";

    FILE *f = fopen(path, "w");
    if (!f) { perror("k_gnuplot: fopen"); return; }

    /* derive output png path: replace/append .png */
    char png[1024];
    const char *dot = strrchr(path, '.');
    if (dot) {
        size_t base = dot - path;
        if (base >= sizeof(png) - 5) base = sizeof(png) - 5;
        strncpy(png, path, base);
        png[base] = '\0';
    } else {
        strncpy(png, path, sizeof(png) - 5);
        png[sizeof(png)-5] = '\0';
    }
    strcat(png, ".png");

    int n = x->n;

    fprintf(f, "set terminal pngcairo size 900,600 enhanced font 'sans,10'\n");
    fprintf(f, "set output '%s'\n", png);
    fprintf(f, "set style data lines\n");
    fprintf(f, "set grid\n");

    if (n <= 64) {
        /* --- Single panel: stem plot --- */
        fprintf(f, "set title '%s  [%d samples]'\n", name, n);
        fprintf(f, "set xlabel 'index'\n");
        fprintf(f, "set ylabel 'value'\n");
        fprintf(f, "set yrange [*:*]\n");
        fprintf(f, "set xrange [-0.5:%d]\n", n);
        /* gnuplot stem: impulses style */
        fprintf(f, "set style data impulses\n");
        fprintf(f, "set style line 1 lc rgb '#0077bb' lw 2\n");
        fprintf(f, "$data << EOD\n");
        for (int i = 0; i < n; i++)
            fprintf(f, "%d %.10g\n", i, x->f[i]);
        fprintf(f, "EOD\n");
        fprintf(f, "plot $data using 1:2 ls 1 title '%s'\n", name);

    } else {
        /* --- Three-panel layout --- */

        /* compute spectrum (first 64 bins, skip DC at 0) */
        int nbins = 64;
        if (nbins > n / 2) nbins = n / 2;
        double *mag = malloc(nbins * sizeof(double));
        if (!mag) { fclose(f); return; }
        dft_mag(x->f, n, mag, nbins);

        fprintf(f, "set multiplot layout 3,1 title '%s  [%d samples]' font 'sans,11'\n",
                name, n);

        /* Panel 1: waveform */
        fprintf(f, "set title 'Waveform'\n");
        fprintf(f, "set xlabel 'sample'\n");
        fprintf(f, "set ylabel 'amplitude'\n");
        fprintf(f, "set xrange [0:%d]\n", n - 1);
        fprintf(f, "set yrange [*:*]\n");
        fprintf(f, "set style data lines\n");
        fprintf(f, "set style line 1 lc rgb '#0077bb' lw 1\n");
        fprintf(f, "$wave << EOD\n");
        /* downsample to 1024 points if very long, to keep script size sane */
        int step = n > 1024 ? n / 1024 : 1;
        for (int i = 0; i < n; i += step)
            fprintf(f, "%d %.10g\n", i, x->f[i]);
        fprintf(f, "EOD\n");
        fprintf(f, "plot $wave using 1:2 ls 1 notitle\n");

        /* Panel 2: spectrum */
        fprintf(f, "set title 'Spectrum (harmonics 1..%d)'\n", nbins - 1);
        fprintf(f, "set xlabel 'harmonic'\n");
        fprintf(f, "set ylabel 'magnitude'\n");
        fprintf(f, "set xrange [0:%d]\n", nbins);
        fprintf(f, "set yrange [0:*]\n");
        fprintf(f, "set style data impulses\n");
        fprintf(f, "set style line 2 lc rgb '#bb3300' lw 2\n");
        fprintf(f, "$spec << EOD\n");
        for (int k = 1; k < nbins; k++)   /* skip DC bin 0 */
            fprintf(f, "%d %.10g\n", k, mag[k]);
        fprintf(f, "EOD\n");
        fprintf(f, "plot $spec using 1:2 ls 2 notitle\n");

        /* Panel 3: phase wheel (x[i] vs x[i + N/4] — Lissajous-style) */
        fprintf(f, "set title 'Phase Portrait (sample vs sample + N/4)'\n");
        fprintf(f, "set xlabel 'x[i]'\n");
        fprintf(f, "set ylabel 'x[i + N/4]'\n");
        fprintf(f, "set xrange [*:*]\n");
        fprintf(f, "set yrange [*:*]\n");
        fprintf(f, "set size ratio 1\n");
        fprintf(f, "set style data dots\n");
        fprintf(f, "set style line 3 lc rgb '#007744' pt 7 ps 0.3\n");
        int quarter = n / 4;
        fprintf(f, "$phase << EOD\n");
        for (int i = 0; i < n; i += step)
            fprintf(f, "%.10g %.10g\n", x->f[i], x->f[(i + quarter) % n]);
        fprintf(f, "EOD\n");
        fprintf(f, "plot $phase using 1:2 ls 3 notitle\n");

        fprintf(f, "unset multiplot\n");
        free(mag);
    }

    fclose(f);
    fprintf(stderr, "k_gnuplot: wrote '%s'  (run: gnuplot %s)\n", path, path);
}
