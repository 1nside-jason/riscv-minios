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
