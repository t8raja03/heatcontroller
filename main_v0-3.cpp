#include "mbed.h"
#include <iostream>

// liput simulointiin ilman oikeita mittauksia
#define AKKU 0
#define NTC 0

#define VOLT_MUUTOS 0.2
#define TEMP_MUUTOS 0.4

Serial raspi(USBTX, USBRX);
AnalogIn vin(A0);
DigitalOut led(LED1);
DigitalOut charge(D2);  // Laturin ohjaus (simuloitu pelkällä ledillä)
DigitalOut heat(D4);    // Lohkolämmittimen ohjaus (simuloitu ledillä)
Ticker ticker;
bool print = 0;
double NV = 3.32;       // Nucleon referenssijännite. Latausta simuloidaan
                        // muuttamalla kerrointa

void isr()
{
    // tickerin keskeytysfunktio, ilmoitetaan että
    // tulostetaan sarjaportille
    print = 1;
}

double getVoltage()
{
    double voltage = vin.read() * NV;
    double R1=11390.0;
    double R2=3360.0;
    voltage = voltage / (R2/(R2+R1));
    return voltage;
}

int main()
{

    ticker.attach(isr, 1);

    double volt;        // akun jännite
    double temp;        // jäähdytysnesteen lämpötila
    char command[6];    // sarjaportilta luettava käsky (string)
    int mains = 0;      // tilan määritys
                        // 0 = pois päältä, 1 = auto, 2 = lataus, 3 = lämmitys
    
    
    // Tarkistetaan, käytetäänkö laskennallisia lämpötila-arvoja
    // vai mitattuja.
    if (AKKU == 1) volt = getVoltage();
    else volt = 12.3;
      
        
    if (NTC == 1);//Tähän lämpötilan haku muuttujaan
    else temp = 3.00;

    while(1)
    {

    // Kun liput AKKU ja NTC asetetaan 0, ei jännitettä
    // tai lämpötilaa haeta antureilta, vaan niiden arvot
    // asetetaan ohjelmallisesti


        ///////////// Ohjauskäskyjen käsittely: /////////////////
        if(raspi.readable())
        {
            raspi.gets(command, 6);

            if (strcmp(command, "led 1") == 0)
            {
                led = 1;
            }
            else if (strcmp(command, "led 0") == 0)
            {
                led = 0;
            }
            else if (strcmp(command, "main0") == 0)
            {
                mains = charge = heat = 0;
            }
            else if (strcmp(command, "main1") == 0)
            {
                mains = 1;
                charge = heat = 0;
            }
            else if (strcmp(command, "main2") == 0)
            {
                mains = 2;
                charge = heat = 0;
            }
            else if (strcmp(command, "main3") == 0)
            {
                mains = 3;
                charge = heat = 0;
            }
            else if (strcmp(command, "zero0") == 0)
            {
                mains = volt = temp = charge = heat = 0;
            }
            cout << endl;
        }

        ///////// Arvojen kirjoitus sarjaportille & ohjelmalogiikka: /////////
        
        
        ////////////// Automaattinen tila //////////////////
        if (mains == 1)
        {
            if (print == 1)
            {
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;

                // Jos lämpötila < 5, aloitetaan lämmitys.
                // Lämmitys pysyy päällä, kunnes lämpötila >= 10
                // Jos tällöin jännite alle 12.4, aloitetaan lataus
                // Jos jännite yli 12.5, ei myöskään ladata
                if (temp < 5)
                {
                    heat = 1;
                    charge = 0;
                    if (NTC == 0) temp += TEMP_MUUTOS;
                    NV=3.32;
                }
                else if (temp >= 10)
                {   
                    heat = 0;
                    
                    if (volt < 12.4 || (charge == 1 && volt < 14.1))
                    {
                        charge = 1;
                        NV = 3.55;
                        if (AKKU == 0) volt += VOLT_MUUTOS;
                    }
                    else if (charge == 1 && volt >= 14.1)
                    // Tässä pitäisi oikeasti mitata latausvirta,
                    // jonka ollessa tarpeeksi pieni lataus lopetetaan
                    {
                        NV = 3.32;
                        charge = 0;
                        if (AKKU == 0) volt = 12.5;
                    }
                    else
                    {
                        charge = 0;
                        if (AKKU == 0 && NTC == 0)
                        {
                            volt -= (VOLT_MUUTOS/4);
                            temp -= TEMP_MUUTOS;
                        }
                    }
                }
                else
                {
                    // tämä on käytännössä tila, jolloin
                    // 5 =< temp < 10. Tässä tilassa vain jatketaan
                    // edellistä tilaa.
                    if (heat == 1)
                    {
                        if (NTC == 0) temp += TEMP_MUUTOS;
                        NV=3.32;
                    }
                    else if (charge == 1)
                    {
                        NV = 3.55;
                        if (AKKU == 0) volt += VOLT_MUUTOS;                        
                    }
                    else
                    {
                        if (AKKU == 0 && NTC == 0)
                        {
                            temp -= TEMP_MUUTOS;
                            volt -= (VOLT_MUUTOS/4);
                        }    
                    }
                }                   
            }                   
        }    
/**********                
                // Arvojen muuttelu testikäyttöä varten:
                if (discharge == 0)
                {
                    if (AKKU == 0) volt += 0.05;
                    charge = 1;
                    heat = 0;
                    NV = 3.55;

                    if (volt >= 14.10) 
                    {
                        // Tässä pitäisi "oikeasti" mitata latausvirta,
                        // jos riittävän pieni niin lataus lopetetaan
                        discharge = 1;
                    }
                }

                else if (heat < 5)
                {
                    if (AKKU == 0) volt -= 0.01;
                    charge = 0;
                    heat = 1;
                    if (NTC == 0) temp += 0.1;
                    NV=3.32;

                    if (temp >= 10)
                    {
                        charge = 1;
                    }
                }

    
            }
        }
****************/        
        /////////////////// Pelkkä lataus: ////////////////////////
        else if (mains == 2)
        {
            // Jos jännite alle 12.4, käynnistetään lataus.
            // Jos jännite yli 12.5, lopetetaan lataus.
            if (print == 1)
            {
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;
                
                if (volt < 12.4)
                {
                    charge = 1;
                    NV = 3.55;
                    if (AKKU == 0) volt += VOLT_MUUTOS;
                }
                else if (charge == 1 && volt >= 14.1)
                {
                    charge = 0;
                    NV = 3.32;
                    if (AKKU == 0) volt = 12.5;
                }
                else
                {
                    if (AKKU == 0) volt -= (VOLT_MUUTOS/4);
                }
            }
        }
            
/*****************        
            if (print == 1)
            {
                //cout.precision(3);
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;

                if (discharge == 0) {
                    volt += 0.01;
                    charge = 1;
                    NV=3.55;

                    if (volt >= 14.10)
                    {
                        discharge = 1;
                    }
                }

                else if (discharge == 1)
                {
                    volt -= 0.01;
                    charge = 0;
                    NV=3.32;

                    if (volt <= 12.40)
                    {
                        discharge = 0;
                    }
                }
 
            }

        } //if (mains==2)
*****************/      
        
        
        /////////////////// Pelkkä lämmitys:   ////////////////////////
        else if (mains == 3)
        {
            // Jos lämpötila laskee alle 5, aloitetaan lämmitys
            // Jos lämpötila nousee yli 10, lopetetaan lämmitys
            if (print == 1)
            {
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;
                
                if (temp < 5)
                {
                    heat = 1;
                    if (NTC == 0) temp += TEMP_MUUTOS;
                }
                else if (heat == 1 && temp < 10)
                {
                    if (NTC == 0) temp += TEMP_MUUTOS;
                }
                else
                {
                    heat = 0;
                    if (NTC == 0) temp -= TEMP_MUUTOS;
                }
            }                   
        }

/******************
        else if (mains == 3)
        {
            if (print == 1)
            {
                cout << "S" << mains << "V" << volt << "T" << temp << "X" << endl;
                print = 0;

                if (heat == 0)
                {
                    temp -= 0.01;
                }

                else if (heat == 1)
                {
                    

                    if (volt <= 12.40)
                    {
                        discharge = 0;
                    }
                }

    
            }
        } //if (mains==3)
**********************/        
        
        
        ////////////////// Järjestelmä pois päältä ///////////////////////
        else
        {
            if (print == 1)
            {
                print = 0;
                cout << "S" << mains << "V" << "OFF" << "T" << "OFF" << "X" << endl;
                charge = 0;
                heat = 0;
                
                NV = 3.32;
                
            }
        }
    }
}
