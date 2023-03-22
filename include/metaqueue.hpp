/**
 * @file metaqueue.hpp
 * @author Eric Octavio Rodriguez Garcia (eric.rodriguezg@elektra.com.mx)
 * @brief This program will use the POSIX queue to enqueue and dequeue messages and wrap
 * them up to the datatype given in the template. Choises are made when the program is
 * compiling depending on the datatype or structure given in the template.
 * @version 0.0.1
 * @date 2022-11-25
 *
 * @copyright Copyright (Eric Octavio Rodriguez Garcia) GPL - 2022.
 *
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string>
#include <exception>
#include <stdexcept>
#include <bitset>
#include <climits>
#include <cstring>
#include <iostream>
#include <errno.h>


#ifndef METAQUEUE_DEFAULT_QUEUE_PERMISSION 
    #define METAQUEUE_DEFAULT_QUEUE_PERMISSION 0660 //> Default queue user permission, User and Group can Read(4) + Write(2).
#endif

#ifndef METAQUEUE_DEFAULT_MAX_MESSAGES 
    #define METAQUEUE_DEFAULT_MAX_MESSAGES 10       //> Maximum number of enqueued messages.
#endif

#ifndef METAQUEUE_DEFAULT_MAX_MESSAGE_SIZE 
    #define METAQUEUE_DEFAULT_MAX_MESSAGE_SIZE 2048 //> Maximum size of a single message.
#endif

#ifndef METAQUEUE_DEFAULT_QUEUE_FLAGS 
    #define METAQUEUE_DEFAULT_QUEUE_FLAGS 0         //> Extra Queue flags
#endif

#ifndef EOK
    #define EOK 0 //> No error!
#endif

/**
 * @brief This namespace contains the set of metafunctions dedicated to queue operations.
 *
 */
namespace QueueMetafunctions
{
    /**
     * @brief
     *
     * @tparam T Datatype which the queue will be storing.
     */
    template <typename T>
    struct get_datatype
    {
        typedef typename std::conditional<
            std::is_same<
                T,
                std::void_t<>>::value,
            std::string, // True
            T>::type     // False
            type;
    };

    /**
     * @brief Create a error string.
     *
     * @param errvalue error number identifier.
     * @return std::string Error description.
     */
    inline std::string create_error(int errvalue)
    {
        char buffer[192] = {0};
        snprintf(buffer, sizeof(buffer), "The error generated was %d That means:%s.", errvalue, strerror(errvalue));
        return std::string(buffer);
    }

    /**
     * @brief Create a timeout object.
     *
     * @param n Seconds of expiracy time.
     * @return timespec object with the seconds set.
     */
    static timespec timeout(int n)
    {
        struct timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += n;
        return tm;
    }

    /**
     * @brief This metafunction will create the data object depending if its a complex class or a simple class(Structure, Primitive types, Scalars).
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam value_size size in bytes sizeof T.
     * @tparam can_be_memcpyed type attribute, can the object be used with memcpy.
     * @tparam is_trivial type attribute, is the object a simple structure which can be used with memcpy too.
     */
    template <typename T, size_t value_size, bool can_be_memcpyed, bool is_trivial>
    struct data_builder
    {
        /**
         * @brief This method will execute the data_builder metafunction.
         *
         * @param buffer char* Buffer to store the data temporary.
         * @param nbytes int, number of bytes read from queue.
         * @return T Instantied data type with the content of the buffer.
         */
        static T create(char *buffer, int nbytes)
        {
            throw std::runtime_error("Imposible to execute, no specialization for this struct: template <typename T, size_t value_size, bool can_be_memcpyed, bool is_trivial> struct data_builder;");
        }
    };

