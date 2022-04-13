#!/usr/bin/python3
#-*- coding:utf-8 -*-

import datetime
import itertools
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Dict, List

import numpy as np
from progress.bar import Bar

N_SAMPLES = 4
N_ITERS = 3

DIR = Path(os.path.realpath(__file__)).parent.parent.absolute()
CMD = DIR.as_posix() + '/opt_scripts/opt.sh'
T_EXEC = 1.65    # Machine dependant (in seconds)
N_COEFS = 3

def f(m:float, n:float, p:int) -> float:
    cmd = [CMD, '-m', str(m), '-n', str(n), '-p', str(p)]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, error = proc.communicate()

    if error:
        print('[ERROR]: {}'.format(error.decode().strip()))
        exit(1)

    return float(out.decode().strip().split('\n')[0])


def print_results(result: Dict[str, float]) -> None:
    for k, v in result.items():
        print('{}: {}'.format(k, v))


def calc(m: List[float], n: List[float], p:List[int]):
    # Vars for finding max
    indices = [0, 0, 0]
    val_max = 0

    alphas_combinations = itertools.product(m, n, p)
    bar = Bar('Progress', fill='#', suffix='%(percent).2f%% [%(elapsed)d / %(eta)d]s',
                max=N_SAMPLES ** N_COEFS)

    for coefs in alphas_combinations:
        result = None
        while result is None:
            result = f(coefs[0], coefs[1], coefs[2])

        if result > val_max:
            val_max = result
            indices[0] = m.index(coefs[0])
            indices[1] = n.index(coefs[1])
            indices[2] = p.index(coefs[2])
            print('\t Actual Max value: {:.4f} % -> m: {:.4f}, n: {:.4f}, p: {:.4f}'.format(
                    val_max, coefs[0], coefs[1], coefs[2]))


        bar.next()
    bar.finish()

    # Show maximum
    alphas_range = ((m[max(indices[0] - 1, 0)], m[min(indices[0] + 1, N_SAMPLES - 1)]),
                    (n[max(indices[1] - 1, 0)], n[min(indices[1] + 1, N_SAMPLES - 1)]),
                    (p[max(indices[2] - 1, 0)],  p[min(indices[2] + 1, N_SAMPLES - 1)]))

    print('Max value: {:.4f} % -> m: {:.4f}, n: {:.4f}, p: {:.4f}'.format(val_max,
            m[indices[0]],
            n[indices[1]],
            p[indices[2]]))

    print('    m Range: {:.4f} - {:.4f}'.format(alphas_range[0][0], alphas_range[0][1]))
    print('    n Range: {:.4f} - {:.4f}'.format(alphas_range[1][0], alphas_range[1][1]))
    print('    p Range: {:.4f} - {:.4f}'.format(alphas_range[2][0], alphas_range[2][1]))

    return alphas_range


if __name__ == '__main__':
    start_time = time.time()
    # segurament hi haura una manera mes eficient que anar fent una variable
    # per cada alfa
    m = list(np.linspace(0.22, 0.52, num=N_SAMPLES))
    n = list(np.linspace(0.48, 0.77, num=N_SAMPLES))
    p = list(np.linspace(-51, -34, num=N_SAMPLES))

    nIters = N_ITERS
    if len(sys.argv) > 1:
        nIters = int(sys.argv[1])

    if nIters > 0:
        print('Time estimation: {}'.format(str(datetime.timedelta(seconds=T_EXEC * nIters * N_SAMPLES**N_COEFS))))
    else:
        print('Running until the heat death of the universe ...')

    i = 0
    while True:
        if i == nIters:
            break

        if nIters > 0:
            print('Iteration {}/{}'.format(i+1, nIters))
        else:
            print('Iteration {}'.format(i+1))
        
        i += 1

        alphas_range = calc(m, n, p)
        
        m = list(np.linspace((alphas_range[0][0]), (alphas_range[0][1]), num=N_SAMPLES))
        n = list(np.linspace((alphas_range[1][0]), (alphas_range[1][1]), num=N_SAMPLES))
        p = list(np.linspace((alphas_range[2][0]), (alphas_range[2][1]), num=N_SAMPLES))

    print('Execution time: {}'.format(str(datetime.timedelta(seconds=time.time() - start_time))))
