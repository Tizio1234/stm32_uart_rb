# stm32_uart_rb

Stm32 uart lwrb library

This library is made for implementing two ring buffers and DMA on both tx and rx on a uart for stm32fx(tested only with stm32f4).

### Usage

There is a self explainatory example called "simple", which reads incoming data on uart1 and logs on uart2, look at the contents of the main function, after mx initialization and copy the setup code, adapting it to user preferences.

The library requires some cubemx setup for each uart that the user may want to use, this is a checklist:

1. Enable the uart in the left Connectivity section

2. in the uart configuration menu select asynchronous

3. in the menu below go to DMA settings

4. add dma request,s first for tx, regular mode, it should be fine as created automatically, second for rx, circular mode, leave the rest as it is

5. now go in the NVIC section of the uart configuration, check the checkbox for the uart global interrupt

6. it's time to enable callback registration, go to the upper Project Manager menu, Advanced settings, on the right, there should be a list of callbacks, enable uart and usart

Everything cubemx related is ready, now the library should work.

## Disclaimer:

This is still a very early version of the library, better documentation and better code will come in the future, so if you find a bug, please report it.