    /**
     * @brief This metafunction will create the data to a simple class(Structure, Primitive types, Scalars).
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam value_size size in bytes sizeof T.
     */
    template <typename T, size_t value_size>
    struct data_builder<T, value_size, true, true>
    {
        /**
         * @brief This method will execute the data_builder metafunction.
         *
         * @param buffer char* Buffer to store the data temporary.
         * @param nbytes int, number of bytes read from queue.
         * @return T Instantied data type with the content of the buffer.
         */
        static T create(char *buffer, int nbytes)
        {
            if (nbytes != value_size)
            {
                std::string error_str("Invalid message size, expected ");
                error_str += value_size;
                error_str += " But received: ";
                error_str += nbytes;
                throw std::runtime_error(error_str);
            }

            T data;
            std::memcpy(&data, buffer, value_size);
            return data;
        }
    };

    /**
     * @brief This metafunction will create a complex data object, the class must have the constructor Obj(const char* data, size_t size) in order to work.
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam value_size size in bytes sizeof T.
     */
    template <typename T, size_t value_size>
    struct data_builder<T, value_size, true, false>
    {
        /**
         * @brief This method will execute the data_builder metafunction.
         *
         * @param buffer char* Buffer to store the data temporary.
         * @param nbytes int, number of bytes read from queue.
         * @return T Instantied data type with the content of the buffer.
         */
        static T create(char *buffer, int nbytes)
        {
            return T(buffer, nbytes);
        }
    };

    /**
     * @brief This metafunction will send the data to the queue, depending of the datatype T, it will copy the information in a different way.
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam can_be_memcpyed type attribute, can the object be used with memcpy.
     * @tparam is_trivial type attribute, is the object a simple structure which can be used with memcpy too.
     */
    template <typename T, bool can_be_memcpyed, bool is_trivial>
    struct push
    {
        typedef typename std::add_lvalue_reference_t<T>::type value_ref; //> Add the datatype a reference,(T=>T&, T&=>T&, T&&=>T&).

        /**
         * @brief Execute the metafunction push, it must use the other specializations in order to work.
         *
         * @param queue_fd Queue file descriptor.
         * @param data Reference to the data.
         * @param priority Integer priority.
         */
        static void run(mqd_t queue_fd, value_ref data, int priority)
        {
            throw std::runtime_error("Imposible to execute, no specialization for this struct: template<typename T, bool can_be_memcpyed, bool is_class> struct push");
        }
    };

    /**
     * @brief This metafunction will send the data to the queue, depending of the datatype T, it will copy the information in a different way. This is supposed to be a complex class, this object must hace the .data() and .size() methods in order to work.
     *
     * @tparam T Datatype which the queue will be working with.
     */
    template <typename T>
    struct push<T, true, false> // Is memcpyable but its not trivial(simple structure).
    {
        typedef typename std::add_lvalue_reference_t<T> value_ref;      //> Add the datatype a reference,(T=>T&, T&=>T&, T&&=>T&).
        typedef typename std::remove_reference_t<value_ref> value_type; //> Remove the reference so we get the pure datatype.

        /**
         * @brief This method will execute the push metafunction.
         *
         * @param queue_fd Queue file descriptor.
         * @param data Reference to the data.
         * @param priority Integer priority.
         */
        static void run(mqd_t queue_fd, value_ref data, int priority)
        {
            errno = EOK;
            int nbytes = mq_send(queue_fd, data.data(), data.size(), priority);
            if (nbytes < 0)
            {
                throw std::runtime_error(create_error(errno));
            }
            return;
        }
    };

    /**
     * @brief This metafunction will send the data to the queue, depending of the datatype T, it will copy the information in a different way. This is supposed to be a simple class, it can be used with memcpy and sizeof, raw bytes copy.
     *
     * @tparam T Datatype which the queue will be working with.
     */
    template <typename T>
    struct push<T, true, true>
    {
        typedef typename std::add_lvalue_reference<T>::type value_ref;      //> Add the datatype a reference,(T=>T&, T&=>T&, T&&=>T&).
        typedef typename std::remove_reference<value_ref>::type value_type; //> Remove the reference so we get the pure datatype.
        static const constexpr size_t value_size = sizeof(value_type);      //> Size in bytes of the structure or primitive data type.

