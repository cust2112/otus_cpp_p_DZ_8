
#include <algorithm>
#include <chrono>
#include <iostream>

#include "bulkcontext.h"


namespace sch = std::chrono;


BulkContext::BulkContext (size_t bulkSize)
    : bulkSize (bulkSize), bulk (newBulk ()), nested (0), msgB (0), msgE (0), endData (0),
      sConsumed (false) {
}


BulkContext::~BulkContext ()
{
    flush ();
}


void BulkContext::addOutput (BulkOutput *r)
{
    outputs.push_back (r);
}


void BulkContext::removeOutput (BulkOutput *r)
{
    outputs.remove (r);
}


void BulkContext::setBulkSize (std::size_t bulkSize)
{
    this->bulkSize = bulkSize;

    bulk->reserve (bulkSize);
}


int BulkContext::nowTimeAsInt ()
{
    int nt = (int) sch::duration_cast <sch::milliseconds> (
                sch::system_clock::now ().time_since_epoch ()
           ).count ();

//    std::cout << "nt = " << nt << std::endl;

    return nt;
}


void BulkContext::outputBulk ()
{
    for (BulkOutput *r : outputs)
        r->output (bulk, bulkStartTime);
}


void BulkContext::moveBulk2Out ()
{
    outputBulk ();
    bulk = newBulk ();
}


void BulkContext::processData (const char *data, size_t size)
{
    setDataView (data, size);


    while (hasData ()) {

        const std::string &s = getLine ();

        if (s == "{") {

            if (nested++ == 0 && bulk->size () > 0)
                moveBulk2Out ();

        } else if (s == "}") {

            if (--nested == 0)
                moveBulk2Out ();

        } else {

            if (bulk->size () == 0)
                bulkStartTime = nowTimeAsInt ();

            bulk->push_back (s);

            if (nested == 0 && bulk->size () == (size_t) bulkSize)
                moveBulk2Out ();
        }
    }
}


void BulkContext::flush ()
{
    if (nested == 0 && bulk->size () > 0)
        moveBulk2Out ();
    else
        bulk->clear ();

    nested = 0;
    s.clear ();
}


SharedVecOfStr BulkContext::newBulk () const
{
    SharedVecOfStr bulk = std::make_shared <VecOfStr> ();

    bulk->reserve (bulkSize);

    return bulk;
}


const char *BulkContext::findEndOfLine () const
{
    return std::find (msgB, endData, '\n');
}


void BulkContext::setDataView (const char *data, size_t size)
{
    msgB = data;
    endData = data + size;
    msgE = findEndOfLine ();
}


void BulkContext::moveData2S ()
{
    if (msgB < msgE) {

        if (sConsumed) {
            s.clear ();
            sConsumed = false;
        }

        s.append (msgB, msgE);
        msgB = msgE;
    }
}


bool BulkContext::hasData ()
{
    if (msgE < endData)
        return true;


    moveData2S ();

    return false;
}


const std::string &BulkContext::getLine ()
{
    moveData2S ();

    if (msgE < endData) {
        msgB = msgE + 1;
        msgE = findEndOfLine ();
    }

    sConsumed = true;

    return s;
}
