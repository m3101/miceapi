
from distutils.core import setup, Extension
setup(name="mmapi", version="0.0.1",
      ext_modules=[Extension("mmapi", ["wrapper.c"])])