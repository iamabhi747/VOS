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
storeNext resw
output resl
i resw
j resw
cchar resw
toff resw
mask resw

.text

start:
  lr space
  rs c3
  sr space
  lr star
  rs c3
  
  sr TEMP
  lr c3
  sr r2
  lr TEMP
  ls i2
  
  sr star

  ;mov storeNext, c1
  ;mov i, {fullName}
  ;mov j, {output}
  lr c4
  ml i
  sr i
  lr cr
  ml j
  sr j

loop:
  lr i
  dv c4
  lr [R1]
  ls [R2]
  rs c3

  cr space
  bt ifspcae
  cr star
  bt ifstar

store:
  sr cchar
  lr j
  dv c4
  sr toff

  ls c4
  nt R1
  ls c3
  rs [R2]
  nt R1
  sr mask

  lr [toff]
  an mask
  sr mask

  lr cchar
  ls c3
  rs [R2]
  or mask
  sr mask

  lr toff
  sr R2
  lr mask
  sr [R2]
  lr c0
  sr storeNext
  im j
  cr R1
  bt continue

ifspcae:
  lr c1
  sr storeNext
  cr R1
  bt continue

ifstar:
  h 0

continue:
  im i
  cr R1
  bt loop