
#ifndef S3C2416_H
#define S3C2416_H
DeviceState *s3c2416_uart_create(hwaddr addr,
                                    int fifo_size,
                                    int channel,
                                    Chardev *chr,
                                    qemu_irq irq_tx,
                                    qemu_irq irq_rx,
                                    qemu_irq irq_err);
#endif
