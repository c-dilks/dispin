#!/usr/bin/env python3
# draw partial wave plots, with shared axes

import matplotlib.pyplot as plt
import sys, getopt
import ROOT as root


# ARGUMENTS ##############
scheme = 2
xTitle = ''
extraTitle = ''
outputEXT = 'png'
translation = 0.0
if len(sys.argv)-1 < 1:
    helpStr = f'''
    USAGE: {sys.argv[0]} [OPTION]... [FILE]...

    OPTIONS

        -s SCHEME
            plot scheme number, where SCHEME can be:
                0: twist3, m=0 only
                2: twist2
                3: twist3 (m=0 included if includeMeq0==True)
                4: DSIDIS terms
                12: twist2, no theta-dependence
                13: twist3, no theta-dependence
                2000+: individual PW
                    digits: twist|L|M|sign(m)
                                      0:+,1:-

        -x X_TITLE
            x-axis title string, which can be Latex syntax

        -e EXTRA_TITLE
            extra title string, to append to default title, which can be Latex syntax

        -o OUTPUT_FORMAT
            string specifying output format:
                png
                pdf
                disabled: open interactive plot, no output

        -t TRANSLATE
            translate stacked plots, offsetting them horizontally, to improve legibility;
            for each nth stacked plot after the first, the offset is +(n-1)*TRANSLATE;
            default = 0 (disable translation)


    FILES
        brufit asym.root file(s), which will be stacked together on the output figure
    
    See hard-coded OPTIONS in {sys.argv[0]} for more settings

    '''
    print(helpStr,file=sys.stderr)
    exit(2)

try: opts,args = getopt.getopt(sys.argv[1:],'s:x:e:o:t:')
except getopt.GetoptError:
    print('\n\nERROR: invalid arguments')
    exit(2)
for opt,arg in opts:
    if(opt=="-s"): scheme = int(arg)
    if(opt=="-x"): xTitle = arg
    if(opt=="-e"): extraTitle = arg
    if(opt=="-o"): outputEXT = arg
    if(opt=="-t"): translation = float(arg)
infiles = args
print(f'''
CALLING {sys.argv[0]}:
    scheme = {scheme}
    xTitle = '{xTitle}'
    extraTitle = '{extraTitle}'
    outputEXT = {outputEXT}
    translation = {translation}
    infiles = {infiles}
''')

# OPTIONS ################
includeMeq0 = False
transparentBG = False
includePrelimLabel = False
asymMax = 0.095 if scheme!=0 else 0.25
asymMin = -asymMax
##########################
if outputEXT!="png": # some features only work for png
    transparentBG = False

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
        "text.latex.preamble": r'\usepackage{amssymb}'
    })


# determine nrows and ncols, and plotmap
# plotmap maps L->M->[row,col], where row,col is of subplot
plotmap = {l:{} for l in range(3)}
enablePW = True
if scheme==0: # twist3 m==0 states only
    nrows,ncols = 1,3
    twist=3
    plotmap[0][0] = [0,0]
    plotmap[1][0] = [0,1]
    plotmap[2][0] = [0,2]
elif scheme==2: # twist2
    nrows,ncols = 2,2
    twist=2
    plotmap[1][1] = [0,0] # [l][m] = [r,c]
    plotmap[2][1] = [1,0]
    plotmap[2][2] = [1,1]
elif scheme==3: # twist3
    twist=3
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
elif scheme==4: # DSIDIS
    nrows,ncols = 1,2
    twist=2
    plotmap[1][0] = [0,0] # (does not use l and m)
    plotmap[1][1] = [0,1]
elif scheme==12: # twist2, no theta-dependence
    nrows,ncols = 1,2
    twist=2
    enablePW = False
    plotmap[1][1] = [0,0]
    plotmap[2][2] = [0,1]
elif scheme==13: # twist3, no theta-dependence
    nrows,ncols = 3,2
    twist=3
    enablePW = False
    plotmap[0][0]  = [0,0]
    plotmap[1][-1] = [1,0]
    plotmap[1][1]  = [1,1]
    plotmap[2][-2] = [2,0]
    plotmap[2][2]  = [2,1]
elif scheme>=2000: # single plot
    twist=int(scheme/1000)
    ell=int(scheme%1000/100)
    emm=int(scheme%100/10)
    if scheme%10==1: emm*=-1
    nrows,ncols = 1,1
    plotmap[ell][emm]  = [0,0]
else:
    print("ERROR: bad scheme number",file=sys.stderr)
    exit(1)

