#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "ui/console.h"
#include "framebuffer.h"
#include "ui/pixel_ops.h"
#include "qemu\log.h"

#include <stdio.h>
#include <stdlib.h>

#define TYPE_S3C2416_LCD "s3c2416-lcd"
#define S3C2416_LCD(obj) OBJECT_CHECK(S3C2416_lcd_state, (obj), TYPE_S3C2416_LCD)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    MemoryRegionSection fbsection;
    QemuConsole *con;

    uint32_t VIDCON[2];
    uint32_t VIDTCON[3];
    uint32_t WINCON[2];
    uint32_t VIDOSD0[2];
    uint32_t VIDOSD1[3];

    uint32_t VIDW00ADD0B[2];
    uint32_t VIDW00ADD1B[2];

    uint32_t cols;
    uint32_t rows;

    int invalidate;
    qemu_irq irq;
} S3C2416_lcd_state;

static void S3C2416_lcd_draw_line(void *opaque, uint8_t *d, const uint8_t *src, int width, int deststep)
{
    for (int i = 0; i < width * deststep; i++) {
        d[i] = src[i];
    }
}


static void S3C2416_lcd_update_display(void *opaque)
{
    S3C2416_lcd_state *s = (S3C2416_lcd_state*)opaque;
    SysBusDevice *sbd; 
    DisplaySurface *surface = qemu_console_surface(s->con);
    //drawfn* fntable;
    //drawfn fn;
    //int dest_width;
    //int src_width;
    //int bpp_offset;
    int first;
    int last;

    sbd = SYS_BUS_DEVICE(s);

    // LCD is disabled;
    if ((s->VIDCON[0] & 3) != 3) {
        return;
    }
    int currentBuf = (s->WINCON[0] >> 23) & 0x1;


    first = 0;

    if (s->invalidate) {
        framebuffer_update_memory_section(&s->fbsection,
            sysbus_address_space(sbd),
            s->VIDW00ADD0B[currentBuf],
            240, 320 * 4);
    }

    framebuffer_update_display(surface, &s->fbsection, 320, 240, 4 * 320, 4 * 320, 4, s->invalidate, S3C2416_lcd_draw_line, NULL, &first, &last);

    if (first >= 0) {
        dpy_gfx_update(s->con, 0, first, s->cols, last - first + 1);
    }
    s->invalidate = 0;
}

static void S3C2416_lcd_write(void *opaque, hwaddr offset,
    uint64_t val, unsigned size)
{
    S3C2416_lcd_state *s = (S3C2416_lcd_state*)opaque;

    //printf("S3C2416_lcd: register write 0x%llx, val: 0x%llx\n", offset, val);
    s->invalidate = 1;

    switch (offset)
    {
    case 0x0: /* VIDCON0 */
        s->VIDCON[0] = val;
        break;
    case 0x4: /* VIDCON1 */
        s->VIDCON[1] = val;
        break;
    case 0x8: /* VIDTCON0 */
        s->VIDTCON[0] = val;
        break;
    case 0xC: /* VIDTCON1 */
        s->VIDCON[1] = val;
        break;
    case 0x10: /* VIDTCON2 */
        s->VIDTCON[2] = val;
        break;
    case 0x14: /* WINCON0 */
        s->WINCON[0] = val;
        break;
    case 0x18: /* WINCON1 */
        s->WINCON[1] = val;
        break;
    case 0x64:
        s->VIDW00ADD0B[0] = val;
        break;
    case 0x68:
        s->VIDW00ADD0B[1] = val;
        break;
    case 0x7c:
        s->VIDW00ADD1B[0] = val;
        break;
    case 0x80:
        s->VIDW00ADD1B[1] = val;
        break;

    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_lcd: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }


    /* VIDCON 0 init: 0x21104 */
    // RGB PARALLEL FMT
    // L0_data = 9+9 bit mode (18bpp)
    // L1_data = 16 bit (16bpp)
    // Vidout = RGB I/F

    /* WINCON0 init: 0x2d */
    // Window 0 enable
    // BPPMODE: Unpakced 24bpp (not-palletized R:8-G:8-B:8)
    // 16 word burst
    // buffer0 select

    /* VIDOSDA0A init: 0x0 */

    /* VIDOSD0B init: 0xefa7f*/
    // RightBotY = 0x27F
    // RightBotX = 0x1DF

    /* VIDINTCON init: 0x3f0102d */
    // LCD interrupt enable
    // LCD FIFO interrupt disable
    // FIFO LEVEL EMPTY
    // Window 0 control enable
    // Video Frame interrupt enable
    // No video frame interrupt 1
    // Video frame interupt 0 back porch
    
    /* VIDW00ADD0B0 init: 0x31a00000 */
    // WINDOW 0 Buffer zero start at 0x31a00000

    /* VIDW00ADD0B1 init: 0x31b00000 */
    // WINDOW 0 Buffer one start at 0x31b00000

    /* VIDW00ADD1B0 init: 0xa0000 */
    // WINDOW 0 Buffer zero end 0xa0000

    /* VIDW00ADD0B1 init: 0xa0000 */
    // WINDOW 0 Buffer one end 0xa0000

    /* WINCON1 init: 0x0 */
    // window 1 disabled

    /* VIDTCON0 init: 0x110300 */
    /* VIDTCON1 init: 0x401100 */
    /* VIDTCON3 init: 0x7793f */

    /* VIDCON1 init 0x80 */

}

