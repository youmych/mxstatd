#pragma once

#include <deque>
#include <threadsafe-queue.h>
#include <data_log_event.h>

namespace mxstatd
{

extern threadsafe_queue<std::deque<Data::Log::Event>> g_EventQueue;

} // mxstatd
