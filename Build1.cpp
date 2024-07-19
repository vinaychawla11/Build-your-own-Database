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

using namespace std;

int randomInt() {
    return std::rand();
}

string saveData(const string& path, const vector<char>& data)
{
    string tmp = path + ".tmp." + to_string(randomInt());
    ofstream file(tmp,ios::out | ios::binary );
    if(!file.is_open())
    {
        return "Error opening  file";
    }

     // Write data to temporary file
    file.write(data.data(), data.size());
    if (!file.good()) {
        file.close();
        std::remove(tmp.c_str());
        return "Error writing to temporary file";
    }

    file.flush();
    if (!file.good()) {
        return "Error flushing file";
    }

    int fd = open(tmp.c_str(),O_WRONLY);
    if (fd == -1) {
        std::remove(tmp.c_str());
        return "Error opening file descriptor";
    }

    // Ensure data is flushed to disk
    if (_commit(fd) == -1) {
        close(fd);
        std::remove(tmp.c_str());
        return "Error syncing file to disk";
    }

    // Close the file descriptor
    if (close(fd) == -1) {
        std::remove(tmp.c_str());
        return "Error closing file descriptor";
    }

    // Rename temporary file to target path
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::remove(tmp.c_str());
        return "Error renaming temporary file";
    }

    // Return an empty string if no errors occurred
    return "";
}

string appendData(const string& path, const vector<char>& data)
{
    int fd = open(path.c_str(),O_WRONLY | O_APPEND);

    if(fd==-1)
    {
        close(fd);
        remove(path.c_str());
        return "Failed to open the file";
    }

    if(write(fd,data.data(),data.size()))
    {
        close(fd);
        return "Failed to append";
    }

}