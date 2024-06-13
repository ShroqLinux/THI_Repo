#include <vector>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <semaphore>


std::vector<int> numList{};
std::counting_semaphore listLock{5};

std::counting_semaphore<1> cntLock{1};

std::counting_semaphore semFree(5);

std::counting_semaphore semStored(0);

int cnt{};
int listCnt{};

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

        /*
        cntLock.acquire();
        int currentCnt = cnt;
        cntLock.release();
        

        
        if (currentCnt >= max) {
            break;
        }
        */
        

        semFree.acquire();

        int newVal = rand() % 1000;
        numList.insert(numList.begin(), newVal);
        std::cout << "Producer: " << newVal << '\n';
        // std::cout << "List count: " << listCnt << '\n';

        semStored.release();
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return;
}

void useNum() {

    while (true) {
        semStored.acquire();

        int thisVal{};

        if (!numList.empty()) {
            int lastVal{};

            thisVal = numList.back();
            numList.pop_back();
            if(thisVal != lastVal)
                std::cout << "Consumer: " << thisVal << '\n';
            lastVal = thisVal;
            /*
            cntLock.acquire();
            cnt++;
            cntLock.release();
            */
        }
        semFree.release();


        if (cnt >= max && numList.empty()) {
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        /*
        if(checkPrime(thisVal)) {

            cntLock.acquire();
            if (cnt >= max) {
                cntLock.release();
                break;
            }

            // std::cout << thisVal << " is Prime" << std::endl;
            cntLock.release();
        
        }
        */
        
        
    }
    return;
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
        std::cout << "Producer Thread " << i << ": " << execTimes[i] << std::endl;
    }
    
    return 0;
}