#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIRECTION_LEFT -1
#define DIRECTION_RIGHT 1
#define REVERSE(direction) (direction == DIRECTION_LEFT ? \
            DIRECTION_RIGHT : DIRECTION_LEFT)

#define TOTAL_REQUESTS 20
#define TOTAL_CYLINDERS 300


int integer_compare(const void *a, const void *b) 
{
    int first = *((int*) a);
    int second = *((int*) b);
    if (first > second) return  1;
    if (first < second) return -1;
    return 0;
}

static void seek(int target, int *position, int *total_head_movements) {
    printf("%d, ", target);
    *total_head_movements += abs(target - *position);
    *position = target;
}

void FCFS_Algorithm(int position, int direction, int *requests)
{
    int total_head_movements = 0;
    printf("FCFS DISK SCHEDULING ALGORITHM:\n\n");

    // First come, first serve algorithm
    for (int i=0; i<TOTAL_REQUESTS; ++i) {
        seek(requests[i], &position, &total_head_movements);
    }

    printf("\n\n");
    printf("FCFS - Total head movements = %d\n\n", total_head_movements);
}

void SSTF_Algorithm(int position, int direction, int *requests)
{
    int total_head_movements = 0;
    printf("SSTF DISK SCHEDULING ALGORITHM\n\n");
    
    //Shortest Seek Time First
    int smallestDiff;
    int posOfSmallest;
    int end = 0;
    int done[TOTAL_REQUESTS] = {0};
    while (end != 1) {
      smallestDiff = 1000;
      for(int i=0; i<TOTAL_REQUESTS; i++){
        if(done[i] != 1 && abs(position - requests[i]) < smallestDiff){
          smallestDiff = abs(position - requests[i]);
          posOfSmallest = i;
        }
      }
      seek(requests[posOfSmallest], &position, &total_head_movements);
      done[posOfSmallest] = 1;
      position = requests[posOfSmallest];
      for(int i=0; i<TOTAL_REQUESTS; i++){
        if(done[i] == 0) {
            break;
        }
        //if all entries are 1, everything has been accounted for
        if(i == TOTAL_REQUESTS-1) {
            end = 1;
        }
      }
    }
    printf("\n\n");
    printf("SSTF - Total head movements = %d\n\n", total_head_movements);
}



void SCAN_Algorithm(int position, int direction, int *sortedrequests)
{
  int total_head_movements = 0;
  int original_position = position;
  printf("SCAN DISK SCHEDULING ALGORITHM\n\n");
  int cylinders[TOTAL_CYLINDERS];
  for(int i=0; i<TOTAL_CYLINDERS; i++){
    cylinders[i] = i;
  }
  //we need to find where to start looking at the requests in the list since its really annoying in C to create 2 lists containing the left and right halves of the request list
  int requestIndex = TOTAL_REQUESTS-1;
  int originalIndex = 0;
  for(int i=0; i<TOTAL_REQUESTS-2; i++){
    if(sortedrequests[i] <= original_position && sortedrequests[i+1] > original_position){
      requestIndex = i;
      originalIndex = requestIndex;
      break;
    }
  }
  //iterate through ALL the cylinders from original pos left to 0
  if(direction == DIRECTION_LEFT){
    for(int i=original_position; i>=0; i--){
      if(requestIndex>=0 && sortedrequests[requestIndex] == i){
        seek(i, &position, &total_head_movements);
        requestIndex--;
      }
    }
    //move to 0 (I would've used seek when we got to 0 but that would print 0 in our output based on how we made seek)
    total_head_movements += position;
    position = 0;
    requestIndex = originalIndex + 1;
    //now go right
    for(int i = original_position; i<TOTAL_CYLINDERS; i++){
      if(requestIndex < TOTAL_REQUESTS && sortedrequests[requestIndex] == i){
        seek(i, &position, &total_head_movements);
        requestIndex++;
      }
      if(requestIndex > TOTAL_REQUESTS){break;}
    }
  }
  //start going right
  else{
    for(int i=original_position; i<TOTAL_CYLINDERS; i++){
      if(requestIndex < TOTAL_REQUESTS && sortedrequests[requestIndex == i]){
        seek(i, &position, &total_head_movements);
        requestIndex++;
      }
    }
    total_head_movements += TOTAL_CYLINDERS - 1 - position;
    position = TOTAL_CYLINDERS - 1;
    requestIndex = originalIndex - 1;
    //change to going left
    for(int i=original_position; i>=0; i--){
      if(requestIndex >= 0 && sortedrequests[requestIndex] == i){
        seek(i, &position, &total_head_movements);
        requestIndex--;
      }
      if(requestIndex < 0){
          break;
      }
    }
  }
  
  printf("\n\n");
  printf("SCAN - Total head movements = %d\n\n", total_head_movements);
}


