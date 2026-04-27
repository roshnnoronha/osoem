#ifndef READLINE_WRAPPER_H
#define READLINE_WRAPPER_H

#include <string>
#include <functional>
#include <vector>
#include "completion_provider.h"

namespace readline_wrapper {

// Initialize readline with completion provider
void initialize(CompletionProvider* provider);

// Read a line with the given prompt
// Returns the input string, or empty string on EOF
std::string readLine(const std::string& prompt);

// Read a line with the given prompt, using completionFn to generate tab completions.
// completionFn receives the current prefix and returns a list of completion candidates.
std::string readLineWithCompletion(
    const std::string& prompt,
    std::function<std::vector<std::string>(const std::string&)> completionFn);

// Cleanup readline resources
void cleanup();

// Add line to history
void addHistory(const std::string& line);

} // namespace readline_wrapper

#endif // READLINE_WRAPPER_H
