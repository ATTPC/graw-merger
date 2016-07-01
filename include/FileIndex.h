#ifndef FILEINDEX_H
#define FILEINDEX_H

#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include "GRAWFile.h"
#include "Constants.h"

class FileIndex
{
public:
    FileIndex() = default;
    FileIndex(const std::vector<std::shared_ptr<GRAWFile>>& files);

    void indexFiles(const std::vector<std::shared_ptr<GRAWFile>>& files);

    std::vector<std::shared_ptr<GRAWFile>> findFilesForEvtId(const evtid_t evtid) const;

private:
    std::multimap<evtid_t, std::shared_ptr<GRAWFile>> filemap;
};


#endif /* end of include guard: FILEINDEX_H */
