/*H******************************************************
 * Group members:
 * Noemi Kallweit
 * Hana Habibi
*H*/

#include "cachelab.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * covert decimal integer to binary
 * @param decimal
 * @return binary
 */
char *decToBin(unsigned int decimal) {
    char *bin = (char*) calloc(32, sizeof(char));
    for(int i = 32; i >= 0; i--) {
        if(decimal%2 == 0){
            bin[i] = '0'; }
        else {
            bin[i] = '1'; }
        decimal = decimal/2;
    }
    return bin;
}
/**
 * covert binary array to decimal number
 * @param bin
 * @return result (decimal number)
 */
int BinToDez(char* bin){
    int result = 0;
    for (int i = 0; i < strlen(bin); i++) {
        if (bin[i] == '1')
            result += pow(2, strlen(bin) - 1 - i);
    }
    return result;
}

int main(int argc, char** argv) {

    //cache line and its members
    struct cache_line {
        int valid_bit;
        int tag;
        int lru_counter;
    };

    //hit counter, miss counter , eviction counter and global lru counter
    int hitc = 0, missc = 0, evictionc = 0;
    int lru = 0;



    int opt;
    //if the verbose flag is set
    int tracebool = 0;
    //block number, associativity number and set number
    int b = 0, E = 0, s = 0;
    //tracefile
    char* inputfile = NULL;
    char* end;

    //reading command line arguments
    while( (opt = getopt(argc, argv, "hvs:E:b:t:")) != -1 ) {
        switch(opt) {
            case 'h':
                //print usage
                printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
                break;

            case 'v':
                //traceinfo boolean
                tracebool = 1;
                break;

            case 's':
                //set
                s = (int) strtol(optarg, &end, 10);
                if (end == optarg || *end != '\0' || errno == ERANGE) {
                    fprintf(stderr, "Can't parse number");
                    return 1;
                }
                break;
            case 'E':
                //associativity
                E = (int) strtol(optarg, &end, 10);
                if (end == optarg || *end != '\0' || errno == ERANGE) {
                    fprintf(stderr, "Can't parse number");
                    return 1;
                }
                break;
            case 'b':
                //block bits
                b = (int) strtol(optarg, &end, 10);
                if (end == optarg || *end != '\0' || errno == ERANGE) {
                    fprintf(stderr, "Can't parse number");
                    return 1;
                }
                break;
            case 't':
                //tracefile input
                 inputfile = optarg;
                break;
            case '?':
            default:
                //error
                fprintf(stderr, "Enter -h for usage!\n");
                exit(1);
        }
    }

    //set number
    int setno = (int)pow(2,s);

    //memory allocation
    typedef struct cache_line cl;
    cl *cache = (cl*) calloc(setno * E * 3, sizeof(int));

    //read file
    FILE* pFile = fopen(inputfile, "r");

    //get Tag from binary address
    unsigned int getTag(char* binaddress){
        int tagbits = strlen(binaddress)-b-s;
        char* tag = (char*)calloc(tagbits, sizeof(char));
        for(int i = 0; i < tagbits;i++){
            tag[i] = binaddress[i];
        }
        int T = BinToDez(tag);
        return T;
    }

    //get set from binary address
    unsigned int getSet(char* binaddress){
        int tagbits = strlen(binaddress)-b-s;
        char *set = (char*)calloc(s, sizeof(char));
        for(int i = tagbits; i < s+tagbits; i++){
            set[i-tagbits] = binaddress[i];
        }

        int S = BinToDez(set);
        return S;
    }

    //read from tracefile
    char mode;
    unsigned int address;
    int size;

    while(fscanf(pFile, " %c %x,%d", &mode, &address, &size) != EOF) {
        //if instruction load ignore trace line
        if(mode == 'I')
            continue;

        //check if the cache line is already set
        int flag = 0;

        int set = getSet(decToBin(address));
        int tagn = getTag(decToBin(address));

        //check if tag is already in cache
        for(int i = 0; i < E; i++){
            if(cache[(set*E)+i].valid_bit == 1){
                if(cache[(set*E)+i].tag == tagn){
                    flag = 1;
                    hitc++;
                    lru++;
                    cache[(set*E)+i].lru_counter = lru;
                    //check for modify data(M)
                    if (mode == 'M') {
                        hitc++;
                        lru++;
                        cache[(set*E)+i].lru_counter = lru;
                    }
                    //if verbose flag is set, print trace
                    if(tracebool) {
                        printf("%c %x,%d hit", mode, address, size);
                        if (mode == 'M')
                            printf(" hit");
                        printf("\n");
                    }
                    //if the tag is found
                    break;
                }
            }
        }

        //check for cold miss
        if(flag == 0) {
            for (int i = 0; i < E; i++) {
                if (cache[(set * E) + i].valid_bit == 0) {
                    flag = 2;
                    missc++;
                    lru++;
                    cache[(set * E) + i].valid_bit = 1;
                    cache[(set * E) + i].lru_counter = lru;
                    cache[(set * E) + i].tag = tagn;
                    //check for modify data(M)
                    if (mode == 'M') {
                        hitc++;
                        lru++;
                        cache[(set*E)+i].lru_counter = lru;
                    }
                    //if verbose flag is set, print trace
                    if (tracebool) {
                        printf("%c %x,%d miss", mode, address, size);
                        if (mode == 'M')
                            printf(" hit");
                        printf("\n");
                    }
                    //if we find the empty line and replace it then break
                    break;
                }
            }

            //check for miss and eviction
            if(flag == 0) {
                //find the smallest lru counter
                int tmp = cache[(set * E)].lru_counter;
                int smallertag = cache[(set * E)].tag;
                for (int i = 1; i < E; i++) {
                    if (cache[(set * E) + i].lru_counter < tmp) {
                        tmp = cache[(set * E) + i].lru_counter;
                        smallertag = cache[(set * E) + i].tag;
                    }
                }
                //replace the block with the smallest lru counter
                for (int i = 0; i < E; i++) {
                    if (cache[(set * E) + i].tag == smallertag) {
                        flag = 1;
                        missc++;
                        evictionc++;
                        lru++;
                        cache[(set * E) + i].lru_counter = lru;
                        cache[(set * E) + i].tag = tagn;
                        //check for modify data(M)
                        if (mode == 'M') {
                            hitc++;
                            lru++;
                            cache[(set*E)+i].lru_counter = lru;
                        }
                        //if verbose flag is set, print trace
                        if (tracebool) {
                            printf("%c %x,%d miss eviction", mode, address, size);
                            if (mode == 'M')
                                printf(" hit");
                            printf("\n");
                        }
                        //if we evict the lru line and replace it then break
                        break;
                    }
                }
            }
        }
    }
    fclose(pFile);

    printSummary(hitc, missc, evictionc);

    free(cache);

    return 0;
}