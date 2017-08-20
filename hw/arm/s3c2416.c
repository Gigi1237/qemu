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
    MemoryRegion *ram = g_new(MemoryRegion, 1);
    MemoryRegion *sram = g_new(MemoryRegion, 1);

    uint8_t *sram_ptr = g_new0(uint8_t, 0x10000);
    assert(sram_ptr);
    memory_region_init_ram_ptr(sram, NULL, "prime.sram", 0x00010000, sram_ptr);
    memory_region_add_subregion(address_space_mem, 0x00000000, sram);

    memory_region_init_ram(ram, NULL, "prime.ram", 0x02000000, &error_fatal);
    memory_region_add_subregion(address_space_mem, 0x30000000, ram);


    DeviceState *dev;
    dev = qdev_create(NULL, "s3c2416-nand");

    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x4E000000);

    dev = sysbus_create_varargs(TYPE_S3C2416_INTC, 0x4A000000,
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_IRQ),
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_FIQ),
        NULL);

    // Create UART
    qemu_irq uart_irq = qdev_get_gpio_in(dev, (28 << 1) | 0);
    qemu_irq lcd_irq = qdev_get_gpio_in(dev, (16 << 1) | 0);
    qemu_irq rtc_irq = qdev_get_gpio_in(dev, (30 << 1) | 0);
    qemu_irq tick_irq = qdev_get_gpio_in(dev, (8 << 1) | 0);

    (void*) exynos4210_uart_create(0x50000000, 32, 0, NULL, uart_irq);

    sysbus_create_simple("s3c2416-lcd", 0x4C800000, lcd_irq);
    
    dev = qdev_create(NULL, "s3c2416-gpio");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x56000000);

    dev = qdev_create(NULL, "s3c2416-sysc");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x4C000000);

    dev = qdev_create(NULL, "s3c2416-memc");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x48000000);

    dev = qdev_create(NULL, "s3c2416-pwm");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x51000000);

    dev = qdev_create(NULL, "s3c2416-wtcon");
    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x53000000);

    /* RTC */
    sysbus_create_varargs("exynos4210.rtc", 0x57000000, rtc_irq, tick_irq, NULL);

    // Load boot code into SRAM
    /* FIXME use a qdev drive property instead of drive_get() */
    DriveInfo *nand = drive_get(IF_MTD, 0, 0);
    if (nand) {
        int ret = blk_pread(blk_by_legacy_dinfo(nand), 0x0,
                            sram_ptr, 0x2000);
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
