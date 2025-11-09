#include "wav.h"
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <ios>
#include <istream>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>

using namespace std;
// 0x52494646
// 0x00000052
// 0x00004900
// 0x00460000
// 0x46000000
// 0x46464952
// 
// 0x52494646
// 0x00000046
// 0x00004600
// 0x00490000
// 0x52000000
//
 
uint32_t read_u32_le(std::istream& in) {
  uint8_t ehex[4];
  in.read(reinterpret_cast<char*>(ehex), 4);
  return (uint32_t)ehex[0]
       | ((uint32_t)ehex[1] << 8)
       | ((uint32_t)ehex[2] << 16)
       | ((uint32_t)ehex[3] << 24);
}

uint32_t read_u32_be(std::istream& in) {
  uint8_t ehex[4];
  in.read(reinterpret_cast<char*>(ehex), 4);
  return (uint32_t)ehex[3]
       | ((uint32_t)ehex[2] << 8)
       | ((uint32_t)ehex[1] << 16)
       | ((uint32_t)ehex[0] << 24);
}

uint16_t read_u16_le(std::istream& in) {
  uint8_t ehex[2];
  in.read(reinterpret_cast<char*>(ehex), 2);
  return (uint32_t)ehex[0]
       | ((uint32_t)ehex[1] << 8);
}

uint16_t read_u16_be(std::istream& in) {
  uint8_t ehex[2];
  in.read(reinterpret_cast<char*>(ehex), 2);
  return (uint32_t)ehex[1]
       | ((uint32_t)ehex[0] << 8);
}

static void skip_bytes(std::istream& in, uint32_t n) {
  in.seekg(n, std::ios::cur);
}

static void skip_chunk_payload(istream& in, uint32_t size){
  skip_bytes(in, size);
  if(size & 1) skip_bytes(in, 1);
}

static bool expected_id(const char got[4], const char expect[4]){
  return strncmp(got, expect, 4) == 0;
}

static bool parse_wav_header(std::istream &in, WavHeader& wav){
  char *chunk_id = wav.chunk_id;
  if(!in.read(chunk_id, 4) || !expected_id(chunk_id, "RIFF")){
    cerr << "not a riff file\n";
    return false;
  }
  chunk_id[4] = '\0';
  
  wav.chunck_size = read_u32_le(in);
  
  char *wave = wav.format;
  if(!in.read(wave, 4) || !expected_id(wave, "WAVE")){
    cerr << "not a wave file\n";
    return false;
  }
  wave[4] = '\0';

  bool seenFmt{false}, seenData{false}, seenList{false};

  while (in && (!seenFmt || !seenData)) {
    char id[4];
    if(!in.read(id, 4)) break;

    uint32_t subChunkSize = read_u32_le(in);

    if (expected_id(id, "fmt ")) {
      if (subChunkSize < 16) { 
        cerr << "fmt chunk too small\n"; 
        return false; 
      }
      
      wav.fmt_size = subChunkSize;
      wav.audio_format = read_u16_le(in);
      wav.num_of_channels = read_u16_le(in);
      wav.sample_rate = read_u32_le(in);
      wav.byte_rate = read_u32_le(in);
      wav.block_align = read_u16_le(in);
      wav.bits_per_sample = read_u16_le(in);

      if(subChunkSize > 16) skip_bytes(in, subChunkSize - 16);
      if(subChunkSize & 1) skip_bytes(in, 1);

      memcpy(wav.fmt_id, "fmt ", 4);
      wav.fmt_id[4] = '\0';
      seenFmt = true;
    } else if(expected_id(id, "data")){
      wav.data_size = subChunkSize;
      memcpy(wav.data_id, "data", 4);
      wav.data_id[4] = '\0';
      skip_chunk_payload(in, subChunkSize);     
      seenData = true;
    } else if(expected_id(id, "LIST")){
      memcpy(wav.list_id, "LIST", 4);
      wav.list_id[4] = '\0';
      wav.list_size = subChunkSize;
      
      if(!in.read(wav.list_subChunk_type, 4)) return false;
      wav.list_subChunk_type[4] = '\0';

      uint32_t remaining = subChunkSize - 4;

      while(remaining >= 8){
      char sid[5]; sid[4] = '\0';
      if(!in.read(sid, 4)) return false;
      uint32_t ssz = read_u32_le(in);
      remaining -= 8;

      if(wav.info_id[0] == '\0'){
      memcpy(wav.info_id, sid, 4);
      wav.info_id[4] = '\0';
      wav.info_size = ssz;
      }
      
      skip_bytes(in, ssz);
      remaining -= ssz;
      if (ssz & 1) { skip_bytes(in, 1); remaining -= 1; }
      }
      if (subChunkSize & 1) skip_bytes(in, 1);
      seenList = true;
    } else {
      skip_chunk_payload(in, subChunkSize);
    }
  }
  if (!seenFmt)  { std::cerr << "Missing fmt chunk\n";  return false; }
  if (!seenData) { std::cerr << "Missing data chunk\n"; return false; }
  if (!seenList)  { std::cerr << "Missing list chunk\n";  return false; }
  return true;
} 



int main(int argc, char* argv[]) {

  if (argc != 2) {
    cerr << "usage: wav <file.wav>\n";
    return 1;
  }
  
  const std::string filename = argv[1];
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "\033[31mError:\033[0m Could not open file '" << filename << "'\n";
    return 1;
  }

  WavHeader wav{};

  if(!parse_wav_header(file, wav)) return 1;

  cout << "\n\n";
  // cout << "*=======**************=======* \n";
  cout << " ******** WAV header ******** \n";
  // cout << "*=======**************=======* \n";

  cout << " =========================================\n";
  cout << " chunk ID                   :   " << wav.chunk_id            << "\t\n";
  cout << " chunk size                 :   " << wav.chunck_size         << "\t\n";
  cout << " format                     :   " << wav.format              << "\t\n";
  cout << "  - fmt sub-chunk                    \n";
  cout << "     fmt ID                 :   " << wav.fmt_id              << "\t\n";
  cout << "     fmt size               :   " << wav.fmt_size            << "\t\n";
  // cout << "     audio format           :   " << wav.audio_format        << "\t\n";
  cout << "     audio format           :   " << ((wav.audio_format == 1) ? "1(PCM16)" : "3(Float-32)") << "\t\n";
  cout << "     num of channels        :   " << wav.num_of_channels     << "\t\n";
  cout << "     sample rate            :   " << wav.sample_rate         << "hz\t\n";
  cout << "     byte rate              :   " << wav.byte_rate           << "\t\n";
  cout << "     block align            :   " << wav.block_align         << "\t\n";
  cout << "     bits per-sample        :   " << wav.bits_per_sample     << "\t\n";
  cout << " - list sub-chunk                     \n";  
  cout << "    list ID                 :   " << wav.list_id             << "\t\n";
  cout << "    list size               :   " << wav.list_size           << "\t\n";
  cout << "     - sub-chunk type       :   " << wav.list_subChunk_type  << "\t\n";
  cout << "         info id            :   " << wav.info_id             << "\t\n";
  cout << "         info size          :   " << wav.info_size           << "\t\n";
  cout << " - data sub-chunk                    \n";  
  cout << "    data ID                 :   " << wav.data_id             << "\t\n";
  cout << "    data size               :   " << wav.data_size           << "\t\n";


  // cout << "=========================================\n";
  
  return 0;
}



