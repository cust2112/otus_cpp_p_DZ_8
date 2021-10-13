#ifndef BULKRECEIVER_H
#define BULKRECEIVER_H

#include <memory>
#include <string>
#include <vector>


typedef std::vector <std::string> VecOfStr;
typedef std::shared_ptr <VecOfStr> SharedVecOfStr;


class BulkOutput {
public:
    virtual ~BulkOutput () noexcept {}

    virtual void output (const SharedVecOfStr bulk, int bulkStartTime) = 0;
};


//#define USE_TRACE

#ifdef USE_TRACE
#define TRACE(x) std::cout << x << std::endl
#else
#define TRACE(x)
#endif


#endif // BULKRECEIVER_H
