#include <stdio.h>       /* standard I/O .h-file              */
#include <stdarg.h>      /* variable argument handling         */
#include <intrins.h>     /* compiler intrinsic functions      */
#include <regst10f269.h> /* ST10F269 specific registers   */

/* Standard Integer Types definition for older C90 compilers */
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;  /* 16-bit on C166 */
typedef unsigned long uint32_t; /* 32-bit on C166 */

static void my_putchar(char c)
{
  while (!S0TIR)
    ; /* wait for previously emitted data */
  S0TIR = 0;
  S0TBUF = c;
  while (!S0TIR)
    ;        /* wait for THIS byte to physically finish */
  S0TIR = 1; /* arm the flag for next operations */
}

static void uart_printf(const char *fmt)
{
  char *s = (char *)fmt;
  while (*s != '\0')
  {
    my_putchar(*s++);
  }
}

static char nibble_to_hex_char(uint8_t nibble)
{
  nibble &= 0x0Fu;
  if (nibble <= 9u)
  {
    return (char)('0' + nibble);
  }
  return (char)('A' + (nibble - 10u));
}

static void print_byte(uint8_t hex)
{
  my_putchar(nibble_to_hex_char((uint8_t)(hex >> 4)));
  my_putchar(nibble_to_hex_char(hex));
}

static void init_spi()
{
  /* Disable the peripheral to allow settings*/
  SSCEN = (volatile)0;

  /* ST10F269/C166 uses SSCCON for SSC (SPI) control */
  SSCCON = (volatile)0x7fd7;

  /* Set baud rate to 10k */
  SSCBR = (volatile)0x007b;

  /* Enable back the peripheral */
  SSCEN = (volatile)1;
}

static uint8_t spi_send8(uint8_t reg)
{
  /* Reset SSCEIC */
  SSCRIC &= (volatile) ~0x0080;

  /* Start the transaction */
  SSCTB = (volatile)reg;

  /* Wait until not busy anymore*/
  while (!((volatile)SSCRIC & 0x0080))
    ;

  /* Return the read value*/
  return (uint8_t)(volatile)SSCRB;
}

static uint16_t spi_read8(uint8_t reg)
{
  uint8_t result;

  /* Set the CS to low*/
  P4 = P4 & 0xffef;

  (void)spi_send8(reg);
  result = spi_send8(0x00);

  /* Set the CS to high */
  P4 |= 0x0010;

  /* Return the read value*/
  return result;
}

/****************/
/* main program */
/****************/
void main(void)
{ /* execution starts here               */
  uint32_t delay;
  uint16_t counter = 0;
  uint16_t reg_content = 0;

  /* initialize the serial interface     */
#ifndef MCB167    /* do not initialize if you use Monitor-166      */
  P3 |= 0x0400;   /* SET PORT 3.10 OUTPUT LATCH (TXD)              */
  DP3 |= 0x0400;  /* SET PORT 3.10 DIRECTION CONTROL (TXD OUTPUT)  */
  DP3 &= 0xF7FF;  /* RESET PORT 3.11 DIRECTION CONTROL (RXD INPUT) */
  S0TIC = 0x80;   /* SET TRANSMIT INTERRUPT FLAG                   */
  S0RIC = 0x00;   /* DELETE RECEIVE INTERRUPT FLAG                 */
  S0BG = 0x51;    /* SET sBAUDRATE TO 9600 BAUD                     */
  S0CON = 0x8011; /* SET SERIAL MODE                               */
#endif

  /* Initial Serial Setup here... */
  /* Init CS */
  DP4 |= 0x0010;
  P4 |= 0x0010;

  /* Set output pins */
  DP3 |= 0x2200;
  P3 |= 0x2200;

  /* Initialize the SPI line */
  init_spi();

  /* Status register must be 0x00 at start */
  uart_printf("Starting EEPROM Reading!\r\n");

  /* Read status register*/
  P4 = P4 & 0xffef;
  (void)spi_send8(0x05);
  reg_content = spi_send8(0x00);
  P4 |= 0x0010;

  uart_printf("Status register: ");
  uart_printf("0x");
  print_byte(reg_content);
  uart_printf("\r\n");

  /* Send write enable command */
  P4 = P4 & 0xffef;
  (void)spi_send8(0x06);
  P4 |= 0x0010;

  /* Read again the status register */
  P4 = P4 & 0xffef;
  (void)spi_send8(0x05);
  reg_content = spi_send8(0x00);
  P4 |= 0x0010;

  uart_printf("Status register: ");
  uart_printf("0x");
  print_byte(reg_content);
  uart_printf("\r\n");

  for (delay = 0; delay < 1000000; delay++)
  {
    _srvwdt_();
    _nop_();
  }

  for (counter = 0; counter <= 0x07ff; counter++)
  {
    /* Read eeprom status register */
    P4 = P4 & 0xffef;
    (void)spi_send8(0x03);
    (void)spi_send8((counter & 0xFF00) >> 8);
    (void)spi_send8(counter & 0xFF);
    reg_content = spi_send8(0x00);
    P4 |= 0x0010;

    uart_printf("0x");
    print_byte((counter & 0xFF00) >> 8);
    print_byte(counter & 0xFF);
    uart_printf(" -> ");
    print_byte(reg_content);
    uart_printf("\r\n");

    /* Delay for the next instruction*/
    for (delay = 0; delay < 10000; delay++)
    {
      _nop_();
    }
  }
}
