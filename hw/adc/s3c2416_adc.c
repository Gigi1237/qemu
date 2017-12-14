#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu-common.h"
#include "qemu/log.h"
#include "hw/sysbus.h"

#define TYPE_S3C2416_ADC "s3c2416-adc"
#define S3C2416_ADC(obj) OBJECT_CHECK(s3c2416_adc_state, (obj), TYPE_S3C2416_ADC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    
    uint32_t ADCCON;
    uint32_t ADCTSC;
    uint32_t ADCDLY;
    uint32_t ADCDAT0;
    uint32_t ADCDAT1;
    uint32_t ADCUPDN;
    uint32_t ADCMUX;
} s3c2416_adc_state;


static uint64_t s3c2416_adc_read(void *opaque, hwaddr offset,
    unsigned size)
{
    s3c2416_adc_state *s = (s3c2416_adc_state*)opaque;
    uint32_t val;
    
    switch (offset)
    {
    // ADCCON
    case 0x0:
        val = s->ADCCON;
        break;

    // ADCTSC
    case 0x4:
        val = s->ADCTSC;
        break;

    // ADCDLY
    case 0x8:
        val = s->ADCDLY;
        break;

    // ADCDAT0
    case 0xC:
        val = s->ADCDAT0;
        break;

    // ADCDAT1
    case 0x10:
        val = s->ADCDAT1;
        break;

    // ADCUPDN
    case 0x14:
        val = s->ADCUPDN;
        break;

    // ADCMUX
    case 0x18:
        val = s->ADCMUX;
        break;

        default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_adc: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
    
    return val;
};

static void s3c2416_adc_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    s3c2416_adc_state *s = (s3c2416_adc_state*)opaque;
    
    switch (offset)
    {
    // ADCCON
    case 0x0:
        s->ADCCON = val;
        break;

    // ADCTSC
    case 0x4:
        s->ADCTSC = val;
        break;

    // ADCDLY
    case 0x8:
        s->ADCDLY = val;
        break;

    // ADCUPDN
    case 0x14:
        s->ADCUPDN = val;
        break;

    // ADCMUX
    case 0x18:
        s->ADCMUX = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "s3c2416_adc: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }
};
 

static const MemoryRegionOps s3c2416_adc_ops = {
    .read = s3c2416_adc_read,
    .write = s3c2416_adc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};


static void s3c2416_adc_init(Object *obj)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    s3c2416_adc_state *s = S3C2416_ADC(obj);

    memory_region_init_io(&s->iomem, OBJECT(s), &s3c2416_adc_ops, s, "s3c2416_adc", 0x00001000);
    sysbus_init_mmio(sbd, &s->iomem);

    s->ADCCON = 0x3FC4;
    s->ADCTSC = 0x58;
    s->ADCDLY = 0xFF;
    s->ADCDAT0 = 0xFF;
    s->ADCDAT1 = 0xFF;
    s->ADCUPDN = 0xFF;
    s->ADCMUX = 0x0;
};

static const TypeInfo s3c2416_adc_type = {
    .name = TYPE_S3C2416_ADC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(s3c2416_adc_state),
    .instance_init = s3c2416_adc_init
};

static void s3c2416_adc_register(void)
{
    type_register_static(&s3c2416_adc_type);
}

type_init(s3c2416_adc_register);
