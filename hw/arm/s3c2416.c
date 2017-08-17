#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "hw/hw.h"
#include "cpu.h"
#include "hw/boards.h"
#include "hw/arm/arm.h"
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


#define TYPE_S3C2416_INTC "S3C2416-intc"
#define S3C2416_INTC(obj) OBJECT_CHECK(S3C2416_intc_state, (obj), TYPE_S3C2416_INTC)

#define TYPE_S3C2416_UART "S3C2416-uart"
#define S3C2416_UART(obj) OBJECT_CHECK(S3C2416_uart_state, (obj), TYPE_S3C2416_UART)

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
    hwaddr addr,
    unsigned size)
{
    PrimeNANDState *s = (PrimeNANDState*)opaque;

    uint64_t ret;
    switch (addr) {
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
        qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented register read! off: 0x%llx\n", addr);
        break;
    }

    return 0;
}

static void prime_write(void *opaque,
    hwaddr addr,
    uint64_t val,
    unsigned size)
{
    PrimeNANDState *s = (PrimeNANDState*)opaque;

    //if ((addr != NFCONT_OFF) && (s->NFCONT & 1u) == 0) {
    //    return; /* Ignore the write, the nand is not enabled */
    //}

    switch (addr) {
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
        qemu_log_mask(LOG_UNIMP, "S3C2416_nand: Unimplemented register write! off: 0x%llx val: 0x%llx\n", addr, val);
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
       0xEC, 0xDA);


    memory_region_init_io(&s->iomem, obj, &prime_nand_ops, s, "prime", 0x40);
    sysbus_init_mmio(dev, &s->iomem);
}

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    uint32_t SRCPND[2];
    uint32_t INTMOD[2];
    uint32_t INTMSK[2];
    uint32_t INTPND[2];
    uint32_t INTOFFSET[2];

    uint32_t SUBSRCPND;
    uint32_t INTSUBMSK;

    qemu_irq irq;
    qemu_irq fiq;
} S3C2416_intc_state;

/// does not do arbitration yet
static int S3C2416_intc_get_next_interrupt(S3C2416_intc_state* s)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (s->SRCPND[i] & (1u << j) & ~(s->INTMSK[i] & (1u << j))) {
                return (j << 1) | i;
            }
        }
    }
    return -1;
}

static void S3C2416_intc_update(S3C2416_intc_state* s)
{
    int irq = S3C2416_intc_get_next_interrupt(s);

    if (irq == -1) {
        s->INTPND[0] = 0;
        s->INTPND[1] = 0;
        qemu_set_irq(s->irq, 0);
        qemu_set_irq(s->fiq, 0);
        return;
    }

    int grp = irq & 1;
    int n = irq >> 1;

    printf("Interrupt called! grp: %i, n: %i\n", grp, n);

    // ITS AN FIQ
    if (s->INTMOD[grp] & (1u << n)) {
        qemu_set_irq(s->fiq, 1);
        return;
    }

    s->INTPND[grp] = (1u << n);
    qemu_set_irq(s->irq, 1);
}

#define END_OF_LIST 0xFFFFFFFF

#define INT_UART0 28
#define INT_SUBINT_RXD0 0
#define INT_SUBINT_TXD0 1
#define INT_SUBINT_ERR0 2

static uint32_t int_with_subsource[] =
{
    INT_UART0,   /* UART 0 */
    END_OF_LIST
};

static bool S3C2416_intc_has_subsource(int n)
{
    for (int i = 0; int_with_subsource[i] != END_OF_LIST; i++)
    {
        if (int_with_subsource[i] == n)
            return true;
    }
    return false;
}

static void S3C2416_intc_set(void *opaque, int n, int level)
{

    S3C2416_intc_state *s = (S3C2416_intc_state*)opaque;

    int grp, irq, enable;
    grp = n & 1;
    irq = n >> 1;
    enable = level & 1;

    // Only GROUP 0 Interrupts have subsources
    if (grp == 0 && S3C2416_intc_has_subsource(irq))
    {
        int subsrc = level >> 1;
        switch (irq)
        {
        case INT_UART0:
            if (enable)
                s->SUBSRCPND |= enable;
            else
                s->SUBSRCPND &= ~(1 << subsrc);
            break;
        default:
            break;
        }

        // Subsource is masked
        if (s->INTSUBMSK & (1 << subsrc)) {
            S3C2416_intc_update(s);
            return;
        }
    }


    //printf("Set IRQ n: %i, level: %i\n", n, level);

    if (enable)
        s->SRCPND[grp] |= 1u << irq;
    else
        s->SRCPND[grp] &= ~(1u << irq);

    S3C2416_intc_update(s);
}

