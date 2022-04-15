#!/usr/bin/env python

import sys
from typing import List

import matplotlib.pyplot as plt
import numpy as np

# Some options
SAVE_IMAGE = True
SEPARATOR_CHAR = '\t'

# Prints explains how to use the script, kinda...
def usage() -> None:
    print('Usage: {} <waveform> <autocorrelation>'.format(sys.argv[0]))


# Parse given arguments and set settings (command, filename)
def setup(args: List[str]) -> str:
    if len(args) <= 2 or args[1] == 'help':
        usage()
        return None
    
    return args[1:3]


# Plot data from files
def plotter(filenames: List[str]) -> None:
    # Los ficheros se tienen que leer por separado pq los vectores x y r tienen
    # longitud diferente y los he guardado en dos ficheros distintos

    # Read waveform file
    with open(filenames[0], 'r') as file:
        initialized = False
        for l in file.readlines():
            nums = [float(n) for n in l.strip().split(SEPARATOR_CHAR)]

            if not initialized:
                dataX = np.array(nums)
                initialized = True
                continue
            
            dataX = np.vstack((dataX, np.array(nums)))

    # Read autocorrelation file
    with open(filenames[1], 'r') as file:
        initialized = False
        for l in file.readlines():
            nums = [float(n) for n in l.strip().split(SEPARATOR_CHAR)]

            if not initialized:
                dataR = np.array(nums)
                initialized = True
                continue
            
            dataR = np.vstack((dataR, np.array(nums)))

    # Plot data
    fig, axs = plt.subplots(2, 1)
    axs[0].plot(dataX[:, 0], dataX[:, 1])
    axs[0].set_xlim((dataX[:, 0][0], dataX[:, 0][-1]))
    axs[0].set_xlabel('Tiempo [s]')
    axs[0].set_ylabel('Amplitud')
    axs[0].grid(which='both', color='#777777', linestyle=':', linewidth=0.5)

    axs[1].plot(dataR[:, 0], dataR[:, 1])
    axs[1].set_xlim((dataR[:, 0][0], dataR[:, 0][-1]))
    axs[1].set_xlabel('Coeficiente k')
    axs[1].set_ylabel('Autocorrelación')
    axs[1].grid(which='both', color='#777777', linestyle=':', linewidth=0.5)

    fig.tight_layout()

    # Save plot to PNG file
    if SAVE_IMAGE:
        idx = filenames[0].rfind('.')
        savefile ='{}.png'.format(filenames[0][:idx])
        plt.savefig(savefile, dpi=300)
        print('Plot saved as {}'.format(savefile))


# Main
def main(args: List[str]) -> None:
    if args is None:
        return

    plotter(args)
    plt.show()


if __name__ == '__main__':
    main(setup(sys.argv))
    sys.exit(0)
