# ----------------------------------------------------------------------
# --- verify that primes is working correctly for at least some known
# --- numbers of 3 primes around 10^n, mostly taken from:
# --- https://primes.utm.edu/lists/small/millions/
# ----------------------------------------------------------------------
for mn in \
"23 31" \
"523 547" \
"7907 7927" \
"104723 104743" \
"1299689 1299721" \
"15485857 15485867" \
"179424671 179424691" \
"2038074739 2038074751" \
"22801763477 22801763513" \
"252097800611 252097800629" \
; do
  ./primes $mn || return
  echo ----------------------------------------
done
