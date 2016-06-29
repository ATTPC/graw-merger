#include "GRAWFile.h"

// --------
// Constructors
// --------

GRAWFile::GRAWFile(const boost::filesystem::path& path, const std::ios::openmode mode)
{
    if (mode & std::ios::in) {
        OpenFileForRead(path);
    }
    else if (mode & std::ios::out) {
        OpenFileForWrite(path);
    }
    else throw Exceptions::File_Open_Failed(path.string());
}

GRAWFile::GRAWFile(const std::string& path, const std::ios::openmode mode)
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

void GRAWFile::OpenFileForRead(const boost::filesystem::path& filePath_in)
{
    DataFile::OpenFileForRead(filePath_in);

    if (filePath.extension() != ".graw") {
        throw Exceptions::Wrong_File_Type(filePath.filename().string());
    }
}

void GRAWFile::OpenFileForRead(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForRead(fp);
}

void GRAWFile::OpenFileForWrite(const boost::filesystem::path& path)
{
    DataFile::OpenFileForWrite(path);
}

void GRAWFile::OpenFileForWrite(const std::string& path)
{
    boost::filesystem::path fp {path};
    OpenFileForWrite(fp);
}

// --------
// Getters for Raw Data and Properties
// --------

uint16_t GRAWFile::GetNextFrameSize()
{
    std::vector<uint8_t> size_raw;
    uint16_t size;

    // Test the input file for validity

    if (!filestream.good()) {
        throw Exceptions::Bad_File(filePath.filename().string());
    }

    std::streamoff storedPos = filestream.tellg();

    uint8_t metaType = static_cast<uint8_t>(filestream.get());

    for (int i = 0; i < 3; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        size_raw.push_back(static_cast<uint8_t>(temp));
    }

    size = Utilities::ExtractByteSwappedInt<decltype(size)>(size_raw.begin(),
                                                            size_raw.end());

    if (size == 0) {
        if (filestream.eof()) {
            isEOF = true;
            throw Exceptions::End_of_File();
        }
        else {
            throw Exceptions::Frame_Read_Error();
        }
    }

    // See if this size is right by checking if we get to the next frame

    filestream.seekg(storedPos + size*GRAWFrame::sizeUnit);

    if (filestream.peek() != metaType) {
        if (filestream.eof()) {
            // This is the last frame in the file.
            isEOF = true;
        }
        else {
            throw Exceptions::Frame_Read_Error();
        }
    }
    filestream.seekg(storedPos); // rewind to start of frame
    return size;
}

std::vector<uint8_t> GRAWFile::ReadRawFrame()
{
    std::vector<uint8_t> frame_raw;
    uint16_t size = GetNextFrameSize();

    for (int i = 0; i < size*GRAWFrame::sizeUnit; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        frame_raw.push_back(static_cast<uint8_t>(temp));
    }

    return frame_raw;
}

GRAWFile::FrameMetadata GRAWFile::ReadFrameMetadata()
{
    std::streamoff startPos = filestream.tellg();
    auto size = GetNextFrameSize();

    filestream.seekg(16, std::ios::cur); // timestamp is 16 bytes from start

    std::vector<uint8_t> timestamp_raw;
    for (int i = 0; i < 6; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        timestamp_raw.push_back(static_cast<uint8_t>(temp));
    }
    ts_t timestamp = Utilities::ExtractByteSwappedInt<ts_t>(timestamp_raw.begin(), timestamp_raw.end());

    std::vector<uint8_t> evtid_raw;
    for (int i = 0; i < 4; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        evtid_raw.push_back(static_cast<uint8_t>(temp));
    }
    evtid_t evtid = Utilities::ExtractByteSwappedInt<evtid_t>(evtid_raw.begin(),
                                                              evtid_raw.end());

    filestream.seekg(startPos + size*GRAWFrame::sizeUnit); // move to start of next frame

    GRAWFile::FrameMetadata meta {};
    meta.filePos = startPos;
    meta.evtId = evtid;
    meta.evtTime = timestamp;

    return meta;
}

template<typename T>
void GRAWFile::AppendBytes(std::vector<uint8_t>& vec, T val, int nBytes)
{
    for (int i = nBytes-1; i >= 0; i--) {
        vec.push_back((val & (0xFF << i*8)) >> i*8);
    }
}

evtid_t GRAWFile::NextFrameEvtId()
{
    std::streamoff storedPos = filestream.tellg();
    filestream.seekg(22, std::ios::cur); // 22 bytes from start of frame to id

    std::vector<uint8_t> id_raw;
    for (int i = 0; i < 4; i++) {
        char temp;
        filestream.read(&temp, sizeof(uint8_t));
        id_raw.push_back(static_cast<uint8_t>(temp));
    }

    if (filestream.eof()) {
        filestream.seekg(storedPos);
        isEOF = true;
        throw Exceptions::End_of_File();
    }

    evtid_t id = Utilities::ExtractByteSwappedInt<evtid_t>(id_raw.begin(),
                                                           id_raw.end());

    filestream.seekg(storedPos);

    return id;
}
