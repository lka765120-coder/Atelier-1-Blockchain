/* Atelier 1 - Blockchain
 * Demonstration pedagogique en C : Merkle, Proof of Work, Proof of Stake
 * et petite blockchain. SHA-256 est fourni par OpenSSL (libcrypto).
 * Ne pas utiliser ce code comme blockchain de production.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_TX 4
#define CHAIN_LEN 3
#define HASH_HEX 65
#define VALIDATOR_COUNT 3

/* PLAN DU PROGRAMME
 * EXERCICE 1 : sha256_hex(), merkle_root(), demo_merkle()
 * EXERCICE 2 : has_zero_prefix(), mine_pow(), demo_pow_difficulty()
 * EXERCICE 3 : choose_validator(), demo_consensus_speed()
 * EXERCICE 4 : make_block(), verify_chain(), build_chain(), demo_blockchains()
 */

typedef enum { CONSENSUS_POW, CONSENSUS_POS } Consensus;

typedef struct {
    const char *name;
    unsigned int stake;
} Validator;

typedef struct {
    int index;
    char previous_hash[HASH_HEX];
    char merkle_root[HASH_HEX];
    char transactions[MAX_TX][128];
    int transaction_count;
    unsigned long long nonce;
    char validator[32];
    char hash[HASH_HEX];
} Block;

static const Validator validators[VALIDATOR_COUNT] = {
    {"Alice", 50}, {"Bob", 30}, {"Charlie", 20}
};

/* Calcule SHA-256 et retourne 64 caractères hexadécimaux. */
static void sha256_hex(const char *text, char output[HASH_HEX]) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)text, strlen(text), digest);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        sprintf(output + (i * 2), "%02x", digest[i]);
    output[64] = '\0';
}

/* ======================== EXERCICE 1 ========================
   Arbre de Merkle : hash des transactions, puis hash des paires.
   Si un niveau contient un nombre impair de noeuds, le dernier est duplique. */
static int merkle_root(const char *transactions[], int count, char root[HASH_HEX]) {
    if (count <= 0 || count > MAX_TX) return 0;
    char level[MAX_TX][HASH_HEX];
    for (int i = 0; i < count; ++i)
        sha256_hex(transactions[i], level[i]);

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

static void block_hash(const Block *block, char output[HASH_HEX]) {
    char header[1024];
    snprintf(header, sizeof(header), "%d|%s|%s|%llu|%s",
             block->index, block->previous_hash, block->merkle_root,
             block->nonce, block->validator);
    sha256_hex(header, output);
}

/* ======================== EXERCICE 2 ========================
   Proof of Work : recherche d'un hash avec N zeros initiaux. */
static int has_zero_prefix(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; ++i)
        if (hash[i] != '0') return 0;
    return 1;
}

/* ======================== EXERCICE 3 ========================
   Proof of Stake : choix d'un validateur proportionnel a sa mise. */
static unsigned long long total_stake(void) {
    unsigned long long total = 0;
    for (int i = 0; i < VALIDATOR_COUNT; ++i) total += validators[i].stake;
    return total;
}

/* Tirage proportionnel aux mises : simulation pedagogique de Proof of Stake. */
static int choose_validator(void) {
    unsigned long long ticket = (unsigned long long)rand() % total_stake();
    unsigned long long cumulative = 0;
    for (int i = 0; i < VALIDATOR_COUNT; ++i) {
        cumulative += validators[i].stake;
        if (ticket < cumulative) return i;
    }
    return VALIDATOR_COUNT - 1;
}

static void mine_pow(Block *block, int difficulty) {
    block->nonce = 0;
    do {
        block_hash(block, block->hash);
        if (has_zero_prefix(block->hash, difficulty)) return;
        ++block->nonce;
    } while (1);
}

static void make_block(Block *block, int index, const char *previous_hash,
                       const char *transactions[], int count,
                       Consensus consensus, int difficulty) {
    memset(block, 0, sizeof(*block));
    block->index = index;
    block->transaction_count = count;
    strcpy(block->previous_hash, previous_hash);
    for (int i = 0; i < count; ++i)
        snprintf(block->transactions[i], sizeof(block->transactions[i]), "%s", transactions[i]);

    const char *tx_ptrs[MAX_TX];
    for (int i = 0; i < count; ++i) tx_ptrs[i] = block->transactions[i];
    merkle_root(tx_ptrs, count, block->merkle_root);

    if (consensus == CONSENSUS_POW) {
        mine_pow(block, difficulty);
    } else {
        int selected = choose_validator();
        snprintf(block->validator, sizeof(block->validator), "%s", validators[selected].name);
        block_hash(block, block->hash);
    }
}

