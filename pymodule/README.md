# mmapi - Multi-Mouse API
A module for abstracting linux devices and making multiple simultaneous mice/pointer handling easier.

For the purpose of creating the structures that support this abstraction, a program using this module has to have access to the files at `/dev/input` and to Shared Memory manipulation. Please make sure both these requirements are met when running your program (Or just run it with superuser permissions with `sudo`).

This project was built and is maintained by a single person, thus, it hasn't been thoroughly tested (except for very basic use cases), so please report any issues and suggestions to its [GitHub repository][git].

* [Python module Documentation (At GitHub repo Wiki)][docs]

[git]: https://github.com/m3101/mmapi
[docs]: https://github.com/m3101/mmapi/wiki/Python-module-documentation