#include "FileIndexer.h"

FileIndexer::FileIndexer(const std::vector<std::shared_ptr<GRAWFile>>& files)
{
    filemap = indexFiles(files);
}

std::multimap<evtid_t, std::shared_ptr<GRAWFile>>
FileIndexer::indexFiles(const std::vector<std::shared_ptr<GRAWFile>>& files)
{
    const int indexDepth = 5;
    std::multimap<evtid_t, std::shared_ptr<GRAWFile>> fmap;

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
        fmap.emplace(minEvtId, file);
    }

    return fmap;
}

std::vector<std::shared_ptr<GRAWFile>>
FileIndexer::findFilesForEvtId(const evtid_t evtid) const
{
    auto upperBd = filemap.upper_bound(evtid);

    std::vector<std::shared_ptr<GRAWFile>> fileList;
    for (auto iter = filemap.begin(); iter != upperBd; iter++) {
        fileList.push_back(iter->second);
    }

    return fileList;
}
