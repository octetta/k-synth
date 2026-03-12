/ Kraftwerk Trigger Pulse — "Computer World", "Numbers"
/ The direct trigger output picked up as audio — a "ping" or "zap"
/ Gearspace: "the very first sound on 'The Man Machine' could be a trigger
/ output directly picked up as a sound or an ultra-fast frequency sweep"
/ Character: ultra-short, very fast rising sine burst, then gone
/ Used as a snappy accent or clock sound — the machine pulse itself
/ 60ms
N: 2646
T: !N
E: e(T*(0-6.9%N))

/ Ultra-fast pitch sweep — the trigger "zap" character
/ Starts very high (the initial trigger voltage spike), drops instantly
F: 80+3000*e(T*(0-600%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ Clipped hard — it IS a clipped signal, not a shaped one
O: d(S*4)

W: w E*O
