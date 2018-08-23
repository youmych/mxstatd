#include <event_queue.h>

namespace mxstatd
{

//-----------------------------------------------------------------------------
EventQueue::Collector::Collector(queue_t& q)
    : m_Queue(q)
    , m_FirstAppendTime( Now() )
{}
//-----------------------------------------------------------------------------
EventQueue::Collector::~Collector()
{
    Flush();
}
//-----------------------------------------------------------------------------
void EventQueue::Collector::AppendEvent(event_t event)
{
    m_Samples.push_back(event);
    if( (m_Samples.size() >= MAX_SAMPLES) ||
        (Now() - m_FirstAppendTime > MAX_COLLECT_TIME) )
    {
        Flush();
    }
    if( m_Samples.size() == 1 )
        m_FirstAppendTime = Now();
}
//-----------------------------------------------------------------------------
void EventQueue::Collector::Flush()
{
    if( !m_Samples.empty() )
        m_Queue.push( std::move(m_Samples) );
    m_FirstAppendTime = Now();
}

} // mxstatd
