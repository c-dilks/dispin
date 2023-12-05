# Pythia Parameters

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

## Vector Meson Decay Fractions

Need to tune some Pythia parameters since the fraction of rho mesons appears to be too small, relative to its BG. Some Pythia 6 parameters we use in `clasdis`:
- fraction $\rho/\pi$, and fraction $K^\*/K$: <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L257-L258>
- $p_T$: <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L266>
- $k_T$ (a LEPTO parameter): <https://github.com/JeffersonLab/clasdis/blob/6f2a40feac88d6726a37b75c05951ab808a74773/clasDIS.F#L244>

_cf._ those from `claspyth`:
- fraction $\rho/\pi$, and fraction $K^\*/K$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L138-L139>
- $p_T$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L140>
- $k_T$: <https://github.com/JeffersonLab/claspyth/blob/1d6bd7fa4e9a187d2fcf158651638a1aecf3a761/input.10.6gev.with-comments#L117>

We'll need to translate these Pythia 6 parameters to Pythia 8, since StringSpinner uses Pythia 8.
