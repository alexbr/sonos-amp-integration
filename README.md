# Arduino Sonos and Amplifier Controller/Integrator

This project enables an arduino to control an amplifier's state via wired IR and optionally a 12V trigger. This was specifically written for a Yamaha A-S2100.

Sonos state is polled regularly. If the sonos is playing, an IR power toggle, or, preferrably, 12V trigger on, will be sent to the amp. If it is stopped, an IR power toggle or 12V trigger off will be sent to the amp.

A webserver also runs on port 80 which accepts the following simple commands:
  * /pwron
  * /pwroff
  * /volup[/\<amount>]
  * /voldown[/\<amount>]
  * /mute
  * /bal
  * /tuner
  * /phono

Additional commands should be easy to add.

If an LCD/button shield is attached, currently playing information is retrieved and displayed on the LCD screen. The buttons can be used to toggle phono input (at which point sonos polling stops or restarts), issue play, pause, next, and previous commands.

The Sonos class is a slimmed down version of the excellent libray at https://github.com/tmittet/sonos
