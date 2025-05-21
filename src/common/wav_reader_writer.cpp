// SPDX-FileCopyrightText: 2019-2024 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: CC-BY-NC-ND-4.0

#include "wav_reader_writer.h"

#include "common/error.h"
#include "common/log.h"

#include <cstdio>
#include <limits>

namespace {

#pragma pack(push, 1)
struct WAV_HEADER
{
  u32 chunk_id;
  u32 chunk_size;
  u32 format;
};

struct WAV_CHUNK_HEADER
{
  u32 chunk_id;
  u32 chunk_size;
};

struct WAV_FULL_HEADER
{
  u32 chunk_id; // RIFF
  u32 chunk_size;
  u32 format; // WAVE

  struct FormatChunk
  {
    u32 chunk_id; // "fmt "
    u32 chunk_size;
    u16 audio_format; // pcm = 1
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
  } fmt_chunk;

  struct DataChunkHeader
  {
    u32 chunk_id; // "data "
    u32 chunk_size;
  } data_chunk_header;
};
#pragma pack(pop)

static constexpr u32 RIFF_VALUE = 0x46464952; // 0x52494646
static constexpr u32 FMT_VALUE = 0x20746d66;  // 0x666d7420
static constexpr u32 DATA_VALUE = 0x61746164; // 0x64617461
static constexpr u32 WAVE_VALUE = 0x45564157; // 0x57415645

} // namespace

WAVWriter::WAVWriter() = default;

WAVWriter::WAVWriter(WAVWriter&& move)
{
  m_file = std::exchange(move.m_file, nullptr);
  m_sample_rate = std::exchange(move.m_sample_rate, 0);
  m_num_channels = std::exchange(move.m_num_channels, 0);
  m_num_frames = std::exchange(move.m_num_frames, 0);
}

WAVWriter::~WAVWriter()
{
  if (IsOpen())
    Close(nullptr);
}

WAVWriter& WAVWriter::operator=(WAVWriter&& move)
{
  m_file = std::exchange(move.m_file, nullptr);
  m_sample_rate = std::exchange(move.m_sample_rate, 0);
  m_num_channels = std::exchange(move.m_num_channels, 0);
  m_num_frames = std::exchange(move.m_num_frames, 0);
  return *this;
}

bool WAVWriter::Open(const char* path, u32 sample_rate, u32 num_channels, Error* error)
{
  if (IsOpen())
    Close(nullptr);

  m_file = std::fopen(path, "wb");
  if (!m_file)
    return false;

  m_sample_rate = sample_rate;
  m_num_channels = num_channels;
  m_num_frames = 0;

  if (!WriteHeader(error))
  {
    m_sample_rate = 0;
    m_num_channels = 0;
    std::fclose(m_file);
    m_file = nullptr;
    return false;
  }

  return true;
}

bool WAVWriter::Close(Error* error)
{
  if (!IsOpen())
    return true;

  bool res = (m_num_frames != std::numeric_limits<u32>::max());
  if (res)
  {
    res = std::fseek(m_file, 0, SEEK_SET) == 0 && WriteHeader(error);
    if (std::fclose(m_file) != 0)
    {
      Error::SetErrno(error, "fclose() failed: ", errno);
      res = false;
    }
  }

  m_file = nullptr;
  m_sample_rate = 0;
  m_num_channels = 0;
  m_num_frames = 0;
  return res;
}

bool WAVWriter::WriteFrames(const s16* samples, u32 num_frames, Error* error)
{
  if (m_num_frames == std::numeric_limits<u32>::max())
  {
    Error::SetStringView(error, "Previous write failed.");
    return false;
  }

  const u32 num_frames_written =
    static_cast<u32>(std::fwrite(samples, sizeof(s16) * m_num_channels, num_frames, m_file));
  if (num_frames_written != num_frames)
  {
    Error::SetErrno(error, "fwrite() failed: ", errno);
    m_num_frames = std::numeric_limits<u32>::max();
    return false;
  }

  m_num_frames += num_frames_written;
  return true;
}

bool WAVWriter::WriteHeader(Error* error)
{
  const u32 data_size = sizeof(SampleType) * m_num_channels * m_num_frames;

  WAV_FULL_HEADER header = {};
  header.chunk_id = RIFF_VALUE;
  header.chunk_size = sizeof(WAV_FULL_HEADER) - 8 + data_size;
  header.format = WAVE_VALUE;
  header.fmt_chunk.chunk_id = FMT_VALUE;
  header.fmt_chunk.chunk_size = sizeof(header.fmt_chunk) - 8;
  header.fmt_chunk.audio_format = 1;
  header.fmt_chunk.num_channels = static_cast<u16>(m_num_channels);
  header.fmt_chunk.sample_rate = m_sample_rate;
  header.fmt_chunk.byte_rate = m_sample_rate * m_num_channels * sizeof(SampleType);
  header.fmt_chunk.block_align = static_cast<u16>(m_num_channels * sizeof(SampleType));
  header.fmt_chunk.bits_per_sample = 16;
  header.data_chunk_header.chunk_id = DATA_VALUE;
  header.data_chunk_header.chunk_size = data_size;

  if (std::fwrite(&header, sizeof(header), 1, m_file) != 1)
  {
    Error::SetErrno(error, "fwrite() failed: ", errno);
    return false;
  }

  return true;
}
