#ifndef SPEC_INDEXER_H
#define SPEC_INDEXER_H

#include "../client/openai_client.h"
#include <string>
#include <vector>

namespace reviewer {

struct SpecChunk {
    std::string filename;
    std::string section;
    std::string text;
    std::vector<float> embedding;
};

class SpecIndexer {
public:
    SpecIndexer(const std::string& specsDir, client::OpenAIClient& client);
    std::string search(const std::string& query, int topK = 5);

private:
    std::vector<SpecChunk> chunks_;
    client::OpenAIClient& client_;

    void indexDirectory(const std::string& specsDir);
    std::vector<SpecChunk> chunkMarkdownFile(const std::string& content,
                                              const std::string& filename);
    float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
};

} // namespace reviewer

#endif // SPEC_INDEXER_H