/* ======================== EXERCICE 4 ========================
   Blockchain : blocs, transactions, chainage et verification d'integrite. */
static int verify_chain(const Block chain[CHAIN_LEN], Consensus consensus,
                        int difficulty, int verbose) {
    char expected_previous[HASH_HEX];
    memset(expected_previous, '0', 64);
    expected_previous[64] = '\0';

    for (int i = 0; i < CHAIN_LEN; ++i) {
        if (strcmp(chain[i].previous_hash, expected_previous) != 0) {
            if (verbose) printf("Bloc %d invalide : previous_hash incorrect.\n", i);
            return 0;
        }
        const char *tx_ptrs[MAX_TX];
        for (int j = 0; j < chain[i].transaction_count; ++j)
            tx_ptrs[j] = chain[i].transactions[j];
        char expected_root[HASH_HEX], expected_hash[HASH_HEX];
        if (!merkle_root(tx_ptrs, chain[i].transaction_count, expected_root) ||
            strcmp(expected_root, chain[i].merkle_root) != 0) {
            if (verbose) printf("Bloc %d invalide : racine de Merkle incorrecte.\n", i);
            return 0;
        }
        block_hash(&chain[i], expected_hash);
        if (strcmp(expected_hash, chain[i].hash) != 0) {
            if (verbose) printf("Bloc %d invalide : hash modifie.\n", i);
            return 0;
        }
        if (consensus == CONSENSUS_POW && !has_zero_prefix(chain[i].hash, difficulty)) {
            if (verbose) printf("Bloc %d invalide : preuve de travail incorrecte.\n", i);
            return 0;
        }
        strcpy(expected_previous, chain[i].hash);
    }
    if (verbose) printf("Blockchain valide : %d blocs verifies.\n", CHAIN_LEN);
    return 1;
}

static void build_chain(Block chain[CHAIN_LEN], Consensus consensus, int difficulty) {
    const char *tx0[] = {"Genesis: 100 jetons pour Alice"};
    const char *tx1[] = {"Alice -> Bob : 10", "Bob -> Charlie : 3", "Alice -> Charlie : 2"};
    const char *tx2[] = {"Charlie -> Alice : 1", "Bob -> Alice : 4"};
    const char *zero_hash = "0000000000000000000000000000000000000000000000000000000000000000";
    make_block(&chain[0], 0, zero_hash, tx0, 1, consensus, difficulty);
    make_block(&chain[1], 1, chain[0].hash, tx1, 3, consensus, difficulty);
    make_block(&chain[2], 2, chain[1].hash, tx2, 2, consensus, difficulty);
}

static void show_chain(const Block chain[CHAIN_LEN], const char *label) {
    printf("\n%s\n", label);
    for (int i = 0; i < CHAIN_LEN; ++i) {
        printf("Bloc %d | hash: %.16s... | precedent: %.16s... | racine Merkle: %.16s...",
               chain[i].index, chain[i].hash, chain[i].previous_hash, chain[i].merkle_root);
        if (chain[i].validator[0]) printf(" | validateur: %s", chain[i].validator);
        else printf(" | nonce: %llu", chain[i].nonce);
        printf("\n");
    }
}

static void demo_merkle(void) {
    const char *tx[] = {"Alice -> Bob : 10", "Bob -> Charlie : 3", "Alice -> Charlie : 2"};
    char root[HASH_HEX];
    printf("EXERCICE 1 - ARBRE DE MERKLE\n");
    for (int i = 0; i < 3; ++i) {
        char leaf[HASH_HEX];
        sha256_hex(tx[i], leaf);
        printf("Feuille %d (%s) : %.16s...\n", i + 1, tx[i], leaf);
    }
    merkle_root(tx, 3, root);
    printf("Racine de Merkle (3 feuilles, derniere dupliquee) : %s\n", root);
}

