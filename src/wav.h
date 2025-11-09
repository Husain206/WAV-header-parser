#include <cstdint>
#include <istream>


struct WavHeader {

  /*
  *******************************
  * the "RIFF" chunk descriptor *
  *******************************
  */


  // Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form).
  char chunk_id[5];

  // 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
  uint32_t chunck_size;
  
  // Contains the letters "WAVE" (0x57415645 big-endian form).
  char format[5];



  /*
  ***********************
  * the "FMT" sub-chunk *
  ***********************
  */

  // Contains the letters "fmt " (0x666d7420 big-endian form).
  char fmt_id[5];

  // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
  uint32_t fmt_size;

  // PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some form of compression.
  uint16_t audio_format;

  // Mono = 1, Stereo = 2, etc.
  uint16_t num_of_channels;

  // 8000, 44100, etc.
  uint32_t sample_rate;

  // == SampleRate * NumChannels * BitsPerSample/8
  uint32_t byte_rate;

  // == NumChannels * BitsPerSample/8 The number of bytes for one sample including all channels. I wonder what happens when
  // this number isn't an integer?
  uint16_t block_align;

  // self-explanatory.
  uint16_t bits_per_sample;



  /*
  ***********************
  * the "DATA" sub-chunk *
  ***********************
  */

  // Contains the letters "data" (0x64617461 big-endian form).
  char data_id[5];

  // == NumSamples * NumChannels * BitsPerSample/8
  uint32_t data_size;



  /*
  ***********************
  * the "LIST" sub-chunk *
  ***********************
  */

  // Contains the letters "LIST" (0x4C495354 big-endian form).
  char list_id[5];

  // The size of the rest of the LIST chunk that follows this number.
  // Includes the list type (e.g., "INFO") and all contained subchunks.
  uint32_t list_size;
  
  // The list type identifier, usually "INFO" (metadata) or "adtl" (cue labels).
  char list_subChunk_type[5];

  // The ID of the first subchunk within the LIST (e.g., "INAM", "IART", etc.).
  char info_id[5];

  // The size of the first subchunk’s data in bytes.
  uint32_t info_size;
};


static bool parse_wav_header(std::istream &in, WavHeader& wav);

uint32_t read_u32_le(std::istream& in);
uint32_t read_u32_be(std::istream& in);
uint16_t read_u16_le(std::istream& in);
uint16_t read_u16_be(std::istream& in);


static void skip_bytes(std::istream& in, uint32_t n);
static void skip_chunk_payload(std::istream& in, uint32_t size);
static bool expected_id(const char got[4], const char expect[4]);
