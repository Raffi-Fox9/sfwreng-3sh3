#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>

#define TOTAL_CHAIRS 3U

static sem_t sem_wakeup_sync;
static sem_t sem_occupied_chairs;
static sem_t sem_empty_chairs;

static pthread_mutex_t mut_queue;
static int student_queue[TOTAL_CHAIRS] = { 0 };
static unsigned int student_queue_counter;

static bool TA_sleeping = 1;

void *run_student_thread(void *arg) {

    int student_no = *(int *) arg;
    int work_done = 0;
    int occupied_chairs;

    printf("Student %d started working.\n", student_no);

    while (work_done < 20) {
        int sleep_time = rand() % 5 + 1;
        sleep(sleep_time);
        work_done += sleep_time;
        printf("Student %d worked for %d seconds (total: %d) and asks the TA for help\n",
                student_no, sleep_time, work_done);

        sem_getvalue(&sem_occupied_chairs, &occupied_chairs);
        if (occupied_chairs == TOTAL_CHAIRS) {
            printf("No free chairs; Student %d will come back later.\n", student_no);
            sleep(10);
        }
        else {
            pthread_mutex_lock(&mut_queue);
            student_queue_counter += 1;
            student_queue[student_queue_counter % TOTAL_CHAIRS] = student_no;
            printf("stu: [%d, %d, %d]\n",
                student_queue[(student_queue_counter-2) % TOTAL_CHAIRS],
                student_queue[(student_queue_counter-1) % TOTAL_CHAIRS],
                student_queue[student_queue_counter % TOTAL_CHAIRS]);
            pthread_mutex_unlock(&mut_queue);

            if (TA_sleeping) {
                printf("Student %d attempts to wake up the TA.\n", student_no);
                sem_post(&sem_wakeup_sync);
            }
            sem_post(&sem_occupied_chairs);
            sem_getvalue(&sem_occupied_chairs, &occupied_chairs);
            printf("Student %d took seat #%d and is waitng for the TA.\n", student_no, occupied_chairs + 1);
            sem_wait(&sem_empty_chairs);
        }
    }

    printf("Student %d handed the work in.\n", student_no);

    free(arg);
    pthread_exit(NULL);
}


void *run_TA_thread(void *arg) {

    int occupied_chairs;
    int student_no;

    for (;;) {
        sem_wait(&sem_wakeup_sync);
        TA_sleeping = false;
        printf("\033[32m" "The TA has woken up\n" "\033[0m");

        for(;;) {
            sem_wait(&sem_occupied_chairs);
            sem_getvalue(&sem_occupied_chairs, &occupied_chairs);

            pthread_mutex_lock(&mut_queue);
            student_no = student_queue[(student_queue_counter - occupied_chairs) % TOTAL_CHAIRS];
            student_queue[(student_queue_counter - occupied_chairs) % TOTAL_CHAIRS] = 0;
            printf("\033[32m" "ta:  [%d, %d, %d]\n" "\033[0m",
                student_queue[(student_queue_counter-2) % TOTAL_CHAIRS],
                student_queue[(student_queue_counter-1) % TOTAL_CHAIRS],
                student_queue[student_queue_counter % TOTAL_CHAIRS]);
            pthread_mutex_unlock(&mut_queue);

            int sleep_time = rand() % 5 + 1;
            sleep(sleep_time);
            printf("\033[32m" "The TA helped student %d for %d seconds.\n" "\033[0m", student_no, sleep_time);

            if (occupied_chairs == 0) {
                // No more students to help
                break;
            }
            sem_post(&sem_empty_chairs);
        }
        printf("\033[32m" "The TA is going to sleep\n" "\033[0m");
        TA_sleeping = true;
    }
    
    pthread_exit(NULL);
}

int main() {
    pthread_t TA_thread;
    pthread_t *student_threads;

    int num_students;
    printf("Please enter how many students there are: ");
    scanf("%d", &num_students);
    if (num_students < 5) {
        printf("Not enough students, setting to 5\n");
        num_students = 5;
    }

    sem_init(&sem_wakeup_sync, 0, 0);
    sem_init(&sem_occupied_chairs, 0, 0);
    sem_init(&sem_empty_chairs, 0, 0);
    pthread_mutex_init(&mut_queue, NULL);

    student_threads = (pthread_t*) malloc(sizeof(pthread_t)* num_students);
    pthread_create(&TA_thread, NULL, run_TA_thread, NULL);
    for(int i = 0; i < num_students; ++i) {
        int *student_no = malloc(sizeof(*student_no));
        *student_no = i + 1;
        pthread_create(&student_threads[i], NULL, run_student_thread, student_no);
    }

    pthread_join(TA_thread, NULL);
    for(int i = 0; i < num_students; i++) {
        pthread_join(student_threads[i], NULL);
    }

    return 0;
}