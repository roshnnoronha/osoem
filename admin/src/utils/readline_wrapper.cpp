#include "readline_wrapper.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <cstring>
#include <vector>

namespace readline_wrapper {

// Global pointer to the completion provider
static CompletionProvider* g_completionProvider = nullptr;

// Store current matches for the generator
static std::vector<std::string> g_currentMatches;
static size_t g_matchIndex = 0;

// Transient completion function pointer (set by readLineWithCompletion)
static std::function<std::vector<std::string>(const std::string&)>* g_transientCompletionFn = nullptr;

// Forward declarations
static char* commandCompletionGenerator(const char* text, int state);
static char* argumentCompletionGenerator(const char* text, int state);
static char* transientGenerator(const char* text, int state);
static char** completionFunction(const char* text, int start, int end);
static char** transientCompletionFunction(const char* text, int start, int end);

void initialize(CompletionProvider* provider) {
    g_completionProvider = provider;

    // Set readline completion function
    rl_attempted_completion_function = completionFunction;

    // Disable default filename completion
    rl_completion_append_character = ' ';

    // Use our custom word break characters (space and tab, but not quotes)
    rl_completer_word_break_characters = const_cast<char*>(" \t");

    // Allow quotes in completions
    rl_completer_quote_characters = const_cast<char*>("\"'");
}

std::string readLine(const std::string& prompt) {
    char* line = readline(prompt.c_str());

    if (line == nullptr) {
        // EOF received
        return "";
    }

    std::string result(line);
    free(line);

    return result;
}

void addHistory(const std::string& line) {
    if (!line.empty()) {
        add_history(line.c_str());
    }
}

std::string readLineWithCompletion(
    const std::string& prompt,
    std::function<std::vector<std::string>(const std::string&)> completionFn) {

    // Save readline state
    auto savedCompletionFn = rl_attempted_completion_function;
    auto savedWordBreak = rl_completer_word_break_characters;

    // Install transient completion (no space word-break so full input is the prefix)
    g_transientCompletionFn = &completionFn;
    rl_attempted_completion_function = transientCompletionFunction;
    rl_completer_word_break_characters = const_cast<char*>("\t");

    char* line = readline(prompt.c_str());

    // Restore readline state
    rl_attempted_completion_function = savedCompletionFn;
    rl_completer_word_break_characters = savedWordBreak;
    g_transientCompletionFn = nullptr;

    if (line == nullptr) return "";
    std::string result(line);
    free(line);
    return result;
}

void cleanup() {
    // Clear history
    clear_history();
    g_completionProvider = nullptr;
}

// Check if we're completing a command (first word) or an argument
static bool isCompletingCommand(int start) {
    // If start is 0, we're at the beginning - completing a command
    if (start == 0) {
        return true;
    }

    // Check if all characters before start are whitespace
    const char* buf = rl_line_buffer;
    for (int i = 0; i < start; i++) {
        if (buf[i] != ' ' && buf[i] != '\t') {
            return false;
        }
    }
    return true;
}

// Get the command from the current line
static std::string getCommand() {
    const char* buf = rl_line_buffer;
    std::string cmd;

    // Skip leading whitespace
    size_t i = 0;
    while (buf[i] == ' ' || buf[i] == '\t') {
        i++;
    }

    // Extract command (first word)
    while (buf[i] != '\0' && buf[i] != ' ' && buf[i] != '\t') {
        cmd += buf[i];
        i++;
    }

    return cmd;
}

// Generator for command completions
static char* commandCompletionGenerator(const char* text, int state) {
    if (g_completionProvider == nullptr) {
        return nullptr;
    }

    if (state == 0) {
        g_currentMatches = g_completionProvider->getCommandCompletions(text);
        g_matchIndex = 0;
    }

    if (g_matchIndex < g_currentMatches.size()) {
        char* match = strdup(g_currentMatches[g_matchIndex].c_str());
        g_matchIndex++;
        return match;
    }

    return nullptr;
}

// Generator for argument completions (paths, entity names)
static char* argumentCompletionGenerator(const char* text, int state) {
    if (g_completionProvider == nullptr) {
        return nullptr;
    }

    if (state == 0) {
        // Remove quotes from the text if present
        std::string cleanText = text;
        if (!cleanText.empty() && (cleanText[0] == '"' || cleanText[0] == '\'')) {
            cleanText = cleanText.substr(1);
        }

        g_currentMatches = g_completionProvider->getPathCompletions(cleanText);
        g_matchIndex = 0;
    }

    if (g_matchIndex < g_currentMatches.size()) {
        char* match = strdup(g_currentMatches[g_matchIndex].c_str());
        g_matchIndex++;
        return match;
    }

    return nullptr;
}

// Count complete argument tokens before position start in the readline buffer.
// Returns 1 when only the command token has been seen (user is typing first arg),
// 2+ when first arg is complete (user is typing second arg or beyond).
static int getArgPosition(int start) {
    const char* buf = rl_line_buffer;
    int count = 0;
    bool inWord = false, inQuotes = false;
    char quoteChar = '\0';
    for (int i = 0; i < start; i++) {
        char c = buf[i];
        if (inQuotes) {
            if (c == quoteChar) { inQuotes = false; inWord = false; }
        } else if (c == '"' || c == '\'') {
            inQuotes = true; quoteChar = c;
            if (!inWord) { count++; inWord = true; }
        } else if (c == ' ' || c == '\t') {
            inWord = false;
        } else if (!inWord) {
            count++; inWord = true;
        }
    }
    return count;
}

// Generator for transient (prompt-level) completions
static char* transientGenerator(const char* text, int state) {
    if (state == 0) {
        if (g_transientCompletionFn == nullptr) {
            g_currentMatches.clear();
            g_matchIndex = 0;
            return nullptr;
        }
        std::string prefix = text;
        // Strip leading quote if present
        if (!prefix.empty() && (prefix[0] == '"' || prefix[0] == '\'')) {
            prefix = prefix.substr(1);
        }
        g_currentMatches = (*g_transientCompletionFn)(prefix);
        g_matchIndex = 0;
    }

    if (g_matchIndex < g_currentMatches.size()) {
        char* match = strdup(g_currentMatches[g_matchIndex].c_str());
        g_matchIndex++;
        return match;
    }
    return nullptr;
}

// Completion function used during readLineWithCompletion
static char** transientCompletionFunction(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, transientGenerator);
}

// Main completion function
static char** completionFunction(const char* text, int start, int end) {
    // Disable default filename completion
    rl_attempted_completion_over = 1;

    if (isCompletingCommand(start)) {
        return rl_completion_matches(text, commandCompletionGenerator);
    }

    // Check if this is a navigation command that needs path completion
    std::string cmd = getCommand();
    if (cmd == "cd" || cmd == "sl" || cmd == "select") {
        rl_completion_append_character = '\0'; // '/' is already embedded in navigable completions
        return rl_completion_matches(text, argumentCompletionGenerator);
    }

    if (cmd == "rm" || cmd == "remove") {
        rl_completion_append_character = ' ';
        return rl_completion_matches(text, argumentCompletionGenerator);
    }

    // set/st: complete first argument (item name) only
    if (cmd == "st" || cmd == "set") {
        rl_completion_append_character = ' ';
        if (getArgPosition(start) == 1) {
            return rl_completion_matches(text, argumentCompletionGenerator);
        }
        return nullptr;
    }

    // import/export: complete filesystem paths
    if (cmd == "im" || cmd == "import" || cmd == "ex" || cmd == "export") {
        return rl_completion_matches(text, rl_filename_completion_function);
    }

    // For other commands, no completion
    return nullptr;
}

} // namespace readline_wrapper
