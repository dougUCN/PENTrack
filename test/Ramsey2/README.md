test/Ramsey2
========================

Makes a ramsey fringe

Parameters for test run of ramsey fringes at LANL

B_0 = 1 uT
B_l = 4 nT
Precession = 180s

gyromagnetic ratio of the neutron
1.832 471 72 x 10^8 rad s^-1 T^-1

w0 = 183.247
wl = 0.732989

Approximate pi/2 pulse time for ramsey resonance: 4.286 seconds

E field of 10 kV/cm applied in parallel to the B field
Neutrons bounce around in a chamber of inner height 0.1m, inner diameter 1m
Walls are 1 cm thick

Run the test with ./RunTest.sh, which applies an on-resonance pi/2 pulse to some neutrons

To make a full blown ramsey fringe on a Torque queuing system, run ramseyFringe.py,
which will auto submit jobs for the parameters specified in code. This assumes ramseyFringe.py,
ramseyConfig.txt, and ramseyPBS.txt are in the /PENTrack folder,
and that the ramseyChamber.stl file is in /PENTrack/in
Run ramseyAnalysis.py (which should be put into /PENTrack/out)
to consolidate all the output files into a ramsey fringe