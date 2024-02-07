from flask import Flask, request

import ctypes
import ctypes.util
import os,io,sys

# <intercept>
class xpnBypass:
    def __init__(self, args, kwargs, open_before, so_file, xpn_prefix):
        # initial attributes
        self.open_before = open_before
        self.xpn_prefix  = xpn_prefix
        self.is_xpn_file = str(args[0]).startswith(xpn_prefix)
        # load XPN mini-lib
        self.xpn  = ctypes.cdll.LoadLibrary(so_file)
        self.xpn.py_read.restype   = ctypes.POINTER(ctypes.c_char_p)
        self.xpn.py_write.argtypes = (ctypes.c_int, ctypes.c_char_p, ctypes.c_ulong)
        self.libc = ctypes.CDLL(ctypes.util.find_library('c'))
        self.libc.free.argtypes  = (ctypes.c_void_p,)
        # open file
        self.fd = ( self.xpn.py_open(args[0].encode(), args[1].encode()) if self.is_xpn_file else open_before(args[0], args[1]) )
    def write(self, buf):
        if self.is_xpn_file:
             ret = self.xpn.py_write(self.fd, buf, len(buf))
        else:
             ret = self.fd.write(buf)
        return ret
    def read(self):
        if self.is_xpn_file:
             ret  = ""
             _ret = self.xpn.py_read(self.fd)
             if _ret != None:
                ret  = str(ctypes.cast(_ret, ctypes.c_char_p).value)
                self.libc.free(_ret)
        else:
             ret = self.fd.read()
        return ret
    def close(self):
        if self.is_xpn_file:
             ret = self.xpn.py_close(self.fd)
        else:
             ret = self.fd.close()
        return ret

def open_bypass(*args, **kwargs):
    return xpnBypass(args, kwargs, old_open, "/beegfs/home/javier.garciablas/dcamarma/xpn_ep/src/xpn/test/integrity/bypass_python/py_xpn.so", "/tmp/expand/P1")

old_open = open
open = open_bypass
# </intercept>

app = Flask(__name__)

@app.route('/enviar-datos', methods=['POST'])
def recibir_datos():
    datos = request.get_data()
    nombre_archivo = request.headers.get('Nombre-Archivo')

    if not datos or not nombre_archivo:
        return '', 400

    try:
        file = "/tmp/expand/P1/" + nombre_archivo
        f = open(file, "w", encoding="utf-8")
        f.write(datos)
        f.close()

        return f'', 200
    except Exception as e:
        #print(e)
        return f'', 500

if __name__ == '__main__':

    app.run(host='0.0.0.0', port=5000) # El servidor estará escuchando en todas las interfaces en el puerto 5000
