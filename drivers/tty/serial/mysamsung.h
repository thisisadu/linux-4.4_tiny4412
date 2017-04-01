#ifndef __MYSAMSUNG_H
#define __MYSAMSUNG_H

struct exy4412_uartcfg {
	unsigned char	   hwport;	 /* hardware port number */
	unsigned char	   unused;
	unsigned short	   flags;
	upf_t		   uart_flags;	 /* default uart flags */
	unsigned int	   clk_sel;

	unsigned int	   has_fracval;

	unsigned long	   ucon;	 /* value of ucon for port */
	unsigned long	   ulcon;	 /* value of ulcon for port */
	unsigned long	   ufcon;	 /* value of ufcon for port */
};

struct exy4412_uart_info {
	char			*name;
	unsigned int		type;
	unsigned int		fifosize;
	unsigned long		rx_fifomask;
	unsigned long		rx_fifoshift;
	unsigned long		rx_fifofull;
	unsigned long		tx_fifomask;
	unsigned long		tx_fifoshift;
	unsigned long		tx_fifofull;
	unsigned int		def_clk_sel;
	unsigned long		num_clks;
	unsigned long		clksel_mask;
	unsigned long		clksel_shift;

	/* uart port features */

	unsigned int		has_divslot:1;

	/* uart controls */
	int (*reset_port)(struct uart_port *, struct exy4412_uartcfg *);
};

struct exy4412_uart_dma {
	dma_filter_fn			fn;
	void				*rx_param;
	void				*tx_param;
	unsigned int			rx_chan_id;
	unsigned int			tx_chan_id;
	struct dma_slave_config		rx_conf;
	struct dma_slave_config		tx_conf;
	struct dma_chan			*rx_chan;
	struct dma_chan			*tx_chan;
	dma_addr_t			rx_addr;
	dma_addr_t			tx_addr;
	dma_cookie_t			rx_cookie;
	dma_cookie_t			tx_cookie;
	char				*rx_buf;
	dma_addr_t			tx_transfer_addr;
	size_t				rx_size;
	size_t				tx_size;
	struct dma_async_tx_descriptor	*tx_desc;
	struct dma_async_tx_descriptor	*rx_desc;
	int				tx_bytes_requested;
	int				rx_bytes_requested;
};

struct exy4412_uart_port {
	unsigned char			rx_claimed;
	unsigned char			tx_claimed;
	unsigned int			pm_level;
	unsigned long			baudclk_rate;
	unsigned int			min_dma_size;

	unsigned int			rx_irq;
	unsigned int			tx_irq;

	unsigned int			tx_in_progress;
	unsigned int			tx_mode;
	unsigned int			rx_mode;

	struct clk			*clk;
	struct clk			*baudclk;
	struct uart_port		port;

	struct exy4412_uartcfg		*cfg;
	struct exy4412_uart_info	*info;
	struct exy4412_uart_dma		*dma;
};

/* register access controls */
#define portaddr(port, reg) ((port)->membase + (reg))
#define portaddrl(port, reg)                                  \
	((unsigned long *)(unsigned long)((port)->membase + (reg)))

#define rd_regb(port, reg) (__raw_readb(portaddr(port, reg)))
#define rd_regl(port, reg) (__raw_readl(portaddr(port, reg)))

#define wr_regb(port, reg, val) __raw_writeb(val, portaddr(port, reg))
#define wr_regl(port, reg, val) __raw_writel(val, portaddr(port, reg))

#endif