static uint64_t S3C2416_intc_read(void *opaque, hwaddr offset,
    unsigned size)
{
    S3C2416_intc_state *s = (S3C2416_intc_state*)opaque;
    int i, reg;
    if (offset < 0x40) {
        i = 0;
        reg = offset;
    }
    else {
        i = 1;
        reg = offset - 0x40;
    }

    switch (reg)
    {
        /* SRCPND */
    case 0:
        return s->SRCPND[i];
        /* INTMOD */
    case 0x4:
        return s->INTMOD[i];
        /* INTMSK */
    case 0x8:
        return s->INTMSK[i];
        /* INTPND */
    case 0x10:
        return s->INTPND[i];
        /* INTOFFSET */
    case 0x14:
        return s->INTOFFSET[i];
        /* SUBSRCPND */
    case 0x18:
        return s->SUBSRCPND;
        /* INTSUBMSK */
    case 0x1C:
        return s->INTSUBMSK;
    case 0x30:
    case 0x34:
        qemu_log_mask(LOG_UNIMP, "S3C2416_intc: \"PRIORITY_MODE\" and \"PRIORITY_UPDATE\" unimplemented!\n");
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_intc: invalid register read! off: %llX\n", offset + 0x4C000000);
        break;
    }
    return 0;
}

static void S3C2416_intc_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    S3C2416_intc_state *s = (S3C2416_intc_state*)opaque;

    int i, reg;
    if (offset < 0x40) {
        i = 0;
        reg = offset;
    }
    else {
        i = 1;
        reg = offset - 0x40;
    }

    switch (reg)
    {
    /* SRCPND */
    case 0:
        s->SRCPND[i] = val;
        break;
    /* INTMOD */
    case 0x4:
        s->INTMOD[i] = val;
        break;
    /* INTMSK */
    case 0x8:
        s->INTMSK[i] = val;
        break;
    /* INTPND */
    case 0x10:
        s->INTPND[i] = val;
        break;
    /* INTOFFSET */
    case 0x14:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_intc: INTOFFSET is readonly");
        break;
        /* SUBSRCPND */
    case 0x18:
        s->SUBSRCPND = val;
        break;
        /* INTSUBMSK */
    case 0x1C:
        s->INTSUBMSK = val;
        break;
    case 0x30:
    case 0x34:
        qemu_log_mask(LOG_UNIMP, "S3C2416_intc: \"PRIORITY_MODE\" and \"PRIORITY_UPDATE\" unimplemented!");
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_intc: invalid register write! off: %llX | val: %llX\n", offset + 0x4C000000, val);  
    }
}

static const MemoryRegionOps S3C2416_intc_ops =
{
    .read = S3C2416_intc_read,
    .write = S3C2416_intc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void S3C2416_intc_init(Object *obj)
{
    DeviceState* dev = DEVICE(obj);
    S3C2416_intc_state* s = S3C2416_INTC(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &S3C2416_intc_ops, s, "S3C2416_Intc", 0x00010000);
    sysbus_init_mmio(sbd, &s->iomem);

    qdev_init_gpio_in(dev, S3C2416_intc_set, 64);
    sysbus_init_irq(sbd, &s->irq);
    sysbus_init_irq(sbd, &s->fiq);
};

static const TypeInfo S3C2416_intc =
{
    .name = TYPE_S3C2416_INTC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S3C2416_intc_state),
    .instance_init = S3C2416_intc_init,
    //.class_init = vpb_sic_class_init,
};


typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion memio;
    uint32_t ULCON;
    uint32_t UCON;
    uint32_t UFCON;
    uint32_t UMCON;
    uint32_t UTRSTAT;
    uint32_t UERSTAT; // change?
    uint32_t UFSTAT;
    uint32_t UMSTAT;
    uint8_t URXH;

    uint8_t FIFO[64];
    qemu_irq irq;
    CharBackend chr;
} S3C2416_uart_state;

static void S3C2416_uart_update(S3C2416_uart_state* s)
{
    if (s->UTRSTAT & (1u << 2))
        qemu_set_irq(s->irq, 1 | (INT_SUBINT_TXD0 << 1));
}

static void S3C2416_uart_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    S3C2416_uart_state *s = (S3C2416_uart_state*)opaque;
    //printf("Write to UART register: 0x%llx, val: 0x%llx\n", offset + 0x50000000, val);
    uint8_t b;
    switch (offset)
    {
    /* ULCON */
    case 0x0:
        s->ULCON = val;
        break;
    /* UCON */
    case 0x4:
        s->UCON = val;
        break;
    /* UFCON */
    case 0x8:
        s->UFCON = val;
        break;
    /* UMCON */
    case 0xC:
        s->UMCON = val;
        break;
    /* UTXH */
    case 0x20:
        b = val;
        s->UTRSTAT |= 1u << 2;
        qemu_chr_fe_write_all(&s->chr, &b, 1);
        S3C2416_uart_update(s);
        break;
    /* UTRSTAT | ReadOnly */
    /* UERSTAT | ReadOnly */
    /* UFSTAT  | ReadOnly */
    /* UMSTAT  | ReadOnly */
    /* URXH    | ReadOnly */
    case 0x10:
    case 0x14:
    case 0x18:
    case 0x1C:
    case 0x24:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_uart: Attempted write to read only register!\n");
        break;
    /* UBRDIV */
    /* UDIVSLOT */
    case 0x28:
    case 0x2C:
        qemu_log_mask(LOG_UNIMP, "S3C2416_uart: Attempted write to unimplemented register!\n");
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_uart: Attempted write to invalid register!\n");
        break;
    }
}

