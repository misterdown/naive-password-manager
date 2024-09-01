#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <chrono>
#include <vector>

constexpr size_t MINIMUM_PASSWORD_LENGHT = 8u;

static const char* usage_str() {
    return
        "Usage: password manager [options]\n"
        "\n"
        "Options:\n"
        "\t--help\t\tdisplay this help message.\n"
        "\t-n <service name> <password>\t\tcreate a new password for the specified service.\n"
        "\t-e <service name> <new_password>\t\rchange a password for the specified service.\n"
        "\t-r <service name>\t\tretrieve the password for the specified service.\n"
        "\t-d <service name>\tDelete the password for the specified service.\n"
        "\t-l\t\tlist all services and passwords.\n"
        "\n"
        "examples:\n"
        "\tpasswordmanager --help\n"
        "\tpasswordmanager -n example_service example_password\n"
        "\tpasswordmanager -e example_service new_example_password\n"
        "\tpasswordmanager -r example_service\n"
        "\tpasswordmanager -d example_service\n"
        "\tpasswordmanager -l\n";
}
#if (defined _WIN32)
#   include <windows.h>
static ::std::string get_self_path() {
    char buffer[512]{0};
    const DWORD pathLenght = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    return ::std::string(buffer, buffer + pathLenght);
}
#elif (defined __linux__) // maybe work
#   include <unistd.h>
static ::std::string get_self_path() {
    char buffer[512]{0};
    const ssize_t pathLenght = readlink("/proc/self/exe", buffer, sizeof(buffer));
    return ::std::string(buffer, buffer + pathLenght);
}
#endif // windows or linux

static ::std::stringstream open_as_string_stream(const ::std::string& filePath) {
    ::std::ifstream file(filePath);
    if (!file.is_open()) {
        ::std::cout << "Failed to open file named " << filePath << '\n';
        exit(1);
    }
    ::std::stringstream strStream;
    strStream << file.rdbuf();
    return strStream;
}
static void write_service_passwords(const ::std::string& filePath, const ::std::vector<::std::pair<::std::string, ::std::string>>& vec) {
    ::std::ofstream file(filePath);
    if (!file.is_open()) {
        ::std::cout << "Failed to open file named " << filePath << '\n';
        exit(1);
    }
    for (const auto& i : vec)
        file << i.first << '\n' << i.second << '\n';
};


static ::std::string path_without_file(const ::std::string& path) {
    size_t lastSlash = 0u;
    size_t i = 0u;
    for (auto c : path) {
        if ((c == '\\') || (c == '/'))
            lastSlash = i;
        ++i;
    }
    return path.substr(0ULL, lastSlash + 1ULL);
}
static void print_usage(const ::std::string& filePath, int argc, char** argv) {
    (void)filePath;
    (void)argc;
    (void)argv;
    ::std::cout << usage_str();
}
static ::std::string code_password(const ::std::string& password) {
    ::std::stringstream result;
    size_t base = ::std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
    result << base;
    for (auto c : password)
        result << '#' << (size_t)c * base ;
    return result.str();
}
static ::std::string decode_password(const ::std::string& password) {
    ::std::stringstream passwordStream(password);
    ::std::stringstream result;
    size_t base;
    passwordStream >> base;
    passwordStream.ignore(1); // ignore '#'
    while (!passwordStream.eof()) {
        size_t chCoded;
        passwordStream >> chCoded;
        passwordStream.ignore(1); // ignore '#'
        result << (char)(chCoded / base);
    }
    return result.str();
}

static void edit_password(const ::std::string& filePath, int argc, char** argv) {
    if (argc != 2) {
        ::std::cout << "wrong arguments count.\n" << usage_str();
        exit(1);
    }

    auto fileData = open_as_string_stream(filePath);
    const ::std::string serviceName = *(argv);
    const ::std::string newPassword = *(argv + 1);
    ::std::vector<::std::pair<::std::string, ::std::string>> servicesPasswords;
    bool serviceExist = false;

    if (newPassword.length() < MINIMUM_PASSWORD_LENGHT) {
        ::std::cout << "too short password length\n";
        exit(1);
    }

    while (!fileData.eof()) {
        servicesPasswords.emplace_back();
        ::std::getline(fileData, servicesPasswords.back().first, '\n');
        if (!serviceExist) {
            if (servicesPasswords.back().first == serviceName) { // use map? maybe
                servicesPasswords.back().second = code_password(newPassword);
                fileData.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                serviceExist = true;
                continue;
            }
        }
        ::std::getline(fileData, servicesPasswords.back().second, '\n');
    }
    servicesPasswords.pop_back(); // pop last empty pair.

    if (serviceExist) {
        write_service_passwords(filePath, servicesPasswords);
        ::std::cout << "success\n";
    } else {
        ::std::cout << "there is no password for " << serviceName << " . Use \'-n\' to create password for this services.";
        exit(1); 
    }
}
static void delete_password(const ::std::string& filePath, int argc, char** argv) {
    if (argc != 1) {
        ::std::cout << "Wrong arguments count.\n" << usage_str();
        exit(1);
    }
    ::std::cout << "are you sure?[y/n]";
    char answer;
    ::std::cin >> answer;
    if (answer != 'y') {
        ::std::cout << "the password was not deleted.\n";
        return;
    }

    auto fileData = open_as_string_stream(filePath);
    const ::std::string serviceName = *(argv);
    ::std::vector<::std::pair<::std::string, ::std::string>> servicesPasswords;
    bool serviceExist = false;

    while (!fileData.eof()) {
        servicesPasswords.emplace_back();
        ::std::getline(fileData, servicesPasswords.back().first, '\n');
        if (servicesPasswords.back().first == serviceName) {
            servicesPasswords.pop_back();
            fileData.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip password...
            serviceExist = true;
            continue;
        }
        ::std::getline(fileData, servicesPasswords.back().second, '\n');
    }
    servicesPasswords.pop_back(); // pop last empty pair.

    if (serviceExist) {
        write_service_passwords(filePath, servicesPasswords);
        ::std::cout << "success\n";
    } else {
        ::std::cout << "there is no password for " << serviceName << ". use '-n' to create a password for this service.";
        exit(1);
    }
}

