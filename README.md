# SIC/XE Assembler Simulator [Group 9]

## Group Members

````text
1) Nikhil Pabbisetty [ ID: 014336907, EMAIL: nikhil.pabbisetty@sjsu.edu ] **TEAM LEADER**
2) Akhil Cherukuri [ ID: 014525420, EMAIL: akhil.cherukuri@sjsu.edu ]
3) Jesus De Haro De Reza [ ID: 011501880, jesus.deharodereza@sjsu.edu ]
4) Ganesh Ram Pamadi [ ID: 013729586, ganeshram.pamadi@sjsu.edu ]
````
## Program Files
````text
Assembly File Name: input_assembly_file.txt
- This file contains the sample program that will be used generate the object code
Instruction File Name: instructions.txt
- This file is used to hold all the instructions available for assembly
Register File Name: registers.txt
- This file contains the various register that are to be loaded for performing the assembly
Assembler Directive File Name: assembler_directives.txt
- This file contains the assembler directives for psuedo-assembly
Object Code Filename: object_code.txt
- This is the final objet code output file
Other Sample Input Filenames:
1. input_assembly_file_with_errors_1_invalid_num_of_args
- This file contains a sample input file that has instruction line with an invalid number of arguments
2. input_assembly_file_with_errors_2_invalid_opcode
- This file contains a sample input file that has instruction line with an invalid opcode
3. input_assembly_file_with_errors_3_invalid_assembler_dir
- This file contains a sample input file that has instruction line with an invalid assembler directive
````

## Run Program

```text
Windows/Linux: g++ main_code.cpp
               ./a.out or ./a.exe

MacOS: clang++ -std=c++11 main_code.cpp
       ./a.out
--------------------------------------------------------
Assembly File Name: input_assembly_file.txt
Instruction File Name: instructions.txt
Register File Name: registers.txt
Assembler Directive File Name: assembler_directives.txt
Object Code Filename: object_code.txt
```

## Project Features

````text
1) CLI Interface
2) Encryption and Decryption of Object Code
3) Error Handling with Error Log
````

## Input Assembly File

````text
COPY START 0
FIRST STL RETADR
  LDB #LENGTH
  BASE LENGTH
CLOOP +JSUB RDREC
  LDA LENGTH
  COMP #0
  JEQ ENDFIL
  +JSUB WRREC
  J CLOOP
ENDFIL LDA EOF
  STA BUFFER
  LDA #3
  STA LENGTH
  +JSUB WRREC
  J @RETADR
EOF BYTE C'EOF'
RETADR RESW 1
LENGTH RESW 1
BUFFER RESB 4096
RDREC CLEAR X
  CLEAR A
  CLEAR S
  +LDT #4096
RLOOP TD INPUT
  JEQ RLOOP
  RD INPUT
  COMPR A,S
  JEQ EXIT
  STCH BUFFER,X
  TIXR T
  JLT RLOOP
EXIT STX LENGTH
  RSUB
INPUT BYTE X'F1'
WRREC CLEAR X
  LDT LENGTH
WLOOP TD OUTPUT
  JEQ WLOOP
  LDCH BUFFER,X
  WD OUTPUT
  TIXR T
  JLT WLOOP
  RSUB
OUTPUT BYTE X'05'
  END FIRST
````

## CLI Output

