```
columns:
1:runnum
2:evnum
3:helicity
4:Q2
5:W
6:x
7:y
8:z
9:pT
10:xFpi+
11:xFpi-
12:Mx
13:phih
14:phiR
15:theta
16:A
17:C
18:W
```

files
-----
- `chris_*.txt` and `timothy_*.txt`: tables with the above columns
- `compare_[a]_[b]*.txt`: comparison of `[a]` to `[b]`:
  - loop through `[a]`'s events, find match in `[b]` by matching run number and event number
  - if a duplicate match is found, pick the one with the closest z and pT, adding the comment `prox matched`
  - if a match is not found, comment `not found`
  - cf. `compare_[b]_[a]*.txt`
- `compare*.png`: distribution of differences for each kinematic variable
  - corresponds to results from `compare*.txt`

