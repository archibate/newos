#ifndef _KERN_IRQ_H
#define _KERN_IRQ_H 1

#define IRQ_PIT		0
#define IRQ_KBD		1
#define IRQ_PIC1	2
#define IRQ_COM1	3
#define IRQ_COM0	4
#define IRQ_ATA1	5
#define IRQ_FDC		6
#define IRQ_ATA0	7
#define IRQ_RTC		8
#define IRQ_MOUSE	12
#define IRQ_FPU		13
#define IRQ_IDE		14
#define NIRQS		16

void install_irq(int irq, void (*proc)(void));
void irq_enable(int irq, int enable);
void irq_done(int irq);
void do_irq(int irq);

#endif