static void new_password(const ::std::string& filePath, int argc, char** argv) {
    const auto checkServiceExist = [](const ::std::string& filePath, const ::std::string& serviceName) {
    ::std::ifstream file(filePath, ::std::ios_base::in);
        if (!file.is_open()) {
            ::std::cout << "falied to open file named " << filePath << ".\n";
            exit(1);
        }
        while (!file.eof()) {
            ::std::string current;
            file >> current;
            if (current == serviceName) {
                ::std::cout << "password for " << serviceName << " already exist\nuse \'-e\' to edit this password."; 
                exit(1);
            }   
            file >> current; // useless. skip word;
        }
    };
    const auto writePassword = [](const ::std::string& filePath, const ::std::string& serviceName,const ::std::string& password) {
        ::std::fstream file(filePath, ::std::ios_base::out |::std::ios_base::app);
        if (!file.is_open()) {
            ::std::cout << "falied to open file named " << filePath << ".\n";
            exit(1);
        }
        file << serviceName << '\n';
        file << password << '\n';
    };
    //----------------------------------------------------------//
    if (argc != 2) {
        ::std::cout << "wrong arguments count.\n" << usage_str();
        exit(1);
    }
    const ::std::string serviceName = *(argv);
    const ::std::string password = code_password(*(argv + 1));
    if (password.length() < MINIMUM_PASSWORD_LENGHT) {
        ::std::cout << "too short password length\n";
        exit(1);
    }
    checkServiceExist(filePath, serviceName);
    writePassword(filePath, serviceName, password);
    ::std::cout << "success\n";
}

static void retrieve_password(const ::std::string& filePath, int argc, char** argv) {
    if (argc != 1) {
        ::std::cout << "wrong arguments count.\n" << usage_str();
        exit(1);
    }
    const ::std::string serviceName = *(argv);

    ::std::ifstream file(filePath);
    if (!file.is_open()) {
        ::std::cout << "falied to open file named " << filePath << ".\n";
        exit(1);
    }
    while (!file.eof()) {
        ::std::string current;
        file >> current;
        if (current == serviceName) {
            file >> current;
            ::std::cout << decode_password(current) << '\n'; 
            ::std::cout << "success\n";
            return;
        }   
        file >> current; // useless. skip word;
    }

    ::std::cout << "service " << serviceName << " not exist.\n";
}

static void list_passwords(const ::std::string& filePath, int argc, char** argv) {
    (void)argc;
    (void)argv;

    ::std::ifstream file(filePath);
    if (!file.is_open()) {
        ::std::cout << "Failed to open file named " << filePath << ".\n";
        exit(1);
    }

    while (!file.eof()) {
        ::std::string serviceName;
        ::std::string password;
        file >> serviceName;
        file >> password;
        if (!serviceName.empty() && !password.empty())
            ::std::cout << "service: " << serviceName << ", password: " << decode_password(password) << '\n';
    }
}

static ::std::unordered_map<::std::string, void(*)(const ::std::string&, int, char**)> commandProccsers {
        {"--help", print_usage},
        {"-n", new_password},
        {"-e", edit_password},
        {"-r", retrieve_password},
        {"-d", delete_password},
        {"-l", list_passwords},
};

int main(int argc, char** argv) {
    if (argc <= 1) {
        ::std::cout << "too few argument.\n" << usage_str();
        exit(1);
    }
    --argc; // skip first argument
    ++argv;

    const ::std::string storingFilePath = path_without_file(get_self_path()) + ::std::string("passwords.really");
    const ::std::string commandString = *(argv); // command argument.
    const auto func = commandProccsers.find(commandString);
    if (func == commandProccsers.end()) {
        ::std::cout << "unknown command.\n" << usage_str();
        exit(1);
    } else {
        --argc; 
        ++argv; // argv must starts at first command argument.
        (func->second)(storingFilePath, argc, argv);
    }
}