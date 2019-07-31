//Copyright 2019, Bradley Peterson, Weber State University, all rights reserved. (7/2019)

#include <cstdio>
#include <random>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <sstream>


using std::cout;
using std::endl;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::string;
using std::stringstream;
using std::mutex;
using std::thread;

//*** Prototypes ***
void recQuickSort(vector<unsigned long>& arr, int first, int last);
void printList();
void pressAnyKeyToContinue();

class ManyBuckets;

//***GLOBAL VARIABLES***
bool useMultiThreading{ true }; //TODO: set this to true when doing the multithreaded part of the assignment
// and leave it as false for the singlethreaded assignment

unique_ptr<unsigned long[]> list;
unique_ptr<ManyBuckets> globalBuckets;

unsigned int listSize{ 0 };
unsigned int numBuckets{ 0 };
unsigned int currentWorkUnit{ 0 };
const unsigned long ULONGMAX = 4294967295;
// TODO for multithreading assignment: Create a mutex object
mutex theMutex;

class ManyBuckets {
public:
    ManyBuckets(const unsigned int numBuckets);
    ~ManyBuckets() {}
    void addItem(unsigned long item);
    //number of buckets
    unsigned int getNumBuckets() const;
    //number of items in bucket
    unsigned int getNumItemsInABucket(const unsigned int bucket) const;
    //gets specific buckets
    vector<unsigned long>& getBucket(const unsigned int bucket);
    //prints all buckets
    void printAllBuckets() const;
    
private:
    vector< vector<unsigned long> > arr;
    unsigned int buckets;
    
};

//*** Provide methods for the bucket class ***
ManyBuckets::ManyBuckets(const unsigned int numBuckets) {
    arr.resize(numBuckets);
    buckets = numBuckets;
    
}

unsigned int ManyBuckets::getNumBuckets() const {
    return buckets;
}

unsigned int ManyBuckets::getNumItemsInABucket(const unsigned int bucket) const {
    return arr[bucket].size();
}

void ManyBuckets::printAllBuckets() const {
    //Displays the contents of all buckets to the screen.
    
    // just uncomment this code when you have arr properly declared as a data member
    printf("******\n");
    for (unsigned int i = 0; i < numBuckets; i++) {
        printf("bucket number %d\n", i);
        for (unsigned int j = 0; j < arr[i].size(); j++) {
            printf("%08x ", arr[i][j]);
            
        }
        printf("\n");
    }
    printf("\n");
}

vector<unsigned long>& ManyBuckets::getBucket(const unsigned int bucket) {
    return arr[bucket];
}

void ManyBuckets::addItem(unsigned long item) {
    // TODO: Put the item in the appropraite bucket.
    if(numBuckets == 1){
        unsigned long numBuk = item/(ULONGMAX/numBuckets);
        arr[numBuk].push_back(item);
    }
    else{
        unsigned long numBuk = item/((ULONGMAX/numBuckets)+1);
        arr[numBuk].push_back(item);
    }
    
}

// TODO:  Loop over every item and call addItem
void placeIntoBuckets() {
    //loop through and add to buckets
    //loop to size of array
    //call globlaBuckets and additem(i)
    for(int i = 0; i < listSize; i++){
        globalBuckets->addItem(list[i]);
    }
}


// TODO: Modify so it has a parameter indicating what bucket to sort on
// then have the code sort on that particular bucket only.
void sortEachBucket(unsigned int numBuckets){
    
    int localWorkUnit = 0;
    while(true){
       
            theMutex.lock();
            localWorkUnit = currentWorkUnit;
            currentWorkUnit++;
            theMutex.unlock();
         if(localWorkUnit < numBuckets){
            recQuickSort(globalBuckets->getBucket(localWorkUnit), 0,globalBuckets->getNumItemsInABucket(localWorkUnit));
        }
        else{
            break;
        }
    }
    
    
}


//TODO: Copy each bucket back out to the original list[] array
//Loop for the amount of buckets
//Get the individual bucket out of the globalBuckets
//Copy all items from that individual bucket into the list array.
void combineBuckets() {
    
    
    //unsigned long j = 0;
    unsigned long k = 0;
    
    for(int i = 0; i < numBuckets; i++){
        for (long j = 0; j < globalBuckets->getBucket(i).size(); j++, k++){
            list[k] = globalBuckets->getBucket(i)[j];
        }
    }
}



