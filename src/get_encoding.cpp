#include <unicode/ucsdet.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

#include "compat.hpp"
#include "cmem.hpp"
#include "get_encoding.hpp"
#include "betterSTD/include/bstd/libstdc/libstdc.hpp"

bool is_valid_utf8(const uchar* s, size_t len) {
    size_t i = 0;

    while (i < len) {
        uchar c = s[i];

        if (c <= 0x7F) {
            i++;
        }
        else if ((c >> 5) == 0x6) {
            if (i+1 >= len || (s[i+1] >> 6) != 0x2)
                return false;
            i += 2;
        }
        else if ((c >> 4) == 0xE) {
            if (i+2 >= len ||
                (s[i+1] >> 6) != 0x2 ||
                (s[i+2] >> 6) != 0x2)
                return false;
            i += 3;
        }
        else if ((c >> 3) == 0x1E) {
            if (i+3 >= len ||
                (s[i+1] >> 6) != 0x2 ||
                (s[i+2] >> 6) != 0x2 ||
                (s[i+3] >> 6) != 0x2)
                return false;
            i += 4;
        }
        else {
            return false;
        }
    }

    return true;
}

Encoding detect_bom(FILE* f) {
    uint8_t bytes[4] = {0};
    size_t n = fread(bytes, 1, 4, f);
    rewind(f);

    if (n >= 3 &&
        bytes[0] == 0xEF &&
        bytes[1] == 0xBB &&
        bytes[2] == 0xBF)
        return Encoding::UTF8;

    if (n >= 4 &&
        bytes[0] == 0xFF &&
        bytes[1] == 0xFE &&
        bytes[2] == 0x00 &&
        bytes[3] == 0x00)
        return Encoding::UTF32_LE;

    if (n >= 4 &&
        bytes[0] == 0x00 &&
        bytes[1] == 0x00 &&
        bytes[2] == 0xFE &&
        bytes[3] == 0xFF)
        return Encoding::UTF32_BE;

    if (n >= 2 &&
        bytes[0] == 0xFF &&
        bytes[1] == 0xFE)
        return Encoding::UTF16_LE;

    if (n >= 2 &&
        bytes[0] == 0xFE &&
        bytes[1] == 0xFF)
        return Encoding::UTF16_BE;

    return Encoding::Unknown;
}

struct _no_bom_data {
    const char* encoding;
    int confidence;
};

_no_bom_data _get_no_bom(const char* buffer, int bufferSize) {
    UErrorCode status = U_ZERO_ERROR;

    UCharsetDetector* csd = ucsdet_open(&status);
    ucsdet_setText(csd, buffer, bufferSize, &status);

    const UCharsetMatch* match = ucsdet_detect(csd, &status);

    if (U_FAILURE(status) || !match) {
        fprintf(stderr, "chario: failed to retrieve encoding (external ICU(ucsdet_detect) failure).\n");
        fflush(stderr);
        ucsdet_close(csd);
        abort();
    }

    const char* encoding = ucsdet_getName(match, &status);
    int confidence = ucsdet_getConfidence(match, &status);

    if (U_FAILURE(status) || !match) {
        fprintf(stderr, "chario: failed to retrieve encoding (external ICU(ucsdet_getName/ucsdet_getConfidence) failure).\n");
        fflush(stderr);
        ucsdet_close(csd);
        abort();
    }

    ucsdet_close(csd);

    // TODO: Fix memory leak
    return _no_bom_data {.encoding = (const char*)mreloc(encoding, strlen(encoding) + 1), .confidence = confidence};
}

Encoding detect_no_bom(const char* buffer, int bufferSize) {
    _no_bom_data result = _get_no_bom(buffer, bufferSize);
    if (result.confidence <= 20)
        return is_valid_utf8((const uchar*)buffer, bufferSize) ? Encoding::UTF8 : Encoding::Unknown;

    if (!strcmp(result.encoding, "UTF-8"))    return Encoding::UTF8;
    if (!strcmp(result.encoding, "UTF-16"))   return Encoding::UTF16;
    if (!strcmp(result.encoding, "UTF-16LE")) return Encoding::UTF16_LE;
    if (!strcmp(result.encoding, "UTF-16BE")) return Encoding::UTF16_BE;
    if (!strcmp(result.encoding, "UTF-32"))   return Encoding::UTF32;
    if (!strcmp(result.encoding, "UTF-32LE")) return Encoding::UTF32_LE;
    if (!strcmp(result.encoding, "UTF-32BE")) return Encoding::UTF32_BE;
    else {
        if (is_valid_utf8((const uchar*)buffer, bufferSize))
            return Encoding::UTF8;
        else
            return Encoding::Unsupported;
    }
}

Encoding get_encoding(FILE* f, size_t fileSampleSize) {
    Encoding bom = detect_bom(f);
    if (bom != Encoding::Unknown) 
        return bom;

    #ifndef _WIN32
    struct stat st;
    if (fstat(fileno(f), &st) != 0) {
        fprintf(stderr, "chario: fstat failed");
        fflush(stderr);
        abort();
    }
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "chario: attempted to open non-regular file %s");
        fflush(stderr);
        abort();
    }
    #else
    struct _stat64 st;
    if (_fstat64(_fileno(f), &st) != 0) {
        fprintf(stderr, "chario: _fstat64 failed");
        fflush(stderr);
        abort();
    }
    #endif

    size_t sample_size = st.st_size < fileSampleSize ? st.st_size : fileSampleSize;
    if (sample_size == 0) {
        return Encoding::UTF8;
    }
    POINT nullp = POINT(NULL);
    POINT datap = alloc_mem(nullp, sample_size); //malloc(sample_size);
    if (datap.isNull()) {
        fprintf(stderr, "chario: alloc_mem() failed (external bstd/libstd failure).\n");
        fflush(stderr);
        abort();
    }
    char* data = (char*)(void*)datap;
    size_t real_sample_size = fread(data, 1, sample_size, f);
    if (real_sample_size == 0) {
        fseek(f, 0, SEEK_SET);
        dealloc_mem(datap, sample_size);
        return Encoding::UTF8;
    }
    if (!protect_mem(datap, sample_size, MemProtect::READ, NULL)) {
        fprintf(stderr, "chario: protect_mem() failed (external bstd/libstd failure).\n");
        fflush(stderr);
        abort();
    };
    
    fseek(f, 0, SEEK_SET);
    Encoding no_bom = detect_no_bom(data, real_sample_size);

    if (!dealloc_mem(datap, sample_size)) {
        // NOT fatal, just a memory leak
        fprintf(stderr, "chario: dealloc_mem() failed repeatedly (external bstd/libstd failure).\n");
        fflush(stderr);
    }

    return no_bom;
}
