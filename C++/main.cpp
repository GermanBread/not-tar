#include <numbers>
#include <fstream>
#include <string.h>
#include <iostream>

#define BUFFER_SIZE 1000000

const char** reconstructStringArr(int elemCount, int stringLength, const char* string) {
    const char** _indices = new const char*[elemCount];
    _indices[0] = &string[0];
    int _indicesCounter = 1;
    for (size_t i = 0; i < stringLength && _indicesCounter < elemCount; i++) {
        if (string[i] == '\0') {
            _indices[_indicesCounter] = &string[i + 1];
            _indicesCounter++;
        }
    }
    return _indices;
}

int countNullbytes(int length, const char* string) {
    int _output = 0;
    for (size_t i = 0; i < length; i++) {
        if (string[i] == '\0') _output++;
    }
    return _output;
}

int sizeofString(const char* str) {
    int _output = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
        _output += sizeof(str[i]);
    }
    return _output;
}

int sizeofString(int length, const char* str) {
    int _output = 0;
    for (size_t i = 0; i < length; i++) {
        _output += sizeof(str[i]);
    }
    return _output;
}

int sizeofSize(int size, const char* arr[]) {
    int _output = 0;
    for (size_t i = 0; i < size; i++) {
        _output += sizeofString(arr[i]);
    }
    return _output;
}

int concat(int arrsize, const char *files[], const char *archive) {
    std::ofstream _oarchive(archive);

    // Flatten the array + count the NULL-bytes we will be appending to each string
    int _headerSize = sizeofSize(arrsize, files) + arrsize;
    _oarchive.write((char*)&_headerSize, sizeof(int));
    for (size_t i = 0; i < arrsize; i++) {
        _oarchive << files[i] << '\0';
    }

    // Will bite me in the back later
    char* _buffer = new char[BUFFER_SIZE];
    // Now go and put the files in the archive thingy
    for (size_t i = 0; i < arrsize; i++) {
        std::ifstream _src(files[i]);

        // gcount is not adequate here, it returns the amount of CHARACTERS. We need the BYTE-count
        int _fsize = 0;
        for (int len = 0;;) {
            len = _src.readsome(_buffer, BUFFER_SIZE);
            if (len == 0) break;
            _fsize += sizeofString(len, _buffer);
        }        //while (_src >> _buffer) { // Extremely unsafe code!
        //    _fsize += sizeofString(_buffer);
        //}
        _oarchive.write((char*)&_fsize, sizeof(int));
        
        // Seek to the beginning again
        _src.clear();
        _src.seekg(0, std::ios::beg);

        for (int left = _fsize; left > 0; left -= BUFFER_SIZE) {
            std::streamsize _len = _src.readsome(_buffer, std::min(BUFFER_SIZE, left));
            _oarchive.write(_buffer, _len);
        }

        _src.close();
    }

    delete _buffer;
    _oarchive.close();
    return 0;
}

int extract(int arrsize, const char *archive, const char *directory) {
    std::ifstream _iarchive(archive);

    char* _dataBuffer = new char[BUFFER_SIZE];
    int _headerSize = 0;
    _iarchive.readsome((char*)&_headerSize, sizeof(int));
    char* _header = new char[_headerSize];
    _iarchive.readsome(_header, _headerSize);
    int _filesCount = countNullbytes(_headerSize, _header);
    const char** _files = reconstructStringArr(_filesCount, _headerSize, _header);

    for (size_t i = 0; i < _filesCount; i++) {
        int _fileSize = 0;
        const char *_filename = _files[i];

        /// BEGIN DANGER ZONE ///
        const std::string _outPath = std::string(directory).append("/").append(_filename);
        
        // Create the output directory now
        // unsafe and messy; <boost> or <filesystem> is not available in my dev env
        system(std::string("mkdir -p ").append(_outPath).c_str());
        system(std::string("rmdir ").append(_outPath).c_str());
        /// END DANGER ZONE ///

        _iarchive.readsome((char*)&_fileSize, sizeof(int));
        std::ofstream _ofile(_outPath.c_str());

        for (int j = _fileSize; j > 0; j -= BUFFER_SIZE) {
            std::streamsize _len = _iarchive.readsome(_dataBuffer, std::min(j, BUFFER_SIZE));
            _ofile.write(_dataBuffer, _len);
        }

        _ofile.close();
    }

    delete[] _files, _header, _dataBuffer;
    _iarchive.close();
    return 0;
}

int main(int argc, const char *argv[]) {
    if (argc < 4) {
        std::cerr << "Needs more arguments\r\n"
            << "--concat file1 file2 ... fileN archive\r\n"
            << "--extract archive directory" << std::endl;
        return 1;
    }

    if (strcmp(argv[1], "--concat") == 0)
        return concat(argc - 3, argv + 2, argv[argc - 1]);
    else if (strcmp(argv[1], "--extract") == 0)
        return extract(argc - 3, argv[2], argv[3]);
    else {
        std::cerr << "Invalid operation" << std::endl;
        return 1;
    }
}

// I WANT TO GO BACK TO C# AAAHHHHHHHH