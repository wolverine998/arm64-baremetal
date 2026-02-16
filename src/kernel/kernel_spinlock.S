.global spin_lock
// x0 = uint32_t *lock
spin_lock:
mov w1, #1
1:
ldaxr w2, [x0]
cbz w2, 2f
wfe
b 1b
2:
stlxr w3, w1, [x0]
cbnz w3, 1b
ret

.global spin_unlock
// x0 = uint32_t *lock
spin_unlock:
stlr wzr, [x0]
ret
