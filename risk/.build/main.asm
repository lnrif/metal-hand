format ELF64 executable 3
entry start

segment readable executable
start:
	; write(...)
	mov rax, 1
	mov rdi, 1
	mov rsi, msg
	mov rdx, len
	syscall
	; exit(0)
	mov rax, 60
	mov rdi, 0
	syscall

segment readable
; message
msg: db 27, "[1;35m", "Hello, Risky!", 27, "[m", 10
len = $ - msg