static uint64_t S3C2416_lcd_read(void *opaque, hwaddr offset,
    unsigned size)
{
    S3C2416_lcd_state *s = (S3C2416_lcd_state*)opaque;
    uint32_t val;

    switch (offset)
    {
    case 0x0: /* VIDCON0 */
        val = s->VIDCON[0];
        break;
    case 0x4: /* VIDCON1 */
        val = s->VIDCON[1];
        break;
    case 0x8: /* VIDTCON0 */
        val = s->VIDTCON[0];
        break;
    case 0xC: /* VIDTCON1 */
        val = s->VIDCON[1];
        break;
    case 0x10: /* VIDTCON2 */
        val = s->VIDTCON[2];
        break;
    case 0x14: /* WINCON0 */
        val = s->WINCON[0];
        break;
    case 0x18: /* WINCON1 */
        val = s->WINCON[1];
        break;
    case 0x64:
        val = s->VIDW00ADD0B[0];
        break;
    case 0x68:
        val = s->VIDW00ADD0B[1];
        break;
    case 0x7c:
        val = s->VIDW00ADD1B[0];
        break;
    case 0x80:
        val = s->VIDW00ADD1B[1];
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "S3C2416_lcd: Attempted write to invalid register! off: 0x%llx\n", offset);
        break;
    }

    //printf("S3C2416_lcd: register read 0x%llx, val 0x%x\n", offset, val);



    return val;
}

static void s3c2416_lcd_invalidate_display(void * opaque)
{
    S3C2416_lcd_state *s = (S3C2416_lcd_state *)opaque;
    s->invalidate = 1;
    if ((s->VIDCON[0] & 3) == 3) {
        qemu_console_resize(s->con, s->cols, s->rows);
    }
}

static const MemoryRegionOps S3C2416_lcd_ops = {
    .read = S3C2416_lcd_read,
    .write = S3C2416_lcd_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const GraphicHwOps S3C2416_lcd_gfx_ops = {
    .invalidate = s3c2416_lcd_invalidate_display,
    .gfx_update = S3C2416_lcd_update_display,
};

static void S3C2416_lcd_realize(DeviceState *dev, Error **errp)
{
    S3C2416_lcd_state *s = S3C2416_LCD(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    memory_region_init_io(&s->iomem, OBJECT(s), &S3C2416_lcd_ops, s, "s3c2416_lcd", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);
    //qdev_init_gpio_in(dev, pl110_mux_ctrl_set, 1);
    s->con = graphic_console_init(dev, 0, &S3C2416_lcd_gfx_ops, s);
    qemu_console_resize(s->con, s->cols, s->rows);
}

static void S3C2416_lcd_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    set_bit(DEVICE_CATEGORY_DISPLAY, dc->categories);
    dc->realize = S3C2416_lcd_realize;
}

static void S3C2416_lcd_init(Object *obj)
{
    S3C2416_lcd_state *s = S3C2416_LCD(obj);

    //s->VIDCON[0] = 0x5270;
    //s->VIDTCON[0] = 0x110300;
    //s->VIDTCON[1] = 0x401100;
    //s->VIDTCON[2] = 0x7793F;
    //s->VIDCON[1] = 0x8080;
    s->cols = 320;
    s->rows = 240;
}
static TypeInfo S3C2416_lcd_info =
{
    .name = TYPE_S3C2416_LCD,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S3C2416_lcd_state),
    .class_init = S3C2416_lcd_class_init,
    .instance_init = S3C2416_lcd_init
};


static void S3C2416_lcd_register(void)
{
    type_register_static(&S3C2416_lcd_info);
}

type_init(S3C2416_lcd_register);