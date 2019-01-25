#include "enc_dec_model.h"
#include <fstream>
#define uchar  unsigned char

bool fill_rand_data(unsigned char *fill_buffer, unsigned int fill_data) 
{
	unsigned int *p = NULL;
	unsigned int rand_loop_end = fill_data / sizeof(unsigned int);
	unsigned int i;
	int number = 0;
	srand((unsigned)time(NULL));
	for (i = 0; i < rand_loop_end; i++)
	{
		p = (unsigned int*)(fill_buffer + i * sizeof(unsigned int));
		number = rand();
		*p = (unsigned int)number;
		srand(*p);
	}
	return true;
}

//模型加密
void encryption_model(std::string prototxtFile, std::string caffemodelFile, char* key, std::string outputfile)
{
	/* 读文件 校验CRC32 */
	uint32_t src_crc32_prototxt = 0;
	uint32_t src_crc32_caffemodel = 0;
	uint32_t enc_crc32_prototxt = 0;
	uint32_t enc_crc32_caffemodel = 0;

	long long src_prototxt_file_size = 0;
	long long src_caffemodel_file_size = 0;
	long long enc_prototxt_file_size = 0;
	long long enc_caffemodel_file_size = 0;

	char *prototxt_file_buffer = NULL;
	char *caffemodel_file_buffer = NULL;

	File_Firmware_Format prototxt_Format;
	File_Firmware_Format caffemodel_Format;

	unsigned int enc_buffer_start=0;
	
	std::ifstream fprototxt;
	std::ifstream fmodel;


	fprototxt.open(prototxtFile);
	fmodel.open(caffemodelFile, std::ios::binary);

	//读取文件，并计算crc32校验码
	if (fprototxt.is_open())
	{
		std::streampos pos = fprototxt.tellg();     //   save   current   position
		fprototxt.seekg(0, std::ios::end);
		src_prototxt_file_size = fprototxt.tellg();
		fprototxt.seekg(pos);     //   restore   saved   position

		enc_prototxt_file_size = ( ( src_prototxt_file_size + 16 ) / 4096 + 1 ) * 4096;
		prototxt_file_buffer = new char[ enc_prototxt_file_size ];
		fprototxt.read( prototxt_file_buffer, src_prototxt_file_size );
		src_crc32_prototxt = crc32( src_crc32_prototxt, prototxt_file_buffer, src_prototxt_file_size );
		fprototxt.close();
	}
	if(fmodel.is_open())
	{
		std::streampos mpos = fmodel.tellg();     //   save   current   position
		fmodel.seekg(0, std::ios::end);
		src_caffemodel_file_size = fmodel.tellg();
		fmodel.seekg(mpos);     //   restore   saved   position

		enc_caffemodel_file_size = ( ( src_caffemodel_file_size + 16 ) / 4096 + 1 ) * 4096;
		caffemodel_file_buffer = new char[ enc_caffemodel_file_size ];
		fmodel.read( caffemodel_file_buffer, src_caffemodel_file_size );
		src_crc32_caffemodel = crc32( src_crc32_caffemodel, caffemodel_file_buffer, src_caffemodel_file_size );
		fmodel.close();
	}
	//rc6加密算法必须是4个int字节对齐，在buf后面添加随机数补充
	
	enc_buffer_start = ( ( src_prototxt_file_size / sizeof( unsigned int ) + 1 ) * sizeof( unsigned int ) );
	fill_rand_data( ( unsigned char * )( prototxt_file_buffer + enc_buffer_start ), ( enc_prototxt_file_size - enc_buffer_start ) );
	enc_buffer_start = ( ( src_caffemodel_file_size / sizeof( unsigned int ) + 1 ) * sizeof( unsigned int ) );
	fill_rand_data( ( unsigned char * )( caffemodel_file_buffer + enc_buffer_start ), ( enc_caffemodel_file_size - enc_buffer_start ) );

	//rc6加密算法
	encryp_buffer( ( unsigned char * )prototxt_file_buffer, enc_prototxt_file_size, key);
	encryp_buffer( ( unsigned char * )caffemodel_file_buffer, enc_caffemodel_file_size,key);
	//计算加密以后的crc32校验码
	
	enc_crc32_prototxt = crc32(enc_crc32_prototxt, prototxt_file_buffer, enc_prototxt_file_size );
	

	enc_crc32_caffemodel = crc32( enc_crc32_caffemodel, caffemodel_file_buffer, enc_caffemodel_file_size );

	prototxt_Format.file_type_flag = 1;
	prototxt_Format.src_file_length = src_prototxt_file_size;
	prototxt_Format.src_file_crc32_check = src_crc32_prototxt;
	prototxt_Format.enc_file_length = enc_prototxt_file_size;
	prototxt_Format.enc_file_crc32_check = enc_crc32_prototxt;

	caffemodel_Format.file_type_flag = 2;
	caffemodel_Format.src_file_length = src_caffemodel_file_size;
	caffemodel_Format.src_file_crc32_check = src_crc32_caffemodel;
	caffemodel_Format.enc_file_length = enc_caffemodel_file_size;
	caffemodel_Format.enc_file_crc32_check = enc_crc32_caffemodel;

	std::ofstream out;
	out.open(outputfile, std::ios::out|std::ios::binary|std::ios::trunc);

	out.write( (char *)&prototxt_Format, sizeof( File_Firmware_Format ) );
	out.write( (char *)&caffemodel_Format, sizeof( File_Firmware_Format ) );
	out.write( (char *)prototxt_file_buffer, enc_prototxt_file_size );
	out.write( (char *)caffemodel_file_buffer, enc_caffemodel_file_size );
	out.close();
	delete[] prototxt_file_buffer;
	delete[] caffemodel_file_buffer;
}



