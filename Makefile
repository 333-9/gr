name = gr
CC = myclang
PREFIX = /usr/local

$(name): $(name).c
	$(CC) -o $@ $<


.PHONY: clean run install

clean:
	rm *.o $(name)

run: $(name)
	./$(name)

install: $(name)
	cp -f $(name) $(PREFIX)/bin/
	cp -f $(name).1 $(PREFIX)/share/man/man1/