static void demo_pow_difficulty(void) {
    const char *tx[] = {"Test de difficulte Proof of Work"};
    char zero_hash[HASH_HEX];
    memset(zero_hash, '0', 64); zero_hash[64] = '\0';
    const int levels[] = {2, 3, 4};
    printf("\nEXERCICE 2 - PROOF OF WORK : DIFFICULTE ET TEMPS\n");
    for (int k = 0; k < 3; ++k) {
        Block b;
        clock_t start = clock();
        make_block(&b, 1, zero_hash, tx, 1, CONSENSUS_POW, levels[k]);
        double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("Difficulte %d zero(s) | nonce %llu | hash %.20s... | temps %.6f s\n",
               levels[k], b.nonce, b.hash, elapsed);
    }
}

static void demo_consensus_speed(void) {
    const char *tx[] = {"Comparaison PoW et PoS"};
    char zero_hash[HASH_HEX];
    memset(zero_hash, '0', 64); zero_hash[64] = '\0';
    Block b;
    clock_t start = clock();
    make_block(&b, 1, zero_hash, tx, 1, CONSENSUS_POW, 4);
    double pow_time = (double)(clock() - start) / CLOCKS_PER_SEC;

    /* On mesure 20 000 selections PoS pour limiter le bruit de l'horloge. */
    const int repetitions = 20000;
    volatile unsigned long long sink = 0;
    start = clock();
    for (int i = 0; i < repetitions; ++i) {
        int selected = choose_validator();
        char value[128], hash[HASH_HEX];
        snprintf(value, sizeof(value), "block|%d|stake|%s", i, validators[selected].name);
        sha256_hex(value, hash);
        sink += (unsigned char)hash[0];
    }
    double pos_total = (double)(clock() - start) / CLOCKS_PER_SEC;
    double pos_average = pos_total / repetitions;
    printf("\nEXERCICE 3 - COMPARAISON DES CONSENSUS\n");
    printf("Proof of Work (difficulte 4) : %.6f s pour un bloc\n", pow_time);
    printf("Proof of Stake : %.6f s pour %d simulations (moyenne %.9f s/selection)\n",
           pos_total, repetitions, pos_average);
    printf("Validateur choisi pour le bloc PoS d'exemple : %s\n", validators[choose_validator()].name);
    printf("Mesure (anti-optimisation) : %llu\n", (unsigned long long)sink);
    printf("Note : mesure pedagogique, les temps dependent de la machine et du niveau PoW.\n");
}

static void demo_blockchains(void) {
    Block pow_chain[CHAIN_LEN], pos_chain[CHAIN_LEN];
    Block altered[CHAIN_LEN];
    const int difficulty = 3;

    printf("\nEXERCICE 4 - BLOCKCHAIN SIMPLE AVEC PROOF OF WORK\n");
    build_chain(pow_chain, CONSENSUS_POW, difficulty);
    show_chain(pow_chain, "Chaine PoW :");
    printf("Verification initiale : ");
    verify_chain(pow_chain, CONSENSUS_POW, difficulty, 1);
    memcpy(altered, pow_chain, sizeof(pow_chain));
    strcpy(altered[1].transactions[0], "Alice -> Bob : 999 (donnee modifiee)");
    printf("Apres modification d'une transaction : ");
    verify_chain(altered, CONSENSUS_POW, difficulty, 1);

    printf("\nBlockchain simple avec Proof of Stake\n");
    build_chain(pos_chain, CONSENSUS_POS, 0);
    show_chain(pos_chain, "Chaine PoS :");
    printf("Verification initiale : ");
    verify_chain(pos_chain, CONSENSUS_POS, 0, 1);
    memcpy(altered, pos_chain, sizeof(pos_chain));
    strcpy(altered[1].transactions[0], "Alice -> Bob : 999 (donnee modifiee)");
    printf("Apres modification d'une transaction : ");
    verify_chain(altered, CONSENSUS_POS, 0, 1);
}

int main(void) {
    srand((unsigned int)time(NULL));
    printf("ATELIER 1 - BLOCKCHAIN (DEMONSTRATION C)\n");
    printf("SHA-256 via OpenSSL | PoW et PoS pedagogiques\n\n");
    demo_merkle();
    demo_pow_difficulty();
    demo_consensus_speed();
    demo_blockchains();
    printf("\nFin de la demonstration.\n");
    return 0;
}
