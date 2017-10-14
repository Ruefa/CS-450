
/*
This code will cause a TekBot connected to the AVR board to
move forward and when it touches an obstacle, it will reverse
and turn away from the obstacle and resume forward motion.

PORT MAP
Port B, Pin 4 -> Output -> Right Motor Enable
Port B, Pin 5 -> Output -> Right Motor Direction
Port B, Pin 7 -> Output -> Left Motor Enable
Port B, Pin 6 -> Output -> Left Motor Direction
Port D, Pin 1 -> Input -> Left Whisker
Port D, Pin 0 -> Input -> Right Whisker
*/

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

int main(void)
{
	DDRB = 0b11110000;	//Configure Port B for motor output
	PORTB = 0b11110000;	//Set initial value for port B with both motors disabled

	DDRD = 0b00000000;	//configure Port D for whisker input

	while (1) // loop forever
	{
		PORTB = 0b01100000;		//Move forwards
		if(PIND == 0b11111110){	//right whisker
			PORTB = 0b00000000;	//Reverse motors
			_delay_ms(1000);	//wait for 1 second

			PORTB = 0b00100000; //Turn left
			_delay_ms(1000);	//wait for 1 second
		}
		else if(PIND == 0b11111101 || PIND == 0b11111100){	//left whisker
			PORTB = 0b00000000;	//Reverse motors
			_delay_ms(1000);	//wait for 1 second

			PORTB = 0b01000000; //Turn right
			_delay_ms(1000);	//wait for 1 second
		}
	}
}