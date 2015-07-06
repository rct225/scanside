import ctypes

testlib = ctypes.cdll.LoadLibrary('/usr/lib/libadvdaq.so')

fd = ctypes.c_ulong()
fname = ctypes.c_char_p()
port = ctypes.c_ulong()
buffer = ctypes.c_ushort()

port.value = 0
fname.value = "/dev/advdaq0"

ret = testlib.DRV_DeviceOpen(fname, ctypes.byref(fd))

test = input('Enter value: ')
print(test)
buffer.value = test

testlib.DRV_DioWriteDOPorts(fd, port, 1, ctypes.cast(ctypes.byref(buffer), ctypes.c_char_p))

testlib.DRV_DeviceClose(ctypes.byref(fd))
  
