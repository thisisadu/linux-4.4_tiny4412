#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_s3c.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/of.h>
#include <asm/irq.h>
#include "mysamsung.h"

static inline struct exy4412_uart_port *to_ourport(struct uart_port *port)
{
	return container_of(port, struct exy4412_uart_port, port);
}

/* translate a port to the device name */

static inline const char *exy4412_serial_portname(struct uart_port *port)
{
	return to_platform_device(port->dev)->name;
}

static int exy4412_serial_txempty_nofifo(struct uart_port *port)
{
	return rd_regl(port, S3C2410_UTRSTAT) & S3C2410_UTRSTAT_TXE;
}

static unsigned int s3c24xx_serial_tx_empty(struct uart_port *port)
{
	struct exy4412_uart_info *info = exy4412_port_to_info(port);
	unsigned long ufstat = rd_regl(port, S3C2410_UFSTAT);
	unsigned long ufcon = rd_regl(port, S3C2410_UFCON);

	if (ufcon & S3C2410_UFCON_FIFOMODE) {
		if ((ufstat & info->tx_fifomask) != 0 ||
		    (ufstat & info->tx_fifofull))
			return 0;

		return 1;
	}

	return exy4412_serial_txempty_nofifo(port);
}

static void exy4412_serial_rx_enable(struct uart_port *port)
{
}

static void exy4412_serial_rx_disable(struct uart_port *port)
{
}

static void exy4412_serial_stop_tx(struct uart_port *port)
{
	struct s3c24xx_uart_port *ourport = to_ourport(port);
	struct circ_buf *xmit = &port->state->xmit;

	if (!tx_enabled(port))
		return;

	if (s3c24xx_serial_has_interrupt_mask(port))
		__set_bit(S3C64XX_UINTM_TXD, portaddrl(port, S3C64XX_UINTM));

	tx_enabled(port) = 0;
	ourport->tx_in_progress = 0;

	if (port->flags & UPF_CONS_FLOW)
		s3c24xx_serial_rx_enable(port);

	ourport->tx_mode = 0;
}

static void enable_tx_pio(struct exy4412_uart_port *ourport)
{
}

static void exy4412_serial_start_tx_pio(struct exy4412_uart_port *ourport)
{
	if (ourport->tx_mode != EXY4412_TX_PIO)
		enable_tx_pio(ourport);
}

static void exy4412_serial_start_next_tx(struct exy4412_uart_port *ourport)
{
}

static void exy4412_serial_start_tx(struct uart_port *port)
{
	struct s3c24xx_uart_port *ourport = to_ourport(port);

	if (!tx_enabled(port)) {
		if (port->flags & UPF_CONS_FLOW)
			s3c24xx_serial_rx_disable(port);

		tx_enabled(port) = 1;
		if (!ourport->dma || !ourport->dma->tx_chan)
			s3c24xx_serial_start_tx_pio(ourport);
	}
}

static void exy4412_uart_copy_rx_to_tty(struct exy4412_uart_port *ourport,
		struct tty_port *tty, int count)
{
}

static void exy4412_serial_stop_rx(struct uart_port *port)
{
}

static inline struct exy4412_uart_info
	*exy4412_port_to_info(struct uart_port *port)
{
	return to_ourport(port)->info;
}

static inline struct s3c2410_uartcfg
	*exy4412_port_to_cfg(struct uart_port *port)
{
	struct exy4412_uart_port *ourport;

	if (port->dev == NULL)
		return NULL;

	ourport = container_of(port, struct exy4412_uart_port, port);
	return ourport->cfg;
}

static int exy4412_serial_rx_fifocnt(struct exy4412_uart_port *ourport,
				     unsigned long ufstat)
{
	struct exy4412_uart_info *info = ourport->info;

	if (ufstat & info->rx_fifofull)
		return ourport->port.fifosize;

	return (ufstat & info->rx_fifomask) >> info->rx_fifoshift;
}

static void s3c64xx_start_rx_dma(struct exy4412_uart_port *ourport);
static void exy4412_serial_rx_dma_complete(void *args)
{
}

static void s3c64xx_start_rx_dma(struct exy4412_uart_port *ourport)
{
}