void bucketSort(bool displayOutput, bool multiThreadingMode) {
    
    //For the upcoming homeowork assignment, I think it will help you the most to split your work into these three functions.
    placeIntoBuckets();
    
    if (displayOutput) {
        printf("Displaying each bucket's contents before sorting: \n");
        globalBuckets->printAllBuckets();
    }
    
    unsigned int numThreadsToUse{ 0 };
    currentWorkUnit = 0; //Reset the currentWorkUnit variable.
    
    if (multiThreadingMode) {
        //Find out how many threads are supported
        unsigned int threadsSupported = std::thread::hardware_concurrency();
        if (threadsSupported == 1 && numBuckets > 1) {
            numThreadsToUse = 2;
        }
        else if (numBuckets < threadsSupported) {
            numThreadsToUse = numBuckets;
        }
        else {
            numThreadsToUse = threadsSupported;
        }
        printf("Starting sort on %d threads\n", numThreadsToUse);
    }
    else {
        numThreadsToUse = 1;
    }
    
    // TODO FOR THE MULTITHREADED ASSIGNMENT (ignore this for the single threaded assignment.)
    // create threads and launch a function to get your mulithreaded work started.
    // That function should be responsible for calling sortEachBucket
    // Note that the above variable numThreadsToUse will have the exact number of threads
    // you should use.
    // Also, join the threads.
    //thread tracking object
    thread* threads = new thread[numThreadsToUse];
    
    //fork to create threads
    for (unsigned int i = 0; i < numThreadsToUse; i++){
        threads[i] = thread(sortEachBucket, currentWorkUnit);
    }
    
    //join threads
    for (unsigned int i = 0; i < numThreadsToUse; i++){
        threads[i].join();
    }
    delete[] threads;
    
    //sortEachBucket(numBuckets);
    
    combineBuckets();
    
    if (displayOutput) {
        printf("Displaying each bucket's contents after sorting: \n");
        globalBuckets->printAllBuckets();
        pressAnyKeyToContinue();
        printf("Displaying what is hopefully a sorted array: \n");
        printList(); //See if it's all sorted.
    }
    
    
}

//***Functions that help our bucket sort work.***
int partition(vector<unsigned long>& arr, int first, int last) {
    unsigned long pivot;
    int index, smallIndex;
    
    unsigned long temp;
    
    //Take the middle value as the pivot.
    //swap(first, (first + last) / 2);
    pivot = arr[first];
    smallIndex = first;
    for (index = first + 1; index < last; index++) {
        if (arr[index] < pivot) {
            smallIndex++;
            //swap the two
            temp = arr[smallIndex];
            arr[smallIndex] = arr[index];
            arr[index] = temp;
        }
    }
    
    //Move pivot into the sorted location
    temp = arr[first];
    arr[first] = arr[smallIndex];
    arr[smallIndex] = temp;
    
    //Tell where the pivot is
    return smallIndex;
    
}

void recQuickSort(vector<unsigned long>& arr, int first, int last) {
    //first is the first index
    //last is the one past the last index (or the size of the array
    //if first is 0)
    
    if (first < last) {
        //Get this sublist into two other sublists, one smaller and one bigger
        int pivotLocation = partition(arr, first, last);
        recQuickSort(arr, first, pivotLocation);
        recQuickSort(arr, pivotLocation + 1, last);
    }
}

void createList() {
    list = std::make_unique<unsigned long[]>(listSize);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long> dis(0, ULONGMAX);
    
    for (unsigned int i = 0; i < listSize; i++) {
        list[i] = dis(gen);
    }
}

void printList() {
    for (unsigned int i = 0; i < listSize; i++) {
        //cout << list[i] << " ";
        printf("%08x ", list[i]);
    }
    printf("\n");
}

void verifySort(unique_ptr<unsigned long[]>& arr, unsigned int arraySize, std::chrono::duration<double, std::milli>& diff, const string& sortTest) {
    double val = diff.count();
    for (unsigned int i = 1; i < arraySize; i++) {
        if (arr[i] < arr[i - 1]) {
            printf("------------------------------------------------------\n");
            printf("SORT TEST %s\n", sortTest.c_str());
            
            if (val != 0.0) {
                printf("Finished bucket sort in %1.16lf milliseconds\n", diff.count());
            }
            printf("ERROR - This list was not sorted correctly.  At index %d is value %08X.  At index %d is value %08X\n", i - 1, arr[i - 1], i, arr[i]);
            printf("------------------------------------------------------\n");
            
            
            return;
        }
    }
    printf("------------------------------------------------------\n");
    printf("SORT TEST %s\n", sortTest.c_str());
    if (val != 0.0) {
        printf("Finished bucket sort in %1.16lf milliseconds\n", diff.count());
    }
    printf("PASSED SORT TEST %s - The list was sorted correctly\n", sortTest.c_str());
    printf("------------------------------------------------------\n");
}


