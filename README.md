# CSI2P II Mini Project 1

## Introduction

Let's consider a CPU, which has 32 bits registers `r0`-`r255` and a 256 bytes memory.

In this project, you need to implement a binary expression calculator.

## Input

The input will contain several binary expressions consisting of integers, operators, parentheses, and three variables `x`, `y`, and `z`.

The following operators will appear in this project:

- `+`, `-`, `*`, `/`, `%`
- `=`
- `++`, `--` (including prefix and suffix, such as `x++`, `--y`, ... and so on)
- `+`, `-` (expressions such as `+x`, `-y`, ... and so on)
- others such as `>>`, `+=`, are unavailable and will not appear.

## Output

The output is a list of assembly codes. The instruction set architecture are listed in the table below.

If the input expressions contains illegal expression, you should handle it with the error handler. For the details, please refer to **Error Handler** below.

## Instruction Set Architecture

### Memory Operation

| Opcode | Operand1 | Operand2 | Meaning                                                    | Cycles |
| ------ | -------- | -------- | ---------------------------------------------------------- | ------ |
| load   | `reg`    | `[Addr]` | Load data in memory `[Addr]` and save into register `reg`. | 200    |
| store  | `[Addr]` | `reg`    | Store the data of register `reg` into memory `[Addr]`.     | 200    |

### Arithmetic Operation

| Opcode | Operand1 | Operand2 | Operand3 | Meaning                                          | Cycles |
| ------ | -------- | -------- | -------- | ------------------------------------------------ | ------ |
| add    | `rd`     | `rs1`    | `rs2`    | Perform `rs1+rs2` and save the result into `rd`. | 10     |
| sub    | `rd`     | `rs1`    | `rs2`    | Perform `rs1-rs2` and save the result into `rd`. | 10     |
| mul    | `rd`     | `rs1`    | `rs2`    | Perform `rs1*rs2` and save the result into `rd`. | 30     |
| div    | `rd`     | `rs1`    | `rs2`    | Perform `rs1/rs2` and save the result into `rd`. | 50     |
| rem    | `rd`     | `rs1`    | `rs2`    | Perform `rs1%rs2` and save the result into `rd`. | 60     |

- Note that both `rs1` and `rs2` can be a register or a value. However, `rd` must be a valid register.
- All operands should be separated by a space.
- **Important: Using the first 8 registers has no penalty. However, using other registers would double the instruction cycle.**
  - For example, `add r0 r1 r7` cost 10 cycles, while `add r8 r0 r23` cost 20 cycles.

## Variables

- The initial value of variables `x`, `y`, and `z` are stored in memory `[0]`, `[4]`, and `[8]` respectively. Before you use them, you have to load them into registers first.
- After the evaluation of the assembly code, the answer of the variables `x`, `y`, and `z` has to be stored in memory `[0]`, `[4]`, and `[8]` respectively.

## Error handler

If this expression could be compiled by GCC, it's a legal expression. Otherwise it's illegal.

Illegal expressions such as:

- x = 5++
- y = (((7/3)
- z = ++(y++)
- and all expressions that cannot pass GCC compilers should be handled by error handler.

When an error occurs, your final output should contains only `Compile Error!`.

**Note that in our testcases, there won't be any undefined behavior expression.**

- You may check if an expression is undefined behavior by compiling with `-Wall` flag. If it is, there should be some warnings that show the word "undefined".

## Sample

### Sample Input 1

```c
x = z + 5
```

### Sample Output 1

```
load r0 [8]
add r0 r0 5
store [0] r0
```

Total cycle cost: 200(load) + 10(add) + 200(store) = 410 cycles.

### Sample Input 2

```c
x = (y++) + (++z)
z = ++(y++)
```

### Sample Output 2

```
Compile Error!
```

- Note that in sample 2, the first expression is correct, while the second one causes compile error.

## Score

The project includes 2 parts:

1. The **6 basic testcases**, which will be provided by TAs.
2. Contest: There will be **24 testcases** at demo time, each represents 5 points. Besides, the code with **less total clock cycles** is better. The top 10% will **get extra points**.

## Submission / Demo

- Submission Deadline: 11/06(三) 23:59
  - 上傳至iLMS
  - 命名為 **學號.c**
- Demo Time: 11/07(四) 晚上 6:30起
  - 請務必出席，我們會當場用iLMS上上傳的code來驗收，並詢問code內容相關的問題。
  - Demo完成後會在一定時間內公布通過的測資數與cycle數。
  - 嚴禁抄襲。
