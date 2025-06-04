/***********************************************************************
		EVENT.C
***********************************************************************/

#include "event.h"
#include "buffer.h"
#include "calendar.h"
#include "queue.h"
#include "rand.h"

extern calendar *cal;
extern double inter;
extern double duration;

void arrival::body() {
    event* ev;

    // genera prossimo arrivo
    double esito;
    GEN_EXP(SEED, inter, esito);
    ev = new arrival(time + esito, buf, sys);
    cal->put(ev);

    // crea nuovo pacchetto
    packet* pack = new packet(time);

    if (buf->full() || buf->status) {
        buf->insert(pack);
    } else if (!sys->waiting_for_ack) {
        // solo se non si sta aspettando un ACK
        //buf->tot_packs += 1.0;
        delete pack;

        GEN_EXP(SEED, duration, esito);
        ev = new service(time + esito, buf, sys); // passa queue*
        cal->put(ev);

        sys->waiting_for_ack = true;
        buf->status = 1;
    } else {
        // Stop-and-wait: se si sta aspettando ack, accoda
        buf->insert(pack);
    }
}


void service::body() {
    packet* pack = buf->get();

    if (pack == nullptr) {
        buf->status = 0;
        return;
    }

    double errore;
	PSEUDO(SEED, errore);


    if (errore < sys->p_errore) {
        buf->insert(pack);

        if (!sys->waiting_for_ack) {
            double esito;
            GEN_EXP(SEED, sys->getDuration(), esito);
            cal->put(new service(time + esito, buf, sys));
            sys->waiting_for_ack = true;
        }
    } else {
        double ack_delay;
        GEN_EXP(SEED, 1.0 / sys->delta_ack, ack_delay);
        cal->put(new ack(time + ack_delay, buf, sys));

        buf->tot_delay += time - pack->get_time();
        buf->tot_packs += 1.0;

        delete pack;
    }
}


void ack::body() {
    sys->waiting_for_ack = false;

    // Se c'è qualcosa in coda, avvia un nuovo servizio
    if (!buf->empty() && !sys->waiting_for_ack) {
        double esito;
        GEN_EXP(SEED, sys->getDuration(), esito);
        cal->put(new service(time + esito, buf, sys)); 
        sys->waiting_for_ack = true;
    }
}
