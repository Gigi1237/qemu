
#include "qemu/osdep.h"
#include "hw/intc/s3c2416_intc.h"
#include "qemu/log.h"

typedef struct interrupt_info
{
    uint8_t grp;
    uint32_t value;
    uint32_t offset;
    uint32_t subint;
} interrupt_info;

static interrupt_info int_info_array[INT_END] = 
{
    [INT_UART0] =   { 0, 0x10000000, 28, 0 },
    [SUBINT_RXD0] = { 0, 0x10000000, 28, 0x1 },
    [SUBINT_TXD0] = { 0, 0x10000000, 28, 0x2 },
    [SUBINT_ERR0] = { 0, 0x10000000, 28, 0x4 },

    [INT_LCD] = { 0, 0x10000, 16, 0},

    [INT_RTC] = { 0, 0x40000000, 30, 0},

    [INT_TICK] = { 0, 0x100, 8, 0},

    [INT_TIMER0] = { 0, 0x400, 10, 0 },
    [INT_TIMER1] = { 0, 0x800, 11, 0 },
    [INT_TIMER2] = { 0, 0x1000, 12, 0 },
    [INT_TIMER3] = { 0, 0x2000, 13, 0 },
    [INT_TIMER4] = { 0, 0x4000, 14, 0 },
};

static int s3c2416_get_int_id(int offset, uint8_t grp)
{
    for (int i = 0; i < INT_END; i++)
    {

        if (int_info_array[i].grp == grp && int_info_array[i].offset == offset)
            return i;
    }
    return 0xFFFFFF;
}

static uint32_t s3c2416_intc_get_subsource(int val)
{
    return int_info_array[val].subint;
}

static uint32_t s3c2416_intc_get_int(int val)
{
    return int_info_array[val].value;
}

static uint32_t s3c2416_intc_get_offset(int val)
{
    return int_info_array[val].offset;
}

static uint8_t s3c2416_intc_get_grp(int val)
{
    return int_info_array[val].grp;
}

/// does not do arbitration yet
static int S3C2416_intc_get_next_interrupt(S3C2416_intc_state* s)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (s->SRCPND[i] & (1u << j) & ~(s->INTMSK[i] & (1u << j))) {
                return s3c2416_get_int_id(j, i);
            }
        }
    }
    return -1;
}

static void S3C2416_intc_update(S3C2416_intc_state* s)
{
    int irq = S3C2416_intc_get_next_interrupt(s);

    assert(irq != 0xFFFFFF);

    if (irq == -1) {
        s->INTPND[0] = 0;
        s->INTPND[1] = 0;
        qemu_set_irq(s->irq, 0);
        qemu_set_irq(s->fiq, 0);
        return;
    }

    int grp = s3c2416_intc_get_grp(irq);
    int n = s3c2416_intc_get_int(irq);

    printf("Interrupt called! grp: %i, n: %i\n", grp, n);

    // ITS AN FIQ
    if (s->INTMOD[grp] & n) {
        qemu_set_irq(s->fiq, 1);
        return;
    }

    s->INTPND[grp] = n;
    s->INTOFFSET[grp] = s3c2416_intc_get_offset(irq);
    qemu_set_irq(s->irq, 1);
}

static bool S3C2416_intc_has_subsource(int n)
{
    return s3c2416_intc_get_subsource(n) != 0;
        
}

static void S3C2416_intc_set(void *opaque, int n, int level)
{

    S3C2416_intc_state *s = (S3C2416_intc_state*)opaque;

    int grp, irq;
    grp = s3c2416_intc_get_grp(n);
    irq = s3c2416_intc_get_int(n);

    // Only GROUP 0 Interrupts have subsources
    if (grp == 0 && S3C2416_intc_has_subsource(n))
    {
        int subsrc = s3c2416_intc_get_subsource(n);
            if (level)
                s->SUBSRCPND |= subsrc;
            else
                s->SUBSRCPND &= ~subsrc;

        // Subsource is masked
        if (s->INTSUBMSK & subsrc) {
            S3C2416_intc_update(s);
            return;
        }
    }


    //printf("Set IRQ n: %i, level: %i\n", n, level);

    if (level)
        s->SRCPND[grp] |= irq;
    else
        s->SRCPND[grp] &= irq;

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
        s->SRCPND[i] &= ~val;
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
        s->INTPND[i] &= ~val;
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

    qdev_init_gpio_in(dev, S3C2416_intc_set, INT_END);
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
