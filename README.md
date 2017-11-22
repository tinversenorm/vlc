# vlc
Protocol code for VLC systems

## Protocol

Frequency keying: "0" is a flashing LED at 30KHz, "1" is at 20KHz.

## Hardware

Using a two stage amplifier and a PIN photodiode receiver. The diode will generate a photocurrent, which is amplified by the op amps and then fed into an embedded system (Raspberry Pi). The transmitter is simply an LED/resistor pair powered by the digital pin on the embedded system.

Currently using the LM741 op amp, considering switching to the TL081.

Past hardware solutions:
- Phototransistor and resistor = received the signal but was slow to amplify and difficult to control amplification (had to bias the transistor)
- Photodiode and one stage amplifier = received the signal, slow to amplify (because a very large resistor was needed), difficult to control DC offset
- Photoresistor = extremely slow and therefore not useful past very low bitrates (100 bits/s)
- LED in reverse bias = very low photocurrent, large amplifier needed = lots of noise

## Software

Using the DSP library "liquid_dsp" and a Raspberry Pi. The software will read from the built in ADC and use a fast fourier transform to identify dominant frequency.

Working on implementing a windowed-sinc band pass filter for the required frequencies.

Past software solutions:
- Arduino with hardware receiver = easy to program but very small on chip memory and weak hardware/slow ADC