//模型解密
bool decrypt_model(std::string encryption_model, std::string &prototxt_string, std::string &model_string, char* key, std::string pro)
{
	std::ifstream input;
	std::ifstream in;
	in.open(encryption_model, std::ios::in|std::ios::binary);

	File_Firmware_Format prototxt_Format;
	File_Firmware_Format caffemodel_Format;

	in.read((char *)&prototxt_Format ,sizeof(File_Firmware_Format));
	in.read((char *)&caffemodel_Format ,sizeof(File_Firmware_Format));

	char *prototxt_file_buffer = NULL;
	char *caffemodel_file_buffer = NULL;

	unsigned int enc_crc32_prototxt = 0;
	unsigned int crc32_prototxt = 0;
	unsigned int enc_crc32_caffemodel = 0;
	unsigned int crc32_caffemodel = 0;


	if(prototxt_Format.file_type_flag == 1)
	{
		long enc_prototxt_file_size = prototxt_Format.enc_file_length;
		long src_prototxt_file_size = prototxt_Format.src_file_length;
		
		prototxt_file_buffer = new char[enc_prototxt_file_size];
		in.read(prototxt_file_buffer ,sizeof(char) * enc_prototxt_file_size);
		enc_crc32_prototxt = crc32(enc_crc32_prototxt, prototxt_file_buffer, enc_prototxt_file_size);
		if (enc_crc32_prototxt != prototxt_Format.enc_file_crc32_check)
		{
			return false;
		}

		decryp_buffer((uchar*)prototxt_file_buffer, enc_prototxt_file_size, key);

		crc32_prototxt = crc32( crc32_prototxt, prototxt_file_buffer, src_prototxt_file_size );
		if (crc32_prototxt != prototxt_Format.src_file_crc32_check)
		{
			return false;
		}

		std::string pbuf(prototxt_file_buffer,src_prototxt_file_size);
		prototxt_string = pbuf;

		delete[] prototxt_file_buffer;
	}
	else
	{
		in.close();
		return false;
	}

	if(caffemodel_Format.file_type_flag == 2)
	{
		long enc_caffemodel_file_size = caffemodel_Format.enc_file_length;
		long src_caffemodel_file_size = caffemodel_Format.src_file_length;

		caffemodel_file_buffer = new char[enc_caffemodel_file_size];
		in.read(caffemodel_file_buffer ,sizeof(char) * enc_caffemodel_file_size);

		enc_crc32_caffemodel = crc32(enc_crc32_caffemodel, caffemodel_file_buffer, enc_caffemodel_file_size);
		if(enc_crc32_caffemodel != caffemodel_Format.enc_file_crc32_check) return false;

		decryp_buffer((uchar*)caffemodel_file_buffer, enc_caffemodel_file_size, key);
		crc32_caffemodel = crc32( crc32_caffemodel, caffemodel_file_buffer, src_caffemodel_file_size );
		if(crc32_caffemodel != caffemodel_Format.src_file_crc32_check) return false;

		std::string mbuf(caffemodel_file_buffer,src_caffemodel_file_size);
		model_string = mbuf;

		delete[] caffemodel_file_buffer;
	}
	else
	{
		in.close();
		return false;
	}
	
	std::ofstream out;
	out.open(pro, std::ios::out| std::ios::app);
	out << prototxt_string;
	out.close();
	return true;
}




