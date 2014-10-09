/*
  wav file io

  2009�N7��
*/

#pragma once

#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>

#include<cmath>
#include<vector>

#include<string>
#include<algorithm>

namespace wav{
  struct Header{
    char riff[4];
    unsigned int filesize;
    char wavefmt[8];
    unsigned int waveformat;
    unsigned short int pcm;
    unsigned short int n_channel;
    unsigned int sampling_rate;
    unsigned int bytes_per_second;
    unsigned short int block_per_sample;
    unsigned short int bits_per_sample;
    char data[4];
    unsigned int n_byte;
  };

  class MonoChannel{
  public:
    MonoChannel(){}
    void init(int n){
      buffer_size = n;
      data = new double[n]; //�o�b�t�@�̑傫���͌��߂Ă��܂��B����ȏ���X�g�b�N���Ă������Ƃ͂ł��Ȃ��B
      current_point = 0;
    }
    ~MonoChannel(){
      delete []data;
    }
    void insert(double x){
      data[current_point] = x;
      current_point = (current_point < buffer_size - 1 ? current_point + 1 : 0);
    };
    double& operator[](int i){
      return data[i + current_point < buffer_size ? i + current_point : i + current_point - buffer_size];
    }
  private:
    double *data;
    int current_point; //�����O�o�b�t�@���Ǘ����邽�߂Ɏg���B
    int buffer_size;
  };

  class Signal{
  public:
    Signal(){}
    void init(int n_channel, int buf_size){
      buffer_size = buf_size;
      data = new MonoChannel[n_channel];
      for(int i = 0; i < n_channel; i++){
        data[i].init(buf_size);
      }
    }
    ~Signal(){
      delete []data;
    }
    int length(){return buffer_size;}
    MonoChannel& operator[](int n){return data[n];}
  private:
    MonoChannel *data;
    int buffer_size;
  };
  template<typename T> double realization(T);
  template<typename T> T quantize(double);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavistream�N���X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class wavistream{
public:
  wavistream(const char*, int);
  ~wavistream();
  void read(int n){if(header.bits_per_sample == 8)  __read<unsigned char>(n); else  __read<signed short>(n);};
  void copy(double* x, int ch, int n){for(int i = 0; i < n; i++)x[i] = signal[ch][i];};
  bool eof(){return feof(fp);}
  wav::Header header;
private:
  wav::Signal signal;
  std::string filename;
  FILE *fp;
  int current_seek;
  template<typename T> void __read(int n);
};

template<typename T> void // unsigned char or signed short
wavistream::__read(int n){
  T tmp;
  double tmp_double;
  //���o��
  fseek(fp, sizeof(wav::Header) + current_seek, SEEK_SET);
  //�f�[�^�̓ǂݍ��݁Bwav�t�@�C���ł̓`�����l�����ɕ���ł���B
  for(int i = 0; i < n ; i++){
    for(int ch = 0; ch < header.n_channel; ch++){
      if(!eof()){
        !fread(&tmp, sizeof(T), 1, fp);
        tmp_double = wav::realization(tmp);
        signal[ch].insert( tmp_double );
      }else{
        signal[ch].insert( 0.0 );
      }
    }
  }
  current_seek += n * header.n_channel * sizeof(T);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavistream�N���X�����B
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//wavostream�N���X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class wavostream{
public:
  wavostream(const char*, int, int);
  ~wavostream();
  void set(double*, int, int);
  void write(int n){if(header.bits_per_sample == 8)  __write<unsigned char>(n); else  __write<signed short>(n);};;
  void write_header();
  wav::Header header;
  
private:
  wav::Signal signal;
  std::string filename;
  FILE *fp;
  template<typename T> void __write(int);
  int signal_length;
  int buffer_size;
};

template<typename T> void // unsigned char or signed short
wavostream::__write(int n){
  for(int j = buffer_size - n; j < buffer_size; j++){
    for(int ch = 0; ch < header.n_channel; ch++){
      //double�̌n����Cunsigned char, signed short�ɗʎq������B
      T tmp = wav::quantize<T>(signal[ch][j]);
      fwrite(&tmp, sizeof(T), 1, fp);
    }
  }
//  current_seek += n * header.n_channel * sizeof(T);
  signal_length += n;
}

