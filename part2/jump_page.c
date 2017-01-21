#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG 0
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

unsigned long msSinceEpoch()
{
  struct timeval time;

  gettimeofday(&time, NULL);

  return time.tv_sec * 1000 + time.tv_usec / 1000;
}

// allocates a vector and an array, one of which should be swapped and one should not
int main (int argc,char **argv)
{
    int numElements = 10;
    int *activeArray;
    int nextInd = 0;
    unsigned long i, numSteps=0;
    long msBetweenLoops = 1000;
    unsigned long elementsPerPage = getpagesize() / sizeof(int);
    unsigned long prevTime, currTime;

    if(argc >= 2){
        numElements = atoi(argv[1]) * elementsPerPage;
    } else {
        printf("%s <numBytes>, will allow you to dictate how many PAGES we should use.\
         ALL will be jumped through\n",argv[0]);
    }

    activeArray = malloc(numElements*sizeof(int));

    printf("Initializing an array of size %d bytes\n", (numElements*sizeof(int)));

    // Initialize the array, every element points to the same offset in the next page
    //  the final page points back to the start
    for(i=0; i<numElements; i++){
        activeArray[i] = (i+elementsPerPage);//(i + elementsPerPage) % (numElements - elementsPerPage);
    }

#if DEBUG
    //debug: dump all array indexes
    printf("\n\n");
    for(i=0; i<numElements; i++){
        printf("%d, ", activeArray[i]);
    }
    printf("\n");
#endif

    prevTime = msSinceEpoch();
    // jump forever through the activeArray, this should cause pages hit by this memory not to be swapped
    while(1){

//printf("on element %08x\n",nextInd);
        nextInd = activeArray[nextInd];
        numSteps++;
        // try not to kill the processor
        if(nextInd == 0){
            currTime = msSinceEpoch();
            printf("Completed a loop after %lu steps, in %lu ms Sleeping for %lu ms.\n", numSteps, currTime-prevTime, msBetweenLoops - (currTime-prevTime));

            numSteps = 0;
            usleep((msBetweenLoops - MIN(msBetweenLoops, (currTime-prevTime))) * 1000);
            prevTime = msSinceEpoch();
        }
        if(nextInd > numElements) nextInd = 0; // guard against memory corruption
    }

    free(activeArray);
}
