//Authors: Allison Hurley & Ben Kocik 
//Purpose: Multithreaded Cpp Program 

#include <iostream>    
#include <thread> 
#include <mutex>

std::mutex mutex;

// Replaces thread struct from c example code
class thread_node {
    public:
        thread_node(const int id, const int count, const int delay_value);
        void join() { handle.join(); }

        static void work(thread_node* node);
        static void build(const int id, const int count, const int delay_value);
        static bool wait_and_remove();

    private:
        const int id;                       // Thread ID
        const int count;                    // Number of times that each thread must print
        const int delay_value;              // Thread delay
        std::thread handle;                 // Cpp thread type (works on both windows and linux)
        thread_node* next;                  // Used for linked list, points to next item

        static thread_node *root;           // Points to the beginning of the list of threads
};

thread_node *thread_node::root = nullptr;

thread_node::thread_node(const int _id, const int _count, const int _delay_value) : 
    id(_id), count(_count), delay_value(_delay_value), handle(work, this) { }

//the printThread function from the example code 
void thread_node::work(thread_node* node) {
    //calculates delay
    auto delay = 100 * (node->id + 1) + (node->delay_value % 300) + 200;

    //
    for(auto i{node->count}; i; --i) {
        {
            //Locks the mutex when the variable comes into existence
            const std::lock_guard<std::mutex> lock(mutex);
            //ensures no interrupting prints
            std::cout << "Thread " << node->id << ": " << node->delay_value <<'\n';
        }
        //Sleep is a windows function - this works for both 
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}
// Creates thread, allocates memory, and puts into linked list
void thread_node::build(const int id, const int count, const int delay_value) {
    // new is a cpp tool that is comparable to the malloc in c but also allocates memory
    thread_node* new_node = new thread_node(id, count, delay_value);
    new_node->next = root;                  // adding to linkedlist
    root = new_node;                        //resassigning head of list
}
// Waits for threads to complete and then deletes the node
bool thread_node::wait_and_remove() {
    if (thread_node* node = root) {
        node->join();                       // WaitForSingleObject in C aka waits for the thread to end
        root = node->next;                  // Move to next thread in list
        delete node;                        // Deletes the thread
        
        return true;
    }

    return false;                           // Exits when all threads are complete
}

int main(int argc, char *argv[])
{	
    if (argc < 3) {
        std::cout << "You must provide at least 2 parameters:\nphase3 count delay-1 [delay-2 ... delay-n]\n";
        return -1;
    }

    int count = atoi(argv[1]);

    for (int id = 2; id < argc; ++id) {
        // Make sure numbers stay within range of 1 - 1000
        if (atoi(argv[id]) >= 1000) { thread_node::build(id - 2, count, 1000); }
        else if (atoi(argv[id]) <= 1) { thread_node::build(id - 2, count, 1); }
        else { thread_node::build(id - 2, count, atoi(argv[id])); }
        // I just want to point out I put an = in the if and else if because
        // if you think about it, that makes the program more efficient lol
    }

    while (thread_node::wait_and_remove()) { }

    return 0;
}

