#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>

double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}

int account_balance = 0;

void *increment(void *arg)
{
    int *deposit_amount = (int *)arg;
    for(int j=0; j<(*deposit_amount); j++){
        for (int i = 0; i < 1000000; i++)
        {
            account_balance++;
        }
    }
}

int main(int argc, char *argv[])
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    double startTime = (start.tv_sec * 1000.0) + (start.tv_usec / 1000.0);
    if(argc != 2){
        printf("Pass the number of threads as the command line argument\n");
        return -1;
    }
    int threadNum = atoi(argv[1]);
    if((threadNum <= 0) || ((threadNum & (threadNum-1)) != 0) ){
        printf("Pass the number of threads in powers of 2\n");
        return -1;
    }
    
    pthread_t th[threadNum];
    int i;
    int deposit_per_thread = 2048000000 / threadNum;
    for (i = 0; i < threadNum; i++)
    {
        int deposit_amount = deposit_per_thread/1000000;
        // sleep(1);
        if (pthread_create(th + i, NULL, &increment, &deposit_amount) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
        printf("Transaction %d has started\n", i);
    }
    for (i = 0; i < threadNum; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            return 2;
        }
        printf("Transaction %d has finished\n", i);
    }
    printf("Account Balance is : %d\n", account_balance);
    gettimeofday(&end, NULL);
    double endTime = (end.tv_sec * 1000.0) + (end.tv_usec / 1000.0);
    printf("Time spent: %.3f ms", (endTime - startTime));

    return 0;
}