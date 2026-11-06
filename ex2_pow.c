/* EXERCICE 2 - Proof of Work et mesure selon la difficulte */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crypto_utils.h"

static int has_zero_prefix(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; ++i) if (hash[i] != '0') return 0;
    return 1;
}

static unsigned long long mine(const char *data, int difficulty, char result[HASH_HEX]) {
    unsigned long long nonce = 0;
    char input[256];
    do {
        snprintf(input, sizeof(input), "%s|%llu", data, nonce);
        sha256_hex(input, result);
        if (has_zero_prefix(result, difficulty)) return nonce;
        ++nonce;
    } while (1);
}

int main(void) {
    const int levels[] = {2, 3, 4};
    printf("EXERCICE 2 - PROOF OF WORK\n");
    for (int i = 0; i < 3; ++i) {
        char hash[HASH_HEX];
        clock_t start = clock();
        unsigned long long nonce = mine("Bloc de demonstration", levels[i], hash);
        double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("Difficulte : %d zero(s) initiaux\n", levels[i]);
        printf("Nonce : %llu\nHash : %s\nTemps CPU : %.6f seconde(s)\n\n",
               nonce, hash, seconds);
    }
    puts("Le temps augmente en general avec la difficulte; il varie selon la machine.");
    return 0;
}
