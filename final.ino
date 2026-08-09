#include<avr/interrupt.h>
#include <avr/io.h>
#include<stdint.h>
#include <stdlib.h>

#define c_freq 16000000UL
#define baud 9600
#define ubbr_val (c_freq/(16UL*(baud)))-1
#define ucsra 0xC0
#define ucsrb 0xC1
#define ucsrc 0xC2

volatile int mode_val = 1;
volatile int overflow_counter = 0;
volatile uint8_t *ddrd = (uint8_t*)0x2A;
volatile uint8_t *ddrb = (uint8_t*)0x24;
volatile uint8_t *ddrc = (uint8_t*)0x27;

volatile uint8_t *portd = (uint8_t*)0x2B;
volatile uint8_t *portb = (uint8_t*)0x25;
volatile uint8_t *portc = (uint8_t*)0x28;


volatile uint8_t *pind = (uint8_t*)0x29;
volatile uint8_t *pinb = (uint8_t*)0x23;
volatile uint8_t *pinc = (uint8_t*)0x26;

volatile uint8_t *timer0_ctrl_A = (uint8_t*)0x44;
volatile uint8_t *timer0_ctrl_B = (uint8_t*)0x45;
volatile uint8_t *timer0_flag = (uint8_t*)0x35;
volatile uint8_t *timer0_counter = (uint8_t*)0x46;
volatile uint8_t *timer0_output_compare_A = (uint8_t*)0x47;
volatile uint8_t *timer0_output_compare_B = (uint8_t*)0x48;
volatile uint8_t *timer0_mask = (uint8_t*)0x6E;

volatile uint8_t *avr_interrupt = (uint8_t*)0x5F;
volatile uint8_t *pin_change_mask_0 = (uint8_t*)0x6B;
volatile uint8_t *pin_change_interrupt_ctrl_0 = (uint8_t*)0x68;
volatile uint8_t *interrupt_ctrl_A = (uint8_t*)0x69;
volatile uint8_t *interrupt_mask = (uint8_t*)0x3D;
volatile uint8_t *pin_change_flag = (uint8_t*)0x3B;

volatile uint8_t *timer1_ctrl_A = (uint8_t*)0x80;
volatile uint8_t *timer1_ctrl_B = (uint8_t*)0x81;
volatile uint8_t *timer1_flag = (uint8_t*)0x36;
volatile uint16_t *timer1_counter = (uint16_t*)0x84;
volatile uint8_t *timer1_counter_H = (uint8_t*)0x85;
volatile uint16_t *timer1_output_compare = (uint16_t*)0x88;
volatile uint8_t *timer1_output_compare_L = (uint8_t*)0x88;
volatile uint8_t *timer1_mask = (uint8_t*)0x6F;




volatile uint8_t *ucsrA = (uint8_t*)ucsra;
volatile uint8_t *ucsrB = (uint8_t*)ucsrb;
volatile uint8_t *ucsrC = (uint8_t*)ucsrc;
volatile uint8_t *baudH = (uint8_t*)0xC5;
volatile uint8_t *baudL = (uint8_t*)0xC4;
volatile uint8_t *udr0 = (uint8_t*)0xC6;

int initialise(){
	*baudH = (unsigned char)(ubbr_val>>8);
	*baudL = (unsigned char)(ubbr_val);
	*ucsrB = (1<<3) | (1<<4);
//  *ucsrC = (1<<2) | (1<<3);
	return 0;
}

int trans(unsigned char data){
	while(!(*ucsrA & (1<<5)));
	*udr0 = data;
	return 0;	
}

int recieve(){
	while(!(*ucsrA & (1<<7)));
	
	return *udr0;
}

int prints(unsigned char *data){
	while(*data){
	trans(*data);
	data++;
	}
	return 0;
}




void delay_ms(int ms){
    int target_ticks = 125;
    *timer0_ctrl_A = 0;
    *timer0_ctrl_B = 0;
    *timer0_counter = 0;
    while (ms>0){
        *timer0_counter = 0;
        *timer0_ctrl_B = (1<<1)|(1<<0);
        while(*timer0_counter<target_ticks);
        ms--;
    }
}

void delay_us(int us){
    int target_ticks = 1;
    *timer0_ctrl_A = 0;
    *timer0_ctrl_B = 0;
    *timer0_counter = 0;
    while (us>0){
		*timer0_counter = 0;
        *timer0_ctrl_B |= (1<<1);
        while(*timer0_counter<target_ticks);
        us--;
    }
}

void change_mode(){
	*portb |= (1<<5);
}

int assign_pin(int pin){
	if(pin == 0){
		*portc = 0;
		*portc |= (1<<5);
	}
	if(pin == 1){
		*portc = 0;
		*portc |= (1<<4);
	}
	if(pin == 2){
		*portc = 0;
		*portc |= (1<<3);
	}
	return 0;
}
uint8_t assign_val(int val){
	if(val == 0){
		return 0b00111111;
	}
	if(val == 1){
		return 0b00000110;
	}
	if(val == 2){
		return 0b01011011;
	}
	if(val == 3){
		return 0b01001111;
	}
	if(val == 4){
		return 0b01100110;
	}
	if(val == 5){
		return 0b01101101;
	}
	if(val == 6){
		return 0b01111100;
	}
	if(val == 7){
		return 0b00000111;
	}
	if(val == 8){
		return 0b01111111;
	}
	if(val == 9){
		return 0b01100111;
	}
	
}
ISR(TIMER1_COMPA_vect){ 
    overflow_counter +=1;   
}

