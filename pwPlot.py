import matplotlib.pyplot as plt
import numpy as np
import sys
#import ROOT as root


# need to rebuild root with python3 for this to work!
#infile = root.TFile("bruspin.DIS.z.mh/asym_mcmc_BL0.root","READ")
#graph = infile.Get("gr_pwAmpT3L0Mp0Lv0P0_BL0")
#graph.Draw("APE")


xarr = [1,2,3,4]
yarr = [2,4,9,16]
earr = [3,2,1,0.5]

includeMeq0 = True
twist = 3


# determine nrows and ncols, and plotmap
# plotmap maps L->M->[row,col], where row,col is of subplot
plotmap = {l:{} for l in range(3)}
if twist==2:
    nrows,ncols = 2,2
    plotmap[1][1] = [0,0]
    plotmap[2][1] = [1,0]
    plotmap[2][2] = [1,1]
elif twist==3:
    if includeMeq0:
        nrows,ncols = 3,5
        plotmap[0][0]  = [0,2]
        plotmap[1][-1] = [1,1]
        plotmap[1][0]  = [1,2]
        plotmap[1][1]  = [1,3]
        plotmap[2][-2] = [2,0]
        plotmap[2][-1] = [2,1]
        plotmap[2][0]  = [2,2]
        plotmap[2][1]  = [2,3]
        plotmap[2][2]  = [2,4]
    else:
        nrows,ncols = 2,4
        plotmap[1][-1] = [0,1]
        plotmap[1][1]  = [0,2]
        plotmap[2][-2] = [1,0]
        plotmap[2][-1] = [1,1]
        plotmap[2][1]  = [1,2]
        plotmap[2][2]  = [1,3]
else:
    print("ERROR: bad twist",file=sys.stderr)
    exit()
print(plotmap)
exit()


print("---")
for l,lmap in plotmap.items():
    for m,coord in lmap.items():
        print(l,m,coord)
        

#fig,axs = plt.subplots(nrows,ncols)
for r in range(nrows):
    for c in range(ncols):
        if r<nrows-1: axs[r][c].set_xticklabels([])
        if c>0:       axs[r][c].set_yticklabels([])
plt.subplots_adjust(wspace=0,hspace=0)

# [row][col]
axs[0][0].errorbar(xarr, yarr, yerr=earr,
    fmt='ok',
    ecolor='xkcd:blue',
    elinewidth=3,
    capsize=4
)
axs[1][3].errorbar(yarr, xarr, yerr=earr,
    fmt='ok',
    ecolor='r',
    elinewidth=3,
    capsize=4
)

plt.show()


