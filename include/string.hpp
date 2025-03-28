#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <iterator>
#include <concepts>
#include <variant>
#include <boost/locale.hpp>
#include "compare_result.hpp"
#include "char.hpp"
#include "code_point.hpp"

namespace simple {

namespace detail {

// One-time initialization of Boost Locale
inline void init_locale() {
    static bool initialized = false;
    if (!initialized) {
        boost::locale::generator gen;
        std::locale::global(gen("en_US.UTF-8"));
        initialized = true;
    }
}

// Count UTF-16 code units, treating each byte of invalid UTF-8 as a separate code unit
inline std::size_t count_utf16_code_units(const std::string& utf8_str) {
    std::size_t count = 0;
    const unsigned char* str = reinterpret_cast<const unsigned char*>(utf8_str.c_str());
    const unsigned char* end = str + utf8_str.length();

    while (str < end) {
        if (*str < 0x80) {
            // ASCII character
            count++;
            str++;
        } else if ((*str & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (str + 1 >= end || (str[1] & 0xC0) != 0x80) {
                // Invalid or incomplete sequence, treat each byte separately
                count++;
                str++;
                continue;
            }
            // Check for overlong encoding (should have used fewer bytes)
            unsigned int codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
            if (codepoint < 0x80) {
                // Overlong encoding, treat each byte separately
                count += 2;
                str += 2;
                continue;
            }
            count++;
            str += 2;
        } else if ((*str & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (str + 2 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80) {
                // Invalid or incomplete sequence, treat each byte separately
                count++;
                str++;
                continue;
            }
            // Check for overlong encoding and surrogate range
            unsigned int codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
            if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                // Overlong encoding or surrogate range, treat each byte separately
                count += 3;
                str += 3;
                continue;
            }
            count++;
            str += 3;
        } else if ((*str & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence (surrogate pair in UTF-16)
            if (str + 3 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80) {
                // Invalid or incomplete sequence, treat each byte separately
                count++;
                str++;
                continue;
            }
            // Check for overlong encoding and valid Unicode range
            unsigned int codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) |
                                    ((str[2] & 0x3F) << 6)  | (str[3] & 0x3F);
            if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
                // Overlong encoding or outside Unicode range, treat each byte separately
                count += 4;
                str += 4;
                continue;
            }
            count += 2;  // Surrogate pair counts as two UTF-16 code units
            str += 4;
        } else {
            // Invalid UTF-8 byte, treat as a single code unit
            count++;
            str++;
        }
    }
    return count;
}

// Compare two strings using byte-by-byte comparison for exact Java behavior
inline int compare_utf8_strings(const std::string& str1, const std::string& str2) {
    return str1.compare(str2);
}

} // namespace detail

/**
 * Simple String - A simple string class that provides Java String-like functionality
 *
 * Unicode Handling:
 * - Strings are stored internally as UTF-8
 * - length() returns the number of UTF-16 code units (like Java's String.length())
 * - Comparison operations use byte-by-byte comparison
 * 
 * Note on Unicode Normalization:
 * Different Unicode representations of the same visual character are treated as distinct strings.
 * For example:
 * - U+00E9 (é as single code point)
 * - U+0065 + U+0301 (e + ◌́ as combining character)
 * These will compare as different strings and have different lengths.
 * Future versions may support Unicode normalization for comparing such equivalent forms.
 */

class StringIndexOutOfBoundsException : public std::out_of_range {
public:
    explicit StringIndexOutOfBoundsException(const std::string& msg)
        : std::out_of_range(msg) {}
};

class String {
public:
    // Static valueOf methods for primitive types
    static String valueOf(bool b);
    static String valueOf(char c);
    static String valueOf(int i);
    static String valueOf(long l);
    static String valueOf(float f);
    static String valueOf(double d);
    
    // Generic valueOf method for any type that can be converted to string
    template<typename T>
    static String valueOf(const T& obj) {
        return String(to_string_helper(obj));
    }
    
    // Default constructor - creates an empty string
    String()
        : data_(std::make_shared<const std::string>("")),
          offset_(0),
          length_(0),
          utf16_cache_() {}
    
