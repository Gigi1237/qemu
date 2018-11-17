#include "qemu/osdep.h"
#include "hw/intc/s3c2416_intc.h"
#include "qemu/log.h"

#ifndef S3C2416_INTC_DEBUG
#define S3C2416_INTC_DEBUG 0
#endif

#if S3C2416_INTC_DEBUG
#define DPRINT(fmt, args...) printf(fmt, ## args);
#else
#define DPRINT(fmt, args...) 
#endif

typedef struct interrupt_info
{
    uint8_t grp;
    uint32_t value;
    uint32_t offset;
    uint32_t subint;
    uint8_t arbiter;
    uint8_t req;
} interrupt_info;

static interrupt_info int_info_array[INT_END] = 
{
    [INT_UART0] =   { 0, 0x10000000, 28, 0, 5, 0 },
    [SUBINT_RXD0] = { 0, 0x10000000, 28, 0x1, 5, 0 },
    [SUBINT_TXD0] = { 0, 0x10000000, 28, 0x2, 5, 0 },
    [SUBINT_ERR0] = { 0, 0x10000000, 28, 0x4, 5, 0 },

    [INT_IIC0]    = { 0, 0x8000000, 27, 0, 4, 5 },
	
    [INT_LCD]     = { 0, 0x10000, 16, 0, 3, 0 },
    [SUBINT_LCD4] = { 0, 0x10000, 16, 0x20000, 3, 0 },
    [SUBINT_LCD3] = { 0, 0x10000, 16, 0x10000, 3, 0 },
    [SUBINT_LCD2] = { 0, 0x10000, 16, 0x8000, 3, 0 },

    [INT_RTC]     = { 0, 0x40000000, 30, 0, 5, 2 },

    [INT_TICK]    = { 0, 0x100, 8, 0, 1, 4 },

    [INT_TIMER0]  = { 0, 0x400, 10, 0, 2, 0 },
    [INT_TIMER1]  = { 0, 0x800, 11, 0, 2, 1 },
    [INT_TIMER2]  = { 0, 0x1000, 12, 0, 2, 2 },
    [INT_TIMER3]  = { 0, 0x2000, 13, 0, 2, 3 },
    [INT_TIMER4]  = { 0, 0x4000, 14, 0, 2, 4 },
    
    [EINT8_15]    = { 0, 0x20, 5, 0, 1, 1 },
    [EINT4_7]     = {0, 0x10, 4, 0, 1, 0 },
    [EINT3]       = {0, 0x8, 3, 0, 0, 3 },
    [EINT2]       = {0, 0x4, 2, 0, 0, 2 },
    [EINT1]       = {0, 0x2, 1, 0, 0, 1 },
    [EINT0]       = {0, 0x1, 0, 0, 0, 0 }
};
static int s3c2416_intc_get_subint_id(int subint)
{
    int i;
    for (i = 0; i < INT_END; i++)
    {
        if (int_info_array[i].subint == (1 << subint))
            return i;
    }
    return 0xFFFFFF;
}

static int s3c2416_get_int_from_val(uint32_t value, uint8_t grp)
{
    int i;
    for (i = 0; i < INT_END; i++)
    {

        if (int_info_array[i].grp == grp && int_info_array[i].value == value)
            return i;
    }
    return 0xFFFFFF;
}

