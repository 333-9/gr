name = gr
CC = myclang

$(name): $(name).c
	$(CC) -o $@ $<


.PHONY: clean run install

clean:
	rm *.o $(name)

run: $(name)
	./$(name)

install: $(name)
	cp -f $(name) /usr/local/bin/
