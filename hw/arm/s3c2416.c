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
#include "hw/block/flash.h"
#include "qemu/log.h"
#include "chardev/char-fe.h"
#include "sysemu/sysemu.h"



#include <stdio.h>
#include <stdlib.h>


#define TYPE_PRIME_NAND "prime-nand"
#define PRIME_NAND(obj) OBJECT_CHECK(PrimeNANDState, (obj), TYPE_PRIME_NAND)



#define NFCE0(s) (s->NFCONT & (1u << 1))
#define NFCONF_OFF 0x0
#define NFCONT_OFF 0x4
#define NFCMMD_OFF 0x8
#define NFADDR_OFF 0xC
#define NFDATA_OFF 0x10
#define NFSBLK_OFF 0x20
#define NFEBLK_OFF 0x24
#define NFSTAT_OFF 0x28
#define NFECCERR0_off 0x2C
#define NFECCERR1_off 0x30


typedef struct
{
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    DeviceState* nand;

    uint32_t NFCONF;
    uint32_t NFCONT;
    uint32_t NFCMMD;
    uint32_t NFADDR;
    uint32_t NFDATA;
    uint32_t NFMECCD[3];
    uint32_t NFSBLK;
    uint32_t NFEBLK;
    uint32_t NFSTAT;
    
    uint8_t end_of_struct;

} PrimeNANDState;

static uint64_t prime_read(void *opaque,
    hwaddr offset,
    unsigned size)
{
    PrimeNANDState *s = (PrimeNANDState*)opaque;

    uint64_t ret;
    switch (offset) {
    case NFCMMD_OFF:
        return s->NFCMMD;
    case NFCONF_OFF:
        return s->NFCONF;
    case NFCONT_OFF:
        return s->NFCONT;
    case NFADDR_OFF:
        return s->NFADDR;
    case NFSBLK_OFF:
        return s->NFSBLK;
        break;
    case NFEBLK_OFF:
        return s->NFEBLK;
        break;
    case NFDATA_OFF:
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
            s->NFDATA = nand_getio(s->nand);
            return s->NFDATA;
        } else {
            s->NFDATA = 0;
            return 0;
        }
    // Untested may be compleltey wrong
    case NFSTAT_OFF:
        if (s->nand != NULL) {
            nand_getpins(s->nand, (int *)&ret);
            s->NFSTAT |= ret | (1u << 4) | (1u << 6);
            return s->NFSTAT;
        } else {
            s->NFSTAT = 0;
            return 0;
        }
    case NFECCERR0_off:
        if (((s->NFCONF >> 23) & 0x3) == 0x2) {
            return 1 << 30;
        }
        else
        {
            qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented ecc other than 4bit\n");
            return 0;
        }
    case NFECCERR1_off:
        if (((s->NFCONF >> 23) & 0x3) == 0x2) {
            return 0;
        }
        else
        {
            qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented ecc other than 4bit\n");
            return 0;
        }
    default:
        /* The rest read-back what was written to them */
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        break;
    }

    return 0;
}

static void prime_write(void *opaque,
    hwaddr offset,
    uint64_t val,
    unsigned size)
{
    PrimeNANDState *s = (PrimeNANDState*)opaque;

    //if ((addr != NFCONT_OFF) && (s->NFCONT & 1u) == 0) {
    //    return; /* Ignore the write, the nand is not enabled */
    //}

    switch (offset) {
    case NFCONF_OFF:
        s->NFCONF = val;
        if (s->nand != NULL)
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
        break;

    case NFCONT_OFF:
        s->NFCONT = val;
        if (s->nand != NULL)
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
        break;

    case NFCMMD_OFF:
        s->NFCMMD = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 1, 0, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFADDR_OFF:
        s->NFADDR = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 1, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFDATA_OFF:
        s->NFDATA = val;
        if (s->nand != NULL) {
            nand_setpins(s->nand, 0, 0, NFCE0(s), 1, 0);
            nand_setio(s->nand, val);
        }
        break;

    case NFSBLK_OFF:
        s->NFSBLK = val;
        break;

    case NFEBLK_OFF:
        s->NFEBLK = val;
        break;

    case NFSTAT_OFF:
        s->NFSTAT &= ~0x70;
        s->NFSTAT |= (val & 0x70);
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
        /* Do nothing because the other registers are read only */
        break;
    }

    //printf("S3C2416_nand write: 0x%llx, val: 0x%llx\n", 0x4e000000 + addr, val);

}

static const MemoryRegionOps prime_nand_ops = {
    .read = prime_read,
    .write = prime_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void prime_nand_init(Object *obj)
{
    PrimeNANDState *s = PRIME_NAND(obj);
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);
    DriveInfo *nand;

    /* FIXME use a qdev drive property instead of drive_get() */
    nand = drive_get(IF_MTD, 0, 0);
    if (!nand) {
        fprintf(stderr, "A flash image must be given with the "
            "'blockmtd' parameter\n");
        exit(1);
    }
    s->nand = nand_init(nand ? blk_by_legacy_dinfo(nand) : NULL,
       0xAD/*0xEC*/, 0xDA);


    memory_region_init_io(&s->iomem, obj, &prime_nand_ops, s, "prime", 0x40);
    sysbus_init_mmio(dev, &s->iomem);
}

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
    //MemoryRegion *boot = g_new(MemoryRegion, 1);
    MemoryRegion *steppingStone = g_new(MemoryRegion, 1);
    //MemoryRegion *gpio = g_new(MemoryRegion, 1);
    //MemoryRegion *dram = g_new(MemoryRegion, 1);

    FILE *f = fopen("D:/BXCBOOT0.BIN", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *mem = malloc(fsize);
    assert(fread(mem, fsize, 1, f) == fsize);

    printf("Read file:\n    + size: %ld\n", fsize);

    memory_region_init_ram_ptr(steppingStone, NULL, "prime.steppingstone", fsize, mem);
    memory_region_add_subregion(address_space_mem, 0x00000000, steppingStone);

    memory_region_init_ram(ram, NULL, "prime.ram", 0x02000000, &error_fatal);
    memory_region_add_subregion(address_space_mem, 0x30000000, ram);



    //// FIX GPIO TEMP
    //memory_region_init_ram(gpio, NULL, "prime.gpio", 0x00010000, &error_fatal);
    //memory_region_add_subregion(address_space_mem, 0x56000000, gpio);

    //// FIX DRAM TEMP
    //memory_region_init_ram(dram, NULL, "prime.dram", 0x00010000, &error_fatal);
    //memory_region_add_subregion(address_space_mem, 0x48000000, dram);

    //// VIVA LE HACK


    DeviceState *dev;
    dev = qdev_create(NULL, TYPE_PRIME_NAND);

    qdev_init_nofail(dev);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, 0x4E000000);

    dev = sysbus_create_varargs(TYPE_S3C2416_INTC, 0x4A000000,
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_IRQ),
        qdev_get_gpio_in(DEVICE(cpu), ARM_CPU_FIQ),
        NULL);

    // Create UART
    qemu_irq uart_irq = qdev_get_gpio_in(dev, (28 << 1) | 0);
    qemu_irq lcd_irq = qdev_get_gpio_in(dev, (16 << 1) | 0);

    (void*) exynos4210_uart_create(0x50000000, 32, 1, NULL, uart_irq);

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

static const TypeInfo prime_nand_info = {
    .name = TYPE_PRIME_NAND,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(PrimeNANDState),
    .instance_init = prime_nand_init
};

static void prime_machine_init(void)
{
    type_register_static(&prime_type);
    type_register_static(&prime_nand_info);
}

type_init(prime_machine_init);
