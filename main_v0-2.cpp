#include "mbed.h"
#include <iostream>

#define AKKU 0	// tällä lipulla saa ilmoitettua, että oikea akku on kytketty (1)

Serial raspi(USBTX, USBRX);
AnalogIn vin(A0);
DigitalOut led(LED1);
DigitalOut charge(D2);
Ticker ticker;
bool print = 0;
double NV = 3.32;
void isr() {
    print = 1;
}
 
double getVoltage() {
    double voltage = vin.read() * NV;
    double R1=11390.0;
    double R2=3360.0;
    voltage = voltage / (R2/(R2+R1));  
    return voltage;
}
 
int main() {
 
    ticker.attach(isr, 1);
 
    //char volt[6]="12.55";
    //char temp[6]="3.01";
    double volt;
    double temp=3.00;
    char command[6];
    uint8_t mains = 1;	// 1 = auto, 2 = lataus, 3 = lämmitys
    bool discharge = 0;
 
 
    while(1) {

	if (AKKU == 1)
 		volt = getVoltage();	// Mitataan jännite
	else
		volt = 12.55;			// debuggaus ilman oikeaa jännitteen mittausta

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
            else if (strcmp(command, "main2") == 0)
                mains = 2;
            else if (strcmp(command, "main3") == 0)
                mains = 3;
            cout << endl;
        }
 
        // Arvojen kirjoitus sarjaportille & ohjelmalogiikka:
        if (mains == 1) {
 		// automaattinen tila:
            if (print == 1) {
                cout.precision(3);
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;
 

                // Arvojen muuttelu testikäyttöä varten:
                if (discharge == 0) {
                    volt += 0.01;
                    charge = 1;
                    NV=3.55;

                    if (volt >= 14.30) {
                        discharge = 1;
                    }
                }

                else if (discharge == 1) {
                    volt -= 0.01;
                    charge = 0;
                    NV=3.32;
                   
                    if (volt <= 12.40) {
                        discharge = 0;
                    }
                }
         
	    
	    	}
        }
		else if (mains == 2) {
		// Lataus:
            if (print == 1) {
                cout.precision(3);
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;
 

                // Arvojen muuttelu testikäyttöä varten:
                if (discharge == 0) {
                    volt += 0.01;
                    charge = 1;
                    NV=3.55;
 
                    if (volt >= 14.30) {
                        discharge = 1;
                    }
                }

                else if (discharge == 1) {
                    volt -= 0.01;
                    charge = 0;
                    NV=3.32;
                   
                    if (volt <= 12.40) {
                        discharge = 0;
                    }
                }
         
	    
	    	}
		}
 
 
    }
}
