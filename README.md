# L3S5 - Network TP 'Sockets'

## Building the Project
To compile the client and server applications, run the following command from the root directory of the project:

```shell
make all
```
This will create two executables:
* build/client
* build/server

## Running the server

To start the server, use the following:
```shell
./build/server <port>
```

* `<port>`: the port number on which the server will listen (`[1, 65535]`).

## Running the client

To run the client, use the following:
```shell
./build/client <address> <port> [raw]
```
* `<address>`: The ip of the srever
* `<port>`: The port number the server is listening on
* `[raw]`: Optional argument. if set to `true`, the client will only display the raw message received.

## Cleaning up

To clean the project (removing the compiled executables), run:
```shell
make clean
```