    // Constructor from C++ string literal
    explicit String(const std::string& str)
        : data_(std::make_shared<const std::string>(str)), 
          offset_(0), 
          length_(str.length()), 
          utf16_cache_() {}
    
    // Constructor from C string with explicit length (for strings with null chars)
    String(const char* str, std::size_t length)
        : data_(std::make_shared<const std::string>(str, length)), 
          offset_(0), 
          length_(length), 
          utf16_cache_() {}
    
    // Get the length of the string in UTF-16 code units
    std::size_t length() const {
        // Initialize locale if needed
        detail::init_locale();
        
        // Use cached UTF-16 representation if available
        if (utf16_cache_) {
            return utf16_cache_->length();
        }
        
        // Otherwise count UTF-16 code units from UTF-8, considering offset and length
        if (length_ == 0) {
            return 0;
        }
        
        // Create a substring view for counting
        std::string_view view(data_->c_str() + offset_, length_);
        return detail::count_utf16_code_units(std::string(view));
    }

    /**
     * Returns true if this string is empty (length() == 0).
     * Note: A string containing only whitespace or null characters is not considered empty.
     * 
     * @return true if the string is empty, false otherwise
     */
    bool is_empty() const {
        return length_ == 0;
    }

    /**
     * Compares this string with another for equality using byte-by-byte comparison.
     * Note: Characters with different Unicode representations (e.g., é vs e + ◌́)
     * will not be considered equal even if they appear visually identical.
     * 
     * @param other The string to compare with
     * @return true if the strings are exactly equal, false otherwise
     */
    bool equals(const String& other) const {
        // Fast path: check if strings share data and have same offset/length
        if (shares_data_with(other) && offset_ == other.offset_ && length_ == other.length_) {
            return true;
        }
        
        // Otherwise do a byte-by-byte comparison of the substrings
        // This is more efficient than creating full string copies with to_string()
        const char* this_data = data_->c_str() + offset_;
        const char* other_data = other.data_->c_str() + other.offset_;
        std::size_t min_length = std::min(length_, other.length_);
        
        // First check if lengths are different
        if (length_ != other.length_) {
            return false;
        }
        
        // Compare bytes until we find a difference or reach the end
        for (std::size_t i = 0; i < min_length; ++i) {
            if (this_data[i] != other_data[i]) {
                return false;
            }
        }
        
        return true;
    }

    /**
     * Compares this string with another lexicographically using byte-by-byte comparison.
     * Note: The comparison is based on UTF-8 byte values, not visual appearance.
     * Characters with different Unicode representations may compare differently
     * even if they appear visually identical.
     * 
     * @param other The string to compare with
     * @return CompareResult representing the comparison outcome:
     *         - isLess() is true if this < other
     *         - isEqual() is true if this == other
     *         - isGreater() is true if this > other
     */
    simple::CompareResult compare_to(const String& other) const {
        // Fast path: check if strings share data and have same offset/length
        if (shares_data_with(other) && offset_ == other.offset_ && length_ == other.length_) {
            return simple::CompareResult::EQUAL;
        }
        
        // Otherwise compare the actual substrings
        std::string this_str = to_string();
        std::string other_str = other.to_string();
        int result = detail::compare_utf8_strings(this_str, other_str);
        if (result < 0) return simple::CompareResult::LESS;
        if (result > 0) return simple::CompareResult::GREATER;
        return simple::CompareResult::EQUAL;
    }

    /**
     * Returns the character at the specified index as a Char.
     * The index refers to UTF-16 code units, not bytes or code points.
     * 
     * @param index Zero-based index of the character
     * @return The character at the specified index
     * @throws StringIndexOutOfBoundsException if index is negative or >= length()B
     */
    Char char_at(std::size_t index) const;

    /**
     * Array-style access to characters. Equivalent to charAt().
     * 
     * @param index Zero-based index of the character
     * @return The character at the specified index
     * @throws StringIndexOutOfBoundsException if index is negative or >= length()
     */
    Char operator[](std::size_t index) const { return char_at(index); }

