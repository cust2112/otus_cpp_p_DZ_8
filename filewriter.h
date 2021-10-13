#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "bulkoutput.h"


class FileWriter : public BulkOutput {
public:
    FileWriter ();
    ~FileWriter ();

    void output (const SharedVecOfStr bulk, int) override;


private:
    typedef std::pair <const SharedVecOfStr, int> BulkWithTime;
    typedef std::queue <BulkWithTime>  BulksQueue;

    BulksQueue bulksQueue;

    std::condition_variable cv;
    std::mutex mtx;

    std::thread t1, t2;
    std::string fName1, fName2;
    static thread_local int prevBulkStartTime, prevFrac;

    bool stop;

    static void makeFileName (std::string &fName, int bulkStartTime);
    void threadFunc (std::string &fName);
};


#endif // FILEWRITER_H
