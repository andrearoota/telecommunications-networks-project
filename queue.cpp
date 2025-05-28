/* -*- C++ -*- */
/*******************************************************
		QUEUE.C - Stop-and-Wait con errori
*******************************************************/
#include "global.h"
#include <stdio.h>
#include "queue.h"
#include "rand.h"
#include "buffer.h"
#include "event.h"
#include "calendar.h"
#include "easyio.h"

calendar *cal; // events calendar
double inter;
double duration;
double error_prob;  // probabilità di errore p
double ack_rate;    // tasso degli ACK δ
double timeout_val; // valore timeout
double Trslen;
double Runlen;
int NRUNmin;
int NRUNmax;

queue::queue(int argc, char *argv[]) : simulator(argc, argv)
{
	cal = new calendar();
	buf = new buffer();
	delay = new Sstat();
	retransmissions = new Sstat();
}

queue::~queue()
{
	delete delay;
	delete retransmissions;
	delete cal;
	delete buf;
}

void queue::input()
{
	printf("MODELLO STOP-AND-WAIT CON ERRORI:\n\n");
	
	printf("Parametri del collegamento:\n");
	printf("1 - Arrivi Poissoniani\n");
	traffic_model = read_int("Modello arrivi", 1, 1, 1);
	load = read_double("Carico di traffico λ (pkt/s)", 0.4, 0.01, 10.0);
	
	printf("1 - Servizio Esponenziale\n");
	service_model = read_int("Modello servizio", 1, 1, 1);
	duration = read_double("Durata media servizio 1/μ (s)", 1.0, 0.01, 100.0);
	
	// Nuovi parametri per stop-and-wait
	error_prob = read_double("Probabilità di errore p", 0.1, 0.0, 0.99);
	ack_rate = read_double("Tasso ACK δ (ack/s)", 10.0, 0.1, 1000.0);
	timeout_val = read_double("Timeout (s) [0=no timeout]", 0.0, 0.0, 100.0);
	
	// Calcola tasso di arrivo basato sul carico
	inter = 1.0 / load;
	
	printf("\nPARAMETRI DI SIMULAZIONE:\n\n");
	Trslen = read_double("Lunghezza transiente (s)", 100, 0.01, 10000);
	Runlen = read_double("Lunghezza RUN (s)", 1000, 0.01, 10000);
	NRUNmin = read_int("Numero di RUN", 5, 2, 100);
	
	printf("\nParametri calcolati:\n");
	printf("Tasso arrivi: %.4f pkt/s\n", 1.0/inter);
	printf("Tasso servizio: %.4f pkt/s\n", 1.0/duration);
	printf("Carico teorico: %.4f\n", (1.0/inter) * duration);
}

void queue::init()
{
	input();
	event *Ev;
	Ev = new arrival(0.0, buf);
	cal->put(Ev);
	buf->status = 0;
	buf->waiting_ack = 0;
	buf->current_packet = NULL;
}

void queue::run()
{
	extern double Trslen;
	extern double Runlen;
	extern int NRUNmin;
	extern int NRUNmax;

	double clock = 0.0;
	event *ev;
	
	// Fase transiente
	printf("Fase transiente...\n");
	while (clock < Trslen)
	{
		ev = cal->get();
		if (ev == NULL) break;
		ev->body();
		clock = ev->time;
		delete (ev);
	}
	
	clear_stats();
	clear_counters();
	
	// Fase di raccolta statistiche
	printf("Raccolta statistiche...\n");
	int current_run_number = 1;
	while (current_run_number <= NRUNmin)
	{
		printf("RUN %d/%d\n", current_run_number, NRUNmin);
		while (clock < (current_run_number * Runlen + Trslen))
		{
			ev = cal->get();
			if (ev == NULL) break;
			ev->body();
			clock = ev->time;
			delete (ev);
		}
		update_stats();
		clear_counters();
		print_trace(current_run_number);
		current_run_number++;
	}
}

