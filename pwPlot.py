import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys
import ROOT as root


# arguments ##############
narg = len(sys.argv)-1
if narg < 3:
    print(
        "USAGE: "+sys.argv[0]+" [brufit asym.root]"+
        " [twist] [Xtitle] [drawPDF(1/0,default=0)]",
        file=sys.stderr)
    exit()
infileN = sys.argv[1]
twist = int(sys.argv[2])
xtitle = sys.argv[3]
drawPDF = int(sys.argv[4]) if narg>=4 else 0
# OPTIONS ################
includeMeq0 = False
enableLatex = True
asymMax = 0.085
asymMin = -asymMax
##########################


# latex
# NOTE: needed to install `dvipng` and `cm-super`
if enableLatex:
    plt.rcParams.update({
        "text.usetex": True,
        "font.size": 14,
        #"font.family": "sans-serif",
        #"font.sans-serif": ["Helvetica"]
        "font.family": "serif",
        "font.sans-serif": ["Palatino"],
        "text.latex.preamble": [r'\usepackage{amssymb}']
    })


# open brufit result
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

# figure size and aspect ratio
plt.rcParams.update({
    "figure.figsize": [3*ncols,3*nrows],
    "savefig.bbox": 'tight'
})


# generate grid of invisible subplots
fig,axs = plt.subplots(
  nrows,ncols,
  subplot_kw=dict(visible=False)
)
plt.subplots_adjust(wspace=0,hspace=0)
fig.suptitle(
  "Twist-"+str(twist)+" $A_{LU}$ Amplitudes",
  fontsize=24
)


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
        drawX = l==2
        if not drawY:
            plt.setp(axs[r,c].get_yticklabels(),visible=False)
        if not drawX:
            plt.setp(axs[r,c].get_xticklabels(),visible=False)

        # get asymmetry graph from brufit asym.root
        twStr = "T"+str(twist)
        lStr = "L"+str(l)
        mStr = "M"+("p" if m>=0 else "m")+str(abs(m))
        endStr = "Lv0P0_"+blStr
        asymN = "gr_pwAmp"+twStr+lStr+mStr+endStr
        asym = infile.Get(asymN)

        # draw asymmetry graph to subplot
        axs[r,c].errorbar(
            list(asym.GetX()),
            list(asym.GetY()),
            yerr=list(asym.GetEY()),
            fmt='ok',
            ecolor='xkcd:ocean',
            elinewidth=3,
            capsize=4
        )

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
        axs[r,c].set_xlabel(xtitle)
        ytitle = "$A_{LU}^{|"+str(l)+",m\\rangle}$"
        if drawY:
            if enableLatex:
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
        elif twist==3:
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
        if enableLatex:
            axs[r,c].text(
                0.7,0.9,
                diff,
                verticalalignment='center',
                transform=axs[r,c].transAxes
            )

        # preliminary label
        if enableLatex:
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
plt.xlim(xlb,xub)
plt.ylim(asymMin,asymMax)


# draw plots
if drawPDF==1:
    plt.savefig(
      "figtest.pdf",
      orientation='portrait'
    )
else:
    plt.show()


# cleanup
infile.Close()

