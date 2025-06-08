/***********************************************************************
		EVENT.C
***********************************************************************/

#include "event.h"
#include "buffer.h"
#include "calendar.h"
#include "rand.h"

extern calendar *cal;
extern double inter;
extern double duration;

//added variable for stop and wait implementation
extern double error_prob;	
extern double ack_rate;	
extern bool waiting_for_ack; 
extern double retry_count;	 


void arrival::body()
{
	event *ev;

	// generation of next arrival
	double next_arrival_time;
	GEN_EXP(SEED, inter, next_arrival_time);
	ev = new arrival(time + next_arrival_time, buf);
	cal->put(ev);

	// Allocate and queue new packet
	packet *pack = new packet(time);
	buf->insert(pack);

	 // Start service if buffer is free and no ACK is in wait
	if (buf->status == 0 && !waiting_for_ack)
	{
		buf->running_p = buf->get();
		buf->status = 1;
		double transmission_time;
		GEN_EXP(SEED, duration, transmission_time);
		ev = new service(time + transmission_time, buf);
		cal->put(ev);
	}
}

// Handles transmission results
void service::body()
{
	packet *pack = buf->running_p;
	if (!pack)
		return; // Safety

	double test_value;
	PSEUDO(SEED, test_value);

	if (test_value < error_prob)
    {
        // Schedule retransmission
        retry_count += 1.0;
        double retry_time;
        GEN_EXP(SEED, duration, retry_time);
        event *ev = new service(time + retry_time, buf);
        cal->put(ev);
    }
    else
    {
        // Move to waiting for ACK phase
        waiting_for_ack = true;
        buf->status = 0;
        double ack_wait_time;
        GEN_EXP(SEED, 1.0 / ack_rate, ack_wait_time);
        event *ev = new ack_arrival(time + ack_wait_time, buf);
        cal->put(ev);
    }
}

//implementation of the new event for ACK arrival
void ack_arrival::body()
{
	if (waiting_for_ack && buf->running_p)
	{
		waiting_for_ack = false;
		double packet_delay = time - buf->running_p->get_time();
		buf->tot_delay += packet_delay;
		buf->tot_packs += 1.0;
		delete buf->running_p;
		buf->running_p = NULL;
		packet *next_pack = buf->get();
		if (next_pack != NULL)
		{
			buf->running_p = next_pack;
			buf->status = 1;
			double transmission_time;
			GEN_EXP(SEED, duration, transmission_time);
			event *ev = new service(time + transmission_time, buf);
			cal->put(ev);
		}
	}
}