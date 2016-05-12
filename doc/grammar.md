#FPTOOL 
## Grammar Definition Version 1.0

program &rarr; statement-list _EOF_  

statement-list &rarr; statement statement-list  
statement-list &rarr; _EMPTY_    

statement &rarr; definition  
statement &rarr; assignment  
statement &rarr; NEWLINE

definition &rarr; DEFINE IDENT '=' defspec ';'    

defspec &rarr; INPUT '(' INTEGER, INTEGER ')'  
defspec &rarr; CSD '(' FLOAT, INTEGER ')'  

assignment &rarr; IDENT '=' expr ';'

expr &rarr; term + expr  
expr &rarr; term - expr  
expr &rarr; term  

term &rarr; '-' factor  
term &rarr; factor * term  
term &rarr; factor >> INTEGER  
term &rarr; factor << INTEGER  
term &rarr; factor >>> INTEGER  
term &rarr; factor <<< INTEGER    
term &rarr; factor
  
factor &rarr; '(' expr ')'  
factor &rarr; INTEGER  
factor &rarr; FLOAT  
factor &rarr; IDENT  