static void enable_rx_dma(struct exy4412_uart_port *ourport)
{
}

static void enable_rx_pio(struct exy4412_uart_port *ourport)
{
}

static irqreturn_t exy4412_serial_rx_chars_dma(void *dev_id)
{
	return IRQ_HANDLED;
}

static void exy4412_serial_rx_drain_fifo(struct exy4412_uart_port *ourport)
{
}

static irqreturn_t exy4412_serial_rx_chars_pio(void *dev_id)
{
	return IRQ_HANDLED;
}


static irqreturn_t exy4412_serial_rx_chars(int irq, void *dev_id)
{
	struct exy4412_uart_port *ourport = dev_id;

	if (ourport->dma && ourport->dma->rx_chan)
		return exy4412_serial_rx_chars_dma(dev_id);
	return exy4412_serial_rx_chars_pio(dev_id);
}

static irqreturn_t exy4412_serial_tx_chars(int irq, void *id)
{
	return IRQ_HANDLED;
}

static struct uart_ops exy4412_serial_ops = {
	.pm		= exy4412_serial_pm,
	.tx_empty	= exy4412_serial_tx_empty,
	.get_mctrl	= exy4412_serial_get_mctrl,
	.set_mctrl	= exy4412_serial_set_mctrl,
	.stop_tx	= exy4412_serial_stop_tx,
	.start_tx	= exy4412_serial_start_tx,
	.stop_rx	= exy4412_serial_stop_rx,
	.break_ctl	= exy4412_serial_break_ctl,
	.startup	= exy4412_serial_startup,
	.shutdown	= exy4412_serial_shutdown,
	.set_termios	= exy4412_serial_set_termios,
	.type		= exy4412_serial_type,
	.release_port	= exy4412_serial_release_port,
	.request_port	= exy4412_serial_request_port,
	.config_port	= exy4412_serial_config_port,
	.verify_port	= exy4412_serial_verify_port,
};

static struct s3c24xx_uart_info exy4412_info = {
  .name		= "Samsung Exynos UART",
  .type		= PORT_S3C6400,
  .has_divslot	= 1,
  .rx_fifomask	= S5PV210_UFSTAT_RXMASK,
  .rx_fifoshift	= S5PV210_UFSTAT_RXSHIFT,
  .rx_fifofull	= S5PV210_UFSTAT_RXFULL,
  .tx_fifofull	= S5PV210_UFSTAT_TXFULL,
  .tx_fifomask	= S5PV210_UFSTAT_TXMASK,
  .tx_fifoshift	= S5PV210_UFSTAT_TXSHIFT,
  .def_clk_sel	= S3C2410_UCON_CLKSEL0,
  .num_clks	= 1,
  .clksel_mask	= 0,
  .clksel_shift	= 0,
};

static struct s3c2410_uartcfg exy4412_cfg = {
  .ucon		= S5PV210_UCON_DEFAULT,
  .ufcon		= S5PV210_UFCON_DEFAULT,
  .has_fracval	= 1,
};

static void exy4412_serial_resetport(struct uart_port *port,
                                     struct s3c2410_uartcfg *cfg)
{
	struct s3c24xx_uart_info *info = s3c24xx_port_to_info(port);
	unsigned long ucon = rd_regl(port, S3C2410_UCON);
	unsigned int ucon_mask;

	ucon_mask = info->clksel_mask;
	if (info->type == PORT_S3C2440)
		ucon_mask |= S3C2440_UCON0_DIVMASK;

	ucon &= ucon_mask;
	wr_regl(port, S3C2410_UCON,  ucon | cfg->ucon);

	/* reset both fifos */
	wr_regl(port, S3C2410_UFCON, cfg->ufcon | S3C2410_UFCON_RESETBOTH);
	wr_regl(port, S3C2410_UFCON, cfg->ufcon);

