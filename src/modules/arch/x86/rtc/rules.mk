M:=\
    $(DIR_SRC)/modules/arch/x86/rtc/rtc.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/rtc/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
