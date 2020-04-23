#include "mbed.h"
#include <iostream>

Serial raspi(USBTX, USBRX);
DigitalOut led(LED1);
Ticker ticker;
bool print = 0;

void isr() {
    print = 1;
}

int main() {
    
    ticker.attach(isr, 1);
    
    //char volt[6]="12.55";
    //char temp[6]="3.01";
    double volt=12.55;
    double temp=3.00;
    char command[6];
    bool mains = 1;
    bool d = 0;

    
    while(1) {
        
        // Ohjauskäskyjen käsittely:
        if(raspi.readable()) {
            raspi.gets(command, 6);
    
            if (strcmp(command, "led 1") == 0)
                led = 1;
            else if (strcmp(command, "led 0") == 0)
                led = 0;
            else if (strcmp(command, "main0") == 0)
                mains = 0;
            else if (strcmp(command, "main1") == 0)
                mains = 1;
            cout << endl;     
        }

        // Arvojen kirjoitus sarjaportille:
        if (mains == 1) {
            
            if (print == 1) {
                cout << "V" << volt << "T" << temp << "X" << endl;
                print = 0;
                
                // Arvojen muuttelu testikäyttöä varten:
                if (d == 0) {
                    volt += 0.01;
                    temp -= 0.01;
                    
                    if (volt >= 12.6) {
                        d = 1;
                    }
                }
                else if (d == 1) {
                    volt -= 0.01;
                    temp += 0.01;
                    
                    if (volt <= 12.4) {
                        d = 0;
                    }
                }
            }
            
        }
        

    }
}
