#include <arch/irq.h>
#include <arch/io.h>
#include <config.h>
#include <stdint.h>
#include <uart.h>

struct uart_regs {
	uint32_t divisor;
	uint32_t rx_data;
	uint32_t rx_rdy;
	uint32_t rx_err;
	uint32_t tx_data;
	uint32_t tx_rdy;
	uint32_t zero0; // reserved
	uint32_t zero1; // reserved
	uint32_t ev_status;
	uint32_t ev_pending;
	uint32_t ev_enable;
} __packed;

static inline void *uart_baseptr(void)
{
	uintptr_t uart_base = CONFIG_UART_START;
	return (void *)uart_base;
}

#define EV_RX_RDY 1
#define EV_RX_ERR 2
#define EV_TX_MTY 4

#define RX_RINGBUF_SIZE (1 << CONFIG_UART_RX_RINGBUF_SIZE_LOG2)
#define RX_RINGBUF_MASK (RX_RINGBUF_SIZE - 1)

static char rx_buf[RX_RINGBUF_SIZE];
static volatile unsigned int rx_produce;
static unsigned int rx_consume;

#define TX_RINGBUF_SIZE (1 << CONFIG_UART_TX_RINGBUF_SIZE_LOG2)
#define TX_RINGBUF_MASK (TX_RINGBUF_SIZE - 1)

static char tx_buf[TX_RINGBUF_SIZE];
static unsigned int tx_produce;
static volatile unsigned int tx_consume;

void uart_isr(void)
{
	struct uart_regs *regs = uart_baseptr();
	uint32_t pending, enable;
	uint32_t rx_produce_next;

	pending = read32(&regs->ev_pending);
	enable  = read32(&regs->ev_enable);

	if (pending & enable & EV_RX_RDY) {
		while (read32(&regs->rx_rdy)) {
			rx_produce_next = (rx_produce + 1) & RX_RINGBUF_MASK;
			if (rx_produce_next != rx_consume) {
				rx_buf[rx_produce] = read32(&regs->rx_data);
				rx_produce = rx_produce_next;
			} else {
				write32(&regs->ev_enable, enable & ~EV_RX_RDY);
				break;
			}
		}
	}

	if (pending & enable & EV_TX_MTY) {
		write32(&regs->ev_pending, EV_TX_MTY);
		while ((tx_consume != tx_produce) && read32(&regs->tx_rdy)) {
			write32(&regs->tx_data, tx_buf[tx_consume]);
			tx_consume = (tx_consume + 1) & TX_RINGBUF_MASK;
		}
	}
}

/* Do not use in interrupt handlers! */
char uart_read(void)
{
	struct uart_regs *regs = uart_baseptr();
	char c;
	uint32_t ie = irq_getie();

	if (ie) {
		while (rx_consume == rx_produce);
	} else if (rx_consume == rx_produce) {
		return 0;
	}

	c = rx_buf[rx_consume];
	rx_consume = (rx_consume + 1) & RX_RINGBUF_MASK;

	if (ie) {
		if (rx_consume == rx_produce) {
			uint32_t enable = read32(&regs->ev_enable);
			if ((enable & EV_RX_RDY) == 0) {
				write32(&regs->ev_enable, enable | EV_RX_RDY);
			}
		}
	}

	return c;
}

int uart_read_nonblock(void)
{
	return (rx_consume != rx_produce);
}

void uart_write(char c)
{
	struct uart_regs *regs = uart_baseptr();
	uint32_t oldmask;
	uint32_t tx_produce_next = (tx_produce + 1) & TX_RINGBUF_MASK;

	if (irq_getie()) {
		while (tx_produce_next == tx_consume);
	} else if (tx_produce_next == tx_consume) {
		return;
	}

	oldmask = irq_getmask();
	irq_setmask(oldmask & ~(1 << CONFIG_UART_IRQNO));
	if ((tx_consume != tx_produce) || !read32(&regs->tx_rdy)) {
		tx_buf[tx_produce] = c;
		tx_produce = tx_produce_next;
	} else {
		write32(&regs->tx_data, c);
	}
	irq_setmask(oldmask);
}

void uart_init(void)
{
	struct uart_regs *regs = uart_baseptr();
	uint32_t pending;

	rx_produce = 0;
	rx_consume = 0;
	tx_produce = 0;
	tx_consume = 0;

	pending = read32(&regs->ev_pending);
	write32(&regs->ev_pending, pending);
	write32(&regs->ev_enable, EV_RX_RDY|EV_TX_MTY);
	irq_setmask(irq_getmask() | (1 << CONFIG_UART_IRQNO));
}

void uart_sync(void)
{
	while (tx_consume != tx_produce);
}
