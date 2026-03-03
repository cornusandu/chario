#include "chario.hpp"
#include "compat.hpp"
#include "get_encoding.hpp"
#include "encoding_check.hpp"
#include <cstdio>
#include <memory>

chario::ofstream::ofstream(const char* path) {
    FILE* handle = fopen(path, "wb");
    this->file_ = handle;

    Encoding encoding = get_encoding(handle, 4096);
    int compat = encoding_is_compatible(encoding);
    if (compat != 0) {
        fprintf(stderr, "chario: attempted to open a file of an unsupported encoding (`%s`) (error code %d).\n", path, compat);
        ::fflush(stderr);
        abort();
    }

    this->encoding = encoding;
}

chario::ofstream::ofstream(FILE* file) {
    this->file_ = file;

    Encoding encoding = get_encoding(file, 4096);
    int compat = encoding_is_compatible(encoding);
    if (compat != 0) {
        fprintf(stderr, "chario: attempted to parse a FILE* of an unsupported encoding (error code %d).\n", compat);
        ::fflush(stderr);
        abort();
    }

    this->encoding = encoding;
}

void chario::ofstream::fflush() {
    ::fflush(this->file_);
}

void chario::ofstream::dont_close() {
    this->close_on_end = false;
}

const chario::stream_data* chario::ofstream::operator->() {
    this->data.fd = this->fd_;
    this->data.file = this->file_;
    this->data.owning = this->close_on_end;
    this->data.encoding = this->encoding;

    return &this->data;
}

chario::ofstream& chario::ofstream::operator<< (const chario::string &str) {
    fprintf(this->file_, "%s", str.c_str());
    return *this;
}

chario::ofstream& chario::ofstream::operator<< (const char* str) {
    fprintf(this->file_, "%s", str);
    return *this;
}

chario::ofstream& chario::ofstream::operator<< (char ch) {
    fprintf(this->file_, "%c", ch);
    return *this;
}
