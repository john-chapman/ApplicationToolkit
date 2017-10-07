#include <apt/compress.h>

#define MINIZ_IMPL
#include <miniz.h>

using namespace apt;

void apt::Compress(const void* _in, uint _inSizeBytes, void*& out_, uint& outSizeBytes_, CompressionFlags _flags)
{
	APT_ASSERT(_in);
	APT_ASSERT(_inSizeBytes);
	APT_ASSERT(!out_);

	int tdeflFlags = TDEFL_WRITE_ZLIB_HEADER;
	if (_flags & CompressionFlags_Speed) {
		tdeflFlags |= TDEFL_GREEDY_PARSING_FLAG;
	}
	out_ = tdefl_compress_mem_to_heap(_in, _inSizeBytes, &outSizeBytes_, tdeflFlags);
	APT_ASSERT(out_);
}

void apt::Decompress(const void* _in, uint _inSizeBytes, void*& out_, uint& outSizeBytes_)
{
	APT_ASSERT(_in);
	APT_ASSERT(_inSizeBytes);
	APT_ASSERT(!out_);

	int tinflFlags = TINFL_FLAG_PARSE_ZLIB_HEADER;
	out_ = tinfl_decompress_mem_to_heap(_in, _inSizeBytes, &outSizeBytes_, tinflFlags);
	APT_ASSERT(out_);
}
