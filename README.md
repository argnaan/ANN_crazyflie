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

Finally clone this repository:
```bash
$ cd ..
$ git clone https://github.com/argnaan/ANN_crazyflie.git
```

## 📁 Repository Structure

The main files in this repository are organized as follows:

- `src/ANN_crazyflie.c`  
  Contains the C implementation of the Out Of Tree Controller, in particular the functions *controllerOutOfTreeInit()*, *controllerOutOfTreeTest()* and *controllerOutOfTree(...)*.

- `get_header.py`  
  Python script used to convert the trained model's weights in pytorch format *best_agent_noise01.pt* into a C-compatible header file (`src/pesi_modello.h`).

- `golden_model.py`  
  Python implementation of the same neural network used as a reference (golden model) to validate the C version's behavior of *ANN_crazyflie.c*.

## Demo Instruction
First of all compile the project:
```bash
$ cd ANN_crazyflie
$ make -j$(nproc)
```

Then, to flash the firmware, connect the Crazyradio USB dongle and, if necessary, grant the appropriate permissions:
```bash
$ make init_dongle
``` 
And flash
```bash
$ make flash_cfloader
```

To check if everything is working fine run the cfclient


## Contribution

## Credits
