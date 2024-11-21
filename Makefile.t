my_test : t.o
        gcc -o my_test t.o -L$(ORACLE_HOME)/lib/ -lclntsh `cat $(ORACLE_HOME)/lib/ldflags`   `cat $(ORACLE_HOME)/lib/sysliblist` -ldl -lm

t.o: t.c
        cc -g -Wall -O -I. -I/$(ORACLE_HOME)/precomp/public -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/plsql/public -I$(ORACLE_HOME)/network/public -c t.c

t.c : t.pc
        proc code=ANSI_C iname=t.pc parse=partial iname=t include=. include=$(ORACLE_HOME)/precomp/public include=$(ORACLE_HOME)/rdbms/public include=$(ORACLE_HOME)/rdbms/demo include=$(ORACLE_HOME)/plsql/public include=$(ORACLE_HOME)/network/public

clean:
        rm -f my_test
        rm -f t.o
        rm -f t.c
        rm -f t.lis
        rm -f core

