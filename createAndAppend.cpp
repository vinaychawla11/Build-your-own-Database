#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>     // For std::remove and std::rename
#include <cstdlib>    // For std::rand and std::srand
#include <ctime>      // For std::time
#include <fcntl.h>    // For open
#include <unistd.h>   // For fsync and close
#include <sys/types.h>
#include <sys/stat.h>

std::ofstream LogCreate(const std::string& path) {
    std::ofstream file(path, std::ios::out | std::ios::app);
    return file;
}

void LogAppend(std::ofstream& file, const std::string& line) {
    file << line << std::endl;
}