#include <event_queue.h>
#include <stat_map.h>
#include <statistics.h>

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


//-----------------------------------------------------------------------------
EventQueue::EventQueue()
{

}
//-----------------------------------------------------------------------------
EventQueue::~EventQueue()
{
    Stop();
}
//-----------------------------------------------------------------------------
void EventQueue::WakeUp()
{
    queue_item_t wakeup;
    m_EventQueue.push(wakeup);
}
//-----------------------------------------------------------------------------
void EventQueue::Start()
{
    if( m_GatherThread.joinable() )
        return;

    is_Done = false;
    m_GatherThread = std::thread([this](){
        DataGate();
    });
}
//-----------------------------------------------------------------------------
void EventQueue::Stop()
{
    is_Done = true;
    WakeUp();
    if( m_GatherThread.joinable() )
        m_GatherThread.join();
}

//-----------------------------------------------------------------------------
void EventQueue::DataGate()
{
    auto stat = STAT().GetStatistics(Data::Log::EventType::ORDER);
    queue_item_t data;
    auto adaptor = [](auto ev) { return ev.TimeMs; };

    for(;;) {
        if( is_Done )
            return;

        m_EventQueue.wait_and_pop(data);
        if( data.empty() )
            continue;

        stat->AppendValues(data, adaptor);
    }
}

} // mxstatd
