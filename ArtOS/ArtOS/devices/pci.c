#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <lowlevel/irqflags.h>
#include <lowlevel/io.h>

#ifdef CONFIG_PCI

#include <lowlevel/pci.h>
#ifdef WITH_PCI_NAMES
#include "pci_list.h>
#endif

/*
 * PCI configuration registers
 */
#define	PCI_CFID	0x00	/* Configuration ID */
#define	PCI_CFCS	0x04	/* Configurtion Command/Status */
#define	PCI_CFRV	0x08	/* Configuration Revision */
#define	PCI_CFLT	0x0c	/* Configuration Latency Timer */
#define	PCI_CBIO	0x10	/* Configuration Base IO Address */
#define	PCI_CFIT	0x3c	/* Configuration Interrupt */
#define	PCI_CFDA	0x40	/* Configuration Driver Area */

#define PHYS_IO_MEM_START	0
#define	PCI_MEM			0
#define	PCI_INTA		0
#define PCI_NSLOTS		22
#define PCI_NBUS		0

#define	PCI_CONF_ADDR_REG	0xcf8
#define	PCI_CONF_FRWD_REG	0xcf8
#define	PCI_CONF_DATA_REG	0xcfc

#define PCI_IO_CONF_START	0xc000

#define MAX_BUS			16
#define MAX_SLOTS		32

static uint32_t mechanism = 0;
static uint32_t adapters[MAX_BUS][MAX_SLOTS] = {[0 ... MAX_BUS-1][0 ... MAX_SLOTS-1] = -1};

//Write the PCI configuration
static void pci_config_write(uint32_t bus, uint32_t slot, uint32_t off, uint32_t val)
{
	if (mechanism == 1) {
		outportl(PCI_CONF_FRWD_REG, bus);
		outportl(PCI_CONF_ADDR_REG, 0xf0);
		outportl(PCI_IO_CONF_START | (slot << 8) | off, val);
	} else {
		outportl(PCI_CONF_ADDR_REG, (0x80000000 | (bus << 16) | (slot << 11) | off));
		outportl(PCI_CONF_DATA_REG, val);
	}
}

//Read the PCI configuration
static uint32_t pci_config_read(uint32_t bus, uint32_t slot, uint32_t off)
{
	uint32_t data = -1;

	outportl(PCI_CONF_ADDR_REG, (0x80000000 | (bus << 16) | (slot << 11) | off));
	data = inportl(PCI_CONF_DATA_REG);

	if ((data == 0xffffffff) && (slot < 0x10)) {
		outportl(PCI_CONF_FRWD_REG, bus);
		outportl(PCI_CONF_ADDR_REG, 0xf0);
		data = inportl(PCI_IO_CONF_START | (slot << 8) | off);
		if (data == 0xffffffff)
			return data;
		if (!mechanism)
			mechanism = 1;
	} else if (!mechanism)
		mechanism = 2;

	return data;
}

static inline uint8_t pci_irq(uint32_t bus, uint32_t slot)
{
	return pci_config_read(bus, slot, PCI_CFIT) & 0xFF;
}

static inline uint32_t pci_iobase(uint32_t bus, uint32_t slot, uint32_t nr)
{
	return pci_config_read(bus, slot, PCI_CBIO + nr*4) & 0xFFFFFFFC;
}

static inline uint32_t pci_type(uint32_t bus, uint32_t slot, uint32_t nr)
{
	return pci_config_read(bus, slot, PCI_CBIO + nr*4) & 0x1;
}

//Show the size of PCI device
static inline uint32_t pci_size(uint32_t bus, uint32_t slot, uint32_t nr)
{
	uint32_t tmp, ret;

	// backup the original value
	tmp = pci_config_read(bus, slot, PCI_CBIO + nr*4);

	// determine size
	pci_config_write(bus, slot, PCI_CBIO + nr*4, 0xFFFFFFFF);
	ret = ~pci_config_read(bus, slot, PCI_CBIO + nr*4) + 1;

	// restore original value
	pci_config_write(bus, slot, PCI_CBIO + nr*4, tmp);

	return ret;
}

//Initialize the PCI
int pci_init(void)
{
	uint32_t slot, bus;
	
	for (bus = 0; bus < MAX_BUS; bus++)
		for (slot = 0; slot < MAX_SLOTS; slot++)
			adapters[bus][slot] = pci_config_read(bus, slot, PCI_CFID);
	
	return 0;
}

//Show device info
int pci_dev_info(uint32_t vendor_id, uint32_t device_id, uint32_t base, pci_info_t* info)
{
	uint32_t slot, bus, i;

	if (!info)
		return -EINVAL;

	if (!mechanism)
		pci_init();

	for (bus = 0; bus < MAX_BUS; bus++) {
		for (slot = 0; slot < MAX_SLOTS; slot++) {
			if (adapters[bus][slot] != -1) {
				if (((adapters[bus][slot] & 0xffff) == vendor_id) && 
				   (((adapters[bus][slot] & 0xffff0000) >> 16) == device_id)) {
						info->slot = slot;
						info->bus = bus;
					for(i=0; i<6; i++) {
						info->base[i] = pci_iobase(bus, slot, i);
						info->type[i] = pci_type(bus, slot, i);
						info->size[i] = (info->base[i]) ? pci_size(bus, slot, i) : 0;	
					}
					info->irq = pci_irq(bus, slot);
					if (base)
						if (!(info->base[0] == base))
							continue;
					return 0;
				}
			}
		}
	}

	return -EINVAL;
}

//Show device model
#ifdef WITH_PCI_NAMES
int pci_show_dev(void)
{
	uint32_t slot, bus;
	uint32_t i, counter = 0;

	if (!mechanism)
		pci_init();

	for (bus = 0; bus < MAX_BUS; bus++) {
                for (slot = 0; slot < MAX_SLOTS; slot++) {

		if (adapters[bus][slot] != -1) {
				counter++;

				kprintf("%d) Vendor ID: 0x%x  Device Id: 0x%x\n",
					counter, adapters[bus][slot] & 0xffff, 
					(adapters[bus][slot] & 0xffff0000) >> 16);

				for (i=0; i<PCI_VENTABLE_LEN; i++) {
					if ((adapters[bus][slot] & 0xffff) ==
					    (uint32_t)PciVenTable[i].VenId)
						kprintf("\tVendor is %s\n",
							PciVenTable[i].VenShort);
				}

				for (i=0; i<PCI_DEVTABLE_LEN; i++) {
					if ((adapters[bus][slot] & 0xffff) ==
					    (uint32_t)PciDevTable[i].VenId) {
						if (((adapters[bus][slot] & 0xffff0000) >> 16) ==
						    PciDevTable[i].DevId) {
							kprintf
							    ("\tChip: %s ChipDesc: %s\n",
							     PciDevTable[i].Chip,
							     PciDevTable[i].ChipDesc);
						}
					}
				}
			}
		}
	}

	return 0;
}
#endif

#endif
