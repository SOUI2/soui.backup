SET (MHOOK_SRCS
		disasm-lib/cpu.c
		disasm-lib/disasm.c
		disasm-lib/disasm_x86.c
		disasm-lib/misc.c
		mhook-lib/mhook.cpp
)
ADD_LIBRARY (mhook STATIC ${MHOOK_SRCS})
