SECTIONS { .llvm_stackmaps : { __sp_maps = .; KEEP(*(.llvm_stackmaps)) __sp_maps_end = .; } } INSERT AFTER .rodata;
