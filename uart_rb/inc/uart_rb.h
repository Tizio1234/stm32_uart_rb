#ifndef UART_RB_H
#define UART_RB_H

#include <lwrb/lwrb.h>
#include <usart.h>

#include <stdbool.h>

typedef struct
{
    lwrb_t rx_rb;
    lwrb_t tx_rb;

    uint8_t *rx_rb_data;
    uint8_t *tx_rb_data;

    lwrb_sz_t rx_rb_data_size;
    lwrb_sz_t tx_rb_data_size;

    UART_HandleTypeDef *huart;

    uint16_t rx_last_pos;

    uint16_t tx_current_size;

    bool initialized;
    bool started;
} uart_rb_t;

typedef enum {
    UART_RB_OK = 0,
    UART_RB_FAIL,
    UART_RB_INVALID_STATE,
    UART_RB_INVALID_ARG,
    UART_RB_RB_FAIL,
    UART_RB_DMA_FAIL,
    UART_RB_UART_FAIL
} uart_rb_err_t;

#define UART_RB_GET_RX_RB(x) (&(x)->rx_rb)
#define UART_RB_GET_TX_RB(x) (&(x)->tx_rb)

/**
 * @brief Callback function for advancing rx ring buffer.
 *
 * This function must be called by user when an uart rx event occurs
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 * @param pos Pos given in the uart rx evt callback
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_rx_evt_cb(uart_rb_t *uart_rb, uint16_t pos);
/**
 * @brief Callback function called when a transmission has completed.
 *
 * This function must be called by user when a tx complete event occurs.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_tx_tc_cb(uart_rb_t *uart_rb);

/**
 * @brief Initializes the UART ring buffer instance.
 *
 * Initializes the UART ring buffer instance with the provided parameters.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 * @param huart Pointer to the UART handle.
 * @param rx_rb_data Pointer to the RX ring buffer data.
 * @param rx_rb_data_size Size of the RX ring buffer data.
 * @param tx_rb_data Pointer to the TX ring buffer data.
 * @param tx_rb_data_size Size of the TX ring buffer data.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_init(uart_rb_t *uart_rb,
                 UART_HandleTypeDef *huart,
                 uint8_t *rx_rb_data, lwrb_sz_t rx_rb_data_size,
                 uint8_t *tx_rb_data, lwrb_sz_t tx_rb_data_size);
/**
 * @brief Deinitializes the UART ring buffer instance.
 *
 * Deinitializes the UART ring buffer instance.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_deinit(uart_rb_t *uart_rb);

/**
 * @brief Starts the UART ring buffer instance.
 *
 * Starts the UART ring buffer instance, enabling the reception of data.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_start(uart_rb_t *uart_rb);
/**
 * @brief Stops the UART ring buffer instance.
 *
 * Stops the UART ring buffer instance, disabling the reception of data.
 *
 * @param uart_rb Pointer to the UART ring buffer instance.
 *
 * @return UART_RB_OK on success, or an error code on failure.
 */
uart_rb_err_t uart_rb_stop(uart_rb_t *uart_rb);

#endif /* UART_RB_H */
