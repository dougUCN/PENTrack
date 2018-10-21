#!/usr/bin/env python
# Makes plots from neutronend.out files

def main():
    import numpy as np
    import argparse
    import matplotlib.pyplot as plt

    nbins = 200
    particle = []
    sxStart = []
    syStart = []
    szStart = []
    sxEnd = []
    syEnd = []
    szEnd = []
    bxEnd = []
    byEnd = []
    bzEnd = []

    parser = argparse.ArgumentParser(description='Plots neutronend.out histograms')
    parser.add_argument("-f", "--file", type=str, help = "Filename (default 000000000000neutronend.out)")
    parser.add_argument("-s", "--save", action="store_true", help = "Saves histogram to endPol.png")
    args = parser.parse_args()

    print("Reminder: Use --help or -h to see optional arguments")
    if (args.file):
        filename = args.file
    else:
        filename = '000000000000neutronend.out'

    try:
        with open (filename,"r") as f1:
            lines = f1.readlines()[1:]
            for num, line in enumerate(lines):
                text = line.split()
                if len(text) < 34:
                    break
                particle.append(int( text[1]) )
                sxStart.append(float( text[10]) )
                syStart.append(float( text[11]) )
                szStart.append(float( text[12]) )
                sxEnd.append(float( text[26]) )
                syEnd.append(float( text[27]) )
                szEnd.append(float( text[28]) )
                bxEnd.append(float( text[32]) )
                byEnd.append(float( text[33]) )
                bzEnd.append(float( text[34]) )
    except IOError:
        print("Error reading ", filename)
        return

    # Find projection of S vector (unit vector) onto B vector (not unit vector)
    endProj = []        # Projection of S_end onto B_end for multiple neutrons
    for bxE, byE, bzE, sxE, syE, szE in zip (bxEnd, byEnd, bzEnd, sxEnd, syEnd, szEnd):
        endProj.append( (sxE*bxE + syE*byE + szE*bzE)/norm(bxE, byE, bzE) )

    # Output total neutrons counted and the average
    print("Total neutrons in simulation: ", particle[-1])
    print("Av Sz end: ", np.average(szEnd))
    print("Average polarization: ", np.average(endProj))

    fig, ax = plt.subplots()
    ax.hist(endProj,range = [-1,1], bins = nbins)
    ax.set_xlabel('Spin projection of S on B')
    ax.set_ylabel('Number of neutrons')
    ax.set_title('Polarization of neutrons at end')

    if (args.save):
        plt.savefig("endPol.png")

    plt.show()

    return

def norm (x, y, z):
    #Finds norm of cartesian vector components
    import numpy as np
    return np.sqrt(x*x + y*y + z*z)

if ( __name__ == '__main__' ):
    main()
