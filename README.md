# Arduino Sonos and Amplifier Controller/Integrator

This project enables an arduino to control an amplifier's state via wired IR. This was specifically written for a Yamaha A-S2100.
Sonos state is polled and based on whether the Sonos is playing or stopped/paused, an IR power command will be sent to the output pins.

A webserver also runs on port 80 which accepts the following simple commands:
  * /volup/<increase amount>
  * /voldown/<decrease amount>
  * /mute
  * /tuner
  * /phono
  
Additional commands should be easy to add.

If an LCD/button shield is attached, durrently playing information is retrieved and displayed on the LCD screen. The buttons can be used to issue play, pause, next, and previous commands.

The Sonos class is a slimmed down version of the excellent libray at https://github.com/tmittet/sonos