ISR(PCINT0_vect){
    if(!(*interrupt_ctrl_A & (1<<0))){

		*timer1_counter = 0;
		overflow_counter = 0;
        *timer1_ctrl_B |= (1<<2);
        *timer1_ctrl_B &= ~(1<<0);
        *timer1_ctrl_B &= ~(1<<1);
        *interrupt_ctrl_A |= (1<<0);
		mode_val +=1;
	//	*portb |= (1<<5);

    }
    else if((*interrupt_ctrl_A & (1<<0))){
        //rising
        *timer1_ctrl_B = 0;
        *interrupt_ctrl_A &= ~(1<<0);
	//	*portb &= ~(1<<5);

    }

}

void display(int dec_val){
		int temp = dec_val;
		int n,r;
		while(temp){
			r=temp%10;
			temp/=10;
			n++;
		}
		int *rs = (int)malloc(n*4);
		n=0;
		temp = dec_val;
		while(temp){
			r=temp%10;
			rs[n] = r;
			temp/=10;
			n++;
		}
		int count;
		if(n==3){
			count =500;
		}
		if(n==2){
		    count = 1000;
		}
		if(n==1){
			count = 2000;
		}
		while(count--){
		int temp_n = n;
		
		int a=0;
		while(temp_n){
			*portd = 0;
			assign_pin(a);
			*portd = (uint8_t)assign_val(rs[a]);
			a++;
			temp_n--;
			if(n == 1){
				*portc = (1<<4)|(1<<3)|(1<<2)|(1<<1);
				*portd = 0b00111111;
			}
			if(n == 2){
				*portc = (1<<3)|(1<<2)|(1<<1);
				*portd = 0b00111111;
			}
			if(n == 3){
				*portc = (1<<2)|(1<<1);
				*portd = 0b00111111;
			}
		}
		}
		
		
}

int main(){
  *ddrd = 0b11111111;
  *ddrb &= ~(1<<0);
  *portb |= (1<<0);
  *ddrb |= (1<<5)|(1<<1)|(1<<2);
  *ddrc = 0b00111111;



  *timer1_ctrl_A = 0;
//  *timer1_ctrl_A |= (1<<6)|(1<<7);
  *timer1_counter = 0;
  *timer1_output_compare = 62499;
  *timer1_mask |= (1<<1);


  *avr_interrupt |= (1<<7);
  *pin_change_mask_0 |= (1<<0);
//  *interrupt_mask |= (1<<0);
  *interrupt_ctrl_A |= (1<<1);
  *interrupt_ctrl_A &= ~(1<<0);
  *pin_change_interrupt_ctrl_0 |= (1<<0);
  *pin_change_flag = (1<<0);
  //initialise();

  volatile float pulse_ticks;
  volatile uint8_t led_val = 0b00000000;


  while(1){
    pulse_ticks = ((uint32_t)(overflow_counter) + ((uint32_t)(*timer1_counter/62500)));
    while(*interrupt_ctrl_A & (1<<0));

	if(pulse_ticks>=2){
		pulse_ticks = 0;
		mode_val-=1;
	}
	if(pulse_ticks <=1){
		if(mode_val >=7 || mode_val <=0 ){
			mode_val =1;
		}
	}
	
	if(mode_val==1){
		led_val += 0b00000001;
		display(led_val);
/*		int a =0;
		while(n){
			assign_pin(a);
			uint8_t value = (uint8_t)assign_val(rs[a]);
			*portd = value;
			a++;
			n--;
			char distance_str[10]; 
		itoa(value, distance_str, 2);
		//itoa(pulse_ticks,distance_str,10);
		prints((unsigned char*)distance_str);
		trans('\n');
		}
		*/
		
	}
	if(mode_val ==2){
	   led_val -= 0b00000001;
		display(led_val);
	}
	if(mode_val==3){
		led_val = (led_val << 1);
		display(led_val);
	}
	if(mode_val==4){
		led_val = (led_val >>1);
		display(led_val);
	}
	if(mode_val == 5){
		int dec_val = led_val;
        int found_prime = 0;
		
        while(!found_prime) {
            dec_val++;
            if (dec_val <= 1) {
                dec_val = 2; 
			}
            int is_prime = 1;
            for(int i = 2; i * i <= dec_val; i++) {
                if(dec_val % i == 0) {
                    is_prime = 0; 
                    break;
                }
            }
    		if(is_prime) {
		        led_val = (uint8_t)dec_val; 
	            found_prime = 1; 
		    }
	    }
	    if (dec_val > 257) {  
	        dec_val = 2;
	    }
		display(led_val);
    }

	
	if(mode_val == 6){
		int dec_val = led_val;
		int temp = --dec_val;
		dec_val += temp;
		led_val = dec_val;
		display(led_val);
	}
	

//	*portd = led_val;
	
	
/*	initialise();
		char distance_str[10]; 
		itoa(pulse_ticks, distance_str, 10);
		//itoa(pulse_ticks,distance_str,10);
		prints((unsigned char*)distance_str);
		trans('\n');
		*/
/* 0 = 0b00111111
	1 = 0b00000110
	2 = 0b01011011
	3 = 0b01001111
	4 = 0b01100110
	5 = 0b01101101
	6 = 0b01111100
	7 = 0b00000111
	8 = 0b01111111
	9 = 0b01100111
*/
  }
}