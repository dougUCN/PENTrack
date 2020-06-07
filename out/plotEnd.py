#!/usr/bin/env python
# Makes plots from neutronend.out files
import numpy as np
import argparse
import pandas as pd
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import h5py
from scipy.optimize import curve_fit
import uproot

def main():
    parser = argparse.ArgumentParser(description='Plots neutronend.out histograms')
    parser.add_argument('-f', '--file', type=str, default='000000000000neutronend.out', help = 'Text file (default 000000000000neutronend.out)')
    parser.add_argument('-q', '--query', type=str, help = 'User defined query to limit neutrons in histogram')
    parser.add_argument('-hdf', '--hdf', type=str, help='Read an hdf file')
    parser.add_argument('-w', '--where', type=str, help='If [--hdf] is enabled, can query while reading in the file')
    parser.add_argument('-r', '--root', type=str, help = 'Read in a root file')
    parser.add_argument('-p', '--polarization', action='store_true', help = 'Plots end polarization of neutrons')
    parser.add_argument('-e', '--energy', action='store_true', help = 'Plots histogram of E_start and velocity distribution')
    parser.add_argument('-t', '--tend', action='store_true', help = 'Plots histogram of tend')
    parser.add_argument('-s', '--save', action='store_true', help = 'Saves histograms to images')
    parser.add_argument('-xyz', '--xyz', action='store_true', help = 'Plots end position of neutrons')
    parser.add_argument('-xz', '--xz', action='store_true', help='Plots end position of neutrons')
    parser.add_argument('-sID', '--stopID', action='store_true', help='Prints fates of neutrons')
    parser.add_argument('-nbins', '--nbins', type=int, help = 'Number of bins in histogram (default 200)', default=200)
    args = parser.parse_args()

    print('Reminder: Use --help or -h to see optional arguments')

    if args.hdf:
        with h5py.File( args.hdf, "r") as hdfInFile:
            # key=hdfInFile.keys()[0]  # For python2
            key = list( hdfInFile.keys() )[0]
        if args.where == None:
            df = pd.read_hdf(args.hdf, key)
        else:
            df = pd.read_hdf(args.hdf, key, where=[args.where])
    elif args.root:
        file = uproot.open(args.root)
        df = file['neutronend'].pandas.df()
    else:
        df = pd.read_csv(args.file, delim_whitespace=True)

    print('Number of neutrons simulated: ', len(df.index))
    if args.query != None:
        df = df.query(args.query)
        print('Number of neutrons after query: ', len(df.index))

    if args.polarization:
        # Find projection of S vector (unit vector) onto B vector (not unit vector)
        endPol = (df['Sxend']*df['BxEnd'] + df['Syend']*df['ByEnd'] + df['Szend']*df['BzEnd'])/np.sqrt(df['BxEnd']**2 + df['ByEnd']**2 + df['BzEnd']**2)
        hist, binEdges = np.histogram(endPol,range = [-1,1], bins = args.nbins)

        # Output total neutrons counted and the average
        print('Total neutrons in histogram: ', int(np.sum(hist)))
        print('Number of NA endPol values: ', endPol.isna().sum())
        print('Av Sz end: ', df['Szend'].mean())
        print('Av Bz end: ', df['BzEnd'].mean())
        print('Av endPol: ', endPol.mean())


        fig, ax = plt.subplots()
        center = (binEdges[:-1] + binEdges[1:]) / 2
        ax.bar(center,hist,width=(binEdges[1]-binEdges[0]) )
        ax.set_xlabel('Spin projection of S on B')
        ax.set_xlim([-1.005,1.005])
        ax.set_ylabel('Number of neutrons')

        if (args.save):
            plt.savefig('endPol.png')

    if (args.xyz):
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        plt.title('Ending positions')
        ax.scatter(df['xend'], df['yend'], df['zend'])
        ax.set_xlabel('x')
        ax.set_ylabel('y')
        ax.set_zlabel('z')
        # ax.set_xlim3d(-0.15,2.1)
        # ax.set_ylim3d(-.04,.04)
        # ax.set_zlim3d(-.04,.04)

    if args.xz:
        fig = plt.figure()
        plt.plot(df['xend'], df['zend'], '.', linestyle='None', markersize=0.5)
        plt.grid(True)
        plt.title('End position')
        plt.xlabel('x [m]')
        plt.ylabel('z [m]')

        df.hist('zend', bins = args.nbins, grid=False, figsize=(8.5,6))
        plt.ylabel('Number of neutrons')
        plt.xlabel('z_end [m]')

        print('<zend>= ', df['zend'].sum()/len(df.index) )
        print('std dev', df['zend'].std() )

    if (args.energy):
        df.hist('Estart', bins = args.nbins, grid=False, figsize=(8.5,6))
        plt.ylabel('Number of neutrons')
        plt.xlabel('Kinetic Energy [eV]')

        df.hist('Eend', bins = args.nbins, grid=False, figsize=(8.5,6))
        plt.ylabel('Number of neutrons')
        plt.xlabel('Kinetic Energy [eV]')

        df.hist('Hstart', bins = args.nbins, grid=False, figsize=(8.5,6))
        plt.ylabel('Number of neutrons')
        plt.xlabel('Total Energy [eV]')

        df.hist('Hend', bins = args.nbins, grid=False, figsize=(8.5,6))
        plt.ylabel('Number of neutrons')
        plt.xlabel('Total Energy [eV]')

        if (args.save):
            plt.savefig('startEnergy.png')

        # df.hist('vxstart', bins = args.nbins)
        # plt.ylabel('Number of neutrons')
        # plt.xlabel('[m/s]')
        #
        # df.hist('vystart', bins = args.nbins)
        # plt.ylabel('Number of neutrons')
        # plt.xlabel('[m/s]')
        #
        # df.hist('vzstart', bins = args.nbins)
        # plt.ylabel('Number of neutrons')
        # plt.xlabel('[m/s]')

        # vr, vtheta, vphi = toSpherical(df['vxstart'], df['vystart'], df['vzstart'])
        # histTotal, binEdges = np.histogram(vphi, bins = args.nbins)
        # fig, ax = plt.subplots()
        # ax.bar(binEdges[1:]-0.005,histTotal,width=0.02)
        # ax.set_xlabel(r'$v_\phi$ distribution')
        # ax.set_ylabel('Number of neutrons')
        #
        # fig, ax = plt.subplots()
        # histTotal, binEdges = np.histogram(vtheta, bins=args.nbins)
        # centers = (binEdges[:-1] + binEdges[1:])/2
        # ax.bar(centers,histTotal, width=(binEdges[1]-binEdges[0]))
        # ax.set_xlabel(r'$v_\theta$ distribution')
        # ax.set_ylabel('Number of neutrons')

    if (args.tend):
        fig = plt.figure()
        histTotal, binEdges = np.histogram(df['tend'], bins=args.nbins)
        centers = (binEdges[:-1] + binEdges[1:])/2
        plt.bar(centers,histTotal, width=(binEdges[1]-binEdges[0]))
        plt.ylabel('Number of neutrons')
        plt.xlabel('tend [s]')
        plt.title('tend')

        fig = plt.figure()
        neutronsLeft = [ len(df.index) - histTotal[0] ]
        for i, bin in enumerate(histTotal[1:]):
            neutronsLeft.append( neutronsLeft[i] - bin )

        plt.bar(centers,neutronsLeft, width=(binEdges[1]-binEdges[0]))
        plt.ylabel('Number of neutrons')
        plt.xlabel('t [s]')
        plt.title('Neutrons left')

        popt, pcov = curve_fit(func, centers[:-1], neutronsLeft[:-1], p0=[len(df.index), 100, 100])
        print("\n### Neutron lifetime ###")
        print("a * np.exp(-x/b) + c")
        print(f"Fit params:  {popt}")
        print("+/-             [",np.sqrt(pcov[0][0]),", ", np.sqrt(pcov[1][1]),", ", np.sqrt(pcov[2][2]),"]")
        plt.plot(centers, func(centers, *popt), color='C1', linestyle='--', label='Fit')
        plt.legend()

    if (args.stopID):
        print('End status of neutrons [stopID]:')
        print(len(df.query('stopID == 0').index), '\tnot categorized [0]')
        print(len(df.query('stopID == -1').index), '\tdid not finish [-1]')
        print(len(df.query('stopID == -2').index), '\thit outer boundaries [-2]')
        print(len(df.query('stopID == -3').index), '\tproduced error during trajectory integration [-3]')
        print(len(df.query('stopID == -4').index), '\tdecayed [-4]')
        print(len(df.query('stopID == -5').index), '\tfound no initial position [-5]')
        print(len(df.query('stopID == -6').index), '\tproduced error during geometry collision detection [-6]')
        print(len(df.query('stopID == -7').index), '\tproduced error during tracking of crossed material boundaries [-7]')
        print(len(df.query('stopID == 1').index), '\tabsorbed in bulk material (see solidend) [1]')
        print(len(df.query('stopID == 2').index), '\tabsorbed on surface (see solidend) [2]')

    if not args.save:
        plt.show()

    return

def toSpherical(x, y, z):
    r = np.sqrt(x**2 + y**2 + z**2)
    phi = np.arctan2(y, x)
    theta = np.arccos(np.array(z)/r)
    return r, theta, phi

def func(x, a, b, c):
    return a * np.exp(-x/b) + c

if ( __name__ == '__main__' ):
    main()
