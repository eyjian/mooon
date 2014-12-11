
#include "util/token_list.h"
#include<string>

void print_token_list(const std::string &str,const std::string &sep)
{
	printf("before token:\n");
	printf("str = %s,sep = %s\n",str.c_str(),sep.c_str());

	util::CTokenList::TTokenList token_list;
	util::CTokenList::parse(token_list,str,sep);

	printf("after token:\n");
	util::CTokenList::TTokenList::iterator iter = token_list.begin();
	for (;iter != token_list.end(); ++iter)
	{
		printf("%s\n", iter->c_str());
	}
	printf("\n");
}

int main()
{

	printf("\n>>>>>>>>>>TEST token_list<<<<<<<<<<\n\n");
	const std::string str1 = "ABCMMMOPK||DEFG||HIJK||",sep1 = "||";
	const std::string str2 = "|||ABCMMMOPK|||DEFG|||HIJK|||",sep2 = "|||";
	const std::string str3 = "ABCDEFG|HIJK|LMN",sep3 = "|";
	const std::string str4 = "|||ABCD||EFGHIJ|||MNP||",sep4 = "|";

	print_token_list(str1,sep1);
	print_token_list(str2,sep2);
	print_token_list(str3,sep3);
	print_token_list(str4,sep4);
	return 0;
}

