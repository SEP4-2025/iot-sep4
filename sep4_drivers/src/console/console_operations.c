#include "console_operations.h"
#include "uart.h"
#include "stdbool.h"

static uint8_t _buff[100];
static uint8_t _index = 0;
volatile static bool _done = false;

void console_rx(uint8_t _rx)
{
  uart_send_blocking(USART_0, _rx);
  if (('\r' != _rx) && ('\n' != _rx))
  {
    if (_index < 100 - 1)
    {
      _buff[_index++] = _rx;
    }
  }
  else
  {
    _buff[_index] = '\0';
    _index = 0;
    _done = true;
    uart_send_blocking(USART_0, '\n');
    //        uart_send_string_blocking(USART_0, (char*)_buff);
  }
}