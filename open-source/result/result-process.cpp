#include<iostream>
#include<fstream>
#include<vector>
using namespace std;

string my_to_string(unsigned int a)
{
   	string str="";
	while(a>0)
	{
		str += (char)('0'+a%10);
		a=a/10;
	}
	if(str=="")
		str = "0";
	string astr;
	astr.resize(str.length());
	for(int i=str.length()-1;i>=0;i--)
		astr[str.length()-1-i] = str[i]; 
     return astr ;
}

int main()
{
	unsigned int round = 10;
	vector<long long> golden_vec;
	ifstream fin("golden-count.txt");
	unsigned int algorithm_type = 1;// 1 means compute error for SWTC, 2 means CBS 
	string algorithm_name;
	if(algorithm_type ==1)
		algorithm_name = "SWTC";
	else
		algorithm_name = "CBS";
	
	long long trcount;
	while(fin>>trcount)
	{
		golden_vec.push_back(trcount);
	}
	fin.close();
	string prefix = "./";
	string postfix = "-count.txt";
	double error_sum = 0;
	double avg_max =0;
	for(int i=0;i<round;i++)
	{
		double round_error = 0;
		double max_error = 0;
		ifstream fin((prefix + algorithm_name + "-" + my_to_string(i) + postfix).c_str());
		unsigned int cnt = 0;
		unsigned int sum = 0;
		while(fin>>trcount)
		{
			if(cnt<50)
			{
				cnt++;
				continue; // skip the first 50 checkpoints, where the first sliding window has not been filled yet and the triangle count is very small.
			}
			double error;
			if(trcount>golden_vec[cnt])
			{
				error = double(trcount-golden_vec[cnt])/golden_vec[cnt];
			}
			else
				error = double(golden_vec[cnt]-trcount)/golden_vec[cnt];
			cnt++;
			sum++;
			round_error += error;
			if(error>max_error)
				max_error = error;
		}
		double avg_error = round_error/sum;
		cout<<"round "<<i<<" average error "<<avg_error<<" max error "<<max_error<<endl;
		error_sum += avg_error; 
		avg_max += max_error;
	} 
	cout<<" total average error "<<error_sum/round<<endl;
	cout<<" total max error "<<avg_max/round<<endl;
 } 
