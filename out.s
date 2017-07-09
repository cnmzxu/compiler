.data
_promopt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
li $v0, 4
la $a0, _promopt
syscall
li $v0, 5
syscall
jr $ra

write:
li $v0, 1
syscall
li $v0, 4
la $a0, _ret
syscall
move $v0, $0
jr $ra

fact:
lw $8, 16($fp)
li $9, 1
bne $8, $9, label1
lw $2, 16($fp)
jr $ra
label1:
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
lw $8, 16($fp)
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
lw $8, 16($fp)
li $9, 1
neg $9, $9
add $4, $8, $9
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
sw $4, 0($sp)
addi $sp, $sp, -4
sw $ra, 0($sp)
addi $sp, $sp, -4
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
sw $fp, 0($sp)
move $fp, $sp
addi $sp, $sp, -4
jal fact
move $sp, $fp
lw $fp, 0($sp)
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
addi $sp, $sp, 4
lw $ra, 0($sp)
move $9, $v0
addi $sp, $sp, 4
mul $2, $8, $9
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
jr $ra
label2:

main:
move $fp, $sp
addi $sp, $sp, -4
addi $sp, $sp, -4
sw $ra, 0($sp)
addi $sp, $sp, -4
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
sw $fp, 0($sp)
move $fp, $sp
addi $sp, $sp, -4
jal read
move $sp, $fp
lw $fp, 0($sp)
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
addi $sp, $sp, 4
lw $ra, 0($sp)
sw $v0, -4($fp)
lw $8, -4($fp)
li $9, 1
ble $8, $9, label3
lw $4, -4($fp)
sw $4, 0($sp)
addi $sp, $sp, -4
sw $ra, 0($sp)
addi $sp, $sp, -4
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
sw $fp, 0($sp)
move $fp, $sp
addi $sp, $sp, -4
jal fact
move $sp, $fp
lw $fp, 0($sp)
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
addi $sp, $sp, 4
lw $ra, 0($sp)
sw $v0, -8($fp)
addi $sp, $sp, 4
j label4
label3:
sw $8, 0($sp)
addi $sp, $sp, -4
li $8, 1
sw $8, -8($fp)
addi $sp, $sp, 4
lw $8, 0($sp)
label4:
lw $4, -8($fp)
sw $4, 0($sp)
addi $sp, $sp, -4
sw $ra, 0($sp)
addi $sp, $sp, -4
sw $8, 0($sp)
addi $sp, $sp, -4
sw $9, 0($sp)
addi $sp, $sp, -4
sw $fp, 0($sp)
move $fp, $sp
addi $sp, $sp, -4
jal write
move $sp, $fp
lw $fp, 0($sp)
addi $sp, $sp, 4
lw $9, 0($sp)
addi $sp, $sp, 4
lw $8, 0($sp)
addi $sp, $sp, 4
lw $ra, 0($sp)
addi $sp, $sp, 4
li $v0, 0
jr $ra

