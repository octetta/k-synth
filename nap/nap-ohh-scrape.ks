/ NAP open hat — scraping metallic, unpleasant
N: 22050
T: !N
R: r T
E: e(T*(0-2%N))
/ rough bandpass
H: R-(0.15 f R)
B: 0.75 f H
/ add a ring to make it more obnoxious
A: 7700*(6.28318%44100)
P: +\(N#A)
G: s P
W: w d((E*B*0.7+E*G*0.15)*2.0)
