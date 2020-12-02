APP=gen-expr

$(APP): gen-expr.c
	gcc -O0 -g3 -Wall -Werror -o $@ $<

.PHONY: clean
clean:
	-rm $(APP) 2> /dev/null
