# heatcontroller
lohkolämmittimen ja laturin älyohjaus Raspberry Pitä ja Nucleo F303_RE-mikrokontrolleria käyttäen.

Käyttöohje:

Asenna Raspberry Pille nodejs ja NPM:
    `sudo apt update && sudo apt upgrade`
    `sudo apt install nodejs npm`
    
Asenna socket.io ja serialport kirjastot Node.js:lle:
    `npm install socket.io serialport`
    
Tee mbed-projekti nucleolle, laita main.cpp:ksi viimeisin versio main_v*.cpp

Kloonaa repositorio Raspberry pi:lle ja tarkista RasPin IP osoite (komennolla `ip a`)

Kytke Nucleo RasPiin ja varmista, että Nucleon tallennustila on kiinnitetty (`mount`)

mene kansioon ja käynnistä serveri
    `node webserver.js`
    
Sivu löytyy osoitteesta http://raspin.ip.osoite:8080