        /**
         * @brief This method will execute the push metafunction.
         *
         * @param queue_fd Queue file descriptor.
         * @param data Reference to the data.
         * @param priority Integer priority.
         */
        static void run(mqd_t queue_fd, value_ref data, int priority)
        {
            errno = EOK;
            int nbytes = mq_send(queue_fd, (const char *)&data, value_size, priority);
            if (nbytes < 0)
            {
                throw std::runtime_error(create_error(errno));
            }
            return;
        }
    };

    /**
     * @brief This metafunction will get the message from the queue and convert it to the datatype T, the method can be wait until a message arrives or wait for a set of seconds.
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam value_size sizeof T
     * @tparam can_be_memcpyed type attribute, can the object be used with memcpy.
     * @tparam is_trivial type attribute, is the object a simple structure which can be used with memcpy too.
     */
    template <typename T, size_t value_size, bool can_be_memcpyed, bool is_trivial>
    struct pop_impl
    {

        /**
         * @brief This method will wait until a new message can be read from the queue, the seconds given are the wait time, if no message arrives between the moment the method is executed (now) and(+) the timeout_seconds parameter, the function will return an empty instance of the object and a false value in the second return value pair.
         *
         * @param queue_fd Queue file descriptor.
         * @param buffer Pointer to the buffer where the data will be stored when reading the queue.
         * @param buffer_size sizeof the buffer.
         * @param timeout_seconds Integer value which represents for how long the method will wait for a new message to arrive.
         * @return std::pair<T, bool> New instance of the datatype T and a bool value which indicates if there was an error while reading the message or timeout reached.
         */
        static std::pair<T, bool> timed(mqd_t queue_fd, char *buffer, size_t buffer_size, int timeout_seconds)
        {
            int nbytes;
            auto tm = QueueMetafunctions::timeout(timeout_seconds);
            if ((nbytes = mq_timedreceive(queue_fd, buffer, buffer_size, NULL, &tm)) > 0)
            {
                if (nbytes <= 0)
                {
                    throw std::runtime_error(create_error(errno));
                }
                return std::make_pair(data_builder<T, value_size, can_be_memcpyed, is_trivial>::create(buffer, nbytes), true);
            }
            return std::make_pair(T(), false);
        }

        /**
         * @brief This method will wait until a new message is received.
         *
         * @param queue_fd Queue file descriptor.
         * @param buffer Pointer to the buffer where the data will be stored when reading the queue.
         * @param buffer_size sizeof the buffer.
         * @param priority Integer priority.
         * @return std::pair<T, bool> New instance of the datatype T and a bool value which indicates if there was an error while reading the message.
         */
        static std::pair<T, bool> wait(mqd_t queue_fd, char *buffer, size_t buffer_size, unsigned int *priority)
        {
            int nbytes;
            if ((nbytes = mq_receive(queue_fd, buffer, buffer_size, priority)) > 0)
            {
                if (nbytes < 0)
                {
                    throw std::runtime_error(create_error(errno));
                }
                return std::make_pair(data_builder<T, value_size, can_be_memcpyed, is_trivial>::create(buffer, nbytes), true);
            }
            return std::make_pair(T(), false);
        }
    };

    /**
     * @brief This metafunction will remove a message from the queue.
     *
     * @tparam T Datatype which the queue will be working with.
     * @tparam can_be_memcpyed type attribute, can the object be used with memcpy.
     * @tparam is_trivial type attribute, is the object a simple structure which can be used with memcpy too.
     */
    template <typename T, bool can_be_memcpyed, bool is_trivial>
    struct pop
    {
        /**
         * @brief Execute the pop metafunction.
         *
         * @param queue_fd Queue file descriptor.
         * @param buffer Pointer to the buffer where the data will be stored when reading the queue.
         * @param buffer_size sizeof the buffer.
         * @param timeout_seconds Integer value which represents for how long the method will wait for a new message to arrive.
         * @param priority Integer priority.
         * @return std::pair<T, bool> New instance of the datatype T and a bool value which indicates if there was an error while reading the message or timeout reached.
         */
        static std::pair<T, bool> run(mqd_t queue_fd, char *buffer, size_t buffer_size, int timeout_seconds, int *priority)
        {
            throw std::runtime_error("Imposible to execute, no specialization for this struct: template<typename T, bool can_be_memcpyed, bool is_class> struct pop");
        }
    };