static uint64_t S3C2416_uart_read(void *opaque, hwaddr offset,
    unsigned size)
{
    S3C2416_uart_state *s = (S3C2416_uart_state*)opaque;

    switch (offset)
    {
        /* ULCON */
    case 0x0:
        return s->ULCON;
        /* UCON */
    case 0x4:
        return s->UCON;
        /* UFCON */
    case 0x8:
        return s->UFCON;
        /* UMCON */
    case 0xC:
        return s->UMCON;
        /* UTRSTAT | ReadOnly */
    case 0x10:
        return s->UTRSTAT;
        /* UERSTAT | ReadOnly */
    case 0x14:
        return s->UERSTAT;
        /* UFSTAT  | ReadOnly */
    case 0x18:
        return s->UFSTAT;
        /* UMSTAT  | ReadOnly */
    case 0x1C:
        return s->UMSTAT;
        /* URXH    | ReadOnly */
    case 0x24:
        return s->URXH;
        /* UTXH | WriteOnly*/
    case 0x20:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_uart: Attempted to read write only register!\n");
        break;
    /* UBRDIV */
    /* UDIVSLOT */
    case 0x28:
    case 0x2C:
        qemu_log_mask(LOG_UNIMP, "S3C2416_uart: Attempted read of unimplemented register!\n");
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_uart: Attempted read of invalid register!\n");
        break;
    }

    return 0x0;
}

static int S3C2416_uart_can_recieve(void* opaqe)
{
    return 0;
}

static void S3C2416_uart_recieve(void *opaque, const uint8_t *buf, int size)
{

}

static void S3C2416_uart_event(void *opaque, int event)
{
    //if (event == CHR_EVENT_BREAK)
    //    pl011_put_fifo(opaque, 0x400);
}

static MemoryRegionOps S3C2416_uart_ops =
{
    .read = S3C2416_uart_read,
    .write = S3C2416_uart_write,
    .endianness = DEVICE_NATIVE_ENDIAN
};

static Property S3C2416_uart_properties[] = {
    DEFINE_PROP_CHR("chardev", S3C2416_uart_state, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void S3C2416_uart_realize(DeviceState *dev, Error **errp)
{
    S3C2416_uart_state *s = S3C2416_UART(dev);
    qemu_chr_fe_set_handlers(&s->chr, S3C2416_uart_can_recieve, S3C2416_uart_recieve,
        S3C2416_uart_event, NULL, s, NULL, true);
}

static void S3C2416_uart_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = S3C2416_uart_realize;
    dc->props = S3C2416_uart_properties;
}

static void S3C2416_uart_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    S3C2416_uart_state *s = S3C2416_UART(obj);

    memory_region_init_io(&s->memio, OBJECT(s), &S3C2416_uart_ops, s, "S3C2416_uart", 0x00001000);
    sysbus_init_mmio(sbd, &s->memio);
    sysbus_init_irq(sbd, &s->irq);

    s->UTRSTAT = 0x6;
}

static const TypeInfo S3C2416_uart =
{
    .name = TYPE_S3C2416_UART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S3C2416_uart_state),
    .instance_init = S3C2416_uart_init,
    .class_init = S3C2416_uart_class_init
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
    //MemoryRegion *boot = g_new(MemoryRegion, 1);
    MemoryRegion *steppingStone = g_new(MemoryRegion, 1);
    //MemoryRegion *gpio = g_new(MemoryRegion, 1);
    //MemoryRegion *dram = g_new(MemoryRegion, 1);

    FILE *f = fopen("D:/BXCBOOT0.BIN", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *mem = malloc(fsize);
    fread(mem, fsize, 1, f);

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
    
    dev = qdev_create(NULL, TYPE_S3C2416_UART);
    SysBusDevice *s = SYS_BUS_DEVICE(dev);
    qdev_prop_set_chr(dev, "chardev", serial_hds[0]);
    qdev_init_nofail(dev);
    sysbus_mmio_map(s, 0, 0x50000000);
    sysbus_connect_irq(s, 0, uart_irq);

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
    type_register_static(&S3C2416_intc);
    type_register_static(&S3C2416_uart);
}

type_init(prime_machine_init);