# LC3 VM

This is a virtual machine that can run assembly programs fr the [lc3](https://en.wikipedia.org/wiki/Little_Computer_3)  computer. The aim is to understand how VMs (like the Java Virtual Machine which runs Java, Kotlin, or Clojure programs) work and build our own simple VM.

## Resourses
* [Write your own virtual machine](https://www.jmeiners.com/lc3-vm/)
* [https://justinmeiners.github.io/lc3-vm/supplies/lc3-isa.pdf](https://justinmeiners.github.io/lc3-vm/supplies/lc3-isa.pdf)
* [LC3-Simulator](https://wchargin.com/lc3web/)

## 16-bit Computer Emulator

This program is a 16-bit computer emulator that simulates the basic functionality of a computer, including:

- **Memory**: the program uses an array of 16-bit integers to simulate memory, with a maximum capacity of `UINT16_MAX`.

- **Registers**: the program uses an array of 16-bit integers to simulate registers. There are `R_COUNT` total registers, including the program counter (`PC`) and the condition register (`COND`).

- **Opcodes**: the program supports a variety of opcodes, such as `ADD`, `LD`, `ST`, `JSR`, `AND`, `LDR`, `STR`, `RTI`, `NOT`, `LDI`, `STI`, `JMP`, `RES`, `LEA`, and `TRAP`.

- **Flags**: the program uses the condition register (`COND`) to store flag information, including the positive (`POS`), zero (`ZRO`), and negative (`NEG`) flags.

- **Traps**: the program supports a variety of traps, such as `GETC`, `OUT`, `PUTS`, `IN`, `PUTSP`, and `HALT`.

- **Keyboard input**: the program uses the keyboard status register (`KBSR`) and the keyboard data register (`KBDR`) to handle keyboard input.

- **Endianness**: the program uses the `swap16()` function to handle endianness conversion.

- **Sign extension**: the program uses the `sign_extend()` function to handle sign extension.
