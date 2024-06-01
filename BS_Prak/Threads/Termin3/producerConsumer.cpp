#include <vector>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <semaphore>
#include <pthread.h>

std::vector<int> numList{};
std::counting_semaphore<1> listLock{1};

std::counting_semaphore<1> cntLock{1};
int cnt{};

bool checkPrime (int toCheck) {

    if(toCheck < 1) {
        return false;
    }

    for (int i = 2; i < toCheck-1; i++) {
        if((toCheck % i) == 0) {
            return false;
        }
    }
    return true;
}

void addNum() {
    srand(pthread_self());
    listLock.acquire();
    numList.insert(numList.begin(), rand());
    listLock.release();
}

void useNum() {
    int thisVal{};

    listLock.acquire();

    if (!numList.empty()) {
        thisVal = numList.back();
        numList.pop_back();
    }

    listLock.release();

    
    if(checkPrime(thisVal)) {
        std::cout << thisVal << std::endl;
        // cntLock.acquire();
        // cnt++;
        // cntLock.release();
    }
    
}

int main (int argc, char* argv[]) {

    if(argc != 3) {
        std::cout << "Usage:" << argv[0] << "(Number producers)" << "(Number consumers)" << std::endl;
        return 1;
    }

    int numProducers = atoi(argv[1]);
    int numConsumers = atoi(argv[2]);

    std::vector<std::thread> threads{};

    for (int i = 0; i < numProducers; i++) {
        threads.emplace_back(addNum);
    }
    for (int i = 0; i < numConsumers; i++) {
        threads.emplace_back(useNum);
    }
    for (auto& thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    return 0;
}