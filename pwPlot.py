import matplotlib.pyplot as plt
import numpy as np
import sys
import ROOT as root


# OPTIONS ################
includeMeq0 = True
twist = 2
##########################


# open brufit result
infileN = "bruspin.DIS.mh/asym_mcmc_BL0.root"
infile = root.TFile(infileN,"READ")
blStr = infileN.split('/')[-1].split('_')[-1].split('.')[0]


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


# generate grid of invisible subplots
fig,axs = plt.subplots(
  nrows,ncols,
  subplot_kw=dict(visible=False)
)
plt.subplots_adjust(wspace=0,hspace=0)


# loop over L and M
for l,lmap in plotmap.items():
    for m,[r,c] in lmap.items():

        # show subplot, and delete y-axis labels if not edge
        axs[r,c].set_visible(True)
        if r>0 or c>0:
            axs[r,c].sharex(axs[0,0])
            axs[r,c].sharey(axs[0,0])
        if   twist==2: drawY = m==1
        elif twist==3: drawY = l==-m
        if not drawY:
            plt.setp(axs[r,c].get_yticklabels(),visible=False)

        # get asymmetry graph from brufit asym.root
        twStr = "T"+str(twist)
        lStr = "L"+str(l)
        mStr = "M"+("p" if m>=0 else "m")+str(abs(m))
        endStr = "Lv0P0_"+blStr
        asymN = "gr_pwAmp"+twStr+lStr+mStr+endStr
        print(asymN)
        asym = infile.Get(asymN)

        # draw asymmetry graph to subplot
        axs[r,c].errorbar(
            list(asym.GetX()),
            list(asym.GetY()),
            yerr=list(asym.GetEY()),
            fmt='ok',
            ecolor='xkcd:blue',
            elinewidth=3,
            capsize=4
        )


# draw plots
plt.show()

# cleanup
infile.Close()

