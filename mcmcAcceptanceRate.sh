#!/bin/bash
# get MCMC acceptance rate

echo ""
echo "MCMC acceptance rates, per PROOF worker:"
grep -i "acceptance rate" $PROOF_LOG/worker-*-*.log |\
sed 's/^.*worker/worker/g'
echo ""

