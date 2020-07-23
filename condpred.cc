// $ g++ -std=c++11 condpred.cc -pthread
// $ ./a.out

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

// A simple work container, intended to be shared between threads.
class WorkContainer {
    public:
    WorkContainer() : some_number_(0) { }
    std::condition_variable & cond() {
        return cond_;
    }

    std::mutex& mutex() {
        return m_;
    }

    int value() {
        return some_number_;
    }

    void set_value(int some_number) {
        some_number_ = some_number;
    }

    private:
        int some_number_;
        std::condition_variable cond_;
        std::mutex m_ // Synchronizes access to the above objects.
};

void Producer(WorkContainer *work) {
    std::chrono::milliseconds sleep_time(15);
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(sleep_time);

        printf("Setting value: %i\n", i);
        work->mutex().lock();
        work->set_value(i);
        work->mutex().unlock();

        // Notify a waiting thread.
        work->cond().notify_one(); //C++11 does not require the thread to have a busy mutex when notify_ * is called.
    }
}

void Consumer(WorkContainer *work) {
    // Wake up for the value 8 only.
    std::unique_lock<std::mutex> lock(work->mutex());
    work->cond().wait(lock, [&work](){
        // An anonymous function (lambda) that uses the parent variable work and checks the condition, returning true or false.
        return work-> value() == 8;
    });

    printf("Woke up at value: %i\n", work-> value());
}

int main() {
    // Create a container for all work-related objects and run the consumer and producer threads (in that order).
    WorkContainer work;
    std::thread con(Consumer, &work);
    std::thread pro(Producer, &work);

    // Wait for both threads to finish their work.
    pro.join();
    con.join();
    return 0;
}