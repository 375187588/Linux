#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include "Bird.h"

using namespace std;

int main()
{
	std::vector<int> v;
	v.push_back(11);
	v.push_back(22);
	v.push_back(33);
	auto it = v.begin();
	while(it != v.end())
	{
	//	std::cout << "c++:" << it << std::endl;
		std::cout << std::endl;
		std::cout << *it;
		std::cout << "c++11:" << *it << std::endl;
		printf("id:%d\n",*it);
		it++;
	}
	CAnimation* p=new CBird("fly bird");
	std::cout << "animation :" << p->GetName() << std::endl;
	return 0;
}
