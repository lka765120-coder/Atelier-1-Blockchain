/* EXERCICE 3 - Comparaison pedagogique PoW / PoS */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "crypto_utils.h"

typedef struct { const char *name; unsigned int stake; } Validator;
static const Validator validators[] = {{"Alice",50},{"Bob",30},{"Charlie",20}};
#define VALIDATORS 3

static int choose_validator(void) {
    unsigned int ticket = (unsigned int)rand() % 100;
    if (ticket < validators[0].stake) return 0;
    if (ticket < validators[0].stake + validators[1].stake) return 1;
    return 2;
}
static int valid_pow(const char *hash, int difficulty) {
    for (int i=0;i<difficulty;i++) if (hash[i]!='0') return 0;
    return 1;
}
static double mine_one_pow(int difficulty) {
    char input[128], hash[HASH_HEX];
    unsigned long long nonce=0;
    clock_t start=clock();
    do {
        snprintf(input,sizeof(input),"bloc-comparaison|%llu",nonce++);
        sha256_hex(input,hash);
    } while (!valid_pow(hash,difficulty));
    return (double)(clock()-start)/CLOCKS_PER_SEC;
}
int main(void) {
    srand((unsigned int)time(NULL));
    const int repetitions=20000;
    volatile unsigned long long anti_opt=0;
    clock_t start=clock();
    for (int i=0;i<repetitions;i++) {
        int v=choose_validator();
        char data[128],hash[HASH_HEX];
        snprintf(data,sizeof(data),"bloc-pos|%d|%s",i,validators[v].name);
        sha256_hex(data,hash);
        anti_opt+=(unsigned char)hash[0];
    }
    double pos_total=(double)(clock()-start)/CLOCKS_PER_SEC;
    double pow_time=mine_one_pow(4);
    printf("EXERCICE 3 - COMPARAISON PoW / PoS\n");
    printf("Proof of Work, difficulte 4 : %.6f seconde(s) pour un bloc\n",pow_time);
    printf("Proof of Stake : %.6f seconde(s) pour %d simulations\n",pos_total,repetitions);
    printf("Temps moyen PoS par selection : %.9f seconde(s)\n",pos_total/repetitions);
    printf("Validateur PoS d'exemple : %s\n",validators[choose_validator()].name);
    printf("Conclusion : PoS est plus rapide dans cette simulation; ce resultat depend de l'implementation.\n");
    (void)anti_opt;
    return 0;
}
