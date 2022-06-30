/*------------------------------------------------------------------------------
  P R I M E S - A L T E R N A T I V E - 2 . C

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

/*------------------------------------------------------------------------------
  Datentypen
------------------------------------------------------------------------------*/
typedef unsigned long long int uint64;
typedef unsigned int           uint32;
typedef struct { uint64 n_start; uint64 n; } Parameters;
typedef uint32 uint_f;
typedef struct { uint64 multiple; uint_f factor; } Factor;
typedef Factor* Sieve;

/*------------------------------------------------------------------------------
  Prototypen
------------------------------------------------------------------------------*/
Parameters get_parameters(int argc, char** argv);
uint64 atoul(const char* s);
void print_primes_up_to(uint64 n);
void print_prime(uint64 prime_number);
uint32 integer_square_root(uint64 x);
uint32 estimate_number_of_primes_up_to(uint32 x);
Sieve create_sieve(uint32 odd_prime_factors_count);
void fill_sieve_and_print_primes(Sieve sieve, uint_f sqrt_n);
void sieve_and_print_other_primes(Sieve sieve, uint_f sqrt_n, uint64 n);
void add_to_sieve(Sieve sieve, uint_f factor);
void sort_sieve(Sieve sieve);
void split_sieve(Sieve sieve, Sieve sieve_head, Sieve* sieve_head_end, Sieve* sieve_tail);
void merge_sieve(Sieve sieve_head, Sieve sieve_head_end, Sieve sieve_tail, Sieve sieve);

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
  print_primes_up_to(p.n);
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
  convert a string to an unsigned long integer
------------------------------------------------------------------------------*/
uint64 atoul(const char* str) {
  uint64 ull = 0;

  while (*str != 0) {
    if (   (*str < '0' || *str > '9')
        || ull > 1844674407370955161ULL
        || (ull *= 10) > 18446744073709551615ULL - (*str - '0')) {
      return 0;
    }
    ull += (*str++ - '0');
  }
  return ull;
}

/*------------------------------------------------------------------------------
  Alle Primzahlen <= n ausgeben
------------------------------------------------------------------------------*/
void print_primes_up_to(uint64 n) {
  if (n < 2) {
    return;
  }
  print_prime(2);

  if (n < 3) {
    return;
  }

  uint32 sqrt_n = odd(integer_square_root(n));
  uint32 prime_factors_count_estimated = estimate_number_of_primes_up_to(sqrt_n);
  Sieve sieve = create_sieve(prime_factors_count_estimated);
  fill_sieve_and_print_primes(sieve, sqrt_n);
  sieve_and_print_other_primes(sieve, sqrt_n, n);
}

/*------------------------------------------------------------------------------
  Gibt die Primzahl und deren Nummer aus.

  Primzahlen die kleiner als n_start sind, werden nicht ausgegeben.
------------------------------------------------------------------------------*/
void print_prime(uint64 prime_number) {
  static uint64 primes_count = 0;
  primes_count += 1;
  if (prime_number >= n_start) {
    printf("%llu. prime = %llu\n", primes_count, prime_number);
  }
}

