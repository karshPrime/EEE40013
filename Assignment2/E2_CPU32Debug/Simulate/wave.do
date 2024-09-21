onerror {resume}
quietly virtual signal -install /cputestbench/theCPU { /cputestbench/theCPU/ir(31 downto 29)} ir_op
quietly virtual signal -install /cputestbench/theCPU { /cputestbench/theCPU/ir(28 downto 26)} ir_aluFunc
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider CPU
add wave -noupdate /cputestbench/theCPU/reset
add wave -noupdate /cputestbench/theCPU/loadPC
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/pc
add wave -noupdate /cputestbench/theCPU/loadIR
add wave -noupdate /cputestbench/theCPU/ir_op
add wave -noupdate /cputestbench/theCPU/ir_aluFunc
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/ir
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/PCSource
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/branchOffset
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/addressBus
add wave -noupdate /cputestbench/theCPU/writeEn
add wave -noupdate /cputestbench/portIO
add wave -noupdate -divider ALU
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/aluOperand1
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/aluOperand2
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/aluDataOut
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/aluOp
add wave -noupdate /cputestbench/theCPU/C
add wave -noupdate /cputestbench/theCPU/V
add wave -noupdate /cputestbench/theCPU/N
add wave -noupdate /cputestbench/theCPU/Z
add wave -noupdate -divider {Data Memory}
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/dataMemAddr
add wave -noupdate /cputestbench/theCPU/dataMemWrite
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/dataMemDataIn
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/dataMemDataOut
add wave -noupdate -divider {Code Memory}
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/codeMemAddr
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/codeMemDataOut
add wave -noupdate -divider Registers
add wave -noupdate /cputestbench/theCPU/RegASource
add wave -noupdate /cputestbench/theCPU/regAWrite
add wave -noupdate -radix unsigned /cputestbench/theCPU/regAAddr
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/regADataIn
add wave -noupdate -radix unsigned /cputestbench/theCPU/regBAddr
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/regBDataOut
add wave -noupdate -radix unsigned /cputestbench/theCPU/regCAddr
add wave -noupdate -radix hexadecimal /cputestbench/theCPU/regCDataOut
add wave -noupdate -group {Reg Subset} -radix hexadecimal /cputestbench/theCPU/registerBank/regBank1(1)
add wave -noupdate -group {Reg Subset} -radix hexadecimal /cputestbench/theCPU/registerBank/regBank1(2)
add wave -noupdate -group {Reg Subset} -radix hexadecimal /cputestbench/theCPU/registerBank/regBank1(3)
add wave -noupdate -group {Reg Subset} -radix hexadecimal /cputestbench/theCPU/registerBank/regBank1(4)
add wave -noupdate -radix hexadecimal -childformat {{/cputestbench/theCPU/registerBank/regBank1(0) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(1) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(2) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(3) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(4) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(5) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(6) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(7) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(8) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(9) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(10) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(11) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(12) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(13) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(14) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(15) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(16) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(17) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(18) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(19) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(20) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(21) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(22) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(23) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(24) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(25) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(26) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(27) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(28) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(29) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(30) -radix hexadecimal} {/cputestbench/theCPU/registerBank/regBank1(31) -radix hexadecimal}} -subitemconfig {/cputestbench/theCPU/registerBank/regBank1(0) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(1) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(2) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(3) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(4) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(5) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(6) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(7) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(8) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(9) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(10) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(11) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(12) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(13) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(14) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(15) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(16) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(17) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(18) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(19) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(20) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(21) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(22) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(23) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(24) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(25) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(26) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(27) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(28) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(29) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(30) {-height 15 -radix hexadecimal} /cputestbench/theCPU/registerBank/regBank1(31) {-height 15 -radix hexadecimal}} /cputestbench/theCPU/registerBank/regBank1
add wave -noupdate -divider Control
add wave -noupdate /cputestbench/theCPU/theController/cpuState
add wave -noupdate /cputestbench/theCPU/theController/nextCpuState
add wave -noupdate -divider {IO Port}
add wave -noupdate /cputestbench/pinDrv
add wave -noupdate /cputestbench/pinOut
add wave -noupdate /cputestbench/pinIn
add wave -noupdate /cputestbench/portIO
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 5} {18671000 ps} 0} {{Cursor 2} {9845453 ps} 0}
quietly wave cursor active 2
configure wave -namecolwidth 150
configure wave -valuecolwidth 167
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ps} {35805 ns}
