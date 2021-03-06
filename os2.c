#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_CONSUMER 8
#define MAX_SUPPLIER 5

void readConsumerConfigFile(int ID);
void readSupplierConfigFile(int ID);
void* doSupThread(void* id);
void* doConThread(void* id);
char* timeStamp();

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct item{
    char Name[256];
    int interval;
    int repeat;
    int failed_count;
}item;
item items_sup[MAX_SUPPLIER],items_con[MAX_CONSUMER];

typedef struct buffer{
    int buf[MAX_SUPPLIER]; // the buffer
	pthread_mutex_t lock[MAX_SUPPLIER];
} buffer_t;
buffer_t buffer;

int main()
{
	pthread_t sup_threads[MAX_SUPPLIER];
	pthread_t con_threads[MAX_CONSUMER];
		
	for( int i=0; i<MAX_SUPPLIER; i++ )
		pthread_create( &(sup_threads[i]), NULL, doSupThread, (void*)(long)i );
		
	for( int i=0; i<MAX_CONSUMER; i++ )
		pthread_create( &(con_threads[i]), NULL, doConThread, (void*)(long)i );
		
	for( int i=0; i<MAX_SUPPLIER; i++ )
		pthread_join(sup_threads[i], NULL);		
			
	for( int i=0; i<MAX_CONSUMER; i++ )
		pthread_join(con_threads[i], NULL);		

}


void* doSupThread(void* id)
{
	pthread_mutex_lock(&mutex);
	int ID = (long)id;
	readSupplierConfigFile(ID);
	pthread_cond_broadcast(&cond);	//Supplier send broadcast to waiting thread after finish readfile
	int tmp_interval = items_sup[ID].interval;
	pthread_mutex_unlock(&mutex);

	while(1) {

		pthread_mutex_lock(&buffer.lock[ID]);
		if(buffer.buf[ID] >= 100){ //product has more than 100
			items_sup[ID].failed_count++;		
			if (items_sup[ID].failed_count > items_sup[ID].repeat){
				items_sup[ID].failed_count = 0;
				tmp_interval *= 2;
				if (tmp_interval > 60){
					tmp_interval = items_sup[ID].interval;
				}
			}
			// TIME STAMP //
			int i=0;
			char* dateAndTime = timeStamp();
			while(dateAndTime[i] != '\n'){
				printf("%c", dateAndTime[i]);
				i++;
			}							
			printf(" %s supplier going to wait.\n", items_sup[ID].Name);
			pthread_mutex_unlock(&buffer.lock[ID]);
			sleep(tmp_interval);

		}
		else{

		items_sup[ID].failed_count = 0;
		tmp_interval = items_sup[ID].interval;	

		// TIME STAMP //
		int i=0;
		char* dateAndTime = timeStamp();
		while(dateAndTime[i] != '\n'){
			printf("%c", dateAndTime[i]);
			i++;
		}
        buffer.buf[ID] +=1 ;
		printf(" %s supplied 1 unit. stock after = %d\n", items_sup[ID].Name, buffer.buf[ID]);
        pthread_mutex_unlock(&buffer.lock[ID]);
		sleep(items_sup[ID].interval);
		}
    }	
	
	return NULL;
}

void* doConThread(void* id)
{
	pthread_mutex_lock(&mutex);
	int ID = (long)id;
	int buf_ind = -1;
	readConsumerConfigFile(ID);
	int tmp_interval = items_con[ID].interval;
	for(int i=0; i<MAX_SUPPLIER; i++){
		if(strcmp(items_con[ID].Name, items_sup[i].Name) == 0){
			buf_ind = i;
			break;
		}
	}	
	//if buf_ind = 1 mean that this thread can't macth supplier's product
	//so have to wait supplier thread read that file
	while(buf_ind == -1){
		pthread_cond_wait(&cond, &mutex);
		//update buf_ind after recieve broadcast
		for(int i=0; i<MAX_SUPPLIER; i++){
			if(strcmp(items_con[ID].Name, items_sup[i].Name) == 0){
				buf_ind = i;
				break;
			}
		}
	}
	pthread_mutex_unlock(&mutex);

    while(1) {

        pthread_mutex_lock(&buffer.lock[buf_ind]);

		if(0 == buffer.buf[buf_ind]){ //product has 0 so consumer cannot bye
			items_con[ID].failed_count++;
			// TIME STAMP //
			int i=0;
			char* dateAndTime = timeStamp();
			while(dateAndTime[i] != '\n'){
				printf("%c", dateAndTime[i]);
				i++;
			}
			if (items_con[ID].failed_count > items_con[ID].repeat){
				items_con[ID].failed_count = 0;
				tmp_interval *= 2;
				if (tmp_interval > 60){
					tmp_interval = items_con[ID].interval;
				}
			}
			printf(" %s consumer going to wait.\n",items_con[ID].Name);
			pthread_mutex_unlock(&buffer.lock[buf_ind]);
			sleep(tmp_interval);
		}
		else{
	
		items_con[ID].failed_count = 0;
		tmp_interval = items_con[ID].interval;

		// TIME STAMP //
		int i=0;
		char* dateAndTime = timeStamp();
		while(dateAndTime[i] != '\n'){
			printf("%c", dateAndTime[i]);
			i++;
		}
        buffer.buf[buf_ind] -= 1;
		printf(" %s consumed 1 unit. stock after = %d\n", items_con[ID].Name, buffer.buf[buf_ind]);
        pthread_mutex_unlock(&buffer.lock[buf_ind]);
		sleep(items_con[ID].interval);
		}
    }

	return NULL;
}

void readConsumerConfigFile(int ID){
	char num[2];
	sprintf(num, "%d", ID+1);
	char conFileName[] = "consumer";
	strcat(conFileName, num);
	strcat(conFileName, ".txt");
	
	FILE *fp;
    fp = fopen(conFileName, "r");
	char buffer[256];

	// NAME
    fgets(buffer, 256, fp);
	int len = strlen(buffer);
	if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
	strcpy(items_con[ID].Name, buffer);

	//interval
	fgets(buffer, 256, fp);
	items_con[ID].interval = atoi(buffer);

	//repeat
	fgets(buffer, 256, fp);
	items_con[ID].repeat = atoi(buffer);

}

void readSupplierConfigFile(int ID){
	char num[2];
	sprintf(num, "%d", ID+1);
	char supFileName[] = "supplier";
	strcat(supFileName, num);
	strcat(supFileName, ".txt");
	
	FILE *fp;
    fp = fopen(supFileName, "r");
	char buffer[256];

	// NAME
    fgets(buffer, 256, fp);
	int len = strlen(buffer);
	if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
	strcpy(items_sup[ID].Name, buffer);

	//interval
	fgets(buffer, 256, fp);
	items_sup[ID].interval = atoi(buffer);
	
	//repeat
	fgets(buffer, 256, fp);
	items_sup[ID].repeat = atoi(buffer);

}

char* timeStamp(){
    time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }
    
    return c_time_string;
}


	
