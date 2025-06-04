

/***************************************************************************
                        MODIFIED QUEUE.CPP - Main Simulation Logic
***************************************************************************/
#include "global.h"
#include <stdio.h>
#include "queue.h"
#include "rand.h"
#include "buffer.h"
#include "event.h"
#include "calendar.h"
#include "easyio.h"

calendar *cal;
double inter;
double duration;
double error_prob;    // NEW
double ack_rate;      // NEW
bool waiting_for_ack; // NEW
// double current_packet_start; // NEW
double tot_retrans; // NEW
double Trslen;
double Runlen;
int NRUNmin;
int NRUNmax;

queue::queue(int argc, char *argv[]) : simulator(argc, argv)
{
    cal = new calendar();
    buf = new buffer();
    delay = new Sstat();
    retrans_stat = new Sstat(); // NEW
    waiting_for_ack = false;    // NEW
    tot_retrans = 0.0;          // NEW
}

queue::~queue()
{
    delete delay;
    delete retrans_stat; // NEW
    delete cal;
    delete buf;
}

void queue::input()
{
    printf("STOP-AND-WAIT PROTOCOL SIMULATION\n");
    printf("==================================\n\n");

    printf("MODEL PARAMETERS:\n\n");

    // Arrivals model
    printf("\n Arrivals model:\n");
    printf("1 - Poisson:>\n");
    traffic_model = read_int("", 1, 1, 1);
    load = read_double("Packet generation rate λ (packets/sec)", 0.4, 0.01, 10.0);
    inter = 1.0 / load; // Inter-arrival time

    // Service model
    printf("\n Service model:\n");
    printf("1 - Exponential:>\n");
    service_model = read_int("", 1, 1, 1);
    duration = read_double("Average service duration 1/μ (s)", 0.4, 0.01, 100);

    // NEW: Stop-and-wait parameters
    printf("\nSTOP-AND-WAIT PARAMETERS:\n");
    error_probability = read_double("Transmission error probability p", 0.1, 0.0, 0.99);
    ack_rate = read_double("Acknowledgment arrival rate δ (acks/sec)", 2.0, 0.01, 100.0);

    printf("\nSIMULATION PARAMETERS:\n\n");
    Trslen = read_double("Simulation transient len (s)", 100, 0.01, 10000);
    Runlen = read_double("Simulation RUN len (s)", 100, 0.01, 10000);
    NRUNmin = read_int("Simulation number of RUNs", 5, 2, 100);

    // Display calculated parameters
    printf("\nCALCULATED PARAMETERS:\n");
    printf("Packet inter-arrival time: %.4f sec\n", inter);
    printf("Traffic intensity ρ = λ/μ = %.4f\n", load * duration);
    printf("Expected retransmissions per packet: p/(1-p) = %.4f\n",
           error_prob / (1.0 - error_prob));
}

void queue::init()
{
    input();

    // Initialize global variables
    ::inter = inter;
    ::duration = duration;
    ::error_prob = error_probability;
    ::ack_rate = ack_rate;
    ::waiting_for_ack = false;
    ::tot_retrans = 0.0;

    // Schedule first packet arrival
    event *ev = new arrival(0.0, buf);
    cal->put(ev);

    buf->status = 0; // Initially idle
}

void queue::run()
{
    double clock = 0.0;
    event *ev;

    // Transient period
    printf("Running transient period...\n");
    while (clock < Trslen)
    {
        ev = cal->get();
        if (ev != NULL)
        {
            clock = ev->time;
            ev->body();
            delete ev;
        }
    }

    clear_stats();
    clear_counters();

    // Steady-state runs
    printf("Running steady-state simulation...\n");
    int current_run = 1;
    while (current_run <= NRUNmin)
    {
        while (clock < (current_run * Runlen + Trslen))
        {
            ev = cal->get();
            if (ev != NULL)
            {
                clock = ev->time;
                ev->body();
                delete ev;
            }
        }
        update_stats();
        print_trace(current_run);
        clear_counters();
        current_run++;
    }
}

void queue::results()
{
    fprintf(fpout, "*********************************************\n");
    fprintf(fpout, "     STOP-AND-WAIT SIMULATION RESULTS      \n");
    fprintf(fpout, "*********************************************\n\n");

    fprintf(fpout, "Input parameters:\n");
    fprintf(fpout, "Transient length (s)         %5.3f\n", Trslen);
    fprintf(fpout, "Run length (s)               %5.3f\n", Runlen);
    fprintf(fpout, "Number of runs               %5d\n", NRUNmin);
    fprintf(fpout, "Traffic load λ (packets/sec) %5.3f\n", load);
    fprintf(fpout, "Average service duration 1/μ (sec) %5.3f\n", duration);

    // NEW: Stop-and-wait parameters
    fprintf(fpout, "Stop-and-wait parameters:\n");
    fprintf(fpout, "Transmission error probability p %8.3f\n", error_prob);
    fprintf(fpout, "ACK arrival rate δ (acks/sec) %8.3f\n", ack_rate);

    fprintf(fpout, "Results:\n");
    fprintf(fpout, "Average Delay                %2.6f   +/- %.2e  p:%3.2f\n",
            delay->mean(),
            delay->confidence(.95),
            delay->confpercerr(.95));
    fprintf(fpout, "D  %2.6f   %2.6f   %.2e %2.6f\n", load, delay->mean(), delay->confidence(.95), duration * (load) / (1 - load));

    // Theoretical comparison (for validation)
    double theoretical_delay = duration / (1.0 - error_probability) + 1.0 / ack_rate;
    fprintf(fpout, "\nTheoretical comparison:\n");
    fprintf(fpout, "Expected transmission time   %8.6f\n", duration / (1.0 - error_probability));
    fprintf(fpout, "Expected ACK delay           %8.6f\n", 1.0 / ack_rate);
    fprintf(fpout, "Total theoretical delay      %8.6f\n", theoretical_delay);
    fprintf(fpout, "Simulation vs Theory ratio   %8.6f\n", delay->mean() / theoretical_delay);
}

void queue::print_trace(int n)
{
    fprintf(fptrc, "*********************************************\n");
    fprintf(fptrc, "            TRACE RUN %d                    \n", n);
    fprintf(fptrc, "*********************************************\n\n");

    fprintf(fptrc, "Packets completed: %.0f\n", buf->tot_packs);
    fprintf(fptrc, "Total delay: %.6f\n", buf->tot_delay);
    fprintf(fptrc, "Average delay: %.6f ± %.2e\n",
            delay->mean(), delay->confidence(0.95));
    fprintf(fptrc, "Retransmissions: %.0f\n", tot_retrans);
    fflush(fptrc);
}

void queue::clear_counters()
{
    buf->tot_delay = 0.0;
    buf->tot_packs = 0.0;
    tot_retrans = 0.0;
}

void queue::clear_stats()
{
    delay->reset();
    retrans_stat->reset();
}

void queue::update_stats()
{
    if (buf->tot_packs > 0)
    {
        *delay += buf->tot_delay / buf->tot_packs;
        *retrans_stat += tot_retrans / buf->tot_packs;
    }
}

int queue::isconfsatisf(double perc)
{
    return delay->isconfsatisfied(perc, 0.95);
}