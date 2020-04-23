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

void charger(bool chrg)
{
    // Muutetaan jännitten laskemiseen käytettyä referenssijännitettä,
    // simuloi laturin korkeampaa jännitettä.
    if (chrg == 0)
    {
        NV = 3.32;
        charge = 0;
    }
    else
    {
        NV=3.55;
        charge = 1;
    }
}

int main()
{

    ticker.attach(isr, 1);  // sekunnin välein tulostetaan sarjaportille
                            // ja lasketaan tila

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
        if(raspi.readable())    // jos sarjaportilta vastaanotetaan dataa:
        {
            raspi.gets(command, 6); // luetaan 6 merkin merkkijono, sisältää
                                    // newlinen \n

            if (strcmp(command, "led 1") == 0)      //nucleon led
            {
                led = 1;
            }
            else if (strcmp(command, "led 0") == 0)
            {
                led = 0;
            }
            else if (strcmp(command, "main0") == 0) // tilan valintakäskyt
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
            else if (strcmp(command, "zero0") == 0)     //testausta varten
            {
                mains = volt = temp = charge = heat = 0;
            }
            cout << endl;
        }

///////// Arvojen kirjoitus sarjaportille & ohjelmalogiikka: /////////
    
    // Sarjaportille lähetetään merkit S, V, T ja X, sekä niiden välissä
    // arvot.
    // Raspberry Pi:n serveri lukee koko merkkijonon ja parsii siitä arvot.
    // S ja V:n välissä tila
    // V ja T välissä jännite
    // T ja X välissä lämpötila
    // X merkitsee merkkijonon loppumista.
        
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
                
                
                if (temp < 5)   // lämpötila alle 5
                {
                    heat = 1;
                    charger(0);
                    if (NTC == 0) temp += TEMP_MUUTOS;
                }
                else if (temp >= 10)    // lämpötila 10 tai yli
                {   
                    heat = 0;
                    
                    if (volt < 12.4 || (charge == 1 && volt < 14.1))
                    // temp>=10 ja volt <12.4 TAI temp>=10, lataus ja volt < 14.1
                    {
                        charger(1);
                        if (AKKU == 0) volt += VOLT_MUUTOS;
                        if (NTC == 0) temp -= TEMP_MUUTOS;
                    }
                    else if (volt >= 14.1)
                    // temp>=10 ja jännite yli 14.1 eli ladattu täyteen, mutta laturi
                    // vielä päällä
                    {
                        charger(0);
                        //NV = 3.32;
                        //charge = 0;
                        if (AKKU == 0) volt = 12.5;
                        if (NTC == 0) temp -= TEMP_MUUTOS;
                    }
                    else
                    // muut vaihtoehdot, käytännössä lämmintä ja akku täynnä
                    {
                        charge = 0;
                        if (AKKU == 0 && NTC == 0)
                        {
                            volt -= (VOLT_MUUTOS/4);
                            temp -= TEMP_MUUTOS;
                        }
                    }
                }
                else    // 5-10 asteen lämpötilat
                {

                    if (heat == 1)
                    // jos lämmitys päällä
                    // Tositilanteessa ei tarvitse tehdä mitään,
                    // lohkolämmitin vain lämmittää. Nämä simulointia varten.
                    {
                        if (NTC == 0) temp += TEMP_MUUTOS;
                        if (AKKU == 0 && volt > 12.5)
                        {
                            volt = 12.5;
                        }
                    }
                    else if (charge == 1)
                    // jos laturi päällä
                    // tämäkin lähinnä simulointia varten
                    {   

                        if (AKKU == 0) volt += VOLT_MUUTOS;
                        if (NTC == 0) temp -= TEMP_MUUTOS;
                        if (volt >= 14.1)
                        {
                            charger(0);
                            //charge = 0;
                            //NV = 3.32;
                            if (AKKU == 0) volt = 12.5;
                        }
                    }
                    else if (heat == 0 && charge == 0 && volt < 12.4)
                    // jos akun jännite alhainen
                    {
                        charger(1);
                        //charge = 1;
                        //NV = 3.55;
                        if (AKKU == 0) volt += VOLT_MUUTOS;
                        if (NTC == 0) temp -= TEMP_MUUTOS;
                    }
                    else
                    // jos lämpötila laskee lämpimämmästä alle 10 asteeseen,
                    // ja akussa on riittävästi virtaa
                    // tämäkin vain simulointia varten.
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
                // Jos akun jännite alhainen
                {
                    charger(1);
                    //charge = 1;
                    //NV = 3.55;
                    if (AKKU == 0) volt += VOLT_MUUTOS;
                }
                else if (charge == 1)
                // Jos laturi päällä
                {   
                    if (volt >= 14.1)
                    // jos laturi päällä, ja akku täynnä
                    {
                        charger(0);
                        //charge = 0;
                        //NV = 3.32;
                        if (AKKU == 0) volt = 12.5;
                    }
                    else
                    // jos laturi päällä, ja akku ei vielä täynnä
                    {
                        if (AKKU == 0) volt += VOLT_MUUTOS;
                    }
                }
                else
                // laturi ei päällä (eli jännite yli 12.4)
                {   
                    if (AKKU == 0) volt -= (VOLT_MUUTOS/4);
                }
            }
        }
            

        
        
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
                // lämpötila alle 5
                {
                    heat = 1;
                    if (NTC == 0) temp += TEMP_MUUTOS;
                }
                else if (heat == 1 && temp < 10)
                // lämmitys päällä ja lämpötila alle 10
                {
                    if (NTC == 0) temp += TEMP_MUUTOS;
                }
                else
                // käytännössä jos lämmitys päällä ja lämpötila ylittää 10
                {
                    heat = 0;
                    if (NTC == 0) temp -= TEMP_MUUTOS;
                }
            }                   
        }


        
        ////////////////// Järjestelmä pois päältä ///////////////////////
        else
        {
            if (print == 1)
            {
                print = 0;
                cout << "S" << mains << "V" << "OFF" << "T" << "OFF" << "X" << endl;
                charger(0);
                //charge = 0;
                heat = 0;
                
                //NV = 3.32;
                
            }
        }
        
        wait_us(1000);
    }
}
