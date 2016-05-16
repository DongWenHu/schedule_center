#define BOOST_ERROR_CODE_HEADER_ONLY
#include "wx_vote_session.hpp"
#include "main_task.hpp"
#include "task_processor.hpp"
#include "config_mgr.hpp"

int main(int argc, char* argv[])
{
    if ( !mpsp::config_mgr::get().init() ||
         !mpsp::device_mgr::get().init() ||
         !mpsp::wx_account_mgr::get().init())
    {
        return -1;
    }
    mpsp::main_task s(mpsp::task_processor::get().get_io_service());
    mpsp::task_processor::get().push_task(boost::bind(&mpsp::main_task::start, &s));

    mpsp::task_processor::get().start();

    return 0;
}