    /**
     * @brief This metafunction will remove a message from the queue.
     *
     * @tparam T Datatype which the queue will be working with.
     */
    template <typename T>
    struct pop<T, true, true>
    {
        typedef typename std::add_lvalue_reference_t<T> value_ref;
        typedef typename std::remove_reference_t<value_ref> value_type;
        static const constexpr size_t value_size = sizeof(value_type);

        /**
         * @brief Execute the pop metafunction.
         *
         * @param queue_fd Queue file descriptor.
         * @param buffer Pointer to the buffer where the data will be stored when reading the queue.
         * @param buffer_size sizeof the buffer.
         * @param timeout_seconds Integer value which represents for how long the method will wait for a new message to arrive.
         * @param priority Integer priority.
         * @return std::pair<value_type, bool> New instance of the datatype T and a bool value which indicates if there was an error while reading the message or timeout reached.
         */
        static std::pair<value_type, bool> run(mqd_t queue_fd, char *buffer, size_t buffer_size, int timeout_seconds, unsigned int *priority)
        {
            int nbytes;
            if (timeout_seconds == -1)
            {
                return pop_impl<value_type, value_size, true, true>::wait(queue_fd, buffer, buffer_size, priority);
            }
            else
            {
                return pop_impl<value_type, value_size, true, true>::timed(queue_fd, buffer, buffer_size, timeout_seconds);
            }
            return std::make_pair(value_type(), false);
        }
    };

    /**
     * @brief This metafunction will remove a message from the queue.
     *
     * @tparam T Datatype which the queue will be working with.
     */
    template <typename T>
    struct pop<T, true, false>
    {
        typedef typename std::add_lvalue_reference_t<T> value_ref;
        typedef typename std::remove_reference_t<value_ref> value_type;
        static const constexpr size_t value_size = sizeof(value_type);

        /**
         * @brief Execute the pop metafunction.
         *
         * @param queue_fd Queue file descriptor.
         * @param buffer Pointer to the buffer where the data will be stored when reading the queue.
         * @param buffer_size sizeof in bytes of the buffer.
         * @param timeout_seconds  Integer value which represents for how long the method will wait for a new message to arrive.
         * @param priority Integer priority.
         * @return std::pair<value_type, bool> New instance of the datatype T and a bool value which indicates if there was an error while reading the message or timeout reached.
         */
        static std::pair<value_type, bool> run(mqd_t queue_fd, char *buffer, size_t buffer_size, int timeout_seconds, unsigned int *priority)
        {
            int nbytes;
            if (timeout_seconds == -1)
            {
                return pop_impl<value_type, value_size, true, false>::wait(queue_fd, buffer, buffer_size, priority);
            }
            else
            {
                return pop_impl<value_type, value_size, true, false>::timed(queue_fd, buffer, buffer_size, timeout_seconds);
            }
            return std::make_pair(value_type(), false);
        }
    };

}; // namespace QueueMetafunctions

/**
 * @brief This class will simplify the way you can send and receive messages from the OS Queue System, default driver is POSIX MQueue.
 *
 * @tparam T Datatype which the queue will be working with.
 * @tparam QueuePermission Permission of the queue, default 0660, User,Group(Read+Write)
 * @tparam MaxMessages Max number of enqueued messages, default 10.
 * @tparam MaxMessageSize Max size of the message in bytes.
 * @tparam QueueFlags Extra Queue flags.
 */
template <typename T = std::void_t<>,
          int QueuePermission = METAQUEUE_DEFAULT_QUEUE_PERMISSION,
          int MaxMessages = METAQUEUE_DEFAULT_MAX_MESSAGES,
          int MaxMessageSize = METAQUEUE_DEFAULT_MAX_MESSAGE_SIZE,
          int QueueFlags = METAQUEUE_DEFAULT_QUEUE_FLAGS>
