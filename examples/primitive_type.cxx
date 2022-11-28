#include <metaqueue.hpp>

int main(){
    //A primitive datatype
    int my_favourite_int = 1993;

    //Creating the Queue, the argument is the name of the queue.
    metaqueue<int> myqueue("myQueue");

    //Enqueue the data into a single message and sent it to the queue.
    myqueue.enqueue(my_favourite_int);

    //Recover the value from the queue.
    auto my_dequeued_value = myqueue.dequeue();

    //Print the result
    std::cout << my_dequeued_value << std::endl;

    return 0;
}