#include <metaqueue.hpp>

struct mySimpleStruct{
    int id;
    char name[32];
    char value;

    void set(int _id, std::string _name, char _value){
        id=_id;
        std::memset(name, 0, sizeof(name));
        std::memcpy(name, _name.c_str(), _name.size());
        value=_value;
    }

    void print(){
        std::cout << "id:" << id << std::endl;
        std::cout << "name:" << name << std::endl;
        std::cout << "another_important_value:" << value << std::endl;
    }
};

int main(){
    //A simple structure(No custom constructor, neither destructor, no pointers)
    mySimpleStruct my_struct;
    my_struct.set(8, "Your Name Here", 'R');

    //Creating the Queue, the argument is the name of the queue.
    metaqueue<mySimpleStruct> myqueue("myQueueStr");

    //Enqueue the data into a single message and sent it to the queue.
    myqueue.enqueue(my_struct);

    //Recover the value from the queue.
    auto my_dequeued_value = myqueue.dequeue();

    //Print the result
    my_dequeued_value.print();

    return 0;
}