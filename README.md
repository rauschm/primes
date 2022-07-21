# primes
print all prime numbers < 2<sup>64</sup>

If only one argument (n) is given, **primes** prints all prime numbers up to n.

If there are two arguments (m,n) then **primes** prints all prime numbers between m and n.

<hr>

Actually one could get the primes between m and n much **faster** than by one of these algorithms! You just would have to calculate all primes up to the square root of n and then only filter out the primes between m and n. If the distance between m and n is small enough the complexity of this method would only be in the order of the square root of the others. But it would then not be able to output the **serial numbers** of these prime numbers, which is actually the exciting information.
