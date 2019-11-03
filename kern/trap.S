; vim: ft=nasm ai
; ----------------------------------------------------------------------------------------------------------------------
; 向量号	助记符	说明				类型		错误号		产生源
; --------------------------------------------------------------------------------------------------------------------
; 0		#DE	除出错				故障		无		DIV或IDIV指令
; 1		#DB	调试				故障/陷阱	无		任何代码或数据引用，或是INT 1指令
; 2		--	NMI中断				中断		无		非屏蔽外部中断
; 3		#BP	断点				陷阱		无		INT 3指令
; 4		#OF	溢出				陷阱		无		INTO指令
; 5		#BR	边界范围超出			故障		无		BOUND指令
; 6		#UD	无效操作码（未定义操作码）	故障		无		UD2指令或保留的操作码。（Pentium Pro中加入的新指令）
; 7		#NM	设备不存在（无数学协处理器）	故障		无		浮点或WAIT/FWAIT指令
; 8		#DF	双重错误			异常终止	有（0）		任何可产生异常、NMI或INTR的指令
; 9		--	协处理器段超越（保留）		故障		无		浮点指令（386以后的CPU不产生该异常）
; 10		#TS	无效的任务状态段TSS		故障		有		任务交换或访问TSS
; 11		#NP	段不存在			故障		有		加载段寄存器或访问系统段
; 12		#SS	堆栈段错误			故障		有		堆栈操作和SS寄存器加载
; 13		#GP	一般保护错误			故障		有		任何内存引用和其他保护检查
; 14		#PF	页面错误			故障		有		任何内存引用
; 15		--	（Intel保留，请勿使用）
; 16		#MF	x87 FPU浮点错误（数学错误）	故障		无		x87 FPU浮点或WAIT/FWAIT指令
; 17		#AC	对齐检查			故障		有（0）		对内存中任何数据的引用
; 18		#MC	机器检查			异常终止	无		错误码（若有）和产生源与CPU类型有关（奔腾处理器引进）
; 19		#XF	SIMD浮点异常			故障		无		SSE和SSE2浮点指令（PIII处理器引进）
; 20-31		--	（Intel保留，请勿使用）
; 32-255	--	用户定义（非保留）中断		中断		无		外部中断或者INT n指令
; ---------------------------------------------------------------------------------------------------------------------

section .data
global idt_entry_table
idt_entry_table:

%macro ient_ne 1
section .text
idt_entry_%1:
	push dword 0
	push dword %1
	jmp int_route
section .data
	dd idt_entry_%1
%endmacro

%macro ient_er 1
section .text
idt_entry_%1:
	push dword %1
	jmp int_route
section .data
	dd idt_entry_%1
%endmacro

; 调用 do_trap() 时的堆栈布局：
; SS
; ESP
; EFLAGS
; CS
; EIP
; ERROR CODE
; INTR NO.
; DS
; ES
; FS
; GS
; EBP
; EDI
; ESI
; EDX
; ECX
; EBX
; EAX
;

