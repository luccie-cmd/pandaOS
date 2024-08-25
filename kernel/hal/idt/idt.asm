bits 64

global loadIDT
loadIDT:
    lidt [rdi]
    ret