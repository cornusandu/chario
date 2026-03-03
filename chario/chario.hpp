#pragma once

#include <string>
#include <cstdio>

enum class Encoding {
    UTF8,
    UTF16,
    UTF16_LE,
    UTF16_BE,
    UTF32,
    UTF32_LE,
    UTF32_BE,
    Unknown,
    Unsupported
};

namespace chario {
    namespace os{
        using wstring = std::wstring;
        using string = std::string;
    }

    using wstring = std::basic_string<char32_t>;
    using swstring = std::basic_string<char16_t>;
    using string = std::string;

    struct stream_data {
        int fd;
        FILE* file;
        unsigned char owning;
        Encoding encoding;
    };

    class ofstream {
        public:
        ofstream(const char* path);
        ofstream(FILE* file);
        
        ofstream& operator<< (const chario::wstring&);
        ofstream& operator<< (const chario::swstring&);
        ofstream& operator<< (const chario::string&);
        ofstream& operator<< (const char*);
        ofstream& operator<< (const char16_t*);
        ofstream& operator<< (const char32_t*);
        ofstream& operator<< (char);
        ofstream& operator<< (char16_t);
        ofstream& operator<< (char32_t);

        void printf(const char* fmt, ...);
        void push(const char* fmt);
        void push_force(const char* fmt);
        void fflush();

        void dont_close();

        const stream_data* operator->();

        private:
        FILE* file_ = nullptr;
        int fd_ = -1;
        unsigned char close_on_end = true;
        chario::stream_data data;
        Encoding encoding;
    };
}
