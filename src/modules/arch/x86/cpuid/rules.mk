M:=\
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/cpuid/main.o \
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
