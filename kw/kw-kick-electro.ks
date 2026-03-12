/ Kraftwerk Electro Kick — "Numbers", "Planet Rock" sample source
/ This is what Afrika Bambaataa used from Kraftwerk: the synth kick
/ from "Trans-Europe Express" / "Numbers" — very electronic, metronomic
/ The ARP Odyssey "Thwap" described in Keyboard magazine:
/ ultra-fast sine sweep through VCA — the proto-electro kick
/ 280ms
N: 12348
T: !N
E: e(T*(0-6.9%N))

/ Sine (not triangle) for the Odyssey analog: cleaner, more sub
/ Very fast sweep: 180Hz -> 42Hz over just 30ms
F: 42+138*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ ARP Odyssey: harder clip character than SDS-V
/ The Odyssey had no dedicated drum mode — sounds were manually tweaked
O: d(S*2.5)

/ Very short, very loud noise transient — the Odyssey "click"
V: e(T*(0-600%N))
R: r T
A: V*R*.5

W: w (E*O*.9)+(A*.25)
