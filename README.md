# Diligence

Diligence is a shim library that instruments additional TLS verification during an OpenSSL TLS handshake.
It intercepts a client-server handshake to perform certifiate validation via Google's Certificate Transparency
framework and OCSP. It also enforces the usage of TLSv1.3 cipher suites. Diligence is implemented using OpenSSL v1.1.1.

To compile `diligence.c`:

`$ gcc -Wall -O2 -fpic -shared -ldl -o diligence.so diligence.c -lssl -lcrypto`

To run an application with Diligence enabled:

`$ LD_PRELOAD=<PATH TO diligence.so> <run program>`

To run all applications with Diligence enabled, include the following line in `.bash_profile` or `.bashrc`:

`LD_PRELOAD=<PATH TO diligence.so>`