# figure size and aspect ratio
plt.rcParams.update({
    "figure.figsize": [4*ncols,3*nrows],
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
if scheme==2 or scheme==3 or scheme==12 or scheme==13:
    mainTitle = "Twist-"+str(twist)+" $F_{LU}/F_{UU}$ Amplitudes"
elif scheme==0:
    mainTitle = "Twist-"+str(twist)+" $m=0$ $F_{LU}/F_{UU}$ Amplitudes"
elif scheme==4:
    mainTitle = "Twist-2 $F_{LU}/F_{UU}$ DSIDIS Amplitudes"
elif scheme>=2000:
    mainTitle = "Twist-"+str(twist)+" $F_{LU}^{|"+str(ell)+","+str(emm)+"\\rangle}/F_{UU}$"
if extraTitle!='':
    mainTitle += ', '+extraTitle
fig.suptitle(
    mainTitle,
    fontsize = 18 if scheme<2000 else 14
)


# loop over L and M #####################################
for l,lmap in plotmap.items():
    for m,[r,c] in lmap.items():

        # show subplot, and delete y-axis labels if not edge
        axs[r,c].set_visible(True)
        if r>0 or c>0:
            axs[r,c].sharex(axs[0,0])
            axs[r,c].sharey(axs[0,0])
        if scheme==0:
            drawX = True
            drawY = l==0
        elif scheme==2:
            drawX = l==2
            drawY = m==1
        elif scheme==3:
            drawX = l==2
            drawY = l==-m
        elif scheme==4:
            drawX = True
            drawY = m==0
        elif scheme==12:
            drawX = True
            drawY = m==1
        elif scheme==13:
            drawX = l==2
            drawY = m<=0
        elif scheme>=2000:
            drawX,drawY = True,True
        if not drawY:
            plt.setp(axs[r,c].get_yticklabels(),visible=False)
        if not drawX:
            plt.setp(axs[r,c].get_xticklabels(),visible=False)

        # get asymmetry graph from brufit asym.root file(s)
        twStr = "T"+str(twist)
        lStr = "L"+str(l)
        mStr = "M"+("p" if m>=0 else "m")+str(abs(m))

        for infileIdx,infileN in enumerate(infiles):
            infile = root.TFile(infileN,"READ")
            blStr = infileN.split('/')[-1].split('_')[-1].split('.')[0]
            endStr = "Lv0P0_"+blStr
            prefix = "gr_pwAmp" if enablePW else "gr_Amp"
            asymN = prefix+twStr+lStr+mStr+endStr
            if scheme==4:
                if m==0:   asymN = "gr_AmpT2L0Mp0Lv0P4_"+blStr
                elif m==1: asymN = "gr_AmpT2L0Mp0Lv1P4_"+blStr
            print("asymN =",asymN)
            asym = infile.Get(asymN)

            # plot formatting
            if infileIdx==0:
                mkrSty = 'o'
                errCol = 'xkcd:coral'
            elif infileIdx==1:
                mkrSty = 's'
                errCol = 'xkcd:darkish blue'
            elif infileIdx==2:
                mkrSty = '^'
                errCol = 'xkcd:jungle green'
            elif infileIdx==3:
                mkrSty = 'v'
                errCol = 'xkcd:violet'
            elif infileIdx==4:
                mkrSty = 'X'
                errCol = 'xkcd:fluro green'
            else:
                mkrSty = 'o'
                errCol = 'xkcd:black'
                print("WARNING: need to define new colors",file=sys.stderr)
            mkrCol = errCol

            # draw asymmetry graph to subplot
            axs[r,c].errorbar(
                list(map(lambda x:x+infileIdx*translation, asym.GetX())), # optionally offsets (translates) stacked plots
                list(asym.GetY()),
                yerr=list(asym.GetEY()),
                marker=mkrSty,
                color=mkrCol,
                ecolor=errCol,
                linestyle='None',
                elinewidth=2,
                markersize=3,
                capsize=2,
                zorder=10+infileIdx,
            )

            # close asym.root file
            infile.Close()

        # END for infileN in infiles

        # zero line
        axs[r,c].axhline(
            0,0,1,
            color='xkcd:steel',
            ls=':',
            lw=1,
            zorder=5,
        )

        # grid
        axs[r,c].grid(
            True,'major','both',
            color='xkcd:light grey',
            linewidth=0.5,
            zorder=0,
        )

        # axis labels
        if drawX: axs[r,c].set_xlabel(xTitle)
        if scheme==0: yeig = "\\ell,0"
        elif scheme==2 or scheme==3 or scheme==12 or scheme==13: yeig = str(l)+",m"
        elif scheme==4: yeig = "DSIDIS"
        elif scheme>=2000: yeig = str(l)+","+str(m)
        if enablePW:
            ytitle = "$F_{LU}^{|"+yeig+"\\rangle}/F_{UU}$"
        else:
            ytitle = "$F_{LU}/F_{UU}$"
        if drawY:
            if enableOutput:
                axs[r,c].set_ylabel(ytitle)
            else:
                axs[r,c].set_ylabel("$F_{LU}/F_{UU}$")

        # partial wave labels
        # |l,m>
        if enablePW:
            ket = "$|"+str(l)+","+str(m)+"\\rangle$"
        else:
            ket = "$|\\ell,"+str(m)+"\\rangle$"
        if scheme==4:
            if m==0: ket="$|\sin(\Delta\phi)\\rangle$"
            elif m==1: ket="$|\sin(2\Delta\phi)\\rangle$"
        axs[r,c].text(
            0.05,0.9,
            ket,
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
        if enablePW:
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
        else:
            if abs(m)==0:
                diffP = "OO+OL+LL"
            elif abs(m)==1:
                diffP = "OT+LT"
            elif abs(m)==2:
                diffP = "TT"
        diff = "$"+diffFF+"_{1,"+diffP+"}^{"+diffT+"}$"
        if enableOutput and scheme!=4:
            axs[r,c].text(
                0.7,0.9,
                diff,
                verticalalignment='center',
                transform=axs[r,c].transAxes
            )

        # preliminary label
        if enableOutput and includePrelimLabel:
            axs[r,c].text(
                0.1,0.1,
                r'\textbf{\large CLAS12 PRELIMINARY}',
                verticalalignment='center',
                transform=axs[r,c].transAxes
            )

# END loop over L and M #####################################


# axis limits
xlb = list(asym.GetX())[0]
xub = list(asym.GetX())[-1]
xlb -= 0.15*abs(xub-xlb)
xub += 0.15*abs(xub-xlb)
plt.xlim(xlb,xub)
plt.ylim(asymMin,asymMax)


# draw plots, either to file or to viewer
if enableOutput:
    outfileN = infiles[0].replace(
        ".root",
        "_sc"+str(scheme)+"."+outputEXT
    )
    plt.savefig(
        outfileN,
        transparent=transparentBG
    )
    print("--- produced "+outfileN)
else:
    plt.show() # FIXME: causes segfault ?
