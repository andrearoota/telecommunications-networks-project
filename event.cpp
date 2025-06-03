/***************************************************************************
						MODIFIED EVENT.CPP - Event Implementations
***************************************************************************/
#include "event.h"
#include "buffer.h"
#include "calendar.h"
#include "rand.h"

extern calendar *cal;
extern double inter;				// inter-arrival time between packet generations
extern double duration;				// transmission duration (service time)
extern double error_prob;			// NEW: error probability p
extern double ack_rate;				// NEW: acknowledgment rate δ
extern bool waiting_for_ack;		// NEW: stop-and-wait state
extern double current_packet_start; // NEW: track packet start time
extern double tot_retrans;			// NEW: retransmission counter

// Packet generation event - creates new packets and queues them
void arrival::body()
{
	event *ev;

	// Generate next packet arrival
	double next_arrival_time;
	GEN_EXP(SEED, inter, next_arrival_time);
	ev = new arrival(time + next_arrival_time, buf);
	cal->put(ev);

	// Create new packet and add to buffer
	packet *pack = new packet(time);
	buf->insert(pack);

	// If not currently transmitting and not waiting for ACK, start transmission
	if (buf->status == 0 && !waiting_for_ack)
	{
		buf->current_packet = buf->get();
		buf->status = 1;
		// Schedula la trasmissione
		double transmission_time;
		GEN_EXP(SEED, duration, transmission_time);
		ev = new service(time + transmission_time, buf);
		cal->put(ev);
	}
}

// Transmission completion event - check for errors and handle ACKs
void service::body()
{
	packet *pack = buf->current_packet;
	if (!pack)
		return; // Safety

	double error_check;
	PSEUDO(SEED, error_check);

	if (error_check < error_prob)
	{
		// Ritrasmetti lo stesso pacchetto
		tot_retrans += 1.0;
		double retrans_time;
		GEN_EXP(SEED, duration, retrans_time);
		event *ev = new service(time + retrans_time, buf);
		cal->put(ev);
	}
	else
	{
		// Attendi ACK
		waiting_for_ack = true;
		buf->status = 0;
		double ack_delay;
		GEN_EXP(SEED, 1.0 / ack_rate, ack_delay);
		event *ev = new ack_arrival(time + ack_delay, buf);
		cal->put(ev);
	}
}

// NEW: Acknowledgment arrival event
void ack_arrival::body()
{
	if (waiting_for_ack && buf->current_packet)
	{
		waiting_for_ack = false;
		double packet_delay = time - buf->current_packet->get_time();
		buf->tot_delay += packet_delay;
		buf->tot_packs += 1.0;
		delete buf->current_packet;
		buf->current_packet = NULL;

		// Avvia prossimo pacchetto se presente
		packet *next_pack = buf->get();
		if (next_pack != NULL)
		{
			buf->current_packet = next_pack;
			buf->status = 1;
			double transmission_time;
			GEN_EXP(SEED, duration, transmission_time);
			event *ev = new service(time + transmission_time, buf);
			cal->put(ev);
		}
	}
	// If not waiting for ACK, this is a spurious ACK - ignore it
}