; {{{
ient_ne 0
ient_ne 1
ient_ne 2
ient_ne 3
ient_ne 4
ient_ne 5
ient_ne 6
ient_ne 7
ient_er 8
ient_ne 9
ient_er 10
ient_er 11
ient_er 12
ient_er 13
ient_er 14
ient_ne 15
ient_ne 16
ient_er 17
ient_ne 18
ient_ne 19
ient_ne 20
ient_ne 21
ient_ne 22
ient_ne 23
ient_ne 24
ient_ne 25
ient_ne 26
ient_ne 27
ient_ne 28
ient_ne 29
ient_ne 30
ient_ne 31
ient_ne 32
ient_ne 33
ient_ne 34
ient_ne 35
ient_ne 36
ient_ne 37
ient_ne 38
ient_ne 39
ient_ne 40
ient_ne 41
ient_ne 42
ient_ne 43
ient_ne 44
ient_ne 45
ient_ne 46
ient_ne 47
ient_ne 48
ient_ne 49
ient_ne 50
ient_ne 51
ient_ne 52
ient_ne 53
ient_ne 54
ient_ne 55
ient_ne 56
ient_ne 57
ient_ne 58
ient_ne 59
ient_ne 60
ient_ne 61
ient_ne 62
ient_ne 63
ient_ne 64
ient_ne 65
ient_ne 66
ient_ne 67
ient_ne 68
ient_ne 69
ient_ne 70
ient_ne 71
ient_ne 72
ient_ne 73
ient_ne 74
ient_ne 75
ient_ne 76
ient_ne 77
ient_ne 78
ient_ne 79
ient_ne 80
ient_ne 81
ient_ne 82
ient_ne 83
ient_ne 84
ient_ne 85
ient_ne 86
ient_ne 87
ient_ne 88
ient_ne 89
ient_ne 90
ient_ne 91
ient_ne 92
ient_ne 93
ient_ne 94
ient_ne 95
ient_ne 96
ient_ne 97
ient_ne 98
ient_ne 99
ient_ne 100
ient_ne 101
ient_ne 102
ient_ne 103
ient_ne 104
ient_ne 105
ient_ne 106
ient_ne 107
ient_ne 108
ient_ne 109
ient_ne 110
ient_ne 111
ient_ne 112
ient_ne 113
ient_ne 114
ient_ne 115
ient_ne 116
ient_ne 117
ient_ne 118
ient_ne 119
ient_ne 120
ient_ne 121
ient_ne 122
ient_ne 123
ient_ne 124
ient_ne 125
ient_ne 126
ient_ne 127
ient_ne 128
ient_ne 129
ient_ne 130
ient_ne 131
ient_ne 132
ient_ne 133
ient_ne 134
ient_ne 135
ient_ne 136
ient_ne 137
ient_ne 138
ient_ne 139
ient_ne 140
ient_ne 141
ient_ne 142
ient_ne 143
ient_ne 144
ient_ne 145
ient_ne 146
ient_ne 147
ient_ne 148
ient_ne 149
ient_ne 150
ient_ne 151
ient_ne 152
ient_ne 153
ient_ne 154
ient_ne 155
ient_ne 156
ient_ne 157
ient_ne 158
ient_ne 159
ient_ne 160
ient_ne 161
ient_ne 162
ient_ne 163
ient_ne 164
ient_ne 165
ient_ne 166
ient_ne 167
ient_ne 168
ient_ne 169
ient_ne 170
ient_ne 171
ient_ne 172
ient_ne 173
ient_ne 174
ient_ne 175
ient_ne 176
ient_ne 177
ient_ne 178
ient_ne 179
ient_ne 180
ient_ne 181
ient_ne 182
ient_ne 183
ient_ne 184
ient_ne 185
ient_ne 186
ient_ne 187
ient_ne 188
ient_ne 189
ient_ne 190
ient_ne 191
ient_ne 192
ient_ne 193
ient_ne 194
ient_ne 195
ient_ne 196
ient_ne 197
ient_ne 198
ient_ne 199
ient_ne 200
ient_ne 201
ient_ne 202
ient_ne 203
ient_ne 204
ient_ne 205
ient_ne 206
ient_ne 207
ient_ne 208
ient_ne 209
ient_ne 210
ient_ne 211
ient_ne 212
ient_ne 213
ient_ne 214
ient_ne 215
ient_ne 216
ient_ne 217
ient_ne 218
ient_ne 219
ient_ne 220
ient_ne 221
ient_ne 222
ient_ne 223
ient_ne 224
ient_ne 225
ient_ne 226
ient_ne 227
ient_ne 228
ient_ne 229
ient_ne 230
ient_ne 231
ient_ne 232
ient_ne 233
ient_ne 234
ient_ne 235
ient_ne 236
ient_ne 237
ient_ne 238
ient_ne 239
ient_ne 240
ient_ne 241
ient_ne 242
ient_ne 243
ient_ne 244
ient_ne 245
ient_ne 246
ient_ne 247
ient_ne 248
ient_ne 249
ient_ne 250
ient_ne 251
ient_ne 252
ient_ne 253
ient_ne 254
ient_ne 255
; }}}

section .text
extern do_trap
int_route:
	push ds
	push es
	push fs
	push gs
	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	mov ebx, 0x10
	mov ds, ebx
	mov es, ebx
	mov fs, ebx
	mov gs, ebx
	call do_trap
return_to_user:
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	pop gs
	pop fs
	pop es
	pop ds
	add esp, 8
	iretd

global __move_to_user
__move_to_user:
	mov esp, [esp + 4]
	jmp return_to_user
