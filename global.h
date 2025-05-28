#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define SEED 2
#define END 100

// Dichiarazioni esterne per i nuovi parametri
extern double error_prob;  // probabilità di errore p
extern double ack_rate;    // tasso degli ACK δ
extern double timeout_val; // valore timeout

#endif