#include <metaqueue.hpp>
#include <string>

int main(){
    //A std string
    std::string my_favourite_string("Hello World From Mexico!");

    //Creating the Queue, the argument is the name of the queue.
    metaqueue<std::string> myqueue("myQueueStr");

    //Enqueue the data into a single message and sent it to the queue.
    myqueue.enqueue(my_favourite_string);

    //Recover the value from the queue.
    auto my_dequeued_value = myqueue.dequeue();

    //Print the result
    std::cout << my_dequeued_value << std::endl;

    return 0;
}