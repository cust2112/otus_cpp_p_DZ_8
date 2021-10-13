#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <list>

#include "bulkoutput.h"


struct BulkContext
{
    explicit BulkContext (size_t bulkSize);
    ~BulkContext ();

    void addOutput (BulkOutput *r);
    void removeOutput (BulkOutput *r);

    void setBulkSize (size_t bulkSize);

    static int nowTimeAsInt ();

    void processData (const char *data, size_t size);
    void flush ();

private:
    size_t bulkSize;
    SharedVecOfStr bulk;
    int bulkStartTime;
    int nested;

    SharedVecOfStr newBulk () const;


    const char *msgB, *msgE, *endData;
    std::string s;
    bool sConsumed;

    const char *findEndOfLine () const;
    void setDataView (const char *data, std::size_t size);
    void moveData2S ();
    bool hasData ();
    const std::string &getLine ();


    typedef std::list <BulkOutput *> Outputs;

    Outputs outputs;

    void outputBulk ();
    void moveBulk2Out ();
};


#endif // INTERPRETER_H
