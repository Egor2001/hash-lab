#! /usr/bin/python

import sys
import matplotlib.pyplot as plt
import pandas as pd

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('usage: ', sys.argv[0], ' [ DATA_CSV,... ]')
        sys.exit()

    names = sys.argv[1:]
    benchs = [pd.read_csv(name) for name in names]

    fig, ax = plt.subplots()
    ax.set_xscale('log')

    ax.set_title('Производительность хеш-таблиц')
    ax.set_xlabel('Размер входных данный, байт x 8')
    ax.set_ylabel('Амортизированное время работы операции, секунды x 1e-6')

    for name, bench in zip(names, benchs):
        ax.scatter(bench.Size, bench.Time, \
                label=name.split('/')[-1].split('.')[0], marker='v')

    # ax.legend(loc='lower right')
    ax.legend(loc='best')

    plt.savefig('plot.png')

