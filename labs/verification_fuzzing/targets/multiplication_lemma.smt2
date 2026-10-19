; Independent mathematical bridge for uint32 checked multiplication.
; Euclidean division: M=q*b+r, 0<=r<b. For b>0, a*b<=M iff a<=q.
(set-logic QF_NIA)
(declare-const a Int)
(declare-const b Int)
(declare-const q Int)
(declare-const r Int)
(assert (and (<= 0 a) (<= a 4294967295) (< 0 b) (<= b 4294967295)))
(assert (and (<= 0 q) (<= 0 r) (< r b) (= 4294967295 (+ (* q b) r))))
(assert (not (= (<= a q) (<= (* a b) 4294967295))))
(check-sat)
