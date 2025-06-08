/*******************************************************
		QUEUE.C
*******************************************************/
#include "global.h"
#include <stdio.h>
#include "queue.h"
#include "rand.h"
#include "buffer.h"
#include "event.h"
#include "calendar.h"
#include "easyio.h"
#include "CsvLogger.h"

calendar *cal;
double inter;
double duration;
double Trslen;
double Runlen;
int NRUNmin;
int NRUNmax;

//added variable for stop and wait implementation
double error_prob;    
double ack_rate;      
bool waiting_for_ack; 
double retry_count;

queue::queue(int argc, char *argv[]) : simulator(argc, argv)
{
    cal = new calendar();
    buf = new buffer();
    delay = new Sstat();
    trans = new Sstat();  //retrasmission statistics
    waiting_for_ack = false;    
    retry_count = 0.0;         
}

queue::~queue()
{
    delete delay;
    delete trans; 
    delete cal;
    delete buf;
}

void queue::input()
{

printf("MODEL PARAMETERS:\n\n");

    printf("\n Arrivals model:\n");
    printf("1 - Poisson:>\n");
    traffic_model=read_int("",1,1,1);
	load=read_double("Traffic load (Erlang)",0.4,0.01,0.999);

    printf("\n Service model:\n");
	printf("1 - Exponential:>\n");
    service_model=read_int("",1,1,1);
    duration=read_double("Average service duration (s)",0.4,0.01,100);

printf("\nSIMULATION PARAMETERS:\n\n");
    error_probability = read_double("Transmission error probability", 0.1, 0.0, 0.99);
    ack_rate = read_double("ACK arrival rate (ack/sec)", 1.0, 0.01, 100.0);
    inter = 1.0 / load;
    printf("\nSIMULATION PARAMETERS:\n\n");
    Trslen=read_double("Simulation transient len (s)", 100, 0.01, 10000);
    Runlen=read_double("Simulation RUN len (s)",  100, 0.01, 10000);
    NRUNmin=read_int("Simulation number of RUNs", 5, 2, 100);   
}

void queue::init()
{
    input();
    // global variables
    ::ack_rate = ack_rate;
    ::duration = duration;
    ::error_prob = error_probability;
    ::inter = inter;
    ::retry_count = 0.0;
    ::waiting_for_ack = false;

    // first arrival
    event *event = new arrival(0.0, buf);
    cal->put(event);
    buf->status = 0;
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
	fprintf(fpout, "           SIMULATION RESULTS                \n");
	fprintf(fpout, "*********************************************\n\n");
	fprintf(fpout, "Input parameters:\n");
	fprintf(fpout, "Transient length (s)         %5.3f\n", Trslen);
	fprintf(fpout, "Run length (s)               %5.3f\n", Runlen);
	fprintf(fpout, "Number of runs               %5d\n", NRUNmin);
	fprintf(fpout, "Traffic load                 %5.3f\n", load);
	fprintf(fpout, "Average service duration     %5.3f\n", duration);

    //new parameters
    fprintf(fpout, "Stop-and-wait parameters:\n");
    fprintf(fpout, "Transmission error probability  %8.3f\n", error_prob);
    fprintf(fpout, "ACK arrival rate (ack/sec) %8.3f\n", ack_rate);
    fprintf(fpout, "Results:\n");
    fprintf(fpout, "Average Delay                %2.6f   +/- %.2e  p:%3.2f\n",
            delay->mean(),
            delay->confidence(.95),
            delay->confpercerr(.95));
    fprintf(fpout, "D  %2.6f   %2.6f   %.2e %2.6f\n", load, delay->mean(), delay->confidence(.95), duration * (load) / (1 - load));

    // Theoretical comparison (for validation)
    double calculated_delay = duration / (1.0 - error_prob) + 1.0 / ack_rate;
    fprintf(fpout, "\n Calculated Value based on input:\n");
    fprintf(fpout, "Total calculated delay    %8.6f\n", calculated_delay);
    fprintf(fpout, "Signed relative error (%%)       %8.2f\n", 
                100.0 * (delay->mean() - calculated_delay) / calculated_delay);

    //saving the results to a CSV file
    CsvLogger csv("results.csv");
    csv.log(load, duration, error_prob, ack_rate, delay->mean(), delay->confidence(0.95));
}  


void queue::print_trace(int n)
{
    fprintf(fptrc, "*********************************************\n");
    fprintf(fptrc, "                 TRACE RUN %d                \n", n);
    fprintf(fptrc, "*********************************************\n\n");

    fprintf(fptrc, "Average Delay                %2.6f   +/- %.2e  p:%3.2f\n",
                        delay->mean(),              
                        delay->confidence(.95),                            
                        delay->confpercerr(.95));
    fflush(fptrc);
   

}

void queue::clear_counters()
{
    buf->tot_delay = 0.0;
    buf->tot_packs = 0.0;
    retry_count = 0.0;
}

void queue::clear_stats()
{
    delay->reset();
    trans->reset(); //added the reset on the new variable
}

void queue::update_stats()
{
    if (buf->tot_packs >= 1)
    {
        *delay += buf->tot_delay / buf->tot_packs;
        *trans += retry_count / buf->tot_packs; //new variable update
    }
}

int queue::isconfsatisf(double perc)
{
    return delay->isconfsatisfied(10, 0.95);
}