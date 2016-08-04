BIN_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot \

OBJ_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot.o \

DEP_MANDELBROT:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_MANDELBROT): $(OBJ_MANDELBROT) $(DEP_MANDELBROT)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_MANDELBROT)
OBJ:=$(OBJ) $(OBJ_MANDELBROT)
