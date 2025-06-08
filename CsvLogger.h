#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <fstream>
#include <string>

class CsvLogger {
private:
    std::ofstream file;
public:
    CsvLogger(const std::string& filename) {
        file.open(filename, std::ios::app); // Apre in modalità append
        if (file.tellp() == 0) { // Se il file è vuoto, scrive intestazione
            file<<"\n";
            //file << "lambda,duration,p,delta,delay,conf95\n";
        }
    }

    ~CsvLogger() {
        if (file.is_open()) file.close();
    }

    void log(double lambda, double duration, double p, double delta, double delay, double conf95) {
        file << lambda << "," << duration << "," << p << "," << delta << "," << delay << "," << conf95 << "\n";
    }
};

#endif
