L:=\
    $(DIR_SRC)/sha/libsha1.a \

O:=\
    $(DIR_SRC)/sha/libsha1.o \

include $(DIR_MK)/lib.mk

B:=\
    $(DIR_SRC)/sha/sha1 \

O:=\
    $(DIR_SRC)/sha/sha1.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/sha/libsha1.a \

include $(DIR_MK)/bin.mk
