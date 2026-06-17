.syntax unified
.thumb

.section .vectors, "a", %progbits
.word __stack_top
.word reset_handler + 1
.rept 46
.word 0
.endr

.text
.thumb_func
.global reset_handler
reset_handler:
	bl main
1:
	b 1b
