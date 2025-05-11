.data
fullName dw "Abhinandan Krishnadev Bhuse*"
space dw " "
star dw "*"
c0 dw 0
c1 dw 1
c2 dw 2
c3 dw 3
c4 dw 4

.bss
storeNext resw 1
output resb
i resw 1
j resw 1
cchar resw 1
toff resw 1
mask resw 1

.text

start:
  lr space
  shr c3
  sr space
  lr star
  shr c3
  ;
  ;sr TEMP
  ;lr c3
  ;sr r2
  ;lr TEMP
  ;ls i2
  ;
  sr star

  mov storeNext, c1
  mov i, {fullName}
  mov j, {output}
  mul i, c4
  mul j, c4

loop:
  div i, c4
  lr [R1]
  shl [R2]
  shr c3

  cr space
  bt ifspcae
  cr star
  bt ifstar

store:
  sr cchar
  div j, c4
  sr toff

  shl c4
  not R1
  shl c3
  shr [R2]
  not R1
  sr mask

  lr [toff]
  and mask
  sr mask

  lr cchar
  shl c3
  shr [R2]
  or mask
  sr mask

  lr toff
  sr R2
  lr mask
  sr [R2]
  mov storeNext, c0
  inc j
  jmp continue

ifspcae:
  mov storeNext, c1 
  jmp continue

ifstar:
  h 0

continue:
  inc i
  jmp loop