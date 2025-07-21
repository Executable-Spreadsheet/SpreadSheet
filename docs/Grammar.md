


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


bucket = expression

expression      := summation expression2
expression2     := TOKEN_CHAR_EQUALS summation expression2
                 | “”
summation       := summation summation2
summation2      := TOKEN_CHAR_PLUS term summation2
                 | TOKEN_CHAR_MINUS term summation2
                 | “”
term            := term term2
term2           := TOKEN_CHAR_ASTERISK reference term2
                 | TOKEN_CHAR_SLASH reference term2
                 | “”
reference       := absolute reference2
reference2      := TOKEN_CHAR_COLON absolute reference2
                 | “”
absolute        := unit absolute2
absolute2       := TOKEN_CHAR_OCTOTHORPE unit absolute2
                 | “”

sum := term
      | term TOKEN_CHAR_PLUS expression
      | term TOKEN_CHAR_MINUS expression

term := reference
      | reference TOKEN_CHAR_ASTERISK term;
      | reference TOKEN_CHAR_SLASH term;

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


> Expression parsing will be done with recursive descent.

unit := FLOAT
      | INT
      | STRING
      | ID ( )
      | ID
      | ( expr )
