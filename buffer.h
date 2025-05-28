/***************************************************************************
			BUFFER.H
***************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include "packet.h"

class buffer
{
	packet *head;
	packet *last;

public:
	int status;
	// Nuovi stati per stop-and-wait
	int waiting_ack;        // 1 se il servitore sta aspettando ACK
	packet *current_packet; // pacchetto attualmente in trasmissione
	int retransmissions;    // numero di ritrasmissioni per il pacchetto corrente

public:
	buffer();
	~buffer() {}
	void insert(packet *pack);
	packet *get();
	packet *full() { return head; }
	double tot_delay;
	double tot_packs;
	double tot_retransmissions; // totale ritrasmissioni
};

#endif