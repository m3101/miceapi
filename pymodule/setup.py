
from distutils.core import setup, Extension
setup(name="mmapi",author="Amélia O. F. da S.",
      author_email="a.mellifluous.one@gmail.com",
      version="0.0.1",
      license="MIT",
      py_modules=["mmapi_async","tester"],
      ext_modules=[Extension("mmapi",
      ["wrapper.c","../src/mmapi_main.c","../src/mmapi_events.c"])
      ])