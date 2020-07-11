def _start_libm():
    global TO_ZERO, TOWARD_MINUS_INF, TOWARD_PLUS_INF, TO_NEAREST
    global set_rounding, get_rounding
    from ctypes import cdll
    from ctypes.util import find_library
    libm = cdll.LoadLibrary(find_library('m'))
    set_rounding, get_rounding = libm.fesetround, libm.fegetround
    # x86
    TO_ZERO = 0xc00
    TOWARD_MINUS_INF = 0x400
    TOWARD_PLUS_INF = 0x800
    TO_NEAREST = 0
 
def _start_msvcrt():
    global TO_ZERO, TOWARD_MINUS_INF, TOWARD_PLUS_INF, TO_NEAREST
    global set_rounding, get_rounding
    from ctypes import cdll
    msvcrt = cdll.msvcrt
    set_rounding = lambda mode: msvcrt._controlfp(mode, 0x300)
    get_rounding = lambda: msvcrt._controlfp(0, 0)
    TO_ZERO = 0x300
    TOWARD_MINUS_INF = 0x100
    TOWARD_PLUS_INF = 0x200
    TO_NEAREST = 0
 
for _start_rounding in _start_libm, _start_msvcrt:
    try:
        _start_rounding()
        break
    except:
        pass
else:
    print "ERROR: You couldn't start the FPU module"