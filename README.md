Guides
======

* [Arduino Guide](http://arduino.cc/en/Guide/ArduinoLeonardo)
* [Getting started with the nRF24L01+](http://maniacbug.wordpress.com/2011/11/02/getting-started-rf24/)


Initialising
------------

    git clone --recursive https://github.com/emfcamp/TiLDA

or

    git clone git@github.com:emfcamp/TiLDA
    cd TiLDA
    git submodule update --init

Linux/OS X (if not using Arduino IDE):
    export ARDUINODIR=/path/to/arduino-1.0.1
    for i in $PWD/libraries/*; do if test -d $i; then ln -s $i $ARDUINODIR/libraries/; fi; done

Windows:
* Copy everything in libraries to the libraries subfolder of arduino-1.0.1.

Building
--------

Linux/OS X (if not using Arduino IDE):

    cd source
    export SERIALDEV=/dev/ttyACM1 # optional
    export ARDUINODIR=~/Downloads/arduino-1.0.1
    make
    sudo ARDUINODIR=$ARDUINODIR make upload

Windows/Linux/OS X:
* Run Arduino IDE
* Open TiLDA.ino
* Select correct port under Tools, Serial Port
* Choose Arduino Leonardo under Tools, Board
* Hit File, Upload

