/***************************************************************************
			BUFFER.C
***************************************************************************/

#include "buffer.h"

buffer::buffer()
{
	head = NULL;
	last = NULL;
	status = 0;
	tot_delay = 0.0;
	tot_packs = 0.0;
	
	// Inizializzazione nuovi parametri stop-and-wait
	waiting_ack = 0;
	current_packet = NULL;
	retransmissions = 0;
	tot_retransmissions = 0.0;
}

void buffer::insert(packet *pack)
{
	if (head == NULL)
	{
		head = pack;
		last = pack;
		last->next = head;
	}
	else
	{
		last->next = pack;
		last = pack;
		last->next = head;
	}
}

packet *buffer::get()
{
	packet *pack;
	if (head == NULL)
		return NULL;
	if (last == head)
	{
		pack = head;
		last = NULL;
		head = NULL;
	}
	else
	{
		pack = head;
		head = head->next;
		last->next = head;
	}
	return pack;
}