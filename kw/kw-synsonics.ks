/ Kraftwerk Synsonics Tom — Mattel Synsonics toy drum machine
/ "Computer World" era: Flür used Synsonics alongside SDS-V
/ The Synsonics had a distinctive toy-ish triangle tone with fast sweep
/ Character: thinner than SDS-V, higher pitched, almost childlike precision
/ Used for accent patterns and counter-rhythms
/ 300ms
N: 13230
T: !N
E: e(T*(0-6.9%N))

/ Synsonics: simple triangle-ish tone, very fast short sweep
/ Higher starting pitch than SDS-V (smaller, cheaper oscillator)
F: 160+240*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
A: 1 0 -.111 0 0.04
S: P $ A

/ Synsonics had almost no separate noise control — purely tonal
/ Just a very small click
V: e(T*(0-800%N))
R: r T
K: V*R

W: w (E*S*.95)+(K*.15)
