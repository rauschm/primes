/*------------------------------------------------------------------------------
  P R I M E S - A L T E R N A T I V E - 1 . C

  Ausgabe aller Primzahlen zwischen n_start und n (< 2^64)

  Wenn nur ein Argument (n) angegeben wird, dann werden alle Primzahlen zwischen
  1 und n ausgegeben.

  Aufruf: primes [Von-Zahl (> 0)] Bis-Zahl (> 0)

  Compile: cc -O2 -o primes primes.c -lm
     oder: cl /nologo /O2 /Fe: primes.exe primes.c
------------------------------------------------------------------------------*/
#include <errno.h>
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

typedef struct {
  uint64  width_mask;
  uint32* data;
} Sieve;

/*------------------------------------------------------------------------------
 Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
void print_primes(uint64 n);
void print_prime(uint64 prime_number);
Sieve build_sieve(uint32 sqrt_n);
void sieve_primes(uint64 n, uint32 sqrt_n, uint64 sieve_width_mask, uint32* sieve_data);
uint64 atoul(const char* s);
uint32 integer_square_root(uint64 x);
uint64 round_up_to_next_power_of_2(uint64 x);

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
    fprintf(stderr, "usage: primes2 [From-Number (in (0,2^64))] To-Number (in (0..2^64))\n");
    exit(1);
  }
  return p;
}

/*------------------------------------------------------------------------------
  Gibt alle Primzahlen <= n aus.
------------------------------------------------------------------------------*/
void print_primes(uint64 n) {
  print_prime(2);
  uint32 sqrt_n = odd(integer_square_root(n));
  Sieve sieve = build_sieve(sqrt_n);
  sieve_primes(n, sqrt_n, sieve.width_mask, sieve.data);
}

/*------------------------------------------------------------------------------
  Gibt die Primzahl und deren Nummer aus.

  Primzahlen < n_start werden nicht ausgegeben.
------------------------------------------------------------------------------*/
void print_prime(uint64 prime_number) {
  static uint64 primes_count = 0;
  primes_count += 1;
  if (prime_number >= n_start) {
    printf("%llu. prime = %llu\n", primes_count, prime_number);
  }
}

/*------------------------------------------------------------------------------
  Baut ein Sieb auf, das ausreichend groß und mit Nullen initialisiert ist.

  Speicher wird in einer Größenordnung von 2 mal der Wurzel von n benötigt.
------------------------------------------------------------------------------*/
Sieve build_sieve(uint32 sqrt_n) {
  Sieve sieve;

  uint64 sieve_width = round_up_to_next_power_of_2((uint64) sqrt_n * 2);
  uint64 sieve_size = sieve_width * sizeof(sieve.data[0]);
  sieve.width_mask = sieve_width - 1;

  if (   sieve_size > (size_t) sieve_size && (errno = ENOMEM)
      || (sieve.data = malloc((size_t) sieve_size)) == NULL) {
    perror("memory error");
    exit(2);
  }
  memset(sieve.data, 0, (size_t) sieve_size);
  return sieve;
}

/*------------------------------------------------------------------------------
  Berechnet mit dem Algorithmus des Eratosthenes alle Primzahlen ab 3 bis n und
  gibt sie aus.
------------------------------------------------------------------------------*/
void sieve_primes(uint64 n, uint32 sqrt_n, uint64 sieve_width_mask, uint32* sieve_data) {
  uint64 i = 0;

  for (uint64 number = 3; number <= n; number += 2) {
    i = (i + 1) & sieve_width_mask;
    uint32 factor;
    if (sieve_data[i] == 0) {
      print_prime(number);
      if (number > sqrt_n) {
        continue;
      }
      factor = (uint32) number;
    } else {
      factor = sieve_data[i];
      sieve_data[i] = 0;
    }

    uint64 j = i;
    do {
      j = (j + factor) & sieve_width_mask;
      if (sieve_data[j] == 0) {
        break;
      }
      if (sieve_data[j] < factor) {
        uint32 smaller_factor = sieve_data[j];
        sieve_data[j] = factor;
        factor = smaller_factor;
      }
    } while (1);
    sieve_data[j] = factor;
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
  uint32 y = (uint32)sqrt((double)x);
  return ((uint64)y * (uint64)y <= x) ? y : (y - 1);
}

/*------------------------------------------------------------------------------
  Rundet zur nächsten Potenz von 2 auf.
------------------------------------------------------------------------------*/
uint64 round_up_to_next_power_of_2(uint64 x) {
  x -= 1;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  x += 1;
  return x;
}
