# StringSpinner Usage guide

Build:
```bash
buildStringSpinner.sh
make
```

Make or symlink directories:
```
diskim.sss
outroot.sss
```

Generate `outroot` files, then the `catTree` (see settings in the script first):
```bash
loop_stringSpinSim.rb
catTreeMaker.sh outroot.sss data
mv <output cattree file> catTrees/
```
- runs `stringSpinSim.cpp`
- Pythia configuration is in `stringSpinSim.cmnd` (see below for details)

Compare kinematics
```bash
root -b -q CompareCatTreeDists.C'("catTrees/catTreeData.sss.0x34.idx.root","catTrees/catTreeData.mcgen.inbending.bg45.0x34.idx.root")'
```

$M_h$ Decomposition
```bash
diagnostics.exe "outroot.sss/*.root" plots.sss.root
MhDecompose.exe "outroot.sss/*.root" plots.sss.root
```

Fit and plot asymmetries
```bash
finalFit_stringspinner.sh
finalPlot_stringspinner.sh
```


## Pythia Parameters

- `SpaceShower:dipoleRecoil`: for DIS this should be `on`, but Albi has it set to `off`
  - if set to `on`, recommended to also set:
    - `PDF:lepton = off` (we do)
    - `TimeShower:QEDshowerByL = off` (we do)
    - `SpaceShower:pTmaxMatch = 2`: why don't we do this? This will fill the whole phase space with parton showers
- `BeamRemnants:primordialKT` and `BeamRemnants:primordialKTremnant`
  - set to `off` and `0`
  - is this where we set $\langle k_T \rangle = 0.64$ for consistency with `clasdis`?
- hadron production switches
  - $\pi^0$, $K^0$, and $\pi^+$ are all set to `off`; is this correct?
  - no others are set to `on`

### Vector Meson Decay Fractions

Need to tune some Pythia parameters since the fraction of rho mesons appears to be too small, relative to its BG. Some Pythia 6 parameters we use in `clasdis`:
- fraction $\rho/\pi$, and fraction $K^\*/K$: <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L257-L258>
- $p_T$: <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L266>
- $k_T$ (a LEPTO parameter): <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L244>

_cf._ those from `claspyth`:
- fraction $\rho/\pi$, and fraction $K^\*/K$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L138-L139>
- $p_T$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L140>
- $k_T$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L117>

We'll need to translate these Pythia 6 parameters to Pythia 8, since StringSpinner uses Pythia 8:

| Pythia 6                      | Pythia 8                                        | Description                                     | Value |
| ---                           | ---                                             | ---                                             | ---   |
| `PARJ(11)`                    | `StringFlav:mesonUDvector`                      | vector/pseudoscalar for light ($u$, $d$) mesons | 0.70  |
| `PARJ(12)`                    | `StringFlav:mesonSvector`                       | vector/pseudoscalar for strange mesons          | 0.75  |
| `PARJ(21)`                    | `StringPT:sigma`                                | width of $p_T$ distribution (in fragmentation)  | 0.5   |
| `PARP(99)` or LEPTO `PARL(3)` | possibly `BeamRemnants:primordialKTremnant`[^1] | width of $k_T$ distribution (in initial)        | 0.64  |

[^1]: we get **no events** when we turn on `BeamRemnants:primordialKT`, so this can't be used yet
