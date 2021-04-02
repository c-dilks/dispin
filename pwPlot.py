import matplotlib.pyplot as plt
import numpy as np
import sys
import ROOT as root


# ARGUMENTS ##############
narg = len(sys.argv)-1
if narg < 3:
    print(
        "USAGE: "+sys.argv[0],
        "[brufit asym.root]",
        "[twist]",
        "[Xtitle]",
        "[output(pdf,png;default=disabled)]",
        "[stackPlots(0/1;default=0)]",
        file=sys.stderr)
    print(" - set twist to 0 for twist3 m==0 only")
    exit()
infileName = sys.argv[1]
twist = int(sys.argv[2])
xtitle = sys.argv[3]
outputEXT = sys.argv[4] if narg>=4 else "disabled"
stackPlotsInt = int(sys.argv[5]) if narg>=5 else 0
# OPTIONS ################
includeMeq0 = False
transparentBG = False
asymMax = 0.095 if twist!=0 else 0.25
asymMin = -asymMax
##########################
stackPlots = True if stackPlotsInt==1 else False
if outputEXT!="png": # some features only work for png
    transparentBG = False
    stackPlots = False


# latex
# NOTE: needed to install `dvipng` and `cm-super`
enableOutput = outputEXT!="disabled"
if enableOutput:
    plt.rcParams.update({
        "text.usetex": True,
        "font.size": 14,
        "font.family": "serif",
        "font.serif": ["Times"],
        #"font.family": "sans-serif",
        #"font.sans-serif": ["Computer Modern Sans Serif"],
        "text.latex.preamble": [r'\usepackage{amssymb}']
    })


# open brufit result
if not stackPlots:
    infiles = [infileName]
    blStr = infileName.split('/')[-1].split('_')[-1].split('.')[0]
else:
    infiles = [
        infileName,
        infileName.replace("BL0","BL1"),
        infileName.replace("BL0","BL2")
    ]
    


# determine nrows and ncols, and plotmap
# plotmap maps L->M->[row,col], where row,col is of subplot
plotmap = {l:{} for l in range(3)}
if twist==2:
    nrows,ncols = 2,2
    plotmap[1][1] = [0,0] # [l][m] = [r,c]
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
elif twist==0: # twist3 m==0 states
    nrows,ncols = 1,3
    plotmap[0][0] = [0,0]
    plotmap[1][0] = [0,1]
    plotmap[2][0] = [0,2]
else:
    print("ERROR: bad twist",file=sys.stderr)
    exit()

# figure size and aspect ratio
plt.rcParams.update({
    "figure.figsize": [3*ncols,3*nrows],
    "figure.dpi": 300,
    "savefig.bbox": 'tight'
})


# generate grid of invisible subplots
fig,axs = plt.subplots(
    nrows,ncols,
    subplot_kw=dict(visible=False),
    squeeze=False
)
plt.subplots_adjust(wspace=0,hspace=0)


# main title
if twist==2 or twist==3:
    maintitle = "Twist-"+str(twist)+" $A_{LU}$ Amplitudes"
elif twist==0:
    maintitle = "Twist-3 m=0 $A_{LU}$ Amplitudes"
if not stackPlots:
    if "pt.mh" in infileName or "z.mh" in infileName:
        if blStr=="BL0": extraStr = "$M_h<0.6$ GeV"
        elif blStr=="BL1": extraStr = "$0.6<M_h<0.95$ GeV"
        elif blStr=="BL2": extraStr = "$M_h>0.95$ GeV"
        maintitle += ", "+extraStr
fig.suptitle(maintitle,fontsize=18)




