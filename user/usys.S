# user/usys.S
.globl getpid
getpid:
    li a7, 1
    ecall
    ret

.globl exit
exit:
    li a7, 3
    ecall
    ret

.globl wait
wait:
    li a7, 4
    ecall
    ret

.globl write
write:
    li a7, 5
    ecall
    ret

.globl open
open:
    li a7, 6
    ecall
    ret

.globl close
close:
    li a7, 7
    ecall
    ret

.globl read
read:
    li a7, 8
    ecall
    ret

.globl unlink
unlink:
    li a7, 9
    ecall
    ret
