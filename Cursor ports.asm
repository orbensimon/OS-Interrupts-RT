;
; Cursor2.asm
;
;This program demonstrates cursor manipulation.
;
;
Stak              SEGMENT PARA STACK 'STACK'
DB                 256 DUP(0)
Stak              ENDS
;
Data               SEGMENT PARA PUBLIC 'Data'
CursorPos          DW      0     ; Number of lines scrolled down
LimitWindow			DW     25*40
LeftRightLimit			dw	0
UpDownLimit			dw  0
Color			db 00001100b
PrevPlaceUpperByte		db 0
PrevPlaceLowerByte db 0
PrevPlace dw 0
Base               DW      0
;
Data               ENDS
Code               SEGMENT PARA PUBLIC 'Code'
                   .386  ; Enable 386 commands
SetCursorPos          PROC    NEAR
; Set Cursor to position in BX
; Input: BX
; Output: None
;
  MOV              DX,3D4H  ; Point to 3D4H - 3D5H port pair
  MOV              AL,14    ; Address of cursor register pos high byte
  MOV              AH,BH    ; Get desired value of cursor pos high byte
  OUT              DX,AX    ; Port(3D4h) = 14, Port(3D5h) = Value of BH
;
  MOV              AL,15    ; Address of cursor register pos low byte
  MOV              AH,BL    ; Get desired value pf cursor pos low byte
  OUT              DX,AX    ; Port(3D4h) = 15, Port(3D5h) = Value of BL
;
  RET                       ; Return to caller
SetCursorPos          ENDP
;
Start              PROC    FAR
;
;STANDARD PROGRAM PROLOGUE
;
  ASSUME           CS:Code
  PUSH             DS          ; Save PSP segment address
  MOV              AX,0
  PUSH             AX          ; Save INT 20h address offset (PSP+0)
  MOV              AX,Data
  MOV              DS,AX       ; Establish Data segment addressability
  ASSUME           DS:Data
;
;Part1 : Initialize the display adapter
;
  MOV              AH,0          ; Select function = 'Set mode'
  MOV              AL,1          ; 40 by 25 color image
  INT              10h           ; Adapter initialized. Page 0 displayed
;
  MOV              AX,0B800h     ; Segment address of memory on color adapter
;
  MOV              ES,AX         ; Set up extra segment register
  MOV              DI,0          ; Initial offset address into segment
  MOV              AL,' '        ; Character space to fill adapter memory
  MOV              AH,00001000B        ; Attribute byte : Intense yellow
  MOV              CX,1000       ; Initialize count, s1 Screen
  CLD                            ; Write forward
  REP              STOSW         ; Write
;
; Write 'A' in mid screen
;
  
;
; Set the cursor address registers
;
  MOV              BX,0
  CALL             SetCursorPos
  push ax
  mov ax,25
  imul ax,40
  mov LimitWindow,ax
  pop ax
;
;PART 2 : Wait for key strike
;
; Wait for key
;
mov di,0
NextLoop:
  MOV              AH,0       ; Wait and read key
  INT              16h        ;
  CMP              AH,1       ; Is it Esc?
  JE               ToReturn   ; Yes - Return to DOS
IsLeft:
  cmp ah,4bh
  jne IsRight
  cmp LeftRightLimit,0
  je NextLoop
	dec bx
	dec LeftRightLimit
	mov di,bx
	add di,bx
	call SetCursorPos
	jmp NextLoop
IsRight:
	cmp ah,4dh
	jne IsUp
	cmp LeftRightLimit,39
	je NextLoop
	inc bx
	inc LeftRightLimit
	mov di,bx
	add di,bx
	call SetCursorPos
	jmp NextLoop
IsUp:
	cmp ah,50h
	jne IsDown
	cmp UpDownLimit,24
	je NextLoop
	add bx,40
	inc UpDownLimit
	mov di,bx
	add di,bx
	call SetCursorPos
	jmp NextLoop
IsDown:
	cmp ah,48h
	jne Isf1
	cmp UpDownLimit,0
	je NextLoop
	sub bx,40
	dec UpDownLimit
	mov di,bx
	add di,bx
	call SetCursorPos
	jmp NextLoop
Isf1:
	cmp ah,3bh
	jne Isf2
	cmp color,255
	jbe IncColor
	push ax
	mov ah,0
	mov Color,ah
	pop ax
	jmp SetColor
IncColor:
	inc Color
SetColor:
	push ax
	mov ax,0b800h
	mov es,ax
	pop ax
	mov di,bx
	add di,bx
	push ax
	mov ax,es:[di]
	mov ah,Color
	mov es:[di],ax
	pop ax
	jmp NextLoop
Isf2:
	cmp ah,3ch
	jne Isf3
	push ax
	mov di,bx
	add di,bx
	mov ax,es:[di]
	mov ah,al
	mov es:[di],ax
	pop ax
	jmp NextLoop
Isf3:
	cmp ah,3dh
	jne Isf4
	mov dx,3d4h
	push ax
	mov ax,000ah
	out dx,ax
	mov ax,0f0bh
	out dx,ax
	pop ax
	jmp NextLoop
Isf4:
	cmp ah,3eh
	jne Isz
	mov dx,3d4h
	push ax
	mov ax,0e0ah
	out dx,ax
	mov ax,0f0bh
	out dx,ax
	jmp NextLoop
Isz:
	cmp al,'z'
	jne Isx
	;MOV            DX,3D4H  ; Point to 3D4H - 3D5H port pair
   ; MOV              AL,14    ; Address of cursor register pos high byte
   ; in ax,dx
   ; mov PrevPlaceUpperByte,ah
   ; mov al,15
   ; in ax,dx
   ; mov PrevPlaceLowerByte,ah
   ; mov ah,PrevPlaceUpperByte
   ; mov al,PrevPlaceLowerByte
   ; mov PrevPlace,ax
   mov PrevPlace,bx
    mov bx,999
    call SetCursorPos
    jmp NextLoop
 Isx:
	
	cmp al,'x'
	jne Anykey
	mov bx,PrevPlace
	call SetCursorPos
	jmp NextLoop
	
	
Anykey:
    cmp al,0
    ja setcursor
	jmp NextLoop
   ; MOV             DX,3D4h  ; Point TO 3D4h - 3D5h port pair
   ; MOV             AX,000Ah ; Cursor start address (0Ah) - Value 0 (00h)
   ; OUT             DX,AX    ; Port(3D4h) = 0Ah, Port(3D5h) = 01h
   ; MOV             AX,0F0Bh ; Cursor end address - Value 15 (0Eh)
   ; OUT             DX,AX    ; Port(3D4h) = 0Bh, Port(3D5h) = 0Eh
	setcursor:
	
	MOV              BYTE PTR ES:[di],al
    add             BX,40
    cmp bx,LimitWindow
	jb InLimit
    mov bx,0
	mov UpDownLimit,0
InLimit:
	inc UpDownLimit
    call SetCursorPos
    add di,80
   
   ; MOV             DX,3D4h   ; Point to 3D4H - 3D5H port pair
   ; MOV             AX,0E0Ah  ; Cursor start address (0Ah) - Value 14 (0Dh)
   ; OUT             DX,AX     ; Port(3D4h) = 0Ah, Port(3D5h) = 01h
   ; MOV             AX,0F0Bh  ; Cursor end address - Value 15 (0Eh)
   ; OUT             DX,AX
; Wait for key
;

;

 
  JMP              NextLoop   ; Repeat main loop
;
ToReturn:
                   MOV  AX,2
                   INT  10h
                   RET
Start              ENDP
Code               ENDS
  END              Start

