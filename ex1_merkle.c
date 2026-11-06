/* EXERCICE 1 - Arbre de Merkle */
#include <stdio.h>
#include "crypto_utils.h"

int main(void) {
    const char *transactions[] = {
        "Alice -> Bob : 10",
        "Bob -> Charlie : 3",
        "Alice -> Charlie : 2"
    };
    char root[HASH_HEX];
    printf("EXERCICE 1 - ARBRE DE MERKLE\n");
    for (int i = 0; i < 3; ++i) {
        char leaf[HASH_HEX];
        sha256_hex(transactions[i], leaf);
        printf("Feuille %d : %s\n  SHA-256 = %s\n", i + 1, transactions[i], leaf);
    }
    if (!merkle_root(transactions, 3, root)) return 1;
    printf("Racine de Merkle (la derniere feuille est dupliquee) :\n%s\n", root);
    return 0;
}
