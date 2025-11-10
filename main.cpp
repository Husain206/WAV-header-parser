#include "src/wav.h"
#include <istream>

using namespace std;

int main(int argc, char* argv[]) {

  ifstream file = read_bin(argc, argv);
  WavHeader wav{};

  if(!parse_wav_header(file, wav)) return 1;
  print_wav(wav);

  return 0;
}

