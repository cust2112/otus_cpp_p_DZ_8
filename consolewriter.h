#ifndef CONSOLEWRITER_H
#define CONSOLEWRITER_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "bulkoutput.h"


class ConsoleWriter : public BulkOutput {
public:
    ConsoleWriter ();
    ~ConsoleWriter ();

    void output (const SharedVecOfStr bulk, int) override;


private:
    typedef std::queue <SharedVecOfStr>  BulksQueue;

    BulksQueue bulksQueue;

    std::condition_variable cv;
    std::mutex mtx;

    std::thread t;
    bool stop;

    void threadFunc ();
};


#endif // CONSOLEWRITER_H
