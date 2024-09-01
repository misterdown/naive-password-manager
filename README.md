# naive-password-manager
A simple command-line password manager written in C++.

- [Features](#features)
- [Usage](#usage)
- [Build](#build)
- [Run](#run)
- [License](#license)


## Features
- Add new passwords for services.
- Edit existing passwords.
- Retrieve passwords for services.
- Delete passwords for services.
- List all services and passwords.

## Usage
```
Usage: password manager [options]

options:
--help                           display this help message.
-n <service name> <password>     create a new password for the specified service.
-e <service name> <new_password> change a password for the specified service.
-r <service name>                retrieve the password for the specified service.
-d <service name>                delete the password for the specified service.
-l                               list all services and passwords.

examples:
passwordmanager --help
passwordmanager -n example_service example_password
passwordmanager -e example_service new_example_password
passwordmanager -r example_service
passwordmanager -d example_service
passwordmanager -l
```

## Build

### Prerequisites
- C++ compiler (e.g., g++, clang)
  - C++11 or later
- CMake
```sh
mkdir build
cd build
cmake ..
make
```

## Run
```
./passwordmanager [options]
```

## License
This project is licensed under the MIT License - see the LICENSE file for details.