class metaqueue
{
    using type = metaqueue<T, QueuePermission, MaxMessages, MaxMessageSize, QueueFlags>; //> Current meta type.
    typedef typename QueueMetafunctions::get_datatype<T>::type value_type;               //> Value type depending on the input.
    typedef typename std::add_lvalue_reference<value_type>::type value_ref;              //> Safe Reference type of the datatype given.
    static const constexpr bool is_memcpyed = std::is_standard_layout<T>::value;         //> Bool which indicates if the object can be memcpied.
    static const constexpr bool is_trivial = std::is_trivial<T>::value;                  //> Check if the datatype is trivial(Simple structure).

private:
    bool dequeued_message;       //> Boolean which indicates if the message could be read from the queue.
    mqd_t queue_fd;              //> Queue file descriptor.
    struct mq_attr attr;         //> Attributes of the queue.
    std::string mailbox_name;    //> Name of the Queue.
    char buffer[MaxMessageSize]; //> Raw buffer where the bytes will be stored while doing queue.

    /**
     * @brief This method will set the buffer to zeros.
     *
     */
    void clean_buffer()
    {
        std::memset(buffer, 0, MaxMessageSize);
    }

    /**
     * @brief This method will crean all variables of the class.
     *
     */
    void clean()
    {
        std::memset(&queue_fd, 0, sizeof(mqd_t));
        std::memset(&attr, 0, sizeof(mq_attr));
        clean_buffer();
    }

    /**
     * @brief Set the name object
     *
     * @param _mailbox_name Name of the Queue.
     */
    void set_name(const char *_mailbox_name)
    {
        if (*_mailbox_name != '/')
        {
            mailbox_name = "/";
            mailbox_name += _mailbox_name;
        }
        else
        {
            mailbox_name = _mailbox_name;
        }
    }

    /**
     * @brief This method will open the queue and assing the file descriptor.
     *
     */
    void init()
    {
        attr.mq_flags = QueueFlags;
        attr.mq_maxmsg = MaxMessages;
        attr.mq_msgsize = MaxMessageSize;
        attr.mq_curmsgs = EOK;

        if ((queue_fd = mq_open(mailbox_name.c_str(), O_RDWR | O_CREAT, QueuePermission, &attr)) == -1)
        {
            perror("Server: mq_open (server)");
            throw std::runtime_error("Error while trying to open the queue:" + mailbox_name);
        }
    }

public:
    /**
     * @brief Construct a new metaqueue object
     *
     * @param queue_name Name of the queue.
     */
    metaqueue(std::string queue_name)
    {
        set_name(queue_name.c_str());
        clean();
        init();
    }

    /**
     * @brief Destroy the metaqueue object.
     *
     */
    ~metaqueue()
    {
        if (mq_close(queue_fd) != 0)
        {
            std::cerr << QueueMetafunctions::create_error(errno) << std::endl;
        }
    }

    /**
     * @brief This method will return the status of the dequeue operation.
     *
     * @return true The message was succesfully dequeued and converted.
     * @return false A problem ocurred while reading from queue or timeout reached.
     */
    bool was_dequeued()
    {
        return dequeued_message;
    }