	/* some delay is required after fifo reset */
	udelay(1);
}
static struct exy4412_uart_port* alloc_init_port(int index,struct platform_device *pdev)
{
	int ret;
	unsigned int			fifosize[CONFIG_SERIAL_SAMSUNG_UARTS];
	struct resource *res;
  struct exy4412_uart_port *ourport = kzalloc(sizeof(struct exy4412_uart_port),GFP_KERNEL);
  ourport->port.lock = __SPIN_LOCK_UNLOCKED(ourport->port.lock);
  ourport->port.iotype = UPIO_MEM;
  ourport->port.uartclk = 0;
  ourport->port.fifosize = 16;
  ourport->port.flags = UPF_BOOT_AUTOCONF;
  ourport->port.line = index;
  ourport->port.fifosize = fifosize[index];
  ourport->port.ops = &exy4412_serial_ops;
  ourport->info = &exy4412_info;
  ourport->cfg  = &exy4412_cfg;
	ourport->baudclk = ERR_PTR(-EINVAL);
	ourport->min_dma_size = max_t(int, ourport->port.fifosize, dma_get_cache_alignment());

  ourport->port.dev = &pdev->dev;
	ourport->port.uartclk = 1;
	if (ourport->cfg->uart_flags & UPF_CONS_FLOW) {
		printk("s3c24xx_serial_init_port: enabling flow control\n");
		ourtport->port.flags |= UPF_CONS_FLOW;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		printk("failed to find memory resource for uart\n");
		return -EINVAL;
	}

	ourport->port.mapbase = res->start;
	ourport->port.membase = devm_ioremap(ourport->port.dev, res->start, resource_size(res));
	if (!ourport->port.membase) {
		printk("failed to remap controller address\n");
		return -EBUSY;
	}

	ret = platform_get_irq(platdev, 0);
	if (ret < 0)
		ourport->port.irq = 0;
	else {
		ourport->port.irq = ret;
		ourport->rx_irq = ret;
		ourport->tx_irq = ret + 1;
	}

	ourport->clk	= clk_get(&pdev->dev, "uart");
	if (IS_ERR(ourport->clk)) {
		return PTR_ERR(ourport->clk);
	}

	ret = clk_prepare_enable(ourport->clk);
	if (ret) {
		clk_put(ourport->clk);
		return ret;
	}

	/* Keep all interrupts masked and cleared */
	if (s3c24xx_serial_has_interrupt_mask(ourport->port)) {
		wr_regl(ourport->port, S3C64XX_UINTM, 0xf);
		wr_regl(ourport->port, S3C64XX_UINTP, 0xf);
		wr_regl(ourport->port, S3C64XX_UINTSP, 0xf);
	}

	exy4412_serial_resetport(&ourport->port, ourport->cfg);
  return ourport;
}

static struct uart_driver exy4412_uart_drv = {
	.owner		= THIS_MODULE,
	.driver_name	= "s3c2410_serial",
	.nr		= CONFIG_SERIAL_SAMSUNG_UARTS,
	.cons		= EXY4412_SERIAL_CONSOLE,
	.dev_name	= EXY4412_SERIAL_NAME,
	.major		= EXY4412_SERIAL_MAJOR,
	.minor		= EXY4412_SERIAL_MINOR,
};

static int exy4412_serial_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;

	int index = of_alias_get_id(np, "serial");

	struct exy4412_uart_port *ourport = alloc_init_port(index,pdev);

  //only register once
	if (!exy4412_uart_drv.state) {
		ret = uart_register_driver(&exy4412_uart_drv);
		if (ret < 0) {
			printk("Failed to register Samsung UART driver\n");
			return ret;
		}
	}

  uart_add_one_port(&exy4412_uart_drv,&ourport->port);

	platform_set_drvdata(pdev, &ourport->port);

	clk_disable_unprepare(ourport->clk);

  return 0;
}

static int exy4412_serial_remove(struct platform_device *dev)
{
}

static const struct of_device_id exy4412_uart_dt_match[] = {
	{ .compatible = "samsung,exynos4210-uart",
		.data = (void *)EXYNOS4210_SERIAL_DRV_DATA },
	{},
};
MODULE_DEVICE_TABLE(of, exy4412_uart_dt_match);

static struct platform_driver samsung_serial_driver = {
	.probe		= exy4412_serial_probe,
	.remove		= exy4412_serial_remove,
	.id_table	= exy4412_serial_driver_ids,
	.driver		= {
		.name	= "samsung-uart",
		.of_match_table	= of_match_ptr(exy4412_uart_dt_match),
	},
};

module_platform_driver(samsung_serial_driver);
