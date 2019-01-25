#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <utility>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include<cstdlib>
#include "crc32.h"
#include "rc6_czq.h"

typedef struct File_Firmware_Format_
{
    int file_type_flag;             /* FILE_TYPE_FLAG */
    long src_file_length;
    unsigned int src_file_crc32_check;
    long enc_file_length;
    unsigned int enc_file_crc32_check;
    char res[512];
}File_Firmware_Format;

//字节对齐
bool fill_rand_data(unsigned char *fill_buffer, unsigned int fill_data);

//模型加密
void encryption_model(std::string prototxtFile, std::string caffemodelFile, char* key,std::string outputfile);

//模型解密
bool decrypt_model(std::string encryption_model, std::string &prototxt_string, std::string &model_string, char* key); //, std::string pro

#endif 
