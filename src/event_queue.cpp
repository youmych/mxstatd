#include <event_queue.h>

namespace mxstatd
{

threadsafe_queue<std::deque<Data::Log::Event>> g_EventQueue;

} // mxstatd
