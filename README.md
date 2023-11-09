# LibSPM
**SovietLinux's Official Package Management Library**
[![CodeFactor](https://www.codefactor.io/repository/github/soviet-linux/libspm/badge)](https://www.codefactor.io/repository/github/soviet-linux/libspm)
## Building
Instructions on building

### Library
```bash
make all
make formats
sudo make install
```
### spm-test
```bash
make all
make formats
sudo make install
make test
```

### Memory checking tool

In order to run the program with the memcheck tool from the cutils library run :
```bash
make MEMCHECK=1 all
sudo make install
```
