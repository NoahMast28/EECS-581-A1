#include <iostream>
#include <string>
#include <cctype>

// Helper: validates one maximal candidate token and extracts address/port.
// Returns true on success; on success sets outAddr and outPort.
// On failure, values are not guaranteed to be meaningful; caller ignores them.
static bool validateCandidate(const std::string& token,
                              unsigned long& outAddr,
                              int& outPort) {
    const size_t n = token.size();
    size_t pos = 0;
    unsigned int octets[4] = {0, 0, 0, 0};
    int port = -1;

    for (int k = 0; k < 4; ++k) {
        if (pos >= n) return false;
        if (!std::isdigit(static_cast<unsigned char>(token[pos]))) return false;

        unsigned int value = 0;
        int digits = 0;
        char firstChar = token[pos];

        while (pos < n && std::isdigit(static_cast<unsigned char>(token[pos]))) {
            if (digits >= 3) return false; // more than 3 digits
            value = value * 10 + (token[pos] - '0');
            ++digits;
            ++pos;
            if (value > 255) return false;
        }

        if (digits == 0) return false;
        if (digits > 1 && firstChar == '0') return false; // leading zero rule

        octets[k] = value;

        if (k < 3) {
            // Expect a period after the first three octets.
            if (pos >= n || token[pos] != '.') return false;
            ++pos; // skip '.'
            if (pos >= n) return false;
            if (!std::isdigit(static_cast<unsigned char>(token[pos]))) return false;
        } else {
            // After the fourth octet, either end or a valid :port.
            if (pos == n) {
                port = -1;
            } else if (token[pos] == ':') {
                ++pos; // skip ':'
                if (pos >= n) return false;
                if (!std::isdigit(static_cast<unsigned char>(token[pos]))) return false;

                unsigned int pvalue = 0;
                int pdigits = 0;
                char pfirstChar = token[pos];

                while (pos < n && std::isdigit(static_cast<unsigned char>(token[pos]))) {
                    if (pdigits >= 5) return false; // more than 5 digits
                    pvalue = pvalue * 10 + (token[pos] - '0');
                    ++pdigits;
                    ++pos;
                    if (pvalue > 65535) return false;
                }

                if (pdigits == 0) return false;
                if (pdigits > 1 && pfirstChar == '0') return false; // leading zero rule
                if (pvalue > 65535) return false;
                if (pos != n) return false; // extra characters after port

                port = static_cast<int>(pvalue);
            } else {
                return false; // stray period, colon, or other token char
            }
        }
    }

    // Compute the 32-bit decimal value.
    unsigned long addr = (static_cast<unsigned long>(octets[0]) << 24) |
                         (static_cast<unsigned long>(octets[1]) << 16) |
                         (static_cast<unsigned long>(octets[2]) << 8) |
                         (static_cast<unsigned long>(octets[3]));

    outAddr = addr;
    outPort = port;
    return true;
}

// Required function prototype.
// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0;
    outPort = -1;

    const size_t n = str.size();
    size_t i = 0;

    auto isTokenChar = [](char c) {
        return std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == ':';
    };

    while (i < n) {
        // Skip garbage characters.
        while (i < n && !isTokenChar(str[i])) {
            ++i;
        }
        if (i >= n) break;

        // Collect a maximal run of token characters.
        size_t start = i;
        while (i < n && isTokenChar(str[i])) {
            ++i;
        }

        std::string candidate = str.substr(start, i - start);

        unsigned long addr = 0;
        int port = -1;
        if (validateCandidate(candidate, addr, port)) {
            outAddress = addr;
            outPort = port;
            return true;
        }
        // If invalid, continue scanning after this run.
    }

    return false;
}

int main() {
    std::string line;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, line)) {
            break; // input stream ended
        }

        if (line == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        unsigned long outAddress = 0;
        int outPort = -1;

        if (extractIPv4(line, outAddress, outPort)) {
            unsigned int a = (outAddress >> 24) & 0xFF;
            unsigned int b = (outAddress >> 16) & 0xFF;
            unsigned int c = (outAddress >> 8) & 0xFF;
            unsigned int d = outAddress & 0xFF;

            std::cout << "Extracted IPv4 address: "
                      << a << "." << b << "." << c << "." << d
                      << " (decimal value: " << outAddress << ", port: ";

            if (outPort == -1) {
                std::cout << "none";
            } else {
                std::cout << outPort;
            }

            std::cout << ")" << std::endl;
        } else {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0;
}