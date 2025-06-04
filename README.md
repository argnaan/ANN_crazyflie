# ANN_crazyflie
Implementation of ANN trained for hovering task inside the ﬂight controller. 

This project is the final project of the course *Cyber-Physical Systems Programming*. 

## Installation Requirements

First of all clone our fork of the [crazyflie firmware](https://github.com/argnaan/crazyflie-firmware) repository
```bash
$ git clone --recursive https://github.com/argnaan/crazyflie-firmware
$ cd crazyflie-firmware
$ git submodule init
$ git submodule update
```

Configure the firmware specifing the platorm, for example in case of Crazyflie 2.1+ :
```bash
$ make cf2_defconfig
```
Follow [these instructions](https://www.bitcraze.io/documentation/repository/crazyflie-firmware/master/building-and-flashing/build/) for more options.

Build the firmware:
```bash
$ make -j$(nproc)
```

Finally clone this repo:
```bash
$ cd ..
$ git clone https://github.com/argnaan/ANN_crazyflie.git
```

```bash
```

```bash
```


## Repository structure

## Demo Instruction

## Contribution

## Credits
