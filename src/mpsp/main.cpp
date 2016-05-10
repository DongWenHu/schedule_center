#define BOOST_ERROR_CODE_HEADER_ONLY
#include "wx_vote_session.hpp"
#include "main_task.hpp"
#include "task_processor.hpp"

int main(int argc, char* argv[])
{
    mpsp::main_task s(mpsp::task_processor::get().get_io_service());
    mpsp::task_processor::get().push_task(boost::bind(&mpsp::main_task::start, &s));

    //测试3秒后运行一个任务
    //boost::asio::deadline_timer::duration_type td(0, 0, 3, 0);
    //mpsp::task_processor::get().run_after(td, &test);

    //测试2016-04-25 15:08:00运行一个任务
    //boost::posix_time::ptime p(boost::gregorian::date(2016, 4, 25), boost::posix_time::hours(15 - 8) + boost::posix_time::minutes(8));
    //mpsp::task_processor::get().run_at(p, &test);
    mpsp::task_processor::get().start();

    return 0;
}