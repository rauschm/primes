/*------------------------------------------------------------------------------
  P R I M E S . C

  Ausgabe aller Primzahlen zwischen n_start und n (< 2^64)

  Wenn nur ein Argument (n) angegeben wird, dann werden alle Primzahlen zwischen
  1 und n ausgegeben.

  Aufruf: primes [Von-Zahl (> 0)] Bis-Zahl (> 0)

  Compile: cc -O2 -o primes primes.c -lm
     oder: cl /nologo /O2 /Fe: primes.exe primes.c
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------------
  Datentypen
------------------------------------------------------------------------------*/
typedef unsigned long long int uint64;
typedef unsigned int           uint32;

typedef struct {
  uint64 n_start;
  uint64 n;
} Parameters;

/*------------------------------------------------------------------------------
  Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
void print_primes(uint64 n);
void print_prime(uint64 prime_number);
uint32 calc_square_roots(uint64 n, uint32* sqrts);
uint32* build_primes(uint32 prime_factors_count_estimated);
char* build_sieve(uint32 sqrt_n);
uint32 calc_prime_factors(uint32 sqrts_top, uint32* sqrts, uint32* primes, char* sieve);
void calc_remaining_primes(uint64 n, uint32 sqrt_n, uint32 primes_top, uint32* primes, char* sieve);
uint64 atoul(const char* str);
uint32 integer_square_root(uint64 x);
uint32 estimate_number_of_primes_up_to(uint32 x);

/*------------------------------------------------------------------------------
  globale Variablen
------------------------------------------------------------------------------*/
uint64 n_start;

/*------------------------------------------------------------------------------
  Macros
------------------------------------------------------------------------------*/
#define odd(n) ((n - 1) | 1)

/*------------------------------------------------------------------------------
  Beginn der Verarbeitung
------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
  Parameters p = get_parameters(argc, argv);
  n_start = p.n_start;
  print_primes(p.n);
  return 0;
}

/*------------------------------------------------------------------------------
  n_start und n aus den Kommandozeilen-Parametern ermitteln
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv) {
  Parameters p;

  if (   argc != 2 && argc != 3
      || argc == 2 && (   (p.n_start = 1, p.n = atoul(argv[1])) < 1)
      || argc == 3 && (   (p.n_start =          atoul(argv[1])) < 1
                       || (               p.n = atoul(argv[2])) < 1)) {
    fprintf(stderr, "usage: primes [From-Number (in (0,2^64))] To-Number (in (0..2^64))\n");
    exit(1);
  }
  return p;
}

/*------------------------------------------------------------------------------
  Gibt alle Primzahlen <= n aus.
------------------------------------------------------------------------------*/
void print_primes(uint64 n) {
  if (n < 2) {
    return;
  }
  print_prime(2);

  if (n < 3) {
    return;
  }

  uint32 sqrts[5];
  uint32 sqrts_top = calc_square_roots(n, sqrts);
  uint32 sqrt_n = sqrts[0];

  uint32 prime_factors_count_estimated = estimate_number_of_primes_up_to(sqrt_n);

  uint32* primes = build_primes(prime_factors_count_estimated);
  char* sieve = build_sieve(sqrt_n);

  uint32 primes_top = calc_prime_factors(sqrts_top, sqrts, primes, sieve);
  calc_remaining_primes(n, sqrt_n, primes_top, primes, sieve); 
}

