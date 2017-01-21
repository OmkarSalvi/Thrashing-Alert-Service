#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG 0

///// stolen from http://stackoverflow.com/questions/3343797/is-this-c-implementation-of-fisher-yates-shuffle-correct

// Correct for modulo bias
static int rand_int(int n) {
  int limit = RAND_MAX - RAND_MAX % n;
  int rnd;

  do {
    rnd = rand();
  } while (rnd >= limit);
  return rnd % n;
}

void shuffle(int *array, int n) {
  int i, j, tmp;

  for (i = n - 1; i > 0; i--) {
    j = rand_int(i + 1);
    tmp = array[j];
    array[j] = array[i];
    array[i] = tmp;
  }
}

// allocates a vector and an array, one of which should be swapped and one should not
int main (int argc,char **argv)
{
    int numElements = 10;
    int *swappedArray;
    int *activeArray;
    int nextInd = 0;
    int i;

    if(argc >= 2){
        numElements = atoi(argv[1]) / sizeof(int) / 2;
    } else {
        printf("%s <numBytes>, will allow you to dictate how many bytes we should use.\
         Half will be swappable, the other half active\n",argv[0]);
    }

    swappedArray = malloc(numElements*sizeof(int));
    activeArray = malloc(numElements*sizeof(int));

    // initialize the array for shuffle
    for(i=0; i<numElements; i++){
        swappedArray[i] = i;
    }

    // fisher-yates shuffle array to get random positions
    shuffle(swappedArray, numElements);

    // string together the randomized numbers into a jump table
    for(i=0; i<numElements; i++){
        activeArray[swappedArray[i]] = swappedArray[(i+1)%numElements];
    }

#if DEBUG
    //debug: dump all array indexes
    for(i=0; i<numElements; i++){
        printf("%d, ", swappedArray[i]);
    }
    printf("\n\n");
    for(i=0; i<numElements; i++){
        printf("%d, ", activeArray[i]);
    }
    printf("\n");
#endif

    // jump forever through the activeArray, this should cause pages hit by this memory not to be swapped
    while(1){
        nextInd = activeArray[nextInd];

        // try not to kill the processor
        if(nextInd == 0){
            //printf("sleeping!\n");
            //sleep(1);
        }

    }
    
    free(swappedArray);
    free(activeArray);
}