    /**
     * @brief This method will try to enqueue a message to the queue.
     *
     * @param data Data reference which will be stored in the queue.
     * @param priority Priority of the message.
     */
    void push(value_ref data, unsigned int priority = 0)
    {
        try
        {
            if (mq_notify(queue_fd, NULL) == -1)
            {
                throw std::runtime_error(QueueMetafunctions::create_error(errno));
            }
            return QueueMetafunctions::push<value_type, is_memcpyed, is_trivial>::run(queue_fd, data, priority);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    /**
     * @brief This method will try to dequeue a message from the queue.
     *
     * @param timeout If timeout is set to -1(default) then the method will wait until a message arrives otherwise if the value is no negative, then it will wait maximum int timeout seconds and the method will return.
     * @param priority Priority of the message.
     * @return value_type Returns the datatype given in the template, the value is valid if and only if the method was_dequeued() returns true.
     */
    value_type pop(int timeout = -1, unsigned int priority = 0)
    {
        try
        {
            clean_buffer();
            dequeued_message = false;
            auto response = QueueMetafunctions::pop<value_type, is_memcpyed, is_trivial>::run(queue_fd, buffer, MaxMessageSize, timeout, &priority);
            dequeued_message = response.second;
            return response.first;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        return {};
    }

    /**
     * @brief This method will try to enqueue a message to the queue.
     *
     * @param data Data reference which will be stored in the queue.
     * @param priority Priority of the message.
     */
    void enqueue(value_ref data, unsigned int priority = 0)
    {
        return push(data, priority);
    }

    /**
     * @brief This method will try to dequeue a message from the queue.
     *
     * @param timeout If timeout is set to -1(default) then the method will wait until a message arrives otherwise if the value is no negative, then it will wait maximum int timeout seconds and the method will return.
     * @param priority Priority of the message.
     * @return value_type Returns the datatype given in the template, the value is valid if and only if the method was_dequeued() returns true.
     */
    value_type dequeue(int timeout = -1, unsigned int priority = 0)
    {
        return pop(timeout, priority);
    }

    /**
     * @brief This method will try to enqueue a message to the queue.
     *
     * @param data Data reference which will be stored in the queue.
     * @param priority Priority of the message.
     */
    void write(value_ref data, unsigned int priority = 0)
    {
        return push(data, priority);
    }

    /**
     * @brief This method will try to dequeue a message from the queue.
     *
     * @param timeout If timeout is set to -1(default) then the method will wait until a message arrives otherwise if the value is no negative, then it will wait maximum int timeout seconds and the method will return.
     * @param priority Priority of the message.
     * @return value_type Returns the datatype given in the template, the value is valid if and only if the method was_dequeued() returns true.
     */
    value_type read(int timeout = -1, unsigned int priority = 0)
    {
        return pop(timeout, priority);
    }

    /**
     * @brief This method will count how many messages are in the current queue.
     * 
     * @return long Number of messages in the current queue.
     */
    long count()
    {
        try
        {
            if (mq_getattr(queue_fd, &attr) == -1)
            {
                throw std::runtime_error(QueueMetafunctions::create_error(errno));
            }
            return attr.mq_curmsgs;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        return -1;
    }

    /**
     * @brief This method will count how many messages are in the selected queue.
     * 
     * @param _queue_name Name of the queue to count.
     * @return long Number of messages in the selected queue.
     */
    static long count(std::string _queue_name)
    {
        try
        {
            std::string queue_name = "/" + _queue_name;
            mqd_t queue_fd;
            struct mq_attr attr;
            if ((queue_fd = mq_open(queue_name.c_str(), O_RDONLY)) == -1)
            {
                perror("Server: mq_open (server)");
                throw std::runtime_error("Error while trying to open the queue:" + queue_name + "\n" + QueueMetafunctions::create_error(errno));
            }

            if (mq_getattr(queue_fd, &attr) == -1)
            {
                if (mq_close(queue_fd) != 0)
                {
                    std::cerr << QueueMetafunctions::create_error(errno) << std::endl;
                }
                throw std::runtime_error(QueueMetafunctions::create_error(errno));
            }

            if (mq_close(queue_fd) != 0)
            {
                std::cerr << QueueMetafunctions::create_error(errno) << std::endl;
            }
            return attr.mq_curmsgs;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        return -1;
    }

    /**
     * This method will destroy the current queue. Carefull with this method, if the queue is destroyed the messages will too.
    */
    void unlink()
    {
        try
        {
            if (mq_unlink(mailbox_name.c_str()) != 0)
            {
                throw std::runtime_error(QueueMetafunctions::create_error(errno));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
};
