$GCC = "c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin\nios2-elf-gcc.exe"
$OC = "c:\intelFPGA_lite\18.1\nios2eds\bin\gnu\H-x86_64-mingw32\bin\nios2-elf-objcopy.exe"
$LINKER_SCRIPT = "c:\intelFPGA_lite\18.1\University_Program\Monitor_Program\build\nios_cpp_build.ld"
$HAL_INC = "c:\intelFPGA_lite\18.1\nios2eds\components\altera_nios2\HAL\inc"

# Flags exactly matching Monitor Program (Array for PowerShell Splatting)
$CC_FLAGS = @(
    "-Wall", "-c", "-g", "-O1", 
    "-ffunction-sections", "-fverbose-asm", "-fno-inline", "-mno-cache-volatile",
    "-mno-hw-mul", "-mno-hw-div",
    "-DSYSTEM_BUS_WIDTH=32", "-DALT_SINGLE_THREADED", "-D_JTAG_UART_BASE=4280291328u"
)

$INC_FLAGS = @("-I", "include", "-I", "generated", "-I", $HAL_INC)

Write-Host "Starting Nios II Build (Exact Monitor Program Replication)..."

# 1. Compile main.c
Write-Host "Compiling main.c..."
& $GCC $CC_FLAGS $INC_FLAGS "src/main.c" -o "src/main.o"
if ($LASTEXITCODE -ne 0) { Write-Error "Failed to compile main.c"; exit 1 }

# 2. Compile platform.c
Write-Host "Compiling platform.c..."
& $GCC $CC_FLAGS $INC_FLAGS "src/platform.c" -o "src/platform.o"
if ($LASTEXITCODE -ne 0) { Write-Error "Failed to compile platform.c"; exit 1 }

# 3. Compile renderer.c
Write-Host "Compiling renderer.c..."
& $GCC $CC_FLAGS $INC_FLAGS "src/renderer.c" -o "src/renderer.o"
if ($LASTEXITCODE -ne 0) { Write-Error "Failed to compile renderer.c"; exit 1 }

# 4. Compile niosII_jtag_uart.c
Write-Host "Compiling niosII_jtag_uart.c..."
& $GCC $CC_FLAGS $INC_FLAGS "src/niosII_jtag_uart.c" -o "src/niosII_jtag_uart.o"
if ($LASTEXITCODE -ne 0) { Write-Error "Failed to compile niosII_jtag_uart.c"; exit 1 }

# 5. Link
Write-Host "Linking..."
& $GCC "-Wl,--defsym" "-Wl,nasys_stack_top=0x4000000" `
    "-Wl,--defsym" "-Wl,nasys_program_mem=0x0" `
    "-Wl,--defsym" "-Wl,nasys_data_mem=0x0" `
    "-Wl,--script=$LINKER_SCRIPT" `
    "src/main.o" "src/platform.o" "src/renderer.o" "src/niosII_jtag_uart.o" `
    -o "main.elf"
if ($LASTEXITCODE -ne 0) { Write-Error "Linking failed"; exit 1 }

# 6. Objcopy (SREC)
Write-Host "Generating SREC..."
& $OC -O srec "main.elf" "main.srec"

Write-Host "Build Complete!" -ForegroundColor Green
