
all: 
	@echo "=================================="
	@echo " Compiling Triangle++   ..."
	@./src/compile.py
	@echo -n " Updating Documentation ..."
	@make --quiet doc/html/index.html
	@echo " Done."
	@echo "=================================="

clean:
	@echo Cleaning up temporary files...
	@rm -rf ./doc/* ./bin/*.o ./bin/*.exe ./bin/del.off \
               ./bin/test* ./lib/* ./src/*.tmp &> /dev/null;

doc/html/index.html: ./include/* ./src/* 
	doxygen &> /dev/null
