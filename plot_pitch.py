#!/usr/bin/env python

import sys
from typing import List

import matplotlib.pyplot as plt
import numpy as np

# Some options
SAVE_IMAGE = False
SEPARATOR_CHAR = '\t'

# Prints explains how to use the script, kinda...
def usage() -> None:
    print('Usage: {} <f0ref> <f0>'.format(sys.argv[0]))


# Parse given arguments
def setup(args: List[str]) -> str:
    if len(args) <= 2 or args[1] == 'help':
        usage()
        return None
    
    return args[1:3]


# Plot data from files
def plotter(filenames: List[str]) -> None:
    # Los ficheros se tienen que leer por separado pq los vectores x y r tienen
    # longitud diferente y los he guardado en dos ficheros distintos

    # Read reference pitch file
    with open(filenames[0], 'r') as file:
        initialized = False
        for l in file.readlines():
            nums = [float(n) for n in l.strip().split(SEPARATOR_CHAR)]

            if not initialized:
                data_f0ref = np.array(nums)
                initialized = True
                continue
            
            data_f0ref = np.vstack((data_f0ref, np.array(nums)))

    # Read pitch estimation file
    with open(filenames[1], 'r') as file:
        initialized = False
        for l in file.readlines():
            nums = [float(n) for n in l.strip().split(SEPARATOR_CHAR)]

            if not initialized:
                data_f0 = np.array(nums)
                initialized = True
                continue
            
            data_f0 = np.vstack((data_f0, np.array(nums)))

    # Time vector
    t = [0.015*i for i in range(data_f0.shape[0])]

    # Plot data
    fig, axs = plt.subplots(1, 1) # Fa mandra mirar com s'ha de fer be :)
    axs.plot(t, data_f0ref[:, 0], 'r.', label='Pitch referencia')
    axs.plot(t, data_f0[:, 0], 'b.', label='Pitch estimado')
    axs.set_xlim((t[0], t[-1]))
    axs.set_xlabel('Tiempo [s]')
    axs.set_ylabel('Pitch [Hz]')
    axs.set_title('Comparación de pitch')
    axs.grid(which='both', color='#777777', linestyle=':', linewidth=0.5)

    
    fig.tight_layout()
    plt.legend()

    # Save plot to PNG file
    if SAVE_IMAGE:
        idx = filenames[0].rfind('.')
        savefile ='{}_pitch.png'.format(filenames[0][:idx])
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
