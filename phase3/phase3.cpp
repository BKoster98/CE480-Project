#include <iostream>    
#include <thread> 
#include <mutex>

std::mutex mutex;

class thread_node {
    public:
        thread_node(const int id, const int count, const int delay_value);
        void join() { handle.join(); }

        static void work(thread_node* node);
        static void build(const int id, const int count, const int delay_value);
        static bool wait_and_remove();

    private:
        const int id;
        const int count;
        const int delay_value;
        std::thread handle;
        thread_node* next;

        static thread_node *root;
};

thread_node *thread_node::root = nullptr;

thread_node::thread_node(const int _id, const int _count, const int _delay_value) : 
    id(_id), count(_count), delay_value(_delay_value), handle(work, this) { }

void thread_node::work(thread_node* node) {
    auto delay = 100 * (node->id + 1) + (node->delay_value % 300) + 200;

    for(auto i{node->count}; i; --i) {
        {
            //Locks the mutex when the variable comes into existence
            const std::lock_guard<std::mutex> lock(mutex);
            //ensures no interrupting prints
            std::cout << "Thread " << node->id << '\n';
        }
        //Sleep is a windows function - this works for both 
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void thread_node::build(const int id, const int count, const int delay_value) {
    thread_node* new_node = new thread_node(id, count, delay_value);
    new_node->next = root;
    root = new_node;
}

bool thread_node::wait_and_remove() {
    if (thread_node* node = root) {
        node->join();
        root = node->next;
        delete node;
        
        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{	
    if (argc < 3) {
        std::cout << "You must provide at least 2 parameters:\nphase3 count delay-1 [delay-2 ... delay-n]\n";
        return -1;
    }

    int count = atoi(argv[1]);

    for (int id = 2; id < argc; ++id) {
        thread_node::build(id - 2, count, atoi(argv[id]));
    }

    while (thread_node::wait_and_remove()) { }

    return 0;
}

