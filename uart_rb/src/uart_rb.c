#include <uart_rb.h>

#define UART_RB_PARAM_CHECK(x) \
	if (!(x))              \
	return UART_RB_INVALID_ARG

/**
 * @brief Transfers data from the TX ring buffer to the UART TX DMA.
 *
 * This function checks if there is data available in the TX ring buffer and if so,
 * it transfers it to the UART TX DMA; it must be called for every tx tc event or when user wants to send data.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
static uart_rb_err_t uart_rb_tx_transfer(uart_rb_t *uart_rb)
{
	// Check if the UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	// Check if there is data available in the TX ring buffer
	if (uart_rb->tx_current_size == 0) {
		// Get the length of the linear block of data available for reading from the TX ring buffer
		uart_rb->tx_current_size =
			lwrb_get_linear_block_read_length(&uart_rb->tx_rb);
		if (uart_rb->tx_current_size > 0) {
			// Transmit the data using the UART TX DMA
			HAL_UART_Transmit_DMA(
				uart_rb->huart,
				lwrb_get_linear_block_read_address(
					&uart_rb->tx_rb),
				uart_rb->tx_current_size);

			// Skip the transferred data in the TX ring buffer
			lwrb_skip(&uart_rb->tx_rb, uart_rb->tx_current_size);
		}
	}
	__set_PRIMASK(primask);

	return UART_RB_OK;
}

uart_rb_err_t uart_rb_rx_evt_cb(uart_rb_t *uart_rb, uint16_t pos)
{
	// Check if the UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	// Check if pos has changed(to be able to handle idle and ht events on the same pos)
	if (pos != uart_rb->rx_last_pos) {
		// Calculate the size of the newly received data
		lwrb_sz_t size = (pos > uart_rb->rx_last_pos) ?
					 (pos - uart_rb->rx_last_pos) :
					 (uart_rb->rx_rb_data_size + pos -
					  uart_rb->rx_last_pos);

		// Update the last received position
		uart_rb->rx_last_pos = pos;

		// Advance the ring buffer's write pointer by the size of the received data
		lwrb_advance(&uart_rb->rx_rb, size);
	}

	return UART_RB_OK;
}

uart_rb_err_t uart_rb_tx_tc_cb(uart_rb_t *uart_rb)
{
	// Check if the UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	// Reset the current size of the transmission
	uart_rb->tx_current_size = 0;

	// Trigger a new transmission
	uart_rb_tx_transfer(uart_rb);

	return UART_RB_OK;
}

/**
 * @brief Callback function called when the TX ring buffer is written.
 *
 * This function is called when the TX ring buffer is written. It triggers a new transmission if
 * there is data available in the TX ring buffer.
 *
 * @param buff Pointer to the TX ring buffer instance.
 * @param evt Event type.
 * @param bp Buffer position.
 */
static void uart_rb_txrb_evt_fn(lwrb_t *buff, lwrb_evt_type_t evt, lwrb_sz_t bp)
{
	// Check if the event is a write event
	if (evt == LWRB_EVT_WRITE) {
		// Get the UART ring buffer instance from the buffer argument
		uart_rb_t *uart_rb = lwrb_get_arg(buff);

		// Trigger a new transmission if there is data available in the TX ring buffer
		uart_rb_tx_transfer(uart_rb);
	}
}

uart_rb_err_t uart_rb_init(uart_rb_t *uart_rb, UART_HandleTypeDef *huart,
			   uint8_t *rx_rb_data, lwrb_sz_t rx_rb_data_size,
			   uint8_t *tx_rb_data, lwrb_sz_t tx_rb_data_size)
{
	// Check if UART ring buffer instance, UART handle and data pointers are valid
	UART_RB_PARAM_CHECK(uart_rb);
	UART_RB_PARAM_CHECK(huart);
	UART_RB_PARAM_CHECK(rx_rb_data);
	UART_RB_PARAM_CHECK(tx_rb_data);

	// Check if already initialized
	if (uart_rb->initialized) {
		return UART_RB_INVALID_STATE;
	}

	// Set UART ring buffer data sizes
	uart_rb->rx_rb_data_size = rx_rb_data_size;
	uart_rb->tx_rb_data_size = tx_rb_data_size;

	// Set UART ring buffer data pointers
	uart_rb->rx_rb_data = rx_rb_data;
	uart_rb->tx_rb_data = tx_rb_data;

	// Set UART handle
	uart_rb->huart = huart;

	// Initialize the ring buffers
	if (!lwrb_init(&uart_rb->rx_rb, uart_rb->rx_rb_data,
		       uart_rb->rx_rb_data_size) ||
	    !lwrb_init(&uart_rb->tx_rb, uart_rb->tx_rb_data,
		       uart_rb->tx_rb_data_size)) {
		return UART_RB_RB_FAIL;
	}

	// Set ring buffer arguments for custom context
	lwrb_set_arg(&uart_rb->rx_rb, uart_rb);
	lwrb_set_arg(&uart_rb->tx_rb, uart_rb);

	// Set TX ring buffer event function to start transfer whenever data is written
	lwrb_set_evt_fn(&uart_rb->tx_rb, uart_rb_txrb_evt_fn);

	// Reset RX and TX buffer pointers
	uart_rb->rx_last_pos = 0;
	uart_rb->tx_current_size = 0;

	// Set initialized flag
	uart_rb->initialized = true;

	return UART_RB_OK;
}

uart_rb_err_t uart_rb_deinit(uart_rb_t *uart_rb)
{
	// Check if UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	// Check if initialized and not started
	if (!uart_rb->initialized || uart_rb->started) {
		return UART_RB_INVALID_STATE;
	}

	// Reset the ring buffers
	lwrb_reset(&uart_rb->rx_rb);
	lwrb_reset(&uart_rb->tx_rb);

	// Reset initialized flag
	uart_rb->initialized = false;

	return UART_RB_OK;
}

uart_rb_err_t uart_rb_start(uart_rb_t *uart_rb)
{
	// Check if UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	// Check if already started or not initialized
	if (uart_rb->started || !uart_rb->initialized) {
		return UART_RB_INVALID_STATE;
	}

	// Start reception of data using DMA, receiving until idle
	HAL_UARTEx_ReceiveToIdle_DMA(uart_rb->huart, uart_rb->rx_rb_data,
				     uart_rb->rx_rb_data_size);

	// Set started flag
	uart_rb->started = true;

	return UART_RB_OK;
}

uart_rb_err_t uart_rb_stop(uart_rb_t *uart_rb)
{
	// Check if UART ring buffer instance is valid
	UART_RB_PARAM_CHECK(uart_rb);

	// Check if already stopped or not started
	if (!uart_rb->started) {
		return UART_RB_INVALID_STATE;
	}

	// Abort the current reception of data using DMA
	HAL_UART_AbortReceive(uart_rb->huart);

	// Set stopped flag
	uart_rb->started = false;

	return UART_RB_OK;
}
