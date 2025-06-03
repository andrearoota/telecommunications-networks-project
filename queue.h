/***************************************************************************
						MODIFIED QUEUE.H - Stop-and-Wait Protocol
***************************************************************************/
#ifndef _QUEUE_H
#define _QUEUE_H

#include "simulator.h"
#include "calendar.h"
#include "event.h"
#include "buffer.h"
#include "packet.h"
#include "stat.h"

class queue : public simulator
{
	virtual void input(void);
	buffer *buf; // queue buffer
	int traffic_model;
	double load;
	int service_model;

	// NEW: Stop-and-wait protocol parameters
	double error_probability;		  // probability p of transmission error
	double ack_rate;				  // acknowledgment arrival rate δ
	bool waiting_for_ack;			  // true if transmitter is waiting for ACK
	double current_packet_start_time; // time when current packet transmission started

	// counters
	double packets;
	double tot_delay;
	double tot_retransmissions; // NEW: count retransmissions due to errors

	// statistics
	Sstat *delay;
	Sstat *retrans_stat; // NEW: retransmission statistics

public:
	queue(int argc, char *argv[]);
	virtual ~queue(void);
	virtual void init(void);
	virtual void run(void);

private:
	virtual void clear_counters(void);
	virtual void clear_stats(void);
	virtual void update_stats(void);
	virtual void print_trace(int Run);
	virtual void results(void);
	virtual int isconfsatisf(double perc);
};
#endif