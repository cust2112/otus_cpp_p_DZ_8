
#include <functional>

#include <cstdio>
#include <fstream>
#include <iostream>

#include "bulkcontext.h"
#include "filewriter.h"

//#define TRACE_FW


#ifdef TRACE_FW
struct TraceMakeFileName
{
    char c;
    int prevBulkStartTime, prevFrac;
    int bulkStartTime;

    TraceMakeFileName ()
        : c ('\0'), prevBulkStartTime (0), prevFrac (0), bulkStartTime (0) {}
    TraceMakeFileName (char c, int prevBulkStartTime, int prevFrac, int bulkStartTime)
        : c (c), prevBulkStartTime (prevBulkStartTime), prevFrac (prevFrac),
          bulkStartTime (bulkStartTime) {}
} tmfn[5];

static int i = 0;
#endif


void FileWriter::makeFileName (std::string &fName, int bulkStartTime)
{
    static_assert (sizeof (bulkStartTime) == 4, "");

#ifdef TRACE_FW
    tmfn[i++] = TraceMakeFileName (fName[0], prevBulkStartTime, prevFrac, bulkStartTime);
#endif


    if (bulkStartTime == prevBulkStartTime)
        ++prevFrac;
    else {
        prevBulkStartTime = bulkStartTime;
        prevFrac = 0;
    }

    sprintf (&fName[4], "%010u_%03d", bulkStartTime, prevFrac);
    fName[18] = '_';
}


void writeFile (const VecOfStr &bulk, std::string &fName)
{
//    TRACE ("writeFile " << fName << ", bulk size " << bulk.size ());
    std::ofstream f;

    f.open (fName);

    if (!f.is_open ())
        std::cerr << "error opening file " << fName;
    else
        for (auto &s : bulk)
            f << s << std::endl;
}


void FileWriter::threadFunc (std::string &fName)
{
#ifdef TRACE_FW
    static thread_local VecOfStr fNames;

    fNames.reserve (5);
#endif

    std::unique_lock <std::mutex> lock (mtx);

    do {

        cv.wait (lock, [this] () { return bulksQueue.size () > 0 || stop; });


        while (bulksQueue.size () > 0) {

            {
                BulkWithTime bwt = std::move (bulksQueue.front ());

                bulksQueue.pop ();
                makeFileName (fName, bwt.second);
#ifdef TRACE_FW
                fNames.push_back (fName);
#endif
                lock.unlock ();

                writeFile (*bwt.first, fName);
            }

            lock.lock ();
        }
    } while (!stop);

#ifdef TRACE_FW
    for (auto &s : fNames)
        std::cout << s << std::endl;

    for (int i = 0; i < ::i; ++i)
        std::cout << tmfn[i].c << ", " << tmfn[i].prevBulkStartTime << ", "
                  << tmfn[i].prevFrac << ", " << tmfn[i].bulkStartTime << std::endl;
#endif
}


thread_local int FileWriter::prevBulkStartTime = 0, FileWriter::prevFrac = 0;


FileWriter::FileWriter ()
    : fName1 ("bulk0123456789_000_1.log"), fName2 ("bulk0123456789_000_2.log"),
      /*prevBulkStartTime (0), prevFrac (0), */stop (false)
{
    t1 = std::thread (&FileWriter::threadFunc, this, std::ref (fName1));
    t2 = std::thread (&FileWriter::threadFunc, this, std::ref (fName2));
}


FileWriter::~FileWriter ()
{
    std::unique_lock <std::mutex> lock (mtx);

    stop = true;

    lock.unlock ();

    cv.notify_all ();


    t1.join ();
    t2.join ();
}


void FileWriter::output (const SharedVecOfStr bulk, int bulkStartTime)
{
    std::unique_lock <std::mutex> lock (mtx);

    bulkStartTime = BulkContext::nowTimeAsInt ();

    bulksQueue.emplace (std::move (bulk), bulkStartTime);

    lock.unlock ();

#ifdef TRACE_FW
    std::cout << "output bulkStartTime = " + std::to_string (bulkStartTime) << std::endl;
#endif

    cv.notify_one ();
}