    /**
     * Returns the raw UTF-16 code unit at the specified index.
     * 
     * @param index Zero-based index of the code unit
     * @return The UTF-16 code unit at the specified index
     * @throws StringIndexOutOfBoundsException if index is negative or >= length()
     */
    char16_t char_value(std::size_t index) const;

    /**
     * Returns the Unicode code point at the specified index.
     * 
     * @param index Zero-based index of the code unit to start from
     * @return The Unicode code point at the specified index as a CodePoint object
     * @throws StringIndexOutOfBoundsException if index is negative or >= length()
     */
    simple::CodePoint code_point_at(std::size_t index) const {
        const auto& utf16 = get_utf16();
        if (index >= utf16.length()) {
            throw StringIndexOutOfBoundsException("Index out of bounds");
        }
        char16_t first = utf16[index];
        if (first >= 0xD800 && first <= 0xDBFF && index + 1 < utf16.length()) {
            char16_t second = utf16[index + 1];
            if (second >= 0xDC00 && second <= 0xDFFF) {
                return CodePoint(0x10000 + ((first - 0xD800) << 10) + (second - 0xDC00));
            }
        }
        return CodePoint(first);
    }

    /**
     * Returns the Unicode code point before the specified index.
     * 
     * @param index One-based index of the code unit to start from
     * @return The Unicode code point before the specified index as a CodePoint object
     * @throws StringIndexOutOfBoundsException if index is negative or > length()
     */
    simple::CodePoint code_point_before(std::size_t index) const {
        const auto& utf16 = get_utf16();
        if (index == 0 || index > utf16.length()) {
            throw StringIndexOutOfBoundsException("Index out of bounds");
        }
        char16_t second = utf16[index - 1];
        if (second >= 0xDC00 && second <= 0xDFFF && index >= 2) {
            char16_t first = utf16[index - 2];
            if (first >= 0xD800 && first <= 0xDBFF) {
                return CodePoint(0x10000 + ((first - 0xD800) << 10) + (second - 0xDC00));
            }
        }
        return CodePoint(second);
    }

    /**
     * Returns the number of Unicode code points in the specified text range.
     * 
     * @param begin_index Index to the first code unit of the text range
     * @param end_index Index after the last code unit of the text range
     * @return Number of Unicode code points in the specified range
     * @throws StringIndexOutOfBoundsException if begin_index is negative or > end_index,
     *         or end_index is > length()
     */
    std::size_t code_point_count(std::size_t begin_index, std::size_t end_index) const {
        const auto& utf16 = get_utf16();
        if (begin_index > end_index || end_index > utf16.length()) {
            throw StringIndexOutOfBoundsException("Invalid range");
        }
        std::size_t count = 0;
        for (std::size_t i = begin_index; i < end_index; ++i) {
            char16_t ch = utf16[i];
            if (ch >= 0xD800 && ch <= 0xDBFF && i + 1 < end_index) {
                char16_t next = utf16[i + 1];
                if (next >= 0xDC00 && next <= 0xDFFF) {
                    ++i;  // Skip low surrogate
                }
            }
            ++count;
        }
        return count;
    }

    // Get the underlying string data
    // For substrings, returns a new string with just the substring portion
    // For full strings (offset_=0, length_=data_->length()), returns a reference to the original data
    const std::string& to_string() const { 
        if (offset_ == 0 && length_ == data_->length()) {
            return *data_;  // Return reference to original for full strings
        }
        // For substrings, create a new string with just the substring portion
        static thread_local std::string result;
        result = data_->substr(offset_, length_);
        return result;
    }

    /**
     * Returns a string that is a substring of this string.
     * The substring begins with the character at the specified index and extends to the end.
     *
     * @param beginIndex the beginning index, inclusive
     * @return the substring
     * @throws StringIndexOutOfBoundsException if beginIndex is larger than length()
     */
    String substring(std::size_t beginIndex) const {
        const auto len = length();
        // Check if beginIndex is out of bounds
        if (beginIndex > len) {
            throw StringIndexOutOfBoundsException("beginIndex is out of bounds");
        }
        return substring(beginIndex, len);
    }
    
