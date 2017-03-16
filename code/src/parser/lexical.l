INT_DEC [+-]?(([1-9][0-9]*)|0)
INT_HEX [+-]?0x([0-9]|[a-f]|[A-F])+
INT_OCT [+-]?0[0-7]+
INT {INT_DEC}|{INT_HEX}|{INT_OCT}
FLOAT_NOR [+-]?[0-9]+\.[0-9]+
FLOAT_EXP [+-]?(([0-9]+\.[0-9]*)|([0-9]*\.[0-9]+))(E|e)[+-]?[0-9]+
FLOAT {FLOAT_NOR}|{FLOAT_EXP}
ID [_a-zA-Z][_a-zA-Z0-9]*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT \!
typr int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
%%

%%
