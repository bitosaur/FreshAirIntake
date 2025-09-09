Fresh Air Intake System based on ESP8266
by Bitosaur
Distributed under MIT License 2019.
Absolutely no Warrantees. Use at your own risk.
The software is provided with all its faults. No claim is made for suitability in any particular situation or fitness for a particular purpose.

Problem: When outside weather condition is such that the HVAC system will benefit from importing air from outside to 
suppliment its heating or cooling operation, a fresh air intake valve actuator will be opened while the HVAC fan is on so outside air can be drawn in. 
The intake valve actuator must be closed at other times so as to not lose indoor air or suck in outdoor air that is not going to aid the HVAC system.
The project assumes:
1) A fresh air intake valve that is "normally closed".
2) The existance of a home automation system, in this case a MiCasaVera which communicates with indoor thermostat for sensor reading as well as for 
calling for heat, cooling and fan.
3) A weather.com API account to read outside weather conditions from close proximity of the HVAC location.
4) An ESP8266 module that commnucates with Wifi Network.
