int main(void)
{
	asm volatile ("int $0x80" :: "a" (1), "c" ("Hello from user!\n"), "d" (17));
	for (;;);
}
