
#include "qemu/osdep.h"
#include "hw/intc/s3c2416_intc.h"
#include "qemu/log.h"

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
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
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
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
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

static void s3c2416_intc_register_types(void)
{
    type_register_static(&S3C2416_intc);
}

type_init(s3c2416_intc_register_types)
