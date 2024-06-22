#include <iostream>
#include <string>
#include <filesystem>

#include <array>

const std::string PROMPT = "$ ";
const std::string SBI[] = {"exit", "echo", "type", "pwd"};
const char* PATH = getenv("PATH");
const char* HOME = getenv("HOME");
const int MAX_PATHS = 64;
//this could be better i think
std::string PATHS[MAX_PATHS];

int exit(std::string input);
void echo(std::string input);
std::string type(std::string input);
void parsePaths(std::string PATHS[MAX_PATHS]);
std::string findCommand(std::string command);
void cd(std::string input);

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, void(*)(FILE*)> pipe(popen(cmd, "r"),
    [](FILE * f) -> void
    {
        std::ignore = pclose(f);
    });
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
  bool validInput = false;
  std::string input, command;
  int count = 0;

  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  parsePaths(PATHS);

  while (true) {
    std::cout << "$ ";
    std::getline(std::cin, input);

    command = input.substr(0, input.find(" "));
    if (command == "exit") {
      return exit(input);
    } else if (command == "echo") {
      echo(input);
      validInput = true;
    } else if (command == "type") {
      std::cout << type(input) << std::endl;
      validInput = true;
    } else if (command == "pwd") {
      std::cout << std::filesystem::current_path().string() << std::endl;
      validInput = true;
    } else if (command == "cd") {
      cd(input);
      validInput = true;
    } else {
      if (findCommand(command) != command + ": not found") {
        std::string result = exec(input.c_str());
        std::cout << result;
        validInput = true;
      }
    }

    if (!validInput) {
      std::cout << input << ": command not found" << std::endl;
    }
    validInput = false;
  }
  return 0;
}

std::string type(std::string input) {
  std::string arg = input.substr(input.find(" ") + 1);

  if (input.find(" ") == std::string::npos) {
    return "type: missing operand";
  }
  for (int i = 0; i < sizeof(SBI) / sizeof(SBI[0]); i++) {
    if (arg == SBI[i]) {
      return SBI[i] + " is a shell builtin";
    }
  }
  return findCommand(arg);

  return arg + ": not found";
}

void echo(std::string input) {
  std::cout << input.substr(input.find(" ") + 1) << std::endl;
}

int exit(std::string input) {
  std::string exitCode = input.substr(input.find(" ") + 1);

  if (exitCode == "exit") {
    std::cout << "exit: missing operand" << std::endl;
    return 0;
  }

  return stoi(exitCode);
}

void parsePaths(std::string PATHS[MAX_PATHS]) {
  int count = 0;
  for (int i = 0; PATH[i] != '\0'; i++) {
    if (PATH[i] == ':') {
      count++;
    } else {
      PATHS[count] += PATH[i];
    }
  } 
}

void cd(std::string input) {
  std::error_code ec;
  std::string destination = input.substr(input.find(" ") + 1);

  if (destination == "~") {
    destination = HOME;
  }
  std::filesystem::current_path(destination, ec);
  if (ec.value() != 0) {
    std::cout << "cd: " << destination << ": " << ec.message() << std::endl;
  }
}

std::string findCommand(std::string command) {
  for (int i = 0; i < MAX_PATHS; i++) {
    if (PATHS[i] == "") {
      break;
    }
    for (const auto &entry : std::filesystem::directory_iterator(PATHS[i])) {
      std::string fileName = entry.path().filename().string();
      if (fileName == command) {
        return command + " is " + entry.path().string();
      }
    }
  }
  return command + ": not found";
}
