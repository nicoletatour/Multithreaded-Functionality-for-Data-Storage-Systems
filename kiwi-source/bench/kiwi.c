#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")


DB* db;
//sunartisi gia anoigma database
void _open_db()
{	db = db_open(DATAS);
}
//sunartisi gia kleisimo database
void _close_db()
{	db_close(db);
}
//sunartiseis gia lock kai unlock
void lock_writes(){
	pthread_mutex_lock(&writescal);
}

void lock_reads(){
	pthread_mutex_lock(&readscal);
}

void unlock_writes(){
	pthread_mutex_unlock(&writescal);
}

void unlock_reads(){
	pthread_mutex_unlock(&readscal);
}


void _write_test(long int count, int r, int nimata)
{
	int i;
	double cost;
	long long start;
	long long end;
	Variant sk, sv;
	// to kainourgio count pou moirazei ta write st nimata
	long int newcount;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);


	newcount=count/nimata;
	start = get_ustime_sec();
	
	for (i = 0; i < newcount; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();
	cost = end -start;
	//lock kai unlock gia asfaleia
	lock_writes();
	rnd_writes_cost+=cost;
	unlock_writes();
	
	
	printf(LINE);

	printf("|Random-Write ekeiii	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
		,count, (double)(cost / count)
		,(double)(count / cost)
		,cost);	
}

void _read_test(long int count, int r, int nimata)
{
	int i;
	int ret;
	double cost;
	long long start,end;
	int found = 0;
	Variant sk;
	Variant sv;
	//new count gia na moirazei ta read sta nimata
	long int newcount;
	char key[KSIZE + 1];

	
	start = get_ustime_sec();
	newcount = count/nimata;
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);
		
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	//db_close(db);

	end = get_ustime_sec();

	cost = end - start;

	lock_reads();
	rnd_reads_cost += cost;
	unlock_reads();
	printf(LINE);
	printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
		count, found,
		(double)(cost / count),
		(double)(count / cost),
		cost);
}