void pressAnyKeyToContinue() {
    printf("Press Enter key to continue\n");
    std::cin.get();
    
}

int main() {
    
    std::chrono::duration<double, std::milli> diff{ 0 };
    double baselineTime{ 9999999.0 };
    double bestMultiThreadedTime{ 9999999.0 };
    int bestMultiThreadedBuckets{ 0 };
    double bestSingleThreadedTime{ 9999999.0 };
    int bestSingleThreadedBuckets{ 0 };
    
    
    //Set the listSize, numBuckets, and numThreads global variables.
    listSize = 100;
    
    if (useMultiThreading) {
        unsigned int threadsSupported = std::thread::hardware_concurrency();
        if (threadsSupported == 1) {
            printf("You are on a machine that only supports 1 thread!\nThat is too bad, you won't see anything exciting in this assignment regarding better timings.\nSimulating 2 threads instead");
        }
        else {
            printf("Your machine supports up to %d threads\n", threadsSupported);
        }
    }
    
    numBuckets = 2;
    createList();
    globalBuckets = std::make_unique<ManyBuckets>(numBuckets);
    printf("\nStarting bucket sort for listSize = %d, numBuckets = %d\n", listSize, numBuckets);
    printf("Displaying the unsorted list array:\n");
    
    printList(); //useful for debugging small amounts of numbers.
    pressAnyKeyToContinue();
    bucketSort(true, true);
    verifySort(list, listSize, diff, "2 buckets");
    pressAnyKeyToContinue();
    
    
    numBuckets = 4;
    createList();
    printf("\nStarting bucket sort for listSize = %d, numBuckets = %d\n", listSize, numBuckets);
    globalBuckets = std::make_unique<ManyBuckets>(numBuckets);
    pressAnyKeyToContinue();
    bucketSort(true, true);
    verifySort(list, listSize, diff, "4 buckets");
    pressAnyKeyToContinue();
    
    printf("\n\n*** Note, remember to run on RELEASE mode, not DEBUG mode.  Release mode is MUCH faster. *** \n\n");
    pressAnyKeyToContinue();
    
    listSize = 4000000;
    numBuckets = 1;
    createList();
    globalBuckets = std::make_unique<ManyBuckets>(numBuckets);
    auto start = std::chrono::high_resolution_clock::now();
    bucketSort(false, false);
    auto end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    baselineTime = diff.count();
    verifySort(list, listSize, diff, "4000000 items in 1 bucket with 1 thread - BASELINE");
    
    bool multiThreadingMode{ true };
    int loopTo = 0;
    if (useMultiThreading) {
        loopTo = 1;
    }
    for (int i = 0; i <= loopTo; i++) {
        
        if (multiThreadingMode) {
            printf("Starting multithreading tests\n");
            pressAnyKeyToContinue();
        }
        else {
            printf("Starting singlethreaded tests\n");
            pressAnyKeyToContinue();
        }
        for (numBuckets = 2; numBuckets <= 1024; numBuckets *= 2) {
            listSize = 4000000;
            createList();
            globalBuckets = std::make_unique<ManyBuckets>(numBuckets);
            start = std::chrono::high_resolution_clock::now();
            bucketSort(false, multiThreadingMode);
            end = std::chrono::high_resolution_clock::now();
            diff = end - start;
            if (multiThreadingMode && (diff.count() < bestMultiThreadedTime)) {
                bestMultiThreadedTime = diff.count();
                bestMultiThreadedBuckets = numBuckets;
            }
            else if (!multiThreadingMode && (diff.count() < bestSingleThreadedTime)) {
                bestSingleThreadedTime = diff.count();
                bestSingleThreadedBuckets = numBuckets;
            }
            stringstream ss;
            ss << listSize << " items in " << numBuckets << " buckets";
            verifySort(list, listSize, diff, ss.str());
        }
        
        multiThreadingMode = !multiThreadingMode; //Flip it from false to true.
    }
    printf("\n-----------------------------------------------------------\n");
    printf("              FINAL RESULTS                      \n");
    printf("The baseline (quicksort on 1 thread/1 bucket):  completed in %g ms\n", baselineTime);
    printf("The best singlethreaded result:     %d buckets completed in %g ms\n", bestSingleThreadedBuckets, bestSingleThreadedTime);
    if (useMultiThreading) {
        printf("The best multithreaded result:      %d buckets completed in %g ms\n", bestMultiThreadedBuckets, bestMultiThreadedTime);
    }
    printf("\n-----------------------------------------------------------\n");
    
    pressAnyKeyToContinue();
    return 0;
}