# loop over L and M
for l,lmap in plotmap.items():
    for m,[r,c] in lmap.items():

        # show subplot, and delete y-axis labels if not edge
        axs[r,c].set_visible(True)
        if r>0 or c>0:
            axs[r,c].sharex(axs[0,0])
            axs[r,c].sharey(axs[0,0])
        if twist==2:
            drawX = l==2
            drawY = m==1
        elif twist==3:
            drawX = l==2
            drawY = l==-m
        elif twist==0:
            drawX = True
            drawY = l==0
        if not drawY:
            plt.setp(axs[r,c].get_yticklabels(),visible=False)
        if not drawX:
            plt.setp(axs[r,c].get_xticklabels(),visible=False)

        # get asymmetry graph from brufit asym.root file(s)
        twStr = "T"+str(twist) if twist!=0 else "T3"
        lStr = "L"+str(l)
        mStr = "M"+("p" if m>=0 else "m")+str(abs(m))

        for infileN in infiles:
            infile = root.TFile(infileN,"READ")
            blStr = infileN.split('/')[-1].split('_')[-1].split('.')[0]
            endStr = "Lv0P0_"+blStr
            asymN = "gr_pwAmp"+twStr+lStr+mStr+endStr
            asym = infile.Get(asymN)

            # plot colors
            colorErr = 'xkcd:ocean'
            markerPt = 'o'
            colorPt = 'k'
            if stackPlots:
                if "pt.mh" in infileN or "z.mh" in infileN:
                    if blStr=="BL0":
                        markerPt = 'o'
                        colorErr = 'xkcd:red'
                    elif blStr=="BL1":
                        markerPt = '*'
                        colorErr = 'xkcd:jungle green'
                    elif blStr=="BL2":
                        markerPt = 'X'
                        colorErr = 'xkcd:true blue'
                    colorPt = colorErr

            # draw asymmetry graph to subplot
            axs[r,c].errorbar(
                list(asym.GetX()),
                list(asym.GetY()),
                yerr=list(asym.GetEY()),
                marker=markerPt,
                color=colorPt,
                ecolor=colorErr,
                linestyle='None',
                elinewidth=3,
                capsize=4
            )

            # close asym.root file
            infile.Close()

        # END for infileN in infiles


        # zero line
        axs[r,c].axhline(
            0,0,1,
            color='xkcd:steel',
            ls=':',
            lw=2
        )

        # grid
        axs[r,c].grid(
            True,'major','both',
            color='xkcd:light grey',
            linewidth=0.5
        )

        # axis labels
        if drawX: axs[r,c].set_xlabel(xtitle)
        yeig = str(l)+",m" if twist!=0 else "\\ell,0"
        ytitle = "$A_{LU}^{|"+yeig+"\\rangle}$"
        if drawY:
            if enableOutput:
                axs[r,c].set_ylabel(ytitle)
            else:
                axs[r,c].set_ylabel("$A_{LU}$")

        # partial wave labels
        # |l,m>
        axs[r,c].text(
            0.05,0.9,
            "$|"+str(l)+","+str(m)+"\\rangle$",
            verticalalignment='center',
            transform=axs[r,c].transAxes
        )
        # DiFF
        if twist==2:
            diffFF = "G"
            diffT = "\\perp"
        elif twist==3 or twist==0:
            diffFF = "H"
            diffT = "\\sphericalangle" if m>0 else "\\perp"
        if l==0:
            diffP = "OO"
            diffT += " ss+pp"
        elif l==1:
            if m==0: diffP = "OL"
            elif abs(m)==1: diffP = "OT"
        elif l==2:
            if m==0: diffP = "LL"
            elif abs(m)==1: diffP = "LT"
            elif abs(m)==2: diffP = "TT"
        diff = "$"+diffFF+"_{1,"+diffP+"}^{"+diffT+"}$"
        if enableOutput:
            axs[r,c].text(
                0.7,0.9,
                diff,
                verticalalignment='center',
                transform=axs[r,c].transAxes
            )

        # preliminary label
        if enableOutput:
            if l==2 and m==2:
                axs[r,c].text(
                    0.02,0.1,
                    r'\textbf{\Large CLAS12 PRELIMINARY}',
                    verticalalignment='center',
                    transform=axs[r,c].transAxes
                )

# axis limits
xlb = list(asym.GetX())[0]
xub = list(asym.GetX())[-1]
xlb -= 0.15*abs(xub-xlb)
xub += 0.15*abs(xub-xlb)
#if "z.mh" in infileName:  xlb,xub = 0.35,0.80 # override
#if "pt.mh" in infileName: xlb,xub = 0.10,0.90 #override
plt.xlim(xlb,xub)
plt.ylim(asymMin,asymMax)


# draw plots, either to file or to viewer
if enableOutput:
    outfileN = infileName.replace(
        ".root",
        "_tw"+str(twist)+"."+outputEXT
    )
    plt.savefig(
        outfileN,
        transparent=transparentBG
    )
    print("--- produced "+outfileN)
else:
    plt.show()



