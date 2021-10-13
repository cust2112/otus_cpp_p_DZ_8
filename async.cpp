
#include <algorithm>
#include <list>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "async.h"
#include "bulkcontext.h"
#include "consolewriter.h"
#include "filewriter.h"


bool operator== (const BulkContext &ctx1, const BulkContext &ctx2)
{
    return &ctx1 == &ctx2;
}


namespace async {


// These must live longer than Contexts
ConsoleWriter cw;
FileWriter fw;


typedef std::list <BulkContext> Contexts;

Contexts freeCtxs, busyCtxs;
std::mutex ctxMtx;


handle_t connect (std::size_t bulk)
{
    BulkContext *ctx;
    std::unique_lock <std::mutex> ul (ctxMtx);


    if (freeCtxs.size () > 0) {

        busyCtxs.splice (busyCtxs.end (), freeCtxs, freeCtxs.begin ());
        ctx = &busyCtxs.back ();

        ctx->setBulkSize (bulk);

    } else {

        busyCtxs.emplace_back (bulk);
        ctx = &busyCtxs.back ();

        ctx->addOutput (&cw);
        ctx->addOutput (&fw);
    }

    ul.unlock ();

    return ctx;
}


void receive (handle_t handle, const char *data, std::size_t size)
{
    BulkContext *ctx = (BulkContext *) handle;

    ctx->processData (data, size);
}


void disconnect (handle_t handle)
{
    BulkContext *ctx = (BulkContext *) handle;
    std::unique_lock <std::mutex> ul (ctxMtx);

    Contexts::iterator i = std::find (busyCtxs.begin (), busyCtxs.end (), *ctx);

    if (i == busyCtxs.end ())
        throw std::runtime_error ("async::disconnect: could not find handle " +
                                  std::to_string ((std::size_t) handle));


    i->flush ();

    freeCtxs.splice (freeCtxs.end (), busyCtxs, i);

    ul.unlock ();  // avoid 'unused' warning
}

}
