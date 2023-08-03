#include "gd32vf103.h"
#include "usart.h" 
#include "eclicw.h"   

int txr=0, txw=0, txq[16]={0}, rxr=0, rxw = 0, rxq[16]={0};                     // 256 Byte wr queue

void u0_TX_Queue(void){
    if (txr!=txw) {                                 // Buffer empty?
       if (usart_flag_get(USART0,USART_FLAG_TBE)) { // ...no! Device redy?
          usart_data_transmit(USART0, txq[txr++]);  //        Yes Write!
          txr%=16;                                 //        wrap around.
        }                                           //        No! Return!
    } else 
       usart_interrupt_disable(USART0, USART_INT_TBE);
}

void ClearQueue(void)
{
  for (int i = 0; i < 16; i++)
  {
    txq[i] = 0;
  }
}

void putch(char ch){
   while (((txw+1)%16)==txr) u0_TX_Queue(); //If buffer full then spin...
   txq[txw++]=ch;                            //...If/when not then store data...
   txw%=16;                                 //...and advance write index!
   usart_interrupt_enable(USART0, USART_INT_TBE);
}
/*
void putch(char ch){
  while (!usart_flag_get(USART0,USART_FLAG_TBE)); // Spin util buffer empty!
  usart_data_transmit(USART0, ch);                // USART0 TX!
}
*/
void putstr(char str[]){
  while (*str) putch(*str++);
}

void u0init(int enable){
     /* initialize USART */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9); //orginalet hade 9 här

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    if (enable) {
        eclicw_enable(USART0_IRQn, 3, 1, &u0_TX_Queue);
    }
}