void C_SCAN_Algorithm(int position, int direction, int *sortedrequests)
{
  int total_head_movements = 0;
  int original_position = position;
  printf("C-SCAN DISK SCHEDULING ALGORITHM\n\n");

  int cylinders[TOTAL_CYLINDERS];
  for(int i=0; i<TOTAL_CYLINDERS; i++){
    cylinders[i] = i;
  }

  int requestIndex = TOTAL_REQUESTS - 1;
  int originalIndex = 0;
  for(int i=0; i<TOTAL_REQUESTS-2; i++){
    if(sortedrequests[i]<=original_position && sortedrequests[i+1] > original_position){
      requestIndex = i;
      originalIndex = requestIndex;
      break;
    }
  }

  if(direction == DIRECTION_LEFT){
    for(int i=original_position; i>=0; i--){
      if(requestIndex>=0 && sortedrequests[requestIndex]==i){
        seek(i, &position, &total_head_movements);
        requestIndex--;
      }
    }
    //move to 0
    total_head_movements += position;
    position = 0;

    //move to the rightmost cylinder
    total_head_movements += TOTAL_CYLINDERS - 1;
    position = TOTAL_CYLINDERS - 1;
    requestIndex = TOTAL_REQUESTS - 1;

    for(int i=TOTAL_CYLINDERS-1; i>original_position; i--){
      if (requestIndex > originalIndex && sortedrequests[requestIndex] == i){
        seek(i, &position, &total_head_movements);
        requestIndex--;
      }
      if (requestIndex == originalIndex) {
          break;
      }
    }
  }
  //start going to the right
  else{
    for(int i=original_position; i<TOTAL_CYLINDERS; i++){
      if(requestIndex < TOTAL_REQUESTS && sortedrequests[requestIndex] == i){
        seek(i, &position, &total_head_movements);
        requestIndex++;
      }
    }
    //update for rightmost cylinder
    total_head_movements += TOTAL_CYLINDERS - 1 - position;
    position = TOTAL_CYLINDERS-1;

    //jump to 0
    total_head_movements += TOTAL_CYLINDERS - 1;
    position = 0;
    requestIndex = 0;

    for(int i=0;i<original_position; i++){
      if(requestIndex<originalIndex && sortedrequests[requestIndex]==i){
        seek(i, &position, &total_head_movements);
        requestIndex++;
      }
      if(requestIndex == originalIndex){break;}
    }
  }
  printf("\n\n");
  printf("C-SCAN - Total head movements = %d\n\n", total_head_movements);
}


void LOOK_Algorithm(int position, int direction, int *sortedrequests)
{
    int total_head_movements = 0;
    int original_position = position;
    printf("LOOK SCHEDULING ALGORITHM\n\n");
    
    if (direction == DIRECTION_LEFT) {
        for (int i=TOTAL_REQUESTS - 1; i >= 0; --i)
            if (sortedrequests[i] <= original_position)
                seek(sortedrequests[i], &position, &total_head_movements);

        for (int i=0; i < TOTAL_REQUESTS; ++i)
            if (sortedrequests[i] > original_position)
                seek(sortedrequests[i], &position, &total_head_movements);
    }
    else {
        for (int i=0; i < TOTAL_REQUESTS; ++i)
            if (sortedrequests[i] >= original_position)
                seek(sortedrequests[i], &position, &total_head_movements);

        for (int i=TOTAL_REQUESTS - 1; i >= 0; --i)
            if (sortedrequests[i] < original_position)
                seek(sortedrequests[i], &position, &total_head_movements);
    }
    
  printf("\n\n");
  printf("LOOK - Total head movements = %d\n\n", total_head_movements);
}

void C_LOOK_Algorithm(int position, int direction, int *sortedrequests)
{
    int total_head_movements = 0;
    int original_position = position;
    printf("C-LOOK DISK SCHEDULING ALGORITHM\n\n");
    if (direction == DIRECTION_LEFT) {
        for (int i=TOTAL_REQUESTS - 1; i >= 0; --i)
            if (sortedrequests[i] <= original_position)
                seek(sortedrequests[i], &position, &total_head_movements);

        for (int i=TOTAL_REQUESTS - 1; i >= 0; --i)
            if (sortedrequests[i] > original_position)
                seek(sortedrequests[i], &position, &total_head_movements);
    }
    else {
        for (int i=0; i < TOTAL_REQUESTS; ++i)
            if (sortedrequests[i] >= original_position)
                seek(sortedrequests[i], &position, &total_head_movements);

        for (int i=0; i < TOTAL_REQUESTS; ++i)
            if (sortedrequests[i] < original_position)
                seek(sortedrequests[i], &position, &total_head_movements);
    }
    
    printf("\n\n");
    printf("C-LOOK - Total head movements = %d\n\n", total_head_movements);
}


int main(int argc, const char **argv)
{
    int position, direction;
    if (argc < 3) {
        // Default arguments (for testing purposes)
        position = 100;
        direction = DIRECTION_RIGHT;
    }
    else {
        position = atoi(argv[1]);
        if (strcmp(argv[1], "LEFT") == 0) {
            direction = DIRECTION_LEFT;
        }
        else if (strcmp(argv[1], "RIGHT") == 0) {
            direction = DIRECTION_RIGHT;
        }
        else {
            fprintf(stderr, "Invalid direction argument\n");
            exit(1);
        }
    }
    printf("Total requests = %d\n", TOTAL_REQUESTS);
    printf("Initial Head Position: %d\n", position);
    printf("Direction of Head: %s\n\n", direction == DIRECTION_LEFT? "LEFT" : "RIGHT");
    

    int requests[TOTAL_REQUESTS];
    FILE *f_requests = fopen("request.bin", "r");
    fread(requests, sizeof(int), TOTAL_REQUESTS, f_requests);
    fclose(f_requests);

    // Use the C standard library quicksort function to sort
    // the list of requests
    int sortedrequests[TOTAL_REQUESTS];
    memcpy(sortedrequests, requests, sizeof(int) * TOTAL_REQUESTS);
    qsort(sortedrequests, TOTAL_REQUESTS, sizeof(int), integer_compare);

    FCFS_Algorithm(position, direction, requests);
    SSTF_Algorithm(position, direction, requests);
    SCAN_Algorithm(position, direction, sortedrequests);
    C_SCAN_Algorithm(position, direction, sortedrequests);
    LOOK_Algorithm(position, direction, sortedrequests);
    C_LOOK_Algorithm(position, direction, sortedrequests);
}