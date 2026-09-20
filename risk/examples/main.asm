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
msg: db "Hello, World!", 10
len = $ - msg

