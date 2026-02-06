# Compiling C Programs for NIOS II on the Command Line

This guide details how to invoke the NIOS II compiler (`nios2-elf-gcc`) directly from the Windows command line or PowerShell, bypassing the Intel/Altera Monitor Program GUI.

## Prerequisites

Ensure you have the Intel FPGA (Quartus) software installed. Based on your system, the relevant paths are:
*   **Root Directory**: `c:\intelFPGA_lite\18.1`
*   **Nios II EDS Directory**: `c:\intelFPGA_lite\18.1\nios2eds`
*   **University Program Directory**: `c:\intelFPGA_lite\18.1\University_Program`

## Step 1: Set up the Environment

You need to add the Nios II compiler binaries to your system PATH or use the full path to the executable.

**Option A: Quick Run (PowerShell)**
You can run the compiler using its full path:
```powershell
& "c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin\nios2-elf-gcc.exe" --version
```

**Option B: Temporary PATH Update (PowerShell)**
Add the compiler to your PATH for the current session:
```powershell
$env:PATH += ";c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin"
```

**Option C: Using the Nios II Command Shell**
The installation provides a script to set up a Unix-like environment. You can locate "Nios II Command Shell" in your Windows Start Menu, or run:
```powershell
c:\intelFPGA_lite\18.1\nios2eds\Nios II Command Shell.bat
```

## Step 2: The Compilation Command

A standard compilation requires:
1.  **C Source Files**: Your `.c` code.
2.  **Linker Script/BSP**: Instructions on where to place code in memory.

### Method 1: Bare Metal (No BSP, Manual Linker Script)
If you want to mimic the Monitor Program's "No System/BSP" mode or simple assembly tests, you can use the linker script provided by the Monitor Program, but you **MUST** instruct it where your memory is.

*   `nasys_program_mem`: Address where code starts.
*   `nasys_data_mem`: Address where data starts (usually same as code for small RAMs).

**Example Command:**
Assuming your program starts at address `0x00000000` (Change this to match your DE1-SoC/Board memory map!):

```powershell
$GCC = "c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin\nios2-elf-gcc.exe"
$LINKER_SCRIPT = "c:\intelFPGA_lite\18.1\University_Program\Monitor_Program\build\nios_as_build.ld"

& $GCC -T $LINKER_SCRIPT `
    -Wl,--defsym,nasys_program_mem=0x00000000 `
    -Wl,--defsym,nasys_data_mem=0x00000000 `
    -o my_program.elf `
    my_program.c
```

### Method 2: With a BSP (Standard Nios II Flow)
If you generated a BSP using Quartus/Platform Designer (formerly Qsys), you should use the `Makefile` generated there or include the `system.h` and libraries.

```powershell
# Assuming you have a generated BSP in ./bsp
& $GCC -o my_program.elf my_program.c -I./bsp -L./bsp -T./bsp/linker.x
```

## Step 3: Verifying the ELF
You can verify the output file using `nios2-elf-objdump` (found in the same bin directory):

```powershell
& "c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin\nios2-elf-objdump.exe" -d my_program.elf | Select-Object -First 20
```
