#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int account_balance = 0;
pthread_mutex_t lock;

void *increment()
{
    for (int i = 0; i < 1000000; i++)
    {
        pthread_mutex_lock(&lock);
        account_balance++;
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[])
{
    int threadNum = 10;
    pthread_t th[threadNum];
    int i;
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("Mutex init failed\n");
        return 1;
    }
    for (i = 0; i < threadNum; i++)
    {
        // sleep(1);
        if (pthread_create(th + i, NULL, &increment, NULL) != 0)
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
    pthread_mutex_destroy(&lock);
    printf("Account Balance is : %d\n", account_balance);
    return 0;
}