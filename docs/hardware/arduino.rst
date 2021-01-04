Arduino
=======

Arduino can be used as a Falcon output device with the :ref:`serialoutput` processor. It is using in background
the serial library developed by `Lucidar. <https://lucidar.me/en/serialib/cross-plateform-rs232-serial-library>`_

The serial port should be manually specify in the config file of the graph used.

.. note::

    For latency purpose, only a character should be sent.
    In case of a string with a 9600 baud, a character is sent every 10ms.


**Example to read 1 char send by falcon to arduino**

.. code-block::

    int BAUDRATE = 9600;  // bit per second
    char buffer = 'n';

    void setup() {
      Serial.begin(BAUDRATE);     // connect to the serial port at BAUDRATE bits per second
      Serial.setTimeout(1); // 1 ms time-out for serial readout
    }

    void loop () {
      Serial.readBytes(&buffer, 1);
      // Process here depending of the received character
      buffer = 'n';
    }