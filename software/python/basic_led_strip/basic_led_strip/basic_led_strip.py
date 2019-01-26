"""
================
led_strip
================

    This module implements the serial interface to the led_strip firmware which provides and
    interface to ws2812 "neopixel" led strips.

    :copyright: (c) 2019  Will Dickson 
    :license: MIT, see LICENSE.txt for more details.
"""
from __future__ import print_function
import serial
import time
import json
import atexit


class BasicLedStrip(serial.Serial):

    """Provides a high level interface performing serial communications with an neopixel style 
    led strip. 

    :param str port: serial port associated with the Rodeostat, e.g. /dev/ttyACM0, COM1, etc.

    :Keyword Arguments:


    """
    
    ResetSleepDt = 2.0
    Baudrate = 115200

    def __init__(self, port, timeout=10.0, debug=False):
        """Constructor

        """
        params = {'baudrate': self.Baudrate, 'timeout': timeout}
        super(BasicLedStrip,self).__init__(port,**params)
        time.sleep(self.ResetSleepDt)
        while self.inWaiting() > 0:
            val = self.read()

    def set(self, led_number, rgb_value, mode='inclusive'):
        """Set the value of led and location=led_number to the specified rgb_value  
        where 

        led_number  = position of led in the array
        rgb_value   = tuple of red, green, blue values 0-255 
            
        """
        r,g,b = rgb_value
        if mode == 'exclusive' or mode == 'excl':
            cmd_dict = {'cmd':'xset', 'i':led_number, 'r':r, 'g': g, 'b': b}
        elif mode == 'inclusive' or mode == 'incl':
            cmd_dict = {'cmd':'iset', 'i':led_number, 'r':r, 'g': g, 'b': b}
        else:
            raise ValueError, "mode must be 'inclusive' or 'exclusive'"
        rsp = self.send_cmd(cmd_dict)

    def set_all(self, rgb_value):
        """Sets all leds to the same r,g,b values given by rgb_value

        """
        r,g,b = rgb_value
        cmd_dict = {'cmd':'aset', 'r':r, 'g':g, 'b':b }
        rsp = self.send_cmd(cmd_dict)

    def off(self):
        """Turns all leds off

        """
        cmd_dict = {'cmd':'off'}
        rsp = self.send_cmd(cmd_dict)

    def number_of_leds(self):
        """Gets the number of leds controllable by the firmware. 

        """
        cmd_dict = {'cmd':'num'}
        rsp = self.send_cmd(cmd_dict)
        return rsp['num']

    def send_cmd(self,cmd_dict):
        """Sends a command to the device.  Low-level method - command is specified 
        using command dictionary.

        """
        cmd_json = json.dumps(cmd_dict) + '\n'
        self.write(cmd_json.encode())
        rsp_json = self.readline()
        rsp_json = rsp_json.strip()
        rsp_dict = json.loads(rsp_json.decode())
        self.check_cmd_msg(cmd_dict,rsp_dict)
        return rsp_dict

    def check_cmd_msg(self,cmd_dict,rsp_dict):
        self.check_for_success(rsp_dict)

    def check_for_success(self,rsp_dict):
        success = rsp_dict["ok"]
        if not success: 
            raise IOError('{0}, {1}'.format(rsp_dict['msg'], rsp_dict))



# Utility
# -------------------------------------------------------------------------------------------------
def rgb_to_uint24(r,g,b):
    val = clamp(r,0,255) << 16 | clamp(g,0,255) << 8 | clamp(b,0,255)
    return val

def uint24_to_rgb(val):
    b = (val & 0x0000ff)
    g = (val & 0x00ff00) >> 8
    r = (val & 0xff0000) >> 16
    return r,g,b

def clamp(x,lower,upper):
    return min(max(lower,x),upper)



