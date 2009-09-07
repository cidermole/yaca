#include <avr/io.h>

#define packstyle r28 // YL reg!!
#define temp r29 // YH reg!!
#define counter r1 // must contain 0 afterwards, clean up!
#define pack2 r2

; Packstyle: when bit 1, it implies that a byte was skipped (like 1byte argument)
; for 2byte args, make 2 zero (0) bits
; LSb stands for the first function argument, registers begin with r25

; void _unpack(uint8_t* pdata, uint8_t packstyle);
; pdata -> r25:r24      packstyle -> r22
.global _unpack
_unpack:
	push	YH
	push	YL		; need for packstyle register

	movw	ZL, r24		; Z -> message data (packed)
	ldi	XL, 8
	mov	counter, XL	; preload counter = 8 (reg < 16...)
	ldi	XH, 0
	ldi	XL, 26		; X -> reg data (target for unpacking)
	mov	packstyle, r22

_up_loop:
	sbrc	packstyle, 0	; If LSb is set...
	sbiw	XL, 1		; ...decrement X -> target reg
	lsr	packstyle	; right shift packstyle
	ld	temp, Z+	; load a message byte
	st	-X, temp	; store message byte in reg
	dec	counter
	brne	_up_loop

	pop	YL
	pop	YH
;	clr	counter ; 0 after loop anyway
	ret

; Requires packstyle in r31
; _take_off() will then use _global_canid
; void _pack_n_go();
.global _pack_n_go
_pack_n_go:
	push	pack2
	push	YH
	push	YL
	in	YL, _SFR_IO_ADDR(SPL)
	in	YH, _SFR_IO_ADDR(SPH)
	mov	pack2, r31
	movw	ZL, YL		; Z -> SRAM
	sbiw	ZL, 7		; Message.data size - 1 (think of stack model. SP points to a free location -> 1 free byte would require no subtraction ("-0")

	sbiw	YL, 15
	in	r0, _SFR_IO_ADDR(SREG)
	cli
	out	_SFR_IO_ADDR(SPH), YH
	out	_SFR_IO_ADDR(SREG), r0
	out	_SFR_IO_ADDR(SPL), YL
	push	YH
	push	YL
	mov	packstyle, pack2

;	ldi	XH, 0		; X -> regs to backup
;	ldi	XL, 22		; r22+: CAN id (first argument as uint32_t)

;	ldi	temp, 4		; Copy CAN-ID to SRAM first
;	mov	counter, temp	; preload counter (reg<16) with 16
;_ploop:
;	ld	temp, X+	; load a reg to backup
;	st	Z+, temp	; store in SRAM
;	dec	counter
;	brne	_ploop

	ldi	XH, 0
	ldi	XL, 26		; X -> source reg
;	lds	packstyle, _global_packstyle
	ldi	temp, 8		; preload counter = 8
	mov	counter, temp

	; Z -> SRAM
_ploop2:
	sbrc	packstyle, 0	; If LSb is set...
	sbiw	XL, 1		; ...decrement X -> source reg
	lsr	packstyle	; right shift packstyle
	ld	temp, -X	; load a message byte from reg
	st	Z+, temp	; store in SRAM
	dec	counter
	brne	_ploop2

	pop	YL
	pop	YH

	movw	r24, YL
	adiw	r24, 1

;	clr	counter		; after loop, counter must be 0...
	rcall	_take_off	; _take_off(Message*)

	adiw	YL, 15
	in	r0, _SFR_IO_ADDR(SREG)
	cli
	out	_SFR_IO_ADDR(SPH), YH
	out	_SFR_IO_ADDR(SREG), r0
	out	_SFR_IO_ADDR(SPL), YL
	pop	YL
	pop	YH
	pop	pack2
	ret

