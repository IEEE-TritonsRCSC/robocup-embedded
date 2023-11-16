**STM32 Interrupt/UART Example**

In this example, I use a gpio interrupt to send a message to the host computer over UART.

***MAIN LOOP***


![Screenshot from 2023-11-15 18-33-41](https://github.com/IEEE-TritonsRCSC/robocup-mechatronics/assets/121917210/41354a26-09aa-4ab5-8968-7c3f0542863b)


In the main loop, we blink the onboard LED off and on and output "LED Toggle" over UART.

***CALLBACK FUNCTION***


![Screenshot from 2023-11-15 18-33-41](https://github.com/IEEE-TritonsRCSC/robocup-mechatronics/assets/121917210/9375de08-6fd5-4f73-a039-2160733b49b6)

This is the callback function that is executed when the button is pressed. In the ioc file, it is configured to execute on the rising edge of the clock. Because the button is active low, this means the message will be sent upon releasing the button.

***SAMPLE OUTPUT***


![Screenshot from 2023-11-15 18-45-29](https://github.com/IEEE-TritonsRCSC/robocup-mechatronics/assets/121917210/689c22ed-358a-460f-abe4-059abef9570d)


Here is an example output from the program. We can see that the stm32 outputs "LED Toggle" to the serial port every second. However, when the onboard button is pressed, the interrupt is triggered and the MCU outputs "Interrupt" to the serial port.
