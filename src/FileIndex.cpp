#include "FileIndex.h"

FileIndex::FileIndex(const std::vector<std::shared_ptr<GRAWFile>>& files)
{
    indexFiles(files);
}

void FileIndex::indexFiles(const std::vector<std::shared_ptr<GRAWFile>>& files)
{
    const int indexDepth = 5*4;  // 4 AsAds => 4 frames, and read 5 events

    for (const auto& file : files) {
        // Read metadata for first `indexDepth` events
        file->Rewind();
        std::vector<GRAWFile::FrameMetadata> metas;
        for (int i = 0; i < indexDepth; i++) {
            metas.push_back(file->ReadFrameMetadata());
        }

        // Find minimum
        auto comp = [] (const GRAWFile::FrameMetadata& a, const GRAWFile::FrameMetadata& b) {
            return a.evtId < b.evtId;
        };
        auto minElem = std::min_element(metas.begin(), metas.end(), comp);
        evtid_t minEvtId = minElem->evtId;

        // Insert into map
        filemap.emplace(minEvtId, file);

        // Rewind again to leave file read pointer at the beginning
        file->Rewind();
    }
}

std::vector<std::shared_ptr<GRAWFile>>
FileIndex::findFilesForEvtId(const evtid_t evtid) const
{
    auto upperBd = filemap.upper_bound(evtid);

    std::vector<std::shared_ptr<GRAWFile>> fileList;
    for (auto iter = filemap.begin(); iter != upperBd; iter++) {
        fileList.push_back(iter->second);
    }

    return fileList;
}
