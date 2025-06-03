

/***************************************************************************
                        MODIFIED EVENT.H - New Event Types
***************************************************************************/
#ifndef _EVENT_H
#define _EVENT_H

#include "global.h"
#include "buffer.h"

class event
{
public:
    event *next;
    double time;
    event();
    event(double Time);
    event(event *Next, double Time);
    ~event() {}
    virtual void body() {}
};

inline event::event()
{
    next = NULL;
    time = -1;
}

inline event::event(event *Next, double Time)
{
    next = Next;
    time = Time;
}

inline event::event(double Time)
{
    time = Time;
}

// Original arrival event - now represents packet generation
class arrival : public event
{
    buffer *buf;

public:
    int source_id;
    virtual void body();
    arrival(double Time, buffer *Buf);
};

// Original service event - now represents transmission completion
class service : public event
{
    buffer *buf;

public:
    virtual void body();
    service(double Time, buffer *Buf) : event(Time) { buf = Buf; }
};

// NEW: Acknowledgment arrival event
class ack_arrival : public event
{
    buffer *buf;

public:
    virtual void body();
    ack_arrival(double Time, buffer *Buf) : event(Time) { buf = Buf; }
};

// NEW: Transmission error event (immediate check after service completion)
class transmission_check : public event
{
    buffer *buf;

public:
    virtual void body();
    transmission_check(double Time, buffer *Buf) : event(Time) { buf = Buf; }
};

inline arrival::arrival(double Time, buffer *Buf) : event(Time)
{
    buf = Buf;
}

#endif