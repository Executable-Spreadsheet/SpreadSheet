


## Grammar



Terminals:
- ID
- FLOAT
- INT
- STRING
- Math ops: + - / * # :
- Non-alphanumeric

Keywords:
- IF
- ELSE
- RETURN
- LET
- Float
- Int
- String
- Cell
- WHILE

type := Int
      | Float
      | String
      | Cell

Note: EPS means null string

header := =
        | = ( )
        | = ( ID : type args )

args := , ID : type args
      | EPS


block := statement block
       | EPS

statement := expr ;
           | if
           | for
           | while
           | ret ;
           | { block }


ret := RETURN expr

if := IF ( expr ) statement
    | IF ( expr ) statement ELSE statement


while := WHILE ( expr ) statement

for := FOR ( ID, expr ) statement


expr := unit
      | unit op expr
      | bucket = expr
      | op expr


Precedence Tiers:
1. \#
2. :
3. \* /
4. \+ \-


bucket := cell
        | ID
        | LET ID : type

cell := \[ expr , expr \]
      | \[ expr , expr , expr \]


> Expression parsing will use the Double E Infix Algorithm
So All we need are precedence table

unit := FLOAT
      | INT
      | STRING
      | ID ( )
      | ID
      | ( expr )