````text
------------OPTAB-------------
ADD 3 18
ADDF 3 58
ADDR 2 90
AND 3 40
CLEAR 2 B4
COMP 3 28
COMPF 3 88
COMPR 2 A0
DIV 3 24
DIVF 3 64
DIVR 2 9C
FIX 1 C4
FLOAT 1 C0
HIO 1 F4
J 3 3C
JEQ 3 30
JGT 3 34
JLT 3 38
JSUB 3 48
LDA 3 00
LDB 3 68
LDCH 3 50
LDF 3 70
LDL 3 08
LDS 3 6C
LDT 3 74
LDX 3 04
LPS 3 D0
MUL 3 20
MULF 3 60
MULR 2 98
NORM 1 C8
OR 3 44
RD 3 D8
RMO 2 AC
RSUB 3 4C
SHIFTL 2 A4
SHIFTR 2 A8
SIO 1 F0
SSK 3 EC
STA 3 0C
STB 3 78
STCH 3 54
STF 3 80
STI 3 D4
STL 3 14
STS 3 7C
STSW 3 E8
STT 3 84
STX 3 10
SUB 3 1C
SUBF 3 5C
SUBR 2 94
SVC 2 B0
TD 3 E0
TIO 1 F8
TIX 3 2C
TIXR 2 B8
WD 3 DC
------------REGTAB-------------
A 0
X 1
L 2
PC 8
SW 9
B 3
S 4
T 5
F 6
------------ADTAB-------------
START 0
BASE 1
BYTE 2
RESW 3
RESB 4
WORD 5
END 6
------------SYMTAB-------------
COPY  0
FIRST  0
CLOOP  6
ENDFIL  1A
EOF  2D
RETADR  30
LENGTH  33
BUFFER  36
RDREC  1036
RLOOP  1040
EXIT  1056
INPUT  105C
WRREC  105D
WLOOP  1062
OUTPUT  1076
-------------------------------
0 COPY START 0 0
0 FIRST STL RETADR 17202D
3 _ LDB #LENGTH 69202D
6 _ BASE LENGTH 0
6 CLOOP +JSUB RDREC 4B101036
A _ LDA LENGTH 32026
D _ COMP #0 290000
10 _ JEQ ENDFIL 332007
13 _ +JSUB WRREC 4B10105D
17 _ J CLOOP 3F2FEC
1A ENDFIL LDA EOF 32010
1D _ STA BUFFER F2016
20 _ LDA #3 10003
23 _ STA LENGTH F200D
26 _ +JSUB WRREC 4B10105D
2A _ J @RETADR 3E2003
2D EOF BYTE C'EOF' 454F46
30 RETADR RESW 1 0
33 LENGTH RESW 1 0
36 BUFFER RESB 4096 0
1036 RDREC CLEAR X B410
1038 _ CLEAR A B400
103A _ CLEAR S B440
103C _ +LDT #4096 75101000
1040 RLOOP TD INPUT E32019
1043 _ JEQ RLOOP 332FFA
1046 _ RD INPUT DB2013
1049 _ COMPR A,S A004
104B _ JEQ EXIT 332008
104E _ STCH BUFFER,X 57C003
1051 _ TIXR T B850
1053 _ JLT RLOOP 3B2FEA
1056 EXIT STX LENGTH 134000
1059 _ RSUB  4F0000
105C INPUT BYTE X'F1' F1
105D WRREC CLEAR X B410
105F _ LDT LENGTH 774000
1062 WLOOP TD OUTPUT E32011
1065 _ JEQ WLOOP 332FFA
1068 _ LDCH BUFFER,X 53C003
106B _ WD OUTPUT DF2008
106E _ TIXR T B850
1070 _ JLT WLOOP 3B2FEF
1073 _ RSUB  4F0000
1076 OUTPUT BYTE X'05' 5
1077 _ END FIRST 0
````

## Decrypted Output Object Code

````text
H^COPY^000000^001077
T^000000^1D^17202D^69202D^4B101036^032026^290000^332007^4B10105D^3F2FEC^032010
T^00001D^1D^0F2016^010003^0F200D^4B10105D^3E2003^454F46^B410^B400^B440^75101000
T^001040^1D^E32019^332FFA^DB2013^A004^332008^57C003^B850^3B2FEA^134000^4F0000^F1
T^00105D^1A^B410^774000^E32011^332FFA^53C003^DF2008^B850^3B2FEF^4F0000^05^
M^000007^05
M^000014^05
M^000027^05
E^000000
````

## Encrypted Output Object Code

````text
K^FRSB^333333^334300
W^333333^4G^40535G^92535G^7E434369^365359^523333^665330^7E43438G^6I5IHF^365343
W^33334G^4G^3I5349^343336^3I533G^7E43438G^6H5336^787I79^E743^E733^E773^08434333
W^334373^4G^H65342^665IID^GE5346^D337^665331^80F336^E183^6E5IHD^467333^7I3333^I4
W^33438G^4D^E743^007333^H65344^665IID^86F336^GI5331^E183^6E5IHI^7I3333^38^
P^333330^38
P^333347^38
P^333350^38
H^333333
````
