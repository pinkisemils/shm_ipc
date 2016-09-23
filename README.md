# IPC with shared memory

## Building
I've only ever built this on `gcc (GCC) 6.2.1 20160830`, but this does not depend on anything fancy.
```bash
make all
```

## Running it
After building it, execute the writer and the reader in that particular order in separate terminals.

```bash
./writer
Start up the reader
```

```bash
./reader
Enter your message in the writer
```
