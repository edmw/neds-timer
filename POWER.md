## Powering Ned’s Timer

Ned‘s Timer is powered by four nickel-metal hydride rechargeable cells in a 4-cell battery holder. Four NiMH cells
provide 4.8&nbsp;Volts generally (1.2&nbsp;Volts each).

### Operating supply voltage

|Component|Voltage|
|--|--|
|Flora Electronic Platform|**3.5V - 16V**|
|NeoPixel Ring|**5V ±0.5%** (lower voltages are acceptable, the LEDs will be slightly dimmer)|

### Voltage Regulator

NiMH cells can provide up to 1.4&nbsp;Volts when fully charged. Four cells will add up to 5.6&nbsp;Volts in total then. Which is
to much for the NeoPixel Ring.

A voltage regulator outputs clean 3.3&nbsp;Volts for Flora Electronic Platform and NeoPixel Ring:

![Ned’s Timer voltage regulator](/FILES/regulator.png?raw=true "Ned’s Timer voltage regulator")
