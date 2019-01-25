#include <fstream>
#include<vector>
#include"crc32.h"
#include"enc_dec_model.h"

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}
void main()
{
	std::string input_txt = "input.txt";
	std::ifstream input_parameter;
	input_parameter.open(input_txt);
	std::string s;
	std::getline(input_parameter, s);
	
	
	if (s == "")
	{
		std::cout << "Donot find input.txt" << std::endl;
		system("pause");
		return;
	}
	std::vector<std::string> split_s;
	SplitString(s, split_s, " ");
	if (4 != split_s.size())
	{
		std::cout << "Number of input parameters not true" << std::endl;
	}
	for (int i = 0; i < split_s.size()-2; i++)
	{
		std::fstream _file;
		_file.open(split_s[i], std::ios::in);
		if (!_file)
	    {
	       std::cout <<"Donot find "<< split_s[i] <<std::endl;
		   system("pause");
		   return;
	    }
	}
	char* key = const_cast<char*>(split_s[2].c_str());
	encryption_model(split_s[0], split_s[1], key, split_s[3]);
}

//caffe工程解析模型
//#include "caffe/caffe.hpp"
//#include"caffe/layer.hpp"
//
//#include <boost/property_tree/xml_parser.hpp>
//#include <google/protobuf/io/coded_stream.h>
//#include <google/protobuf/io/zero_copy_stream_impl.h>
//#include <google/protobuf/text_format.h>
//#include"enc_dec_model.h"
//
////从string中加载模型
//int load_model(std::string encryption_model, caffe::Net<float>* & _net)
//{
//	char* key = "123456789";
//	std::string model_string;
//	std::string prototxt_string;
//	decrypt_model(encryption_model, prototxt_string, model_string, key);
//
//	//解析prototxt
//	caffe::NetParameter _prototxt;
//	std::istringstream net_prototxt(prototxt_string);
//	google::protobuf::io::IstreamInputStream * prototxt_input = new google::protobuf::io::IstreamInputStream((std::istream *)(&net_prototxt));
//	google::protobuf::io::ZeroCopyInputStream* prototxt_instream = prototxt_input;
//	google::protobuf::TextFormat::Parse(prototxt_instream, &_prototxt);
//	_prototxt.mutable_state()->set_phase(caffe::TEST);
//
//	//解析caffemodel
//	caffe::NetParameter _model;
//	std::istringstream net_model(model_string);
//	google::protobuf::io::IstreamInputStream * model_input = new google::protobuf::io::IstreamInputStream((std::istream *)(&net_model));
//	google::protobuf::io::CodedInputStream* coded_input_model = new google::protobuf::io::CodedInputStream(model_input);
//	coded_input_model->SetTotalBytesLimit(INT_MAX, 536870912);
//	_model.ParseFromCodedStream(coded_input_model);
//
//	//初始化网络
//	_net = new caffe::Net<float>(_prototxt);
//	//_net.reset(new caffe::Net<float>(_prototxt));  //定义一个网络
//
//	if (NULL == _net)
//	{
//		return false;
//	}
//
//	_net->CopyTrainedLayersFrom(_model); //加载权重
//
//	delete prototxt_instream;
//	delete coded_input_model;
//	delete model_input;
//
//	return true;
//}
