condiduno
=========

A simple arduino project that uses a DHT22 sensor and the Ethernet Shield (for
use of the SD-Card slot) to save temperature and humidity readings onto an SD
Card.

Libraries used

* SdFat (https://code.google.com/p/sdfatlib/)
* DHTLib (http://playground.arduino.cc/Main/DHTLib#.UxWGAh9p85g)


Output sample:

::

    0, 22.40, 50.40
    1, 22.45, 50.30


A counter use due to the absence of a real-time clock on the arduino board. The
counter is incrementing even after power loss.


Schematics
==========

![alt tag](https://raw.github.com/flazzarini/condiduno/master/schematics_bb.png)
