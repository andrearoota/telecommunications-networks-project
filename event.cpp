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
extern double error_prob;  // probabilità di errore p
extern double ack_rate;	   // tasso degli ACK δ
extern double timeout_val; // valore timeout (opzionale)

void arrival::body()
{
	event *ev;

	// Generazione del prossimo arrivo
	double esito;
	GEN_EXP(SEED, inter, esito);
	ev = new arrival(time + esito, buf);
	cal->put(ev);

	// Inserimento del nuovo pacchetto nella coda
	packet *pack = new packet(time);

	// Se il servitore è libero (non sta trasmettendo né aspettando ACK)
	if (!buf->status && !buf->waiting_ack)
	{
		// Avvia immediatamente la trasmissione
		buf->current_packet = pack;
		buf->retransmissions = 0;

		// Genera tempo di servizio (trasmissione)
		GEN_EXP(SEED, duration, esito);
		ev = new service(time + esito, buf);
		cal->put(ev);
		buf->status = 1;
	}
	else
	{
		// Metti il pacchetto in coda
		buf->insert(pack);
	}
}

void service::body()
{
	// Fine trasmissione del pacchetto
	buf->status = 0;

	// Determina se c'è un errore di trasmissione
	double random_val;
	GEN_UNIF(SEED, 0.0, 1.0, random_val);

	if (random_val < error_prob)
	{
		// Errore di trasmissione - ritrasmetti
		buf->retransmissions++;
		buf->tot_retransmissions += 1.0;

		// Programma ritrasmissione
		double esito;
		GEN_EXP(SEED, duration, esito);
		event *ev = new service(time + esito, buf);
		cal->put(ev);
		buf->status = 1;
	}
	else
	{
		// Trasmissione corretta - attendi ACK
		buf->waiting_ack = 1;

		// Programma arrivo dell'ACK
		double esito;
		GEN_EXP(SEED, ack_rate, esito);
		event *ev = new ack_arrival(time + esito, buf);
		cal->put(ev);
	}
}

void ack_arrival::body()
{
	// ACK ricevuto - pacchetto completato
	if (buf->waiting_ack && buf->current_packet != NULL)
	{
		// Calcola il delay totale (dal momento di generazione)
		buf->tot_delay += time - buf->current_packet->get_time();
		buf->tot_packs += 1.0;

		delete buf->current_packet;
		buf->current_packet = NULL;
		buf->waiting_ack = 0;
		buf->retransmissions = 0;

		// Verifica se ci sono altri pacchetti in coda
		packet *next_pack = buf->get();
		if (next_pack != NULL)
		{
			// Avvia trasmissione del prossimo pacchetto
			buf->current_packet = next_pack;
			buf->retransmissions = 0;

			double esito;
			GEN_EXP(SEED, duration, esito);
			event *ev = new service(time + esito, buf);
			cal->put(ev);
			buf->status = 1;
		}
	}
}

void timeout::body()
{
	// Implementazione timeout (opzionale)
	if (buf->waiting_ack && buf->current_packet != NULL)
	{
		// Timeout scaduto - ritrasmetti
		buf->waiting_ack = 0;
		buf->retransmissions++;
		buf->tot_retransmissions += 1.0;

		double esito;
		GEN_EXP(SEED, duration, esito);
		event *ev = new service(time + esito, buf);
		cal->put(ev);
		buf->status = 1;
	}
}