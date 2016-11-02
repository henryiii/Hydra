from __future__ import print_function, division
from plumbum import local, cli, TEE
import re
import pandas as pd
import numpy as np

TIMES = re.compile(r"Time = ([-+]?\d*\.\d+|\d+) ms")

class TimeIt(cli.Application):

    def main(self):
        PhSp = local['../build/PhSp']
        for j in range(1): #(1,24):
            times = np.empty([10,2])
            for i in range(10):
                _, output, _ = PhSp & TEE
                times[i] = map(float, TIMES.findall(output))
            generate = times[:,0].mean()
            gstd = times[:,0].std()
            copy = times[:,1].mean()
            cstd = times[:,1].std()

            print("Generate:", generate, gstd)
            print("Copy:", copy,  cstd)

if __name__ == "__main__":
    TimeIt()