static int s3c2416_get_int_id(int offset, uint8_t grp)
{
    int i;
    for (i = 0; i < INT_END; i++)
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

static void S3C2416_intc_update_subint(S3C2416_intc_state* s)
{
    int j;
    for (j = 0; j < 32; j++)
    {
        if ((s->SUBSRCPND & (1u << j)) & ~(s->INTSUBMSK & (1u << j))) {
            int n = s3c2416_intc_get_subint_id(j);
            int grp = s3c2416_intc_get_grp(n);
            s->SRCPND[grp] |= s3c2416_intc_get_int(n);
        }
    }
}

static int_id S3C2416_intc_arbitrate_group(S3C2416_intc_state* s, uint8_t group)
{
    int i,j;
    int_id ids[6] = { INT_END, INT_END, INT_END, INT_END, INT_END, INT_END };
    int order[6];
    uint8_t sel;
    
    if (group != 6)
    {
        for (i = 0; i < 2; i++)
        {
            for (j = 0; j < 32; j++)
            {
                if (s->SRCPND[i] & (1u << j) & ~(s->INTMSK[i] & (1u << j))) {
                    interrupt_info inf;
                    int_id id = s3c2416_get_int_id(j, i);
                    inf = int_info_array[id];
                    
                    if (inf.arbiter != group) {
                        DPRINT("interrupt %d discarded wrong group %uc, needed %uc\n", id, group, inf.arbiter)
                        continue;
                    }
                    
                    ids[inf.req] = id;
                }
            }
        }
    }
    else {
        ids[0] = S3C2416_intc_arbitrate_group(s, 0);
        ids[1] = S3C2416_intc_arbitrate_group(s, 1);
        ids[2] = S3C2416_intc_arbitrate_group(s, 2);
        ids[3] = S3C2416_intc_arbitrate_group(s, 3);
        ids[4] = S3C2416_intc_arbitrate_group(s, 4);
        ids[5] = S3C2416_intc_arbitrate_group(s, 5);
    }
    
    sel = ((s->PRIORITY_MODE[0] & (7 << (4 * group))) >> (4 * group)) & 7;
    
    /* Arbiter mode:
        0 = Fixed ends & Rotate middle
        1 = Rotate all
    */
    if (s->PRIORITY_MODE[0] & (1 << ((4 * group) + 3))) {
        order[0] = 0;
        order[5] = 5;
        switch (sel) {
            default:
            case 0:
                order[1] = 1;
                order[2] = 2;
                order[3] = 3;
                order[4] = 4;
                break;
            case 1:
                order[1] = 2;
                order[2] = 3;
                order[3] = 4;
                order[4] = 1;
                break;
            case 2:
                order[1] = 3;
                order[2] = 4;
                order[3] = 1;
                order[4] = 2;
                break;
            case 3:
                order[1] = 4;
                order[2] = 1;
                order[3] = 2;
                order[4] = 3;
                break;
        }
        if (s->PRIORITY_UPDATE[0] & (1 << group)) {
            sel = (sel + 1) & 3;
        }
    }
    else {
        switch (sel) {
            default:
            case 0:
                order[0] = 0;
                order[1] = 1;
                order[2] = 2;
                order[3] = 3;
                order[4] = 4;
                order[5] = 5;
                break;
            case 1:
                order[0] = 1;
                order[1] = 2;
                order[2] = 3;
                order[3] = 4;
                order[4] = 5;
                order[5] = 0;
                break;
            case 2:
                order[0] = 2;
                order[1] = 3;
                order[2] = 4;
                order[3] = 5;
                order[4] = 0;
                order[5] = 1;
                break;
            case 3:
                order[0] = 3;
                order[1] = 4;
                order[2] = 5;
                order[3] = 0;
                order[4] = 1;
                order[5] = 2;
                break;
            case 4:
                order[0] = 4;
                order[1] = 5;
                order[2] = 0;
                order[3] = 1;
                order[4] = 2;
                order[5] = 3;
                break;
            case 5:
                order[0] = 5;
                order[1] = 0;
                order[2] = 1;
                order[3] = 2;
                order[4] = 3;
                order[5] = 4;
                break;
        }
        
        if (s->PRIORITY_UPDATE[0] & (1 << group)) {
            sel = (sel + 1) & 7;
            if (sel > 5) {
                sel = 0;
            }
        }
    }
    
    for (i = 0; i < 6; i++) {
        int_id id = ids[order[i]];
        if (id != INT_END) {
            DPRINT("arbiter %d, returned intID: %d\n", group, id);
            return id;
        }
    }
    return INT_END;
}

/// does not do arbitration yet
static int S3C2416_intc_get_next_interrupt(S3C2416_intc_state* s)
{
    int i;
    
    // FIQ First
    for (i = 0; i < 2; i++)
    {
        if (s->SRCPND[i] & s->INTMOD[i]) {
            return s3c2416_get_int_from_val(s->INTMOD[i], i);
        }
    }

    int_id id = S3C2416_intc_arbitrate_group(s, 6);
    
    if (id != INT_END)
        return id;
    
    return -1;
}

static void S3C2416_intc_update(S3C2416_intc_state* s)
{   
    // Clear INTOFFSET
    if (s->last_int != INT_END)
    {
        uint32_t grp = s3c2416_intc_get_grp(s->last_int);
        uint32_t val = s3c2416_intc_get_int(s->last_int);
        uint32_t subint = s3c2416_intc_get_subsource(s->last_int);
        if ((s->INTPND[grp] & val) | (s->SRCPND[grp] & val) | (s->SUBSRCPND & subint))
            return;
        
        s->INTOFFSET[grp] = 0;
        s->last_int = INT_END;
        
        if (s->INTMOD[grp] & val)
            qemu_set_irq(s->fiq, 0);
        else
            qemu_set_irq(s->irq, 0);
    }
    
    S3C2416_intc_update_subint(s);
    int irq = S3C2416_intc_get_next_interrupt(s);

    assert(irq != 0xFFFFFF);

    if (irq == -1) {
        s->INTPND[0] = 0;
        s->INTPND[1] = 0;
        return;
    }

    int grp = s3c2416_intc_get_grp(irq);
    int n = s3c2416_intc_get_int(irq);
    s->last_int = irq;

    DPRINT("Interrupt called! grp: %08x, n: %08x, irq: %08x\n", grp, n, irq)

    // ITS AN FIQ
    if (s->INTMOD[grp] & n) {
        qemu_set_irq(s->fiq, 1);
        return;
    }

    s->INTPND[grp] |= n;
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

    DPRINT("Called IRQ n: %i, level: %i\n", n, level)
    // Only GROUP 0 Interrupts have subsources
    if (grp == 0 && S3C2416_intc_has_subsource(n))
    {
        int subsrc = s3c2416_intc_get_subsource(n);
            if (level)
                s->SUBSRCPND |= subsrc;

        // Subsource is masked
        if (s->INTSUBMSK & subsrc) {
            S3C2416_intc_update(s);
            return;
        }
    }


    DPRINT("Set IRQ n: %i, level: %i\n", n, level)
    DPRINT("Set grp %i IRQ: %x, sub %i level: %i\n",grp, irq, s3c2416_intc_get_subsource(n), level)

    if (level)
        s->SRCPND[grp] |= irq;

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
        /* PRIORITY_MODE */
    case 0x30:
        return s->PRIORITY_MODE[i];
        /* PRIORITY_UPDATE */
    case 0x34:
        return s->PRIORITY_UPDATE[i];
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
    DPRINT("interrupt write reg: %04x val: %llx\n",reg,val)
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
        s->SUBSRCPND &= ~val;
        break;
        /* INTSUBMSK */
    case 0x1C:
        s->INTSUBMSK = val;
        break;
        /* PRIORITY_MODE */
    case 0x30:
        s->PRIORITY_MODE[i] = val;
        break;
        /* PRIORITY_UPDATE */
    case 0x34:
        s->PRIORITY_UPDATE[i] = (uint8_t)val;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset %"HWADDR_PRIx"\n",
                      __func__, offset);
    }
    S3C2416_intc_update(s);
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
    s->INTMSK[0]=0xffffffff;
    s->INTMSK[1]=0xffffffff;
    s->INTSUBMSK=0xffffffff;
    s->last_int = INT_END;
    
    s->PRIORITY_MODE[0] = 0x0;
    s->PRIORITY_MODE[1] = 0x0;
    s->PRIORITY_MODE[0] = 0x7F;
    s->PRIORITY_MODE[1] = 0x7F;
};
static void S3C2416_intc_reset(DeviceState *dev)
{
    S3C2416_intc_state* s = S3C2416_INTC(dev);

    s->INTMSK[0]=0xffffffff;
    s->INTMSK[1]=0xffffffff;
    s->INTSUBMSK=0xffffffff;
    s->last_int = INT_END;
    
    s->PRIORITY_MODE[0] = 0x0;
    s->PRIORITY_MODE[1] = 0x0;
    s->PRIORITY_MODE[0] = 0x7F;
    s->PRIORITY_MODE[1] = 0x7F;
};
static void S3C2416_intc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->reset = S3C2416_intc_reset;
}

static const TypeInfo S3C2416_intc =
{
    .name = TYPE_S3C2416_INTC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S3C2416_intc_state),
    .instance_init = S3C2416_intc_init,
    .class_init = S3C2416_intc_class_init,
};

static void s3c2416_intc_register_types(void)
{
    type_register_static(&S3C2416_intc);
}

type_init(s3c2416_intc_register_types)
