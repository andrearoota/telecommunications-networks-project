/*******************************************************
			 EVENT . H
*******************************************************/

#ifndef _EVENT_H
#define _EVENT_H

#include "global.h"
#include "buffer.h"
#include "queue.h"

class event
{
public:
	event *next; // next event
	double time; // event time
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

class arrival : public event
{
	queue* sys;
	buffer *buf;

public:
	int source_id;
	virtual void body();
	arrival(double Time, buffer *Buf,  queue* Sys);
};

class service : public event
{
	queue* sys;
	buffer *buf;

public:
	virtual void body();
	service(double Time, buffer *Buf,  queue* Sys) : event(Time) { 
		buf = Buf;
		sys = Sys; 
	}
};
class ack : public event {
    buffer* buf;
    queue* sys; // queue system to which the ack belongs	
public:
    ack(double Time, buffer* Buf, queue* Sys) : event(Time) {
        buf = Buf;
        sys = Sys;
    }

    virtual void body();
};


inline arrival::arrival(double Time, buffer* Buf, queue* Sys) : event(Time) {
    buf = Buf;
    sys = Sys;
}

#endif
