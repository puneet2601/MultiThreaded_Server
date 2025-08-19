#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Global variables
long sum = 0;
long odd = 0;
long even = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
long task_index=0;
long max_task;
long task_count = 0;

typedef struct {
    char type;
    int num;
} Task;

Task task_queue[10001];

pthread_mutex_t queue, ind;

void processtask(Task task);
void *fun(){
    while(1){
        pthread_mutex_lock(&queue);
        // printf("Task index: %ld\n", task_index);
        if(task_index >= task_count){
            pthread_mutex_unlock(&queue);
            break;
        }
        Task task = task_queue[task_index];
        task_index++;
        pthread_mutex_unlock(&queue);
        processtask(task);

    }
}
void processtask(Task task)
{
    if(task.type == 'p'){
    // simulate burst time
        sleep(task.num);
        pthread_mutex_lock(&ind);
        // update global variables
        sum += task.num;
        if (task.num % 2 == 1)
        {
            odd++;
        }
        else
        {
            even++;
        }
        if (task.num < min)
        {
            min = task.num;
        }
        if (task.num > max)
        {
            max = task.num;
        }
        pthread_mutex_unlock(&ind);
        printf("Task Completed\n");
    }
    else{

        sleep(task.num);
        printf("Wait Over\n");

    }
}

int main(int argc, char *argv[])
{
    
    if (argc != 2)
    {
        printf("Usage: sum <infile>\n");
        exit(EXIT_FAILURE);
    }
    char *fn = argv[1];
    // Read from file
    FILE *fin = fopen(fn, "r");
    fscanf(fin, "%ld\n", &max_task);
    printf("The number of tasks are : %ld \n", max_task);
    char type;
    long num;
    
    while (fscanf(fin, "%c %ld\n", &type, &num) == 2)
    {
        task_queue[task_count].type = type;
        task_queue[task_count].num = num;
        task_count++;
    }
    fclose(fin);
    pthread_mutex_init(&queue, NULL);
    pthread_mutex_init(&ind, NULL);
    pthread_t threads[1000];
    for(int i=0; i<1000; i++){
        pthread_create(threads+i, NULL, fun, NULL);
    }
    for (int i = 0; i < 1000; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&queue);
    pthread_mutex_destroy(&ind);
    // Print global variables
    printf("%ld %ld %ld %ld %ld\n", sum, odd, even, min, max);

    return (EXIT_SUCCESS);
}
