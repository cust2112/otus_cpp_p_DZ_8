
#include <iostream>

#include "consolewriter.h"


void write2Console (const VecOfStr &bulk)
{
    std::cout << "bulk: ";

    for (size_t i = 0; i < bulk.size (); ++i) {

        if (i > 0)
            std::cout << ", ";

        std::cout << bulk[i];
    }

    std::cout << std::endl
              << std::flush
                 ;
}


void ConsoleWriter::threadFunc ()
{
    std::unique_lock <std::mutex> lock (mtx);

    do {

        cv.wait (lock, [this] () { return bulksQueue.size () > 0 || stop; });


        while (bulksQueue.size () > 0) {

            {
                SharedVecOfStr bulk = std::move (bulksQueue.front ());

                bulksQueue.pop ();
                lock.unlock ();

                write2Console (*bulk);
            }

            lock.lock ();
        }
    } while (!stop);
}


ConsoleWriter::ConsoleWriter ()
    : stop (false)
{
    t = std::thread (&ConsoleWriter::threadFunc, this);
}


ConsoleWriter::~ConsoleWriter ()
{
    std::unique_lock <std::mutex> lock (mtx);

    stop = true;

    lock.unlock ();

    cv.notify_all ();


    t.join ();
}


void ConsoleWriter::output (const SharedVecOfStr bulk, int)
{
    std::unique_lock <std::mutex> lock (mtx);

    bulksQueue.emplace (std::move (bulk));

    lock.unlock ();

    cv.notify_one ();
}
