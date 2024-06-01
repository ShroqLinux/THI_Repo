#include <vector>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <semaphore>


std::vector<int> numList{};
std::counting_semaphore<1> listLock{1};

std::counting_semaphore<1> cntLock{1};
int cnt{};

int max{};

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
    while (true) {

        cntLock.acquire();
        int currentCnt = cnt;
        cntLock.release();

        if (currentCnt >= max) {
            break;
        }

        listLock.acquire();
        numList.insert(numList.begin(), rand());
        listLock.release();
    }
}

void useNum() {
    int thisVal{};

    while (true) {
        listLock.acquire();

        if (!numList.empty()) {
            thisVal = numList.back();
            numList.pop_back();
        }

        listLock.release();

        if(checkPrime(thisVal)) {

            cntLock.acquire();
            if (cnt >= max) {
                cntLock.release();
                break;
            }

            std::cout << thisVal << " is Prime" << std::endl;
            cnt++;
            cntLock.release();
        }
    }
}

int main (int argc, char* argv[]) {

    if(argc != 4) {
        std::cout << "Usage:" << argv[0] << "(Number producers)" << "(Number consumers)" << "(Max prime numbers)" << std::endl;
        return 1;
    }

    int numProducers = atoi(argv[1]);
    int numConsumers = atoi(argv[2]);
    max = atoi(argv[3]);

    std::vector<std::thread> threads{};
    std::vector<float> execTimes(numProducers);

    std::mutex execTimesMutex;

    for (int i = 0; i < numProducers; i++) {
        threads.emplace_back([&, i]() {
            struct timespec start, end;
            clock_gettime(CLOCK_REALTIME, &start);
            addNum();
            clock_gettime(CLOCK_REALTIME, &end);
            float execTime = (end.tv_sec - start.tv_sec) + 
                             (end.tv_nsec - start.tv_nsec) / 1000000000.0f;
            {
                std::lock_guard<std::mutex> lock(execTimesMutex);
                execTimes[i] = execTime;
            }
        });
    }

    for (int i = 0; i < numConsumers; i++) {
        threads.emplace_back(useNum);
    }

    for (auto& thread: threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    for(size_t i = 0; i < execTimes.size(); i++) {
        std::cout << "Thread " << i << ": " << execTimes[i] << std::endl;
    }
    
    return 0;
}