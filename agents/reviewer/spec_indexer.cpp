#include "spec_indexer.h"
#include "../utils/utils.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace reviewer {

SpecIndexer::SpecIndexer(const std::string& specsDir, client::OpenAIClient& client)
    : client_(client) {
    indexDirectory(specsDir);
}

void SpecIndexer::indexDirectory(const std::string& specsDir) {
    fs::path dir(specsDir);
    if (!fs::exists(dir) || !fs::is_directory(dir))
        throw std::runtime_error("SpecIndexer: specs directory not found: " + specsDir);

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".md") continue;
        std::ifstream file(entry.path());
        if (!file.is_open()) continue;
        std::ostringstream ss;
        ss << file.rdbuf();
        auto newChunks = chunkMarkdownFile(ss.str(), entry.path().filename().string());
        for (auto& c : newChunks)
            chunks_.push_back(std::move(c));
    }

    size_t total = chunks_.size();
    if (total == 0) {
        std::cerr << "SpecIndexer: no chunks found in " << specsDir << "\n";
        return;
    }

    for (size_t i = 0; i < total; ++i) {
        std::cerr << "Indexing specs... " << (i + 1) << "/" << total << " chunks\r";
        std::cerr.flush();
        chunks_[i].embedding = client_.embed(chunks_[i].text);
    }
    std::cerr << "Indexing specs... done (" << total << " chunks indexed)        \n";
}

std::vector<SpecChunk> SpecIndexer::chunkMarkdownFile(const std::string& content,
                                                       const std::string& filename) {
    const size_t MAX_CHARS = 1500;

    struct Section { std::string heading; std::string text; };
    std::vector<Section> sections;
    Section cur;
    cur.heading = "(Preamble)";

    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line[0] == '#') {
            if (!utils::trim(cur.text).empty())
                sections.push_back(cur);
            size_t p = line.find_first_not_of('#');
            cur.heading = (p != std::string::npos) ? utils::trim(line.substr(p)) : line;
            cur.text = line + "\n";
        } else {
            cur.text += line + "\n";
        }
    }
    if (!utils::trim(cur.text).empty())
        sections.push_back(cur);

    std::vector<SpecChunk> result;
    for (const auto& sec : sections) {
        if (sec.text.size() <= MAX_CHARS) {
            result.push_back({filename, sec.heading, sec.text, {}});
            continue;
        }

        std::string accumulated;
        size_t partN = 0;
        size_t pos = 0;
        const std::string& t = sec.text;

        auto flushAccumulated = [&]() {
            if (accumulated.empty()) return;
            std::string label = (partN == 0)
                ? sec.heading
                : sec.heading + " (part " + std::to_string(partN) + ")";
            result.push_back({filename, label, accumulated, {}});
            accumulated.clear();
            ++partN;
        };

        while (pos < t.size()) {
            size_t next = t.find("\n\n", pos);
            std::string para;
            if (next == std::string::npos) {
                para = t.substr(pos);
                pos = t.size();
            } else {
                para = t.substr(pos, next - pos + 2);
                pos = next + 2;
            }
            if (accumulated.size() + para.size() > MAX_CHARS && !accumulated.empty())
                flushAccumulated();
            accumulated += para;
        }
        flushAccumulated();
    }
    return result;
}

float SpecIndexer::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    float dot   = std::inner_product(a.begin(), a.end(), b.begin(), 0.0f);
    float normA = std::sqrt(std::inner_product(a.begin(), a.end(), a.begin(), 0.0f));
    float normB = std::sqrt(std::inner_product(b.begin(), b.end(), b.begin(), 0.0f));
    if (normA == 0.0f || normB == 0.0f) return 0.0f;
    return dot / (normA * normB);
}

std::string SpecIndexer::search(const std::string& query, int topK) {
    if (chunks_.empty())
        return "No specification chunks are indexed. The specs directory may be empty.";

    auto queryEmb = client_.embed(query);

    std::vector<std::pair<float, size_t>> scores;
    scores.reserve(chunks_.size());
    for (size_t i = 0; i < chunks_.size(); ++i)
        scores.emplace_back(cosineSimilarity(queryEmb, chunks_[i].embedding), i);

    std::sort(scores.begin(), scores.end(),
              [](const auto& x, const auto& y) { return x.first > y.first; });

    int k = std::min(topK, static_cast<int>(scores.size()));
    std::ostringstream ss;
    ss << "## Relevant Specification Sections\nQuery: " << query << "\n\n";
    for (int i = 0; i < k; ++i) {
        const auto& chunk = chunks_[scores[i].second];
        ss << "---\n";
        ss << "**File:** " << chunk.filename
           << " | **Section:** " << chunk.section << "\n\n";
        ss << chunk.text << "\n";
    }
    return ss.str();
}

} // namespace reviewer
