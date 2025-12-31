// Authenticated Device Control Interface
//Version 1.0 (more features coming soon)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h> 

#define BUFFER_SIZE 32

volatile char rx_buffer[BUFFER_SIZE];
volatile uint8_t rx_index = 0;
volatile uint8_t cmd_ready = 0; 
uint8_t logged_in = 0;          //flag


void uartinit(uint16_t baud_setting) {
    // setting Baud Rate (103 = 9600 baud @ 16MHz)
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;

    // Enable RX (Receive), TX (Transmit), and RX INTERRUPT (RXCIE0)
    // RXCIE0 bit enables the ISR
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

void writedata(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}


void print_msg(const char* str) {
    while (*str) {
        writedata(*str++);
    }
}


ISR(USART_RX_vect) {
    char received = UDR0; //reading the register immediately

    //checking for "Enter" key (Newline \n or Carriage Return \r)
    if (received == '\n' || received == '\r') {
        if (rx_index > 0) { 
            rx_buffer[rx_index] = '\0'; 
            cmd_ready = 1;              
            rx_index = 0;              
        }
    } 
    else {
     
        if (rx_index < BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = received;
            
        
            if(logged_in) writedata(received); 
            else writedata('*'); 
        }
    }
}


void process_command() {
    print_msg("\r\n"); 

    //MODE 1: LOCKED (checking Password)
    if (logged_in == 0) {
      
        if (strcmp((char*)rx_buffer, "1357") == 0) {
            logged_in = 1;
            print_msg("Welcome Vijeeth.What you want to do today.\r\nadmin> ");
        } else {
            print_msg("Access Denied.\r\nlogin: ");
        }
    }
    
    // MODE 2: ADMIN 
    else {
        if (strcmp((char*)rx_buffer, "led on") == 0) {
            PORTB |= (1 << 5); // Pin 13 ON
            print_msg("OK: LED is ON\r\n");
        }
        else if (strcmp((char*)rx_buffer, "led off") == 0) {
            PORTB &= ~(1 << 5); // Pin 13 OFF
            print_msg("OK: LED is OFF\r\n");
        }
        else if (strcmp((char*)rx_buffer, "logout") == 0) {
            logged_in = 0;
            print_msg("Logged out.\r\nlogin: ");
        }
        else {
            print_msg("Unknown Command.\r\n");
        }
        
       
        if (logged_in) print_msg("admin> ");
    }

    cmd_ready = 0; 
}

void setup() {
  
    uartinit(103);
    

    DDRB |= (1 << 5); 

    sei();

    print_msg("System Ready.\r\nlogin: ");
}

void loop() {

    if (cmd_ready) {
        process_command();
    }
}