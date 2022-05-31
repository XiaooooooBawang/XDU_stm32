	AREA RESET, DATA, READONLY
	EXPORT __Vectors
__Vectors
	DCD	Reset_Handler 
	
	AREA |.text|, CODE, READONLY
	ENTRY
Reset_Handler	PROC 
	EXPORT	Reset_Handler	
;add your code here
	mov r3,#0x18
	ldr r0,[r3,#0]
	ldr r1,[r3,#4]
	add r0,r1
	mov r3,#0x20000000
	str r0,[r3]
	b .
	NOP
 
	endp
a DCD 7
c DCD 10
	NOP
	end
		