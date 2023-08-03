#include "gd32vf103.h"
#include "usb_serial_if.h"
#include "usb_delay.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>

#define USE_USB
#define USE_USB_PRINTF
#define BUFFER_SIZE 512

// Function prototypes
void send_data(char data[]);
void empty_usart0_receive_buffer(void);
void enter_command_mode(void);

void empty_buffer(char data1[], char data2[]){
    for(int i = 0; i < BUFFER_SIZE; i++){
        data1[i] = '\0';
        data2[i] = '\0';
    }
}

int main(void){
    // Initialize variables
    uint32_t count = 0;
    char usb_data_buffer[BUFFER_SIZE] = {'\0'};
    char message[BUFFER_SIZE] = {'\0'};
    uint32_t message_index = 0;

    // Initialize USB and USART
	usb_delay_1ms(1);
    configure_usb_serial();
    u0init(1);

    // Enter command mode for the Bluetooth module
    enter_command_mode();

    // Wait until USB serial becomes available
	while (!usb_serial_available()) {
        usb_delay_1ms(100);
    }

    while (1){
        // Read new messages into the buffer from USB serial
        count = read_usb_serial(usb_data_buffer);

        if (count > 0){
            // If a message was received, echo back to the sender
            printf("%s", usb_data_buffer);
            for (int i = 0; i < count; i++) {
                char current_char = usb_data_buffer[i];

                if (current_char == '\r'){
                    // Null-terminate the message and send it
                    message[message_index] = '\0';
                    send_data(message);
                    printf("\r\n");

                    // Clear the message buffer for the next message
                    memset(message, 0, sizeof(message));

                    // empty_buffer(message, usb_data_buffer);
                    message_index = 0;
                } else {
                    // Check if the buffer is full, and if so, reset it
                    if (message_index >= BUFFER_SIZE - 1){ // Leave space for the null-terminator
                        printf("Message buffer full. Resetting...\r\n");
                        memset(message, 0, sizeof(message));

                        // empty_buffer(message, usb_data_buffer);
                        message_index = 0;
                    }
                    if(current_char == '\b'){
                        message[message_index--] = '\0';
                        printf(" ");
                    }else{
                        // Add the character to the message buffer
                        message[message_index++] = current_char;
                    }

                    
                }
            }
        }

        // Check if data is available on USART0 (received from the Bluetooth module)
        while (usart_flag_get(USART0, USART_FLAG_RBNE)){
            // Read the character from USART0 and print it to the console via USB
            char received_char = usart_data_receive(USART0);
            printf("%c", received_char);
        }
        fflush(0);
    }
}

// Function to send data to the Bluetooth module
void send_data(char data[]){
    // Handles the case when only carriage return is sent
    if (data[0] == '\0'){
        send_data(" ");
        fflush(0);
        return;
    }
    
    if(!strcmp(data,"$$$")){
        enter_command_mode();
        return;
    }

    int index = 0;
    while (data[index] != '\0'){
        usart_data_transmit(USART0, data[index++]);
        usb_delay_1ms(1);
    }
    // Add a carriage return to mark the end of the message
    usart_data_transmit(USART0, '\r');

}

// Function to empty the USART0 receive buffer
void empty_usart0_receive_buffer(void){
    while (usart_flag_get(USART0, USART_FLAG_RBNE)) {
        // Read and discard the received data
        usart_data_receive(USART0);
    }
}

// Function to enter command mode for the Bluetooth module
void enter_command_mode(void){
    usb_delay_1ms(1000);
    int index = 0;
    for(int i = 0; i < 3; i++){
        usart_data_transmit(USART0, '$');
        usb_delay_1ms(1);
    }
    usb_delay_1ms(1000);
    // Add a carriage return to mark the end of the message
    usart_data_transmit(USART0, '\r');
    empty_usart0_receive_buffer();
    printf("\r\nCMD> ");
}
