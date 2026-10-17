// Package main demonstrates descriptor size and typed-nil interface semantics.
package main

import (
    "fmt"
    "unsafe"
)

// Fault is an example concrete error type with a pointer receiver.
type Fault struct{}

// Error implements error, including a receiver that may be nil.
func (*Fault) Error() string { return "example fault" }

func main() {
    var ptr *Fault
    var err error = ptr
    var values []byte
    fmt.Printf("pointer is nil: %t\n", ptr == nil)
    fmt.Printf("interface is nil: %t\n", err == nil)
    fmt.Printf("slice descriptor bytes: %d\n", unsafe.Sizeof(values))
    backing := []int{10, 20, 30}
    view := backing[:2]
    view[0] = 99
    if backing[0] != 99 || err == nil {
        panic("representation check failed")
    }
    fmt.Printf("write through slice changes backing value: %d\n", backing[0])
}
