==Arduino control of an amplifier via wired IR

This project enables an arduino to control an amplifier's state via wired IR
based on whether a sonos server is playing or not. It does this by polling a
local sonos server, determining it's state, and then sending power on/off and
source input commands to pin's connected to the amplifier's remote input.

Currently playing information is also retrieved and displayed on an LCD screen.
