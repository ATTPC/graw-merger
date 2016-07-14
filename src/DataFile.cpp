#include "DataFile.h"

DataFile::DataFile(const boost::filesystem::path& path, const std::ios::openmode mode)
{
    if (mode & std::ios::in) {
        OpenFileForRead(path);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(path);
    }
    else throw Exceptions::File_Open_Failed(path.string());
}

DataFile::DataFile(const std::string& path, const std::ios::openmode mode)
{
    boost::filesystem::path fp {path};

    if (mode & std::ios::in) {
        OpenFileForRead(fp);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(fp);
    }
    else throw Exceptions::File_Open_Failed(fp.string());
}

void DataFile::OpenFileForWrite(const boost::filesystem::path& path)
{
    namespace fs = boost::filesystem;

    if (isInitialized) throw Exceptions::Already_Init();

    filePath = path;

    filestream.open(path.string(), std::ios::out|std::ios::trunc|std::ios::binary);

    if (filestream.good()) {
        isInitialized = true;
    }
    else {
        throw Exceptions::File_Open_Failed(path.string());
    }
}

void DataFile::OpenFileForWrite(const std::string& path)
{
    namespace fs = boost::filesystem;

    fs::path fp {path};

    OpenFileForWrite(fp);
}

void DataFile::OpenFileForRead(const boost::filesystem::path& path)
{
    namespace fs = boost::filesystem;

    if (isInitialized) throw Exceptions::Already_Init();

    filePath = path;

    if (!fs::exists(path)) {
        throw Exceptions::Does_Not_Exist(path.string());
    }
    if (!fs::is_regular_file(path)) {
        throw Exceptions::Wrong_File_Type(path.string());
    }

    filestream.open(path.string(), std::ios::in|std::ios::binary);

    if (filestream.good()) {
        isInitialized = true;
    }
    else {
        throw Exceptions::Bad_File(path.string());
    }
}

void DataFile::OpenFileForRead(const std::string& path)
{
    namespace fs = boost::filesystem;

    fs::path fp {path};

    OpenFileForRead(fp);
}

void DataFile::CloseFile()
{
    if (filestream.is_open() and this->isInitialized) filestream.close();
}

std::streamoff DataFile::GetPosition()
{
    if (!isInitialized) throw Exceptions::Not_Init();
    return filestream.tellg();
}

const std::string DataFile::GetFilename() const
{
    return filePath.filename().string();
}

bool DataFile::eof() const
{
    if (!isInitialized) throw Exceptions::Not_Init();
    return (filestream.eof() or isEOF);
}

bool DataFile::is_open() const
{
    return filestream.is_open();
}