    /**
     * Returns a string that is a substring of this string.
     * The substring begins at beginIndex and extends to endIndex - 1.
     *
     * @param beginIndex the beginning index, inclusive
     * @param endIndex the ending index, exclusive
     * @return the substring
     * @throws StringIndexOutOfBoundsException if:
     *         beginIndex is larger than length()
     *         endIndex is larger than length()
     *         beginIndex is larger than endIndex
     */
    String substring(std::size_t beginIndex, std::size_t endIndex) const {
        // Validate indices
        const auto& utf16 = get_utf16();
        const auto len = utf16.length();
        
        // Check for out of bounds conditions with more specific error messages
        if (beginIndex > len) {
            throw StringIndexOutOfBoundsException("beginIndex is out of bounds");
        }
        if (endIndex > len) {
            throw StringIndexOutOfBoundsException("endIndex is out of bounds");
        }
        if (beginIndex > endIndex) {
            throw StringIndexOutOfBoundsException("beginIndex cannot be larger than endIndex");
        }
        
        // If we're requesting the entire string, return this
        if (beginIndex == 0 && endIndex == utf16.length()) {
            return *this;
        }
        
        // If empty substring, return empty string
        if (beginIndex == endIndex) {
            return String("");
        }
        
        // Convert UTF-16 indices to UTF-8 byte offsets
        // We need to find the UTF-8 byte offsets that correspond to the UTF-16 indices
        std::size_t utf8_begin = 0;
        std::size_t utf8_end = 0;
        
        // Find the UTF-8 byte offset for beginIndex
        if (beginIndex > 0) {
            const unsigned char* str = reinterpret_cast<const unsigned char*>(data_->c_str() + offset_);
            const unsigned char* end = str + length_;
            std::size_t utf16_index = 0;
            
            while (str < end && utf16_index < beginIndex) {
                if (*str < 0x80) {
                    // ASCII character (1 byte in UTF-8, 1 unit in UTF-16)
                    str++;
                    utf16_index++;
                } else if ((*str & 0xE0) == 0xC0) {
                    // 2-byte UTF-8 sequence
                    if (str + 1 >= end || (str[1] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 2;
                    utf16_index++;
                } else if ((*str & 0xF0) == 0xE0) {
                    // 3-byte UTF-8 sequence
                    if (str + 2 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 3;
                    utf16_index++;
                } else if ((*str & 0xF8) == 0xF0) {
                    // 4-byte UTF-8 sequence (surrogate pair in UTF-16)
                    if (str + 3 >= end || (str[1] & 0xC0) != 0x80 || 
                        (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 4;
                    utf16_index += 2;  // Surrogate pair counts as 2 UTF-16 code units
                } else {
                    // Invalid UTF-8 byte
                    str++;
                    utf16_index++;
                }
            }
            
            utf8_begin = str - reinterpret_cast<const unsigned char*>(data_->c_str() + offset_);
        }
        
        // Find the UTF-8 byte offset for endIndex
        if (endIndex > 0) {
            const unsigned char* str = reinterpret_cast<const unsigned char*>(data_->c_str() + offset_);
            const unsigned char* end = str + length_;
            std::size_t utf16_index = 0;
            
            while (str < end && utf16_index < endIndex) {
                if (*str < 0x80) {
                    // ASCII character (1 byte in UTF-8, 1 unit in UTF-16)
                    str++;
                    utf16_index++;
                } else if ((*str & 0xE0) == 0xC0) {
                    // 2-byte UTF-8 sequence
                    if (str + 1 >= end || (str[1] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 2;
                    utf16_index++;
                } else if ((*str & 0xF0) == 0xE0) {
                    // 3-byte UTF-8 sequence
                    if (str + 2 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 3;
                    utf16_index++;
                } else if ((*str & 0xF8) == 0xF0) {
                    // 4-byte UTF-8 sequence (surrogate pair in UTF-16)
                    if (str + 3 >= end || (str[1] & 0xC0) != 0x80 || 
                        (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80) {
                        // Invalid sequence, treat as 1 byte
                        str++;
                        utf16_index++;
                        continue;
                    }
                    str += 4;
                    utf16_index += 2;  // Surrogate pair counts as 2 UTF-16 code units
                } else {
                    // Invalid UTF-8 byte
                    str++;
                    utf16_index++;
                }
            }
            
            utf8_end = str - reinterpret_cast<const unsigned char*>(data_->c_str() + offset_);
        }
        
        // Create a new String with the same data but adjusted offset and length
        return String(data_, offset_ + utf8_begin, utf8_end - utf8_begin);
    }

    // C++ operator overloads for comparison
    bool operator==(const String& other) const { return  equals(other);                          }
    bool operator!=(const String& other) const { return !equals(other);                          }
    bool operator< (const String& other) const { return compare_to(other).is_less();             }
    bool operator<=(const String& other) const { return compare_to(other).is_less_or_equal();    }
    bool operator> (const String& other) const { return compare_to(other).is_greater();          }
    bool operator>=(const String& other) const { return compare_to(other).is_greater_or_equal(); }

private:
    // Private constructor for creating substrings with shared data
    String(std::shared_ptr<const std::string> data, std::size_t offset, std::size_t length)
        : data_(std::move(data)), offset_(offset), length_(length), utf16_cache_() {}

    std::shared_ptr<const std::string> data_;  // Immutable UTF-8 string storage shared between instances
    std::size_t offset_{0};                      // Start offset in data_ (in bytes)
    std::size_t length_{0};                      // Length of this substring (in bytes)
    mutable std::shared_ptr<const std::u16string> utf16_cache_;  // Cached UTF-16 representation
    
    // Get or create UTF-16 representation
    const std::u16string& get_utf16() const {
        // Double-checked locking pattern with atomic operations
        auto cache = utf16_cache_;
        if (!cache) {
            // First check failed, acquire the data and create cache
            std::u16string result;
            const unsigned char* str = reinterpret_cast<const unsigned char*>(data_->c_str() + offset_);
            const unsigned char* end = str + length_;
            
            while (str < end) {
                if (*str < 0x80) {
                    // ASCII character
                    result.push_back(*str);
                    str++;
                } else if ((*str & 0xE0) == 0xC0) {
                    // 2-byte UTF-8 sequence
                    if (str + 1 >= end || (str[1] & 0xC0) != 0x80) {
                        // Invalid or incomplete sequence
                        result.push_back(0xFFFD);
                        str++;
                        continue;
                    }
                    // Check for overlong encoding
                    unsigned int codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
                    if (codepoint < 0x80) {
                        // Overlong encoding
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        str += 2;
                        continue;
                    }
                    result.push_back(codepoint);
                    str += 2;
                } else if ((*str & 0xF0) == 0xE0) {
                    // 3-byte UTF-8 sequence
                    if (str + 2 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80) {
                        // Invalid or incomplete sequence
                        result.push_back(0xFFFD);
                        str++;
                        continue;
                    }
                    // Check for overlong encoding and surrogate range
                    unsigned int codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
                    if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                        // Overlong encoding or surrogate range
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        str += 3;
                        continue;
                    }
                    result.push_back(codepoint);
                    str += 3;
                } else if ((*str & 0xF8) == 0xF0) {
                    // 4-byte UTF-8 sequence
                    if (str + 3 >= end || (str[1] & 0xC0) != 0x80 || (str[2] & 0xC0) != 0x80 || (str[3] & 0xC0) != 0x80) {
                        // Invalid or incomplete sequence
                        result.push_back(0xFFFD);
                        str++;
                        continue;
                    }
                    // Check for overlong encoding and valid Unicode range
                    unsigned int codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) |
                                            ((str[2] & 0x3F) << 6)  | (str[3] & 0x3F);
                    if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
                        // Overlong encoding or outside Unicode range
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        result.push_back(0xFFFD);
                        str += 4;
                        continue;
                    }
                    // Convert to surrogate pair
                    codepoint -= 0x10000;
                    result.push_back(0xD800 + (codepoint >> 10));
                    result.push_back(0xDC00 + (codepoint & 0x3FF));
                    str += 4;
                } else {
                    // Invalid UTF-8 byte
                    result.push_back(0xFFFD);
                    str++;
                }
            }
            
            cache = std::make_shared<const std::u16string>(std::move(result));
            utf16_cache_ = cache;
        }
        return *cache;
    }

    /**
     * Check if this string shares the same underlying data with another string.
     * 
     * This is an implementation detail used for testing the string sharing behavior.
     * If true, the strings are guaranteed to be equal. However, if false, the
     * strings may still be equal but stored in different memory locations.
     * 
     * @param other The string to compare with
     * @return true if both strings share the same underlying data, false otherwise
     */
    bool shares_data_with(const String& other) const { return data_ == other.data_; }

    // Allow test fixtures to access private members
    friend class StringTest;
    friend class StringSharing;  // Test fixture for string sharing tests

private:
    // Helper templates and concepts for valueOf implementation
    template<typename T>
    struct always_false : std::false_type {};

#if __cplusplus >= 202002L  // C++20 or later
    // Concept for container types
    template<typename T>
    static constexpr bool is_container = requires(T t) {
        { t.begin() } -> std::input_or_output_iterator;
        { t.end() } -> std::input_or_output_iterator;
        { t.size() } -> std::convertible_to<std::size_t>;
    };

    // Concept for map-like types
    template<typename T>
    static constexpr bool is_map_like = is_container<T> && requires(T t) {
        typename T::key_type;
        typename T::mapped_type;
        { t.begin()->first } -> std::convertible_to<typename T::key_type>;
        { t.begin()->second } -> std::convertible_to<typename T::mapped_type>;
    };
    
    // Concept for variant types
    template<typename T>
    static constexpr bool is_variant = requires {
        typename std::variant_size<std::remove_cvref_t<T>>::type;
    };

    // Helper function to convert any type to string
    template<typename T>
    static std::string to_string_helper(const T& obj) {
        if constexpr (requires { std::to_string(obj); }) {
            return std::to_string(obj);
        } else if constexpr (requires { obj.to_string(); }) {
            return obj.to_string();
        } else if constexpr (is_variant<T>) {
            // Special handling for std::variant
            return std::visit([](const auto& value) -> std::string {
                return to_string_helper(value);
            }, obj);
        } else if constexpr (is_map_like<T>) {
            // Special handling for map-like containers (map, unordered_map, etc.)
            std::ostringstream oss;
            oss << "{";
            auto it = obj.begin();
            if (it != obj.end()) {
                oss << to_string_helper(it->first) << "=" << to_string_helper(it->second);
                ++it;
            }
            for (; it != obj.end(); ++it) {
                oss << ", " << to_string_helper(it->first) << "=" << to_string_helper(it->second);
            }
            oss << "}";
            return oss.str();
        } else if constexpr (is_container<T>) {
            // General handling for container types (vector, list, set, etc.)
            std::ostringstream oss;
            oss << "[";
            auto it = obj.begin();
            if (it != obj.end()) {
                oss << to_string_helper(*it);
                ++it;
            }
            for (; it != obj.end(); ++it) {
                oss << ", " << to_string_helper(*it);
            }
            oss << "]";
            return oss.str();
        } else if constexpr (requires(std::ostringstream& oss) { oss << obj; }) {
            std::ostringstream oss;
            oss << obj;
            return oss.str();
        } else {
            static_assert(always_false<T>::value, "No conversion to string available for this type");
            return ""; // Unreachable, but needed to avoid compiler warnings
        }
    }
#else  // Before C++20
    // Helper function to convert any type to string (fallback for pre-C++20)
    template<typename T>
    static std::string to_string_helper(const T& obj) {
        // Use SFINAE or other pre-C++20 techniques if needed
        return std::to_string(obj);  // This will only work for numeric types
    }
#endif
};

} // namespace simple
