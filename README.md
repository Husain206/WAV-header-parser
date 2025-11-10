 ## wav parser
 a simple wav header parser :D
 
 ### usage:

```cpp
  int main(int argc, char* argv[]) {

  // read the wav file in binary
  ifstream file = read_bin(argc, argv);

  // init the wav struct
  WavHeader wav{};

  // self-explanatory
  if(!parse_wav_header(file, wav)) return 1;

  // self-explanatory
  print_wav(wav);

  return 0;
}

```
