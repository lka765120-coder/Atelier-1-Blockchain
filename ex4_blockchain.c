/* EXERCICE 4 - Petite blockchain combinant Merkle, PoW et PoS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "crypto_utils.h"
#define BLOCKS 3
#define VALIDATORS 3

typedef enum { POW, POS } Consensus;
typedef struct { const char *name; unsigned int stake; } Validator;
typedef struct {
    int index;
    char previous_hash[HASH_HEX], root[HASH_HEX], hash[HASH_HEX];
    char tx[MAX_TX][128];
    int tx_count;
    unsigned long long nonce;
    char validator[24];
} Block;
static const Validator validators[VALIDATORS]={{"Alice",50},{"Bob",30},{"Charlie",20}};

static int select_validator(void) {
    unsigned int ticket=(unsigned int)rand()%100;
    if(ticket<50) return 0;
    if(ticket<80) return 1;
    return 2;
}
static void hash_block(const Block *b,char output[HASH_HEX]) {
    char header[1024];
    snprintf(header,sizeof(header),"%d|%s|%s|%llu|%s",b->index,b->previous_hash,b->root,b->nonce,b->validator);
    sha256_hex(header,output);
}
static int starts_zero(const char *hash,int difficulty) {
    for(int i=0;i<difficulty;i++)if(hash[i]!='0')return 0;
    return 1;
}
static void create_block(Block *b,int index,const char *previous,const char *tx[],int count,Consensus c,int difficulty) {
    memset(b,0,sizeof(*b)); b->index=index; b->tx_count=count; strcpy(b->previous_hash,previous);
    for(int i=0;i<count;i++)snprintf(b->tx[i],sizeof(b->tx[i]),"%s",tx[i]);
    const char *tx_ptrs[MAX_TX]; for(int i=0;i<count;i++)tx_ptrs[i]=b->tx[i];
    merkle_root(tx_ptrs,count,b->root);
    if(c==POS)snprintf(b->validator,sizeof(b->validator),"%s",validators[select_validator()].name);
    do { hash_block(b,b->hash); if(c==POS||starts_zero(b->hash,difficulty))return; ++b->nonce; } while(1);
}
static void build_chain(Block chain[BLOCKS],Consensus c,int difficulty) {
    const char *t0[]={"Genesis : 100 jetons pour Alice"};
    const char *t1[]={"Alice -> Bob : 10","Bob -> Charlie : 3","Alice -> Charlie : 2"};
    const char *t2[]={"Charlie -> Alice : 1","Bob -> Alice : 4"};
    const char *zero="0000000000000000000000000000000000000000000000000000000000000000";
    create_block(&chain[0],0,zero,t0,1,c,difficulty);
    create_block(&chain[1],1,chain[0].hash,t1,3,c,difficulty);
    create_block(&chain[2],2,chain[1].hash,t2,2,c,difficulty);
}
static int verify_chain(const Block chain[BLOCKS],Consensus c,int difficulty) {
    char expected_previous[HASH_HEX]; memset(expected_previous,'0',64); expected_previous[64]='\0';
    for(int i=0;i<BLOCKS;i++) {
        const char *tx_ptrs[MAX_TX]; for(int j=0;j<chain[i].tx_count;j++)tx_ptrs[j]=chain[i].tx[j];
        char root[HASH_HEX],hash[HASH_HEX];
        if(strcmp(expected_previous,chain[i].previous_hash)!=0)return 0;
        if(!merkle_root(tx_ptrs,chain[i].tx_count,root)||strcmp(root,chain[i].root)!=0)return 0;
        hash_block(&chain[i],hash); if(strcmp(hash,chain[i].hash)!=0)return 0;
        if(c==POW&&!starts_zero(hash,difficulty))return 0;
        strcpy(expected_previous,chain[i].hash);
    }
    return 1;
}
static void show_chain(Block chain[BLOCKS],Consensus c) {
    for(int i=0;i<BLOCKS;i++) {
        printf("Bloc %d | hash %.16s... | precedent %.16s...",i,chain[i].hash,chain[i].previous_hash);
        if(c==POW)printf(" | nonce %llu",chain[i].nonce); else printf(" | validateur %s",chain[i].validator);
        puts("");
    }
}
static void demo(Consensus c,const char *name) {
    Block chain[BLOCKS],altered[BLOCKS]; const int difficulty=3;
    printf("\nBlockchain avec %s\n",name); build_chain(chain,c,difficulty); show_chain(chain,c);
    printf("Verification avant modification : %s\n",verify_chain(chain,c,difficulty)?"VALIDE":"INVALIDE");
    memcpy(altered,chain,sizeof(chain)); strcpy(altered[1].tx[0],"Alice -> Bob : 999 (modifie)");
    printf("Verification apres modification : %s\n",verify_chain(altered,c,difficulty)?"VALIDE":"INVALIDE");
}
int main(void) {
    srand((unsigned int)time(NULL));
    puts("EXERCICE 4 - BLOCKCHAIN SIMPLE");
    demo(POW,"Proof of Work"); demo(POS,"Proof of Stake");
    puts("\nLa modification d'une transaction invalide la racine Merkle et la chaine.");
    return 0;
}