/*------------------------------------------------------------------------------
  Berechnet ISQRT = die ganzzahlige 32-Bit Qudratwurzel einer 64-Bit-Zahl.
  Es gilt: ISQRT^2 <= x.
------------------------------------------------------------------------------*/
uint32 integer_square_root(uint64 x) {
  uint32 y = (uint32) sqrt((double) x);
  return (uint64) y * (uint64) y <= x ? y : y - 1;
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

/*------------------------------------------------------------------------------
  Das Sieb enthält für jeden ungeraden Primfaktor einen Eintrag (plus 1 mehr).
------------------------------------------------------------------------------*/
Sieve create_sieve(uint32 odd_prime_factors_count) {
  Sieve sieve = malloc(odd_prime_factors_count * sizeof(Factor));
  if (sieve == NULL) {
    perror("memory error");
    exit(2);
  }
  return sieve;
}

/*------------------------------------------------------------------------------
  Füllt das Sieb der Reihe nach mit allen ungeraden Primzahlen <= Wurzel aus n
  und gibt sie aus.

  Das Sieb ist eine Liste aller ungeraden Primzahlen <= Wurzel aus n, und deren
  nächstem ungeraden Vielfachen; nach letzteren ist das Sieb sortiert.

  Wenn eine Zahl vorne im Sieb steht, dann ist sie keine Primzahl. Die nächsten
  Vielfachen ihrer Primfaktoren müssen dann neu einsortiert werden.

  Wenn die Zahl nicht vorne im Sieb steht, dann ist sie eine Primzahl. Diese
  wird hinten am Ende des Siebs angefügt.
------------------------------------------------------------------------------*/
void fill_sieve_and_print_primes(Sieve sieve, uint_f sqrt_n) {
  Sieve sieve_end = sieve;

  print_prime(3);
  add_to_sieve(sieve, 3);

  for (uint_f number = 5; number <= sqrt_n; number += 2) {
    if (number == sieve[0].multiple) { // keine Primzahl
      sort_sieve(sieve);
    } else { // Primzahl
      print_prime(number);
      add_to_sieve(sieve, number);
    }
  }
}

/*------------------------------------------------------------------------------
  Gibt der Reihe nach die übrigen Primzahlen <= n aus, indem Zahlen, die
  Vielfache mindestens eines Primfaktors sind, ausgeschlossen werden.
------------------------------------------------------------------------------*/
void sieve_and_print_other_primes(Sieve sieve, uint_f sqrt_n, uint64 n) {
  for (uint64 number = (uint64) sqrt_n + 2; number <= n; number += 2) {
    if (number == sieve[0].multiple) { // keine Primzahl
      sort_sieve(sieve);
    } else { // Primzahl
      print_prime(number);
    }
  }
}

/*------------------------------------------------------------------------------
  Fügt dem Sieb eine neue ungerade Primzahl hinzu, und zwar am Ende, weil die
  neue Zahl (bzw. ihr Vielfaches) die bis dahin gößte ist.
  Das nächste relevante Vielfache ist ihr Quadrat, weil alle ihre kleineren
  Vielfachen bereits Vielfache einer kleineren Primzahl sind.
------------------------------------------------------------------------------*/
void add_to_sieve(Sieve sieve, uint_f factor) {
  static uint32 sieve_len = 0;

  sieve[sieve_len].multiple = (uint64) factor * (uint64) factor;
  sieve[sieve_len++].factor = factor;

  /* die folgende Zuweisung spart beim Mischen eine zusätzliche Ende-Prüfung */
  sieve[sieve_len].multiple = (uint64) -1;
}

/*------------------------------------------------------------------------------
  Sortiert das Sieb neu, nachdem die Primfaktoren mit den gleichen Vielfachen
  am Anfang des Siebs neue Vielfache erhalten haben.

  Neu einsortiert werden müssen nur die Primfaktoren mit einem neuen Vielfachen.
  Diese schon sortierte Liste wird mit den übrigen (sowieso sortierten)
  Primfaktoren nur noch gemischt.

  Das Produkt der ersten 16 ungeraden Primzahlen ist größer als 2^64. Mehr als
  16 gleiche Vielfache kann es also nicht geben.
------------------------------------------------------------------------------*/
void sort_sieve(Sieve sieve) {
  Factor sieve_head[16];
  Sieve  sieve_head_end;
  Sieve  sieve_tail;

  split_sieve(sieve, sieve_head, &sieve_head_end, &sieve_tail);
  merge_sieve(sieve_head, sieve_head_end, sieve_tail, sieve);
}

/*------------------------------------------------------------------------------
  Kopiert die Primfaktoren mit den gleichen Vielfachen vom Anfang des Siebs in
  einen anderen Bereich und vergibt dabei die neuen Vielfachen.

  Diese Liste ist bereits korrekt sortiert, da bei gleichen Vielfachen die
  kleineren Primfaktoren zuerst kommen. Bei der neuen Vielfachenbildung
  erhalten die kleineren Primfaktoren dann auch kleinere neue Vielfache.
------------------------------------------------------------------------------*/
void split_sieve(Sieve sieve, Sieve sieve_head, Sieve* sieve_head_end, Sieve* sieve_tail) {
  Sieve  head = sieve_head;
  uint64 multiple = sieve->multiple;

  do {
    head->multiple = multiple + (uint64) sieve->factor * 2;
    head++->factor = sieve++->factor;
  } while (sieve->multiple == multiple);
  
  *sieve_head_end = head;
  *sieve_tail = sieve;
}

/*------------------------------------------------------------------------------
  Mischt die kopierten Primfaktoren mit den ehemals gleichen und jetzt neu
  vergebenen Vielfachen vom Anfang des Siebs mit dem übrigen Sieb.

  Obwohl das Sieb nach Vielfachen UND (bei Gleichheit) nach Primfaktoren
  sortiert ist, reicht ein Vergleich der Vielfachen, denn die Sortierung nach
  Primfaktoren ist automatisch gegeben.
------------------------------------------------------------------------------*/
void merge_sieve(Sieve sieve_head, Sieve sieve_head_end, Sieve sieve_tail, Sieve sieve) {
  while (sieve_head < sieve_head_end) {
    *sieve++ = (sieve_head->multiple <= sieve_tail->multiple)
             ? *sieve_head++
             : *sieve_tail++;
  }
}
