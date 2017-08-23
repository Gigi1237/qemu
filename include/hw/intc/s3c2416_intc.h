#ifndef S3C2416_INTC_H
#define S3C2416_INTC_H

#include "hw/sysbus.h"

#define TYPE_S3C2416_INTC "S3C2416-intc"
#define S3C2416_INTC(obj) OBJECT_CHECK(S3C2416_intc_state, (obj), TYPE_S3C2416_INTC)


enum InterruptId
{
    INT_ADC,
    SUBINT_TC,
    SUBINT_ADC,

    INT_RTC,
    
    INT_UART0,
    SUBINT_RXD0,
    SUBINT_TXD0,
    SUBINT_ERR0,

    INT_IIC0,
    INT_USBH,
    INT_USBD,
    INT_NAND,
    
    INT_UART1,
    SUBINT_ERR1,
    SUBINT_TXD1,
    SUBINT_RXD1,

    INT_SPI0,
    INT_SDI0,
    INT_SDI1,

    INT_UART3,
    SUBINT_RXD3,
    SUBINT_TXD3,
    SUBINT_ERR3,

    INT_DMA,
    SUBINT_DMA0,
    SUBINT_DMA1,
    SUBINT_DMA2,
    SUBINT_DMA3,
    SUBINT_DMA4,
    SUBINT_DMA5,

    INT_LCD,
    SUBINT_LCD2,
    SUBINT_LCD3,
    SUBINT_LCD4,

    INT_UART2,
    SUBINT_RXD2,
    SUBINT_TXD2,
    SUBINT_ERR2,

    INT_TIMER4,
    INT_TIMER3,
    INT_TIMER2,
    INT_TIMER1,
    INT_TIMER0,
    
    INT_WDT_AC97,
    SUBINT_WDT,
    SUBINT_AC97,

    INT_TICK,
    nBATT_FLT,
    EINT8_15,
    EINT4_7,
    EINT3,
    EINT2,
    EINT1,
    EINT0,
    INT_I2S0,
    INT_PCM0,
    INT_2D,
    
    INT_END
};

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

#endif
