#ifndef __ARCH_PCI_H__
#define __ARCH_PCI_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t slot, bus, irq;
	uint32_t base[6];
	uint32_t size[6];
	uint8_t  type[6];
} pci_info_t;

/** Initialize the PCI environment
 *
 * return
 * - 0 on success
 */
int pci_init(void);

/** Determine the IObase address and the interrupt number of a specific device
 *
 * vendor_id The device's vendor ID
 * device_id the device's ID
 * info Pointer to the record pci_info_t where among other the IObase address will be stored
 * base Search for the preferred IO address. Zero, if any address is useful
 *
 * 
 * - 0 on success
 * - -EINVAL on failure
 */
int pci_dev_info(uint32_t vendor_id, uint32_t device_id, uint32_t base, pci_info_t* info);

#ifdef WITH_PCI_NAMES
/** Print information of existing pci adapters
 *
 * 0 in any case
 */
int pci_show_dev(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
