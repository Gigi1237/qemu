#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "hw/hw.h"
#include "cpu.h"
#include "hw/boards.h"
#include "hw/arm/arm.h"
#include "hw/arm/exynos4210.h"
#include "hw/intc/s3c2416_intc.h"
#include "hw/sysbus.h"
#include "exec/address-spaces.h"
#include "exec/memory.h"
#include "qemu/log.h"
#include "chardev/char-fe.h"
#include "sysemu/sysemu.h"
#include "sysemu/block-backend.h"
#include "hw/i2c/i2c.h"
#include "hw/arm/s3c2416.h"
#include "hw/usb/hcd-ehci.h"


#include <stdio.h>
#include <stdlib.h>

static struct arm_boot_info hp_prime_board_binfo = {
    .board_id = -1, /* device-tree-only board */
    .nb_cpus = 1,
};

static void prime_init(MachineState *machine)
{
    ObjectClass *cpu_oc;
    Object *cpuobj;
    ARMCPU *cpu;

    //Init CPU
    cpu_oc = cpu_class_by_name(TYPE_ARM_CPU, "arm926");
    if (!cpu_oc) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    cpuobj = object_new(object_class_get_name(cpu_oc));
    object_property_set_bool(cpuobj, true, "realized", &error_fatal);


    cpu = ARM_CPU(cpuobj);


    // Init memory before nand
    MemoryRegion *address_space_mem = get_system_memory();
    MemoryRegion *dram = g_new(MemoryRegion, 4);
    MemoryRegion *sram = g_new(MemoryRegion, 1);
    MemoryRegion *boot = g_new(MemoryRegion, 1);

    uint8_t *boot_ptr = g_new0(uint8_t, 0x10000);
	uint8_t *dram_ptr = g_new0(uint8_t, 0x02000000);
    uint8_t *sram_ptr = g_new0(uint8_t, 0xe000);
    assert(boot_ptr);
	assert(dram_ptr);
    assert(sram_ptr);
	
    memory_region_init_ram_ptr(boot, NULL, "prime.boot", 0x00010000, boot_ptr);
    memory_region_add_subregion(address_space_mem, 0x00000000, boot);

	memory_region_init_ram_ptr(dram, NULL, "prime.dram0", 0x02000000, dram_ptr);
    memory_region_add_subregion(address_space_mem, 0x30000000, dram);
	memory_region_init_ram_ptr(&dram[1], NULL, "prime.dram1", 0x02000000, dram_ptr);
    memory_region_add_subregion(address_space_mem, 0x32000000, &dram[1]);
	memory_region_init_ram_ptr(&dram[2], NULL, "prime.dram2", 0x02000000, dram_ptr);
    memory_region_add_subregion(address_space_mem, 0x34000000, &dram[2]);
	memory_region_init_ram_ptr(&dram[3], NULL, "prime.dram3", 0x02000000, dram_ptr);
    memory_region_add_subregion(address_space_mem, 0x36000000, &dram[3]);
    
    memory_region_init_ram_ptr(sram, NULL, "prime.sram", 0xe000, sram_ptr);
    memory_region_add_subregion(address_space_mem, 0x40000000, sram);


    DeviceState *dev;
    dev = qdev_create(NULL, "s3c2416-nand");

    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x4E000000);

    dev = sysbus_create_varargs(TYPE_S3C2416_INTC, 0x4A000000,
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_IRQ),
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_FIQ),
        NULL);

    /* --- Get IRQs --- */
    qemu_irq lcd_irq[3] = {
        qdev_get_gpio_in(dev, SUBINT_LCD4),
        qdev_get_gpio_in(dev, SUBINT_LCD3),
        qdev_get_gpio_in(dev, SUBINT_LCD2)
    };

    qemu_irq rtc_irq = qdev_get_gpio_in(dev, INT_RTC);

    qemu_irq tick_irq = qdev_get_gpio_in(dev, INT_TICK);

    qemu_irq rxd0_sub = qdev_get_gpio_in(dev, SUBINT_RXD0);
    qemu_irq txd0_sub = qdev_get_gpio_in(dev, SUBINT_TXD0);
    qemu_irq err0_sub = qdev_get_gpio_in(dev, SUBINT_ERR0);

    qemu_irq timer[5] = 
    {
        qdev_get_gpio_in(dev, INT_TIMER0),
        qdev_get_gpio_in(dev, INT_TIMER1),
        qdev_get_gpio_in(dev, INT_TIMER2),
        qdev_get_gpio_in(dev, INT_TIMER3),
        qdev_get_gpio_in(dev, INT_TIMER4)
    };
	
	qemu_irq i2c_irq = qdev_get_gpio_in(dev, INT_IIC0);
    
    qemu_irq gpio_irq[2] = {
        qdev_get_gpio_in(dev, EINT4_7),
        qdev_get_gpio_in(dev, EINT8_15)
    };

	/* --- UART --- */
    (void*) s3c2416_uart_create(0x50000000, 64, 0, NULL, rxd0_sub,txd0_sub,err0_sub);

	/* --- LCD --- */
    //sysbus_create_simple("s3c2416-lcd", 0x4C800000, lcd_irq);
    sysbus_create_varargs("exynos4210.fimd", 0x4C800000,
        lcd_irq[0],
        lcd_irq[1],
        lcd_irq[2],
        NULL);
    
	/* --- GPIO --- */
    dev = qdev_create(NULL, "s3c2416-gpio");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x56000000);

	/* --- SYSC --- */
    dev = qdev_create(NULL, "s3c2416-sysc");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x4C000000);

	/* --- MEMC --- */
    //dev = qdev_create(NULL, "s3c2416-memc");
    //qdev_init_nofail(dev);
    //sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x48000000);

	/* --- PWM --- */
    sysbus_create_varargs("exynos4210.pwm", 0x51000000,
        timer[0],
        timer[1],
        timer[2],
        timer[3],
        timer[4],
        NULL);

	/* --- WTCON --- */
    //dev = qdev_create(NULL, "s3c2416-wtcon");
    //qdev_init_nofail(dev);
    //sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x53000000);

    /* --- RTC --- */
    sysbus_create_varargs("exynos4210.rtc", 0x57000000, rtc_irq, tick_irq, NULL);
	
	/* --- I2C --- */
	dev = qdev_create(NULL, "exynos4210.i2c");
	qdev_init_nofail(dev);
	sysbus_connect_irq(SYS_BUS_DEVICE(dev), 0, i2c_irq);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x54000000);
    I2CBus* i2c = (I2CBus *)qdev_get_child_bus(dev, "i2c");
    
    /* --- Touch --- */
    dev = i2c_create_slave(i2c, "nt11002", 0x1);
    qdev_connect_gpio_out(dev, 0, gpio_irq[0]);

    /* --- ADC --- */
    //dev = qdev_create(NULL, "s3c2416-adc");
    //qdev_init_nofail(dev);
    //sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x58000000);

    // Load boot code into SRAM
    /* FIXME use a qdev drive property instead of drive_get() */
    DriveInfo *nand = drive_get(IF_MTD, 0, 0);
    if (nand) {
        int ret = blk_pread(blk_by_legacy_dinfo(nand), 0x0,
                            boot_ptr, 0x2000);
        assert(ret >= 0);
    }
    else if (!machine->kernel_filename) {
        fprintf(stderr, "No NAND image given with '-mtdblock' and no ELF file "
                "specified with '-kernel', aborting.\n");
        exit(1);
    }

    // Load ELF kernel, if provided
    hp_prime_board_binfo.kernel_filename = machine->kernel_filename;
    hp_prime_board_binfo.initrd_filename = machine->initrd_filename;
    hp_prime_board_binfo.kernel_cmdline = machine->kernel_cmdline;
    hp_prime_board_binfo.ram_size = 32*1024*1024;

    arm_load_kernel(ARM_CPU(first_cpu), &hp_prime_board_binfo);
}

static void prime_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);

    mc->desc = "HP Prime (S3C2416)";
    mc->init = prime_init;
}

static const TypeInfo prime_type = {
    .name = MACHINE_TYPE_NAME("hp-prime"),
    .parent = TYPE_MACHINE,
    .class_init = prime_class_init,
};



static void prime_machine_init(void)
{
    type_register_static(&prime_type);
}

type_init(prime_machine_init);
