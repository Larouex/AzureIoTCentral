# IoT Central Device Training
## Module 08 - Communicating with Sensors, LED, etc

### We will setup and run a simple Python application to cycle through 3 Led's

* Navigate to Module08 directory
* Run the following command...

```
pip3 install -r requirements.txt
```

* Run the foillowing command...

```
python3 ./mod8.py
```

This will execute the following...

``` Python
import RPi.GPIO as GPIO
import time
import struct 

# --------------------------------------------------------
# Status
# --------------------------------------------------------
Alert = 5
Wait = 6
Good = 16

# --------------------------------------------------------
# Set Mode Leds
# --------------------------------------------------------
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(Alert, GPIO.OUT)
GPIO.setup(Wait, GPIO.OUT)
GPIO.setup(Good, GPIO.OUT)

# --------------------------------------------------------
# Start the Loop...
# --------------------------------------------------------
try:
    while(1):

        print ("Setting the Alert LED [ON]")
        GPIO.output(Alert, GPIO.HIGH)
        time.sleep(5) 
        GPIO.output(Alert, GPIO.LOW)

        print ("Setting the Wait LED [ON]")
        GPIO.output(Wait, GPIO.HIGH)
        time.sleep(5) 
        GPIO.output(Wait, GPIO.LOW)

        print ("Setting the Good LED [ON]")
        GPIO.output(Good, GPIO.HIGH)
        time.sleep(5) 
        GPIO.output(Good, GPIO.LOW)

except (RuntimeError, TypeError, NameError):
    print ("Failed to Set up the LED Routine. Error %s" % RuntimeError)
    time.sleep(10)
    pass
except KeyboardInterrupt:  
    # Turn off all LED's
    GPIO.output(Alert, GPIO.LOW)
    GPIO.output(Wait, GPIO.LOW)
    GPIO.output(Good, GPIO.LOW)
    GPIO.cleanup()
   
```




## [Module 09 - Create your Azure IoT Central Application](../Module09/README.md)