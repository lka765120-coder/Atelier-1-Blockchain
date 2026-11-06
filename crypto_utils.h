#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#define HASH_HEX 65
#define MAX_TX 4

static inline void sha256_hex(const char *text, char output[HASH_HEX]) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)text, strlen(text), digest);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        sprintf(output + i * 2, "%02x", digest[i]);
    output[64] = '\0';
}

static inline int merkle_root(const char *transactions[], int count, char root[HASH_HEX]) {
    if (count <= 0 || count > MAX_TX) return 0;
    char level[MAX_TX][HASH_HEX];
    for (int i = 0; i < count; ++i) sha256_hex(transactions[i], level[i]);
    int nodes = count;
    while (nodes > 1) {
        int next_nodes = 0;
        for (int i = 0; i < nodes; i += 2) {
            int right = (i + 1 < nodes) ? i + 1 : i;
            char pair[HASH_HEX * 2];
            memcpy(pair, level[i], 64);
            memcpy(pair + 64, level[right], 64);
            pair[128] = '\0';
            sha256_hex(pair, level[next_nodes++]);
        }
        nodes = next_nodes;
    }
    strcpy(root, level[0]);
    return 1;
}
#endif
