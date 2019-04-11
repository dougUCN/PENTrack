#!/usr/bin/env python
# Makes plots from multiple neutronend.out files

def main():
    import numpy as np
    import matplotlib.pyplot as plt
    import pandas as pd
    import argparse

    nbins = 200

    parser = argparse.ArgumentParser(description='Plots neutronend.out histograms for multiple runs')
    parser.add_argument("-r", "--runs", type=int, nargs=2, required=True, help="Start and end values of PENTrack runs")
    parser.add_argument("-q", "--query", type=str, help = "User defined query to limit neutrons in histogram", default="all")
    parser.add_argument("-f", "--folder", type=str, help = "Folder location (default ./)", default="./")
    parser.add_argument("-s", "--save", action="store_true", help = "Saves histogram to endPol.png")
    args = parser.parse_args()

    runNum = np.arange(args.runs[0],args.runs[1]+1)
    histTotal = np.zeros(nbins)
    endPolTotal = 0
    endPolCount = 0
    simTotal = 0
    missedRuns = []

    if (args.folder[-1] != "/"):
        folder = args.folder + "/"
    else:
        folder = args.folder

    for i in runNum:
        runName = folder + str(i).zfill(12) + "neutronend.out"
        try:
            df = pd.read_csv(runName, delim_whitespace=True, usecols=[1,10,11,12,18,19,20,21,26,27,28,32,33,34])
        except:
            missedRuns.append(i)
            continue

        simTotal += len(df.index)
        if args.query != "all":
            # Queries may involve any property listed below:
            # ['particle', 'Sxstart', 'Systart', 'Szstart','tend', 'xend', 'yend', 'zend',
            # 'Sxend', 'Syend', 'Szend', 'BxEnd', 'ByEnd', 'BzEnd']
            df = df.query(args.query)

        # print(df)
        # df.hist(column='BxEnd', bins=200)
        # df.hist(column='ByEnd', bins=200)
        # df.hist(column='BzEnd', bins=200)

        endPol = (df['Sxend']*df['BxEnd'] + df['Syend']*df['ByEnd'] + df['Szend']*df['BzEnd'])/np.sqrt(df['BxEnd']**2 + df['ByEnd']**2 + df['BzEnd']**2)
        endPolCount += len(endPol.index)
        endPolTotal += endPol.sum()
        histTemp, binEdges = np.histogram(endPol,range = [-1,1], bins = nbins)
        histTotal += histTemp

    if len(missedRuns) == len(runNum):
        print("Seems like no files were read correctly :(")
        return
        
    print("Error reading run numbers-- ", missedRuns)
    print("Average polarization: ", endPolTotal/endPolCount)
    print("Number of neutrons in histogram: ", int(np.sum(histTotal)))
    print("Number of neutrons simulated: ", simTotal)

    fig, ax = plt.subplots()
    ax.bar(binEdges[1:]-0.005,histTotal,width=0.01)
    ax.set_xlabel('Spin projection of S on B')
    ax.set_xlim([-1.005,1.005])
    ax.set_ylabel('Number of neutrons')
    ax.set_title('Polarization of neutrons at end')

    if (args.save):
        plt.savefig("endPol.png")
    plt.show()


    return


if ( __name__ == '__main__' ):
    main()