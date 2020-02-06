
from setuptools import setup, Extension

from os import path
from io import open

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(name="miceapi",author="Amélia O. F. da S.",
      author_email="a.mellifluous.one@gmail.com",
      description="An API for managing multiple simultaneous mice input on Linux",
      long_description=long_description,
      long_description_content_type='text/markdown',
      url="https://github.com/m3101/mmapi",
      version="1.0.3.dev1",
      license="MIT",
      classifiers=[
        "Development Status :: 2 - Pre-Alpha",
        "Programming Language :: C",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator",
        "Topic :: Software Development :: User Interfaces"
      ],
      keywords="mouse mice touchpad multiple simultaneous",
      python_requires='>=3',
      py_modules=["miceapi_async"],
      ext_modules=[Extension("miceapi",
      ["wrapper.c","../src/mmapi_main.c","../src/mmapi_events.c"])
      ])