void queue::results()
{
	extern double Trslen;
	extern double Runlen;
	extern int NRUNmin;
	extern int NRUNmax;

	fprintf(fpout, "*********************************************\n");
	fprintf(fpout, "     RISULTATI SIMULAZIONE STOP-AND-WAIT    \n");
	fprintf(fpout, "*********************************************\n\n");
	
	fprintf(fpout, "Parametri di ingresso:\n");
	fprintf(fpout, "Lunghezza transiente (s)     %5.3f\n", Trslen);
	fprintf(fpout, "Lunghezza RUN (s)            %5.3f\n", Runlen);
	fprintf(fpout, "Numero di RUN                %5d\n", NRUNmin);
	fprintf(fpout, "Carico di traffico λ         %5.3f pkt/s\n", 1.0/inter);
	fprintf(fpout, "Durata media servizio 1/μ    %5.3f s\n", duration);
	fprintf(fpout, "Probabilità di errore p      %5.3f\n", error_prob);
	fprintf(fpout, "Tasso ACK δ                  %5.3f ack/s\n", ack_rate);
	fprintf(fpout, "Timeout                      %5.3f s\n", timeout_val);
	
	fprintf(fpout, "\nRisultati:\n");
	fprintf(fpout, "Tempo medio di attraversamento: %2.6f ± %.2e s (p=%3.2f%%)\n",
			delay->mean(),
			delay->confidence(.95),
			delay->confpercerr(.95));
	
	fprintf(fpout, "Numero medio ritrasmissioni:    %2.6f ± %.2e\n",
			retransmissions->mean(),
			retransmissions->confidence(.95));
	
	// Calcolo teorico (approssimato)
	double rho = (1.0/inter) * duration;
	double expected_retrans = error_prob / (1.0 - error_prob);
	double theoretical_delay = duration * (1.0 + expected_retrans) + 1.0/ack_rate;
	
	fprintf(fpout, "\nValori teorici (approssimati):\n");
	fprintf(fpout, "Utilizzo del canale ρ:          %2.6f\n", rho);
	fprintf(fpout, "Ritrasmissioni attese:          %2.6f\n", expected_retrans);
	fprintf(fpout, "Delay teorico approssimato:     %2.6f s\n", theoretical_delay);
	
	// Output per post-processing
	fprintf(fpout, "\nDati per grafici:\n");
	fprintf(fpout, "DELAY %2.6f %2.6f %.2e\n", 
			1.0/inter, delay->mean(), delay->confidence(.95));
	fprintf(fpout, "RETRANS %2.6f %2.6f %.2e\n", 
			error_prob, retransmissions->mean(), retransmissions->confidence(.95));
}

void queue::print_trace(int n)
{
	fprintf(fptrc, "*********************************************\n");
	fprintf(fptrc, "                 TRACE RUN %d                \n", n);
	fprintf(fptrc, "*********************************************\n\n");

	fprintf(fptrc, "Tempo medio attraversamento:    %2.6f ± %.2e s (p=%3.2f%%)\n",
			delay->mean(),
			delay->confidence(.95),
			delay->confpercerr(.95));
			
	fprintf(fptrc, "Numero medio ritrasmissioni:    %2.6f ± %.2e\n",
			retransmissions->mean(),
			retransmissions->confidence(.95));
			
	fflush(fptrc);
}

void queue::clear_counters()
{
	buf->tot_delay = 0.0;
	buf->tot_packs = 0.0;
	buf->tot_retransmissions = 0.0;
}

void queue::clear_stats()
{
	delay->reset();
	retransmissions->reset();
}

void queue::update_stats()
{
	if (buf->tot_packs > 0)
	{
		*delay += buf->tot_delay / buf->tot_packs;
		*retransmissions += buf->tot_retransmissions / buf->tot_packs;
	}
}

int queue::isconfsatisf(double perc)
{
	return delay->isconfsatisfied(10, .95);
}