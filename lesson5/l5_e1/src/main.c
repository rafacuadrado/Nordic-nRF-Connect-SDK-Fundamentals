/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Controlling LEDs through UART. Press 1-3 on your keyboard to toggle LEDS 1-3 on your development
 * kit */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
/* STEP 3 - Include the header file of the UART driver in main.c */
#include <zephyr/drivers/uart.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* STEP 10.1.1 - Define the size of the receive buffer */
#define RECEIVE_BUFF_SIZE 10
/* STEP 10.2 - Define the receiving timeout period */
#define RECEIVE_TIMEOUT 100
/* STEP 5.1 - Get the device pointers of the LEDs through gpio_dt_spec */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
//obtenemos los punteros de los leds, este caso la librería de los gpio nos proporciona una estructura llamada gpio_dt_spec, 
//que contiene toda la información necesaria para controlar un gpio, como el puerto, el pin, el flags, etc. y 
//con la función GPIO_DT_SPEC_GET() le pasamos el nodo del device tree que queremos obtener, en este caso los alias de los
// leds, y el campo gpios que es el que contiene la información del gpio. 
//y lo asignamos a las variables led0, led1 y led2 respectivamente
/* STEP 4.1 - Get the device pointer of the UART hardware */
const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(uart0));// esto me obtiene el punterop al uart0, que es el que 
//esta definido en el device tree, y lo asigna a la variable uart, que es un puntero a struct device
/* STEP 9.1 - Define the transmission buffer, which is a buffer to hold the data to be sent over
 * UART */
static uint8_t tx_buf[] =   {"nRF Connect SDK Fundamentals Course\r\n"
                             "Press 1-3 on your keyboard to toggle LEDS 1-3 on your development kit\r\n"};
/* STEP 10.1.2 - Define the receive buffer */
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
/* STEP 7 - Define the callback function for UART */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {

	case UART_RX_RDY:
		if ((evt->data.rx.len) == 1) {

			if (evt->data.rx.buf[evt->data.rx.offset] == '1') {
				gpio_pin_toggle_dt(&led0);
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '2') {
				gpio_pin_toggle_dt(&led1);
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '3') {
				gpio_pin_toggle_dt(&led2);
			}
		}
		break;
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
		break;

	default:
		break;
	}
}//esta funcion mira si se han recibido datos en el uart si es un 1 activa led 0, si es un 2 activa led1 y 
//si es un 3 activa led2, y si se ha deshabilitado la recepcion vuelve a habilitarla, esta funcion se llamara 
//cada vez que se reciba un dato por el uart, o se deshabilite la recepcion, o se produzca cualquier evento relacionado 
//con el uart, y el evento se pasara como parametro a la funcion para que pueda actuar en consecuencia.
//aqui vemos la funcion data.rx.buf que es el buffer de los datos recibido y exactamente se mira usando [] 
//el offset que es la posicion del buffer donde se ha recibido el dato, y se compara con los caracteres '1', '2' y '3' para activar 
//los leds correspondientes.

int main(void)
{
	int ret;

	/* STEP 4.2 - Verify that the UART device is ready */
		if (!device_is_ready(uart)){
		printk("UART device not ready\r\n");
		return 1 ;
	}//como siempre antes de usar algo comprobamos que está ready
	/* STEP 5.2 - Verify that the LED devices are ready */
	if (!device_is_ready(led0.port)){
		printk("GPIO device is not ready\r\n");
		return 1;
	}//comprobamos leds
	/* STEP 6 - Configure the GPIOs of the LEDs */
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ; 
	}
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ;
	}
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 1 ;
	}//configuramos los pines de los leds como salida y con un valor inicial de activo, es decir, encendidos.
	/* STEP 8 - Register the UART callback function */
	ret = uart_callback_set(uart, uart_cb, NULL);
		if (ret) {
			return 1;
		}//registramos la funcion de callback para el uart, esta funcion se llamara cada vez que se produzca un evento 
		//relacionado con el uart, como la recepcion de datos, la deshabilitacion de la recepcion, etc. y el 
		//evento se pasara como parametro a la funcion para que pueda actuar en consecuencia.
	/* STEP 9.2 - Send the data over UART by calling uart_tx() */
	ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);//con uart tx enviamos datos a la uart
	if (ret) {
		return 1;
	}
	/* STEP 10.3  - Start receiving by calling uart_rx_enable() and pass it the address of the
	 * receive  buffer */
	ret = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
	if (ret) {
		return 1;
	}
	while (1) {
		k_msleep(SLEEP_TIME_MS);
	}
}