#include "bench.h"
//metavlites gia metrisi diarkeias ergasiwn eggrafis-anagnosis
long long start;
long long end;

double rnd_writes_cost;
pthread_mutex_t writescal;
double rnd_reads_cost;
pthread_mutex_t readscal;
int nims; //ta nimata
long long secondspassed;
long int count; //sunoliko arithmo read-write


void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}

//sunartisi gia ektypwsi statistikwn
void print_myresults(const char* operation, long count, double operation_cost, long long seconds_passed) {
    printf(LINE);
    printf("-------------- time passed ------------ %.3lld seconds\n", seconds_passed);
    printf("|%s\t(done:%ld): %.6f sec/op; %.1f ops/sec(estimated); cost:%.3f(sec)\n",
           operation, count, (double)(operation_cost / count),
           (double)(count / operation_cost), operation_cost);
}

//gia klisi tou _write_test
void *write_request(void *arg){
	struct arguments *d = (struct arguments *)arg;
	_write_test(d->mycount, d->r, d->mythreads);
	return 0;
}
//gia klisi tou _read_test
void *read_request(void *arg){
	struct arguments *d = (struct arguments *)arg;
	_read_test(d->mycount, d->r, d->mythreads);
	return 0;
}

//sunartisi gia dimiourgia kai join twn nimatwn mas
void create_and_join_threads(void *(*operation)(void *), struct arguments *params, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        int result = pthread_create(&threads[i], NULL, operation, (void *)params);
        if (result != 0) {
            
            for (int j = 0; j < i; j++) {
              
                pthread_join(threads[j], NULL);
            }
            free(threads); 
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
}



int main(int argc,char** argv)
{
	//arxikopoiiseis
	int percentage;
	int i;
	pthread_t id1[1000];
	pthread_t id2[1000];
	//gia parametrous pou tha xrisimopoihsoun ta nimata
	struct arguments one;
	struct arguments two;
	struct arguments three;


	pthread_mutex_init(&writescal,NULL);
	pthread_mutex_init(&readscal,NULL);
	rnd_writes_cost=0;
	rnd_reads_cost=0;

	srand(time(NULL));
	if (argc < 3) {
		fprintf(stderr,"Usage: db-bench <write | read> <count>\n");
		exit(1);
	}
	//apo to termatiko otan trexoume to programma to trito stoixeio na einai to count, pose eggraffes i anagnoseis theloume
	count = atoi(argv[2]);
	//kai to tetarto posa nimata theloume na to moirasoun
	nims = atoi(argv[3]);
	// an ginei write, an o xristis epilexei -deutero orisma- to write
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;

		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;
		//anoigoume ti vasi
		_open_db();
		
		one.r=r;
		one.mycount=count;
		one.mythreads=nims;
		//ksekiname to xrono
		start = get_ustime_sec();
		//kaloume tin sunartisi gia pthread_create kai pthread_join twn nimatwn
		create_and_join_threads(write_request, &one, nims);
		//dilwnoume to telos tis diadikasias
		end = get_ustime_sec();
		//vriskoume to xrono
		secondspassed = end - start; 
		//kleinoume ti vasi
		_close_db();
		//ektupwnoume statistika
		print_myresults("Random-Write", one.mycount, rnd_writes_cost, secondspassed);

		//_write_test(count, r);
	} else if (strcmp(argv[1], "read") == 0) {
		int r = 0;

		_print_header(count);
		_print_environment();
		if (argc == 5)
			r = 1;

		_open_db();
		one.r=r;
		one.mycount=count;
		one.mythreads=nims;
		start = get_ustime_sec();
		//kaloume ti sunartisi gia pthread_create kai pthread_join
		create_and_join_threads(read_request, &one, nims);
		end = get_ustime_sec();
		secondspassed = end - start;
		_close_db();
		print_myresults("Random Read", one.mycount, rnd_reads_cost, secondspassed);

	}else if (strcmp(argv[1], "readwrite") == 0) {
		int r = 0;
		percentage = 0;
		_print_header(count);
		_print_environment();
		if (argc == 6)
			r = 1;

		_open_db();
		//dilwnoume to percentage na pairnei to 5o orisma pou dinei o xristis , gia ta statistika, poso tis ekato write kai read
		percentage = atoi(argv[4]);
		if (percentage < 0 || percentage > 100) {
			printf(LINE);
    		fprintf(stderr, "The percentage must be a number between 0 - 100, please be reasonable \n");
    		exit(1);
		}
		two.r=r;
		two.mycount=(long) (count*percentage/100);
		two.mythreads=(int) (nims*percentage/100);
		three.r=r;
		three.mycount=(long) (count*(100-percentage)/100);
		three.mythreads=(int) (nims*(100-percentage)/100);
		//edw epeidi theloume na ginei prwta create kai meta join to kanoume xeirokinita xwris tin klhsh ths sunartisis pou dhmiourgisame pio panw
	
		//int i;
		for(i=0;i<(nims*percentage/100);i++)
			pthread_create(&id1[i],NULL,write_request,(void *)&two);

		for(i=0;i<(nims*(100-percentage)/100);i++)
			pthread_create(&id2[i],NULL,read_request,(void *)&three);

		for(i=0;i<(nims*(100-percentage)/100);i++)
			pthread_join(id2[i],NULL);

		for(i=0;i<(nims*percentage/100);i++)
			pthread_join(id1[i],NULL);

		_close_db();
		//print statistics
		print_myresults("Random-Write", two.mycount, rnd_writes_cost, secondspassed);

		print_myresults("Random-Read", three.mycount, rnd_reads_cost, secondspassed);

		
	} else {
		fprintf(stderr,"Usage: db-bench <write | read> <count> <random>\n");
		exit(1);
	}

	return 1;
}
