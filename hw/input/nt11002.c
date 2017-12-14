#include "qemu/osdep.h"
#include "hw/hw.h"
#include "hw/i2c/i2c.h"
//#include "qemu/timer.h"
//#include "ui/console.h"

#include "qemu-common.h"
#include "qapi/error.h"
#include "hw/sysbus.h"
#include "ui/console.h"
#include "ui/input.h"


#define TYPE_NT11002 "nt11002"
#define NT11002(obj) OBJECT_CHECK(nt11002_state, (obj), TYPE_NT11002)

typedef struct
{
    I2CSlave parent_obj;
    
    uint8_t i2c_cycle;
    uint8_t i2c_dir;
    uint8_t status;
    
    qemu_irq nirq;
} nt11002_state;

//static uint8_t nt_read(nt11002_state *s, int reg, int byte)
//{
    // int ret;
    //
    //switch (reg) {
    //case LM832x_CMD_READ_ID:
    //    ret = 0x0400;
    //    break;
    //
    //case LM832x_CMD_READ_INT:
    //    ret = s->status;
    //    if (!(s->status & INT_NOINIT)) {
    //        s->status = 0;
    //        lm_kbd_irq_update(s);
    //    }
    //    break;
    //
    //case LM832x_CMD_READ_PORT_SEL:
    //    ret = s->gpio.dir;
    //    break;
    //case LM832x_CMD_READ_PORT_STATE:
    //    ret = s->gpio.mask;
    //    break;
    //
    //case LM832x_CMD_READ_FIFO:
    //    if (s->kbd.len <= 1)
    //        return 0x00;

        /* Example response from the two commands after a INT_KEYPAD
         * interrupt caused by the key 0x3c being pressed:
         * RPT_READ_FIFO: 55 bc 00 4e ff 0a 50 08 00 29 d9 08 01 c9 01
         *     READ_FIFO: bc 00 00 4e ff 0a 50 08 00 29 d9 08 01 c9 01
         * RPT_READ_FIFO: bc 00 00 4e ff 0a 50 08 00 29 d9 08 01 c9 01
         *
         * 55 is the code of the key release event serviced in the previous
         * interrupt handling.
         *
         * TODO: find out whether the FIFO is advanced a single character
         * before reading every byte or the whole size of the FIFO at the
         * last LM832x_CMD_READ_FIFO.  This affects LM832x_CMD_RPT_READ_FIFO
         * output in cases where there are more than one event in the FIFO.
         * Assume 0xbc and 0x3c events are in the FIFO:
         * RPT_READ_FIFO: 55 bc 3c 00 4e ff 0a 50 08 00 29 d9 08 01 c9
         *     READ_FIFO: bc 3c 00 00 4e ff 0a 50 08 00 29 d9 08 01 c9
         * Does RPT_READ_FIFO now return 0xbc and 0x3c or only 0x3c?
         */
    //    s->kbd.start ++;
    //    s->kbd.start &= sizeof(s->kbd.fifo) - 1;
    //    s->kbd.len --;
    //
    //    return s->kbd.fifo[s->kbd.start];
    //case LM832x_CMD_RPT_READ_FIFO:
    //    if (byte >= s->kbd.len)
    //        return 0x00;
    //
    //    return s->kbd.fifo[(s->kbd.start + byte) & (sizeof(s->kbd.fifo) - 1)];
    //
    //case LM832x_CMD_READ_ERROR:
    //    return s->error;
    //
    //case LM832x_CMD_READ_ROTATOR:
    //    return 0;
    //
    //case LM832x_CMD_READ_KEY_SIZE:
    //    return s->kbd.size;
    //
    //case LM832x_CMD_READ_CFG:
    //    return s->config & 0xf;
    //
    //case LM832x_CMD_READ_CLOCK:
    //    return (s->clock & 0xfc) | 2;
    //
    //default:
    //    lm_kbd_error(s, ERR_CMDUNK);
    //    fprintf(stderr, "%s: unknown command %02x\n", __FUNCTION__, reg);
    //    return 0x00;
    //}
    //
    //return ret >> (byte << 3); 
//}

static int nt_i2c_event(I2CSlave *i2c, enum i2c_event event)
{
    nt11002_state *s = NT11002(i2c);

    //printf("nt11002 i2c event recieved!\n");
    
    switch (event) {
    case I2C_START_RECV:
    case I2C_START_SEND:
        s->i2c_cycle = 0;
        s->i2c_dir = (event == I2C_START_SEND);
        break;

    default:
        break;
    }

    return 0;
}

static int nt_i2c_rx(I2CSlave *i2c)
{
    //nt11002_state *s = NT11002(i2c);
    //printf("nt11002: I RECIEVED SOME CRAP\n");

    return 0;
    //return nt_read(s, s->reg, s->i2c_cycle ++);
}

static int nt_i2c_tx(I2CSlave *i2c, uint8_t data)
{
    //printf("nt11002: Recieved data: %x\n", data);

    return 0;
}

static int nt11002_init(I2CSlave *i2c)
{
    nt11002_state *s = NT11002(i2c);

    s->status = 0;
    //s->model = 0x8323;
   qdev_init_gpio_out(DEVICE(i2c), &s->nirq, 1);

    //lm_kbd_reset(s);

    //qemu_register_reset((void *) lm_kbd_reset, s);

    return 0;
}

static void nt11002_class_init(ObjectClass *klass, void *data)
{
    //DeviceClass *dc = DEVICE_CLASS(klass);
    I2CSlaveClass *k = I2C_SLAVE_CLASS(klass);

    k->init = nt11002_init;
    k->event = nt_i2c_event;
    k->recv = nt_i2c_rx;
    k->send = nt_i2c_tx;
}

static const TypeInfo nt11002_info = {
    .name          = TYPE_NT11002,
    .parent        = TYPE_I2C_SLAVE,
    .instance_size = sizeof(nt11002_state),
    .class_init    = nt11002_class_init,
};

static void nt11002_register_types(void)
{
    type_register_static(&nt11002_info);
}

type_init(nt11002_register_types)