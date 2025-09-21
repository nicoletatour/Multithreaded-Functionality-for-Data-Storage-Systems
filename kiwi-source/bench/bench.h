#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);
//mutex gia sugxronismo prosvasis
extern pthread_mutex_t writescal;
//gia na kratisei tuxaia to kostos gia write
extern double rnd_writes_cost;
//mutex gia sugxronismo prosvasis
extern pthread_mutex_t readscal;
//gia na kratisei tuxaia to kostos gia read
extern double rnd_reads_cost;

struct arguments
{   
    long int mycount;
    int r;
    int mythreads;
};
//sunartisi gia dimiourgia kai enosi nimatwn
void create_and_join_threads(void *(*operation)(void *), struct arguments *params, int num_threads);

