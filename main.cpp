/* -*- C++ -*- */
#include <stdio.h>
#include "global.h"
#include "queue.h"
#include "simulator.h"

int main(int argc, char *argv[])
{
	simulator *eval;

	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("**********************************************************\n\n");
	printf("     SIMULATORE STOP-AND-WAIT CON ERRORI DI TRASMISSIONE\n\n");
	printf("**********************************************************\n\n");
	printf("Modello:\n");
	printf("- Arrivi: Processo di Poisson con tasso λ\n");
	printf("- Servizio: Durata esponenziale con media 1/μ\n");
	printf("- Errori: Probabilità p per ogni trasmissione\n");
	printf("- ACK: Processo di Poisson con tasso δ\n");
	printf("- Protocollo: Stop-and-Wait\n\n");

	eval = new queue(argc, argv);
	eval->init();
	eval->run();
	eval->results();
	delete (eval);
	
	return 0;
}