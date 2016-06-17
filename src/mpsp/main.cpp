#ifdef WIN32
#define BOOST_ERROR_CODE_HEADER_ONLY
#endif
#include "wx_vote_session.hpp"
#include "main_task.hpp"
#include "task_processor.hpp"
#include "config_mgr.hpp"
#include "tasks_mgr.hpp"
#include <boost/interprocess/ipc/message_queue.hpp>

const char* const k_msg_queue_name = "F5E9B985-E78C-46E7-8477-851B7C44FF64";

bool is_running()
{
#ifdef _DEBUG
    boost::interprocess::message_queue::remove(k_msg_queue_name);
#endif
    try
    {
        boost::interprocess::message_queue mq(
            boost::interprocess::create_only,
            k_msg_queue_name,
            1,
            sizeof(int));
    }
    catch (boost::interprocess::interprocess_exception &/*ex*/)
    {
        //printf("%s\n", ex.what());
        return true;
    }

    return false;
}

int main(int argc, char* argv[])
{
    if (is_running())
    {
        printf("There is already a instance running!\n");
        return -1;
    }

    if ( !mpsp::config_mgr::get().init() ||
         !mpsp::device_mgr::get().init() ||
         !mpsp::wx_account_mgr::get().init()||
         !mpsp::tasks_mgr::get().init())
    {
        return -1;
    }
    mpsp::main_task s(mpsp::task_processor::get().get_io_service());
    mpsp::task_processor::get().push_task(boost::bind(&mpsp::main_task::start, &s));

    mpsp::task_processor::get().start();

    try
    {
        boost::interprocess::message_queue::remove(k_msg_queue_name);
    }
    catch (...)
    {

    }

    return 0;
}