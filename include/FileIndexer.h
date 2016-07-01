#ifndef FILEINDEXER_H
#define FILEINDEXER_H

#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include "GRAWFile.h"
#include "Constants.h"

class FileIndexer
{
public:
    FileIndexer(const std::vector<std::shared_ptr<GRAWFile>>& files);

    static std::multimap<evtid_t, std::shared_ptr<GRAWFile>>
    indexFiles(const std::vector<std::shared_ptr<GRAWFile>>& files);

    std::vector<std::shared_ptr<GRAWFile>> findFilesForEvtId(const evtid_t evtid) const;

private:
    std::multimap<evtid_t, std::shared_ptr<GRAWFile>> filemap;
};


#endif /* end of include guard: FILEINDEXER_H */