/*------------------------------------------------------------------------------
  Gibt eine Primzahl und deren Nummer aus.

  Zahlen die kleiner als n_start sind, werden nicht ausgegeben.
------------------------------------------------------------------------------*/
void print_prime(uint64 prime_number) {
  static uint64 primes_count = 0;
  primes_count += 1;
  if (prime_number >= n_start) {
    printf("%llu. prime = %llu\n", primes_count, prime_number);
  }
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Quadratwurzeln von n, solange bis der Wert 3 erreicht.
  Statt 1 (== sqrt(3..8)) wird 3 verwendet. 
  Zurückgegeben wird der Index der kleinsten (= Anzahl - 1).
------------------------------------------------------------------------------*/
uint32 calc_square_roots(uint64 n, uint32* sqrts) {
  uint32 top = -1;
  do {
    sqrts[++top] = n = odd(integer_square_root(n));
  } while (n > 3);
  if (sqrts[top] == 1) {
    sqrts[top] = 3;
  }
  return top;
}

/*------------------------------------------------------------------------------
  Baut ein Array für die Primfaktoren auf, das ausreichend groß ist.
------------------------------------------------------------------------------*/
uint32* build_primes(uint32 prime_factors_count_estimated) {
  uint32* primes;
  size_t primes_size = sizeof(primes[0]) * prime_factors_count_estimated;
  if ((primes = malloc(primes_size)) == NULL) {
    perror("memory error");
    exit(2);
  }
  return primes;
}

/*------------------------------------------------------------------------------
  Baut ein Sieb auf, das ausreichend groß und mit Nullen initialisiert ist.

  Speicher wird in einer Größenordnung der Wurzel von n benötigt.
------------------------------------------------------------------------------*/
char* build_sieve(uint32 sqrt_n) {
  char* sieve;
  size_t sieve_size = sizeof(sieve[0]) * sqrt_n;  
  if ((sieve = malloc(sieve_size)) == NULL) {
    perror("memory error");
    exit(3);
  }
  memset(sieve, 0, sieve_size);
  return sieve;
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Primzahlen <= sqrt(n).
  Die Primzahlen werden auch ausgegeben.
  Zurückgegeben wird der Index der größten Primzahl (= Anzahl - 2).
------------------------------------------------------------------------------*/
uint32 calc_prime_factors(uint32 sqrts_top, uint32* sqrts, uint32* primes, char* sieve) {
  uint32 primes_top = 0;
  primes[0] = 3;
  print_prime(3);

  while (sqrts_top > 0) {
    sqrts_top -= 1;

    /* Nicht-Primzahlen markieren */
    for (uint32 i = 0; i <= primes_top; i++) {
      for (uint32 j = (sqrts[sqrts_top + 1] - 3) / 2 + primes[i]
                    - (sqrts[sqrts_top + 1] - primes[i]) / 2 % primes[i];
           j <= (sqrts[sqrts_top] - 3) / 2;
           j += primes[i]) {
        sieve[j] = 1;
      }
    }

    /* Primzahlen notieren und ausgeben */
    for (uint32 j = (sqrts[sqrts_top + 1] - 1) / 2; j <= (sqrts[sqrts_top] - 3) / 2; j++) {
      if (sieve[j] == 0) {
        primes[++primes_top] = j * 2 + 3;
        print_prime(primes[primes_top]);
      } else {
        sieve[j] = 0;
      }
    }
  }

  return primes_top;
}

/*------------------------------------------------------------------------------
  Berechnet alle ungeraden Primzahlen > sqrt(n) und <= n.
  Die Primzahlen werden auch ausgegeben.
------------------------------------------------------------------------------*/
void calc_remaining_primes(uint64 n, uint32 sqrt_n, uint32 primes_top, uint32* primes, char* sieve) { 
  for (uint64 z = 2ULL + sqrt_n; ; z += 2ULL * sqrt_n) {

    /* Nicht-Primzahlen markieren */
    for (uint32 i = 0; i <= primes_top; i++) {
      uint32 j = primes[i] - 1 - (z - 2 - primes[i]) % (primes[i] * 2) / 2;
      while (1) {
        sieve[j] = 1;
        if (j >= sqrt_n - primes[i]) {
          break;
        }
        j += primes[i];
      }
    }
    
    /* Primzahlen notieren und ausgeben */
    for (uint32 j = 0; j < sqrt_n; j++) {
      if (sieve[j] == 0) {
        if (z > n - 2ULL * j) {
          return;
        }
        print_prime(z + 2ULL * j);
      } else {
        sieve[j] = 0;
      }
    }
  }
}

/*==============================================================================
  allgemeine Funktionen
==============================================================================*/

/*------------------------------------------------------------------------------
  convert a string to an unsigned long integer
------------------------------------------------------------------------------*/
uint64 atoul(const char* str) {
  uint64 ull = 0;

  while (*str != 0) {
    if ((*str < '0' || *str > '9')
      || ull > 1844674407370955161ULL
      || (ull *= 10) > 18446744073709551615ULL - (*str - '0')) {
      return 0;
    }
    ull += (*str++ - '0');
  }
  return ull;
}

/*------------------------------------------------------------------------------
  Berechnet ISQRT = die ganzzahlige 32-Bit Qudratwurzel einer 64-Bit-Zahl.
  Es gilt: ISQRT^2 <= x.
------------------------------------------------------------------------------*/
uint32 integer_square_root(uint64 x) {
  uint32 y = (uint32) sqrt((double) x);
  return ((uint64) y * (uint64) y <= x) ? y : (y - 1);
}

/*------------------------------------------------------------------------------
  Berechnet eine Abschätzung EPRIM für die Anzahl der Primzahlen <= x.
  Es gilt: EPRIM >= pi(x)
------------------------------------------------------------------------------*/
uint32 estimate_number_of_primes_up_to(uint32 x) {
  return (uint32) (158 + (double) x
                         / (log(x) - 1.052400915 - (log(4294967295U) - log(x))
                                                 * 0.08149));
//return (uint32) (158 + (double) x / (log(x) * 1.08149 - 2.859906955));
}
