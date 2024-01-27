#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<map>
#include<iostream>
#include"Common\undirected\undirected-algorithm.h"
using namespace std;

int main()
{
	ifstream fin("./stackoverflow.txt");
	unsigned int algorithm_type = 2;
	double sample_rate = 0.04;
	unsigned int s, d, l;
	long long t;
	unsigned int window_length = 4000000;
	double time_unit = 3.775; // average time span
	unsigned int window_size = window_length * time_unit;
	unsigned int sample_size = window_length * sample_rate;
	unsigned int random_round = 10;
	unsigned int random_offset = 0;
	string prefix = "./result/";
	string postfix = "-count.txt";
	vector<unsigned int> src;
	vector<unsigned int> dst;
	vector<long long> timestamp;
	unordered_set<unsigned long long> edges;
	int check_num = 50;
	int checpoint_length = window_size / check_num;
	unsigned int slice_num = 10;
	while (fin >> s >> d >> l >> t)
	{
		src.push_back(s);
		dst.push_back(d);
		timestamp.push_back(t);
	}
	if (algorithm_type == 0) //golden triangle
	{
		GoldenCounter* gc = new GoldenCounter(window_size);
		ofstream fout((prefix + "golden" + postfix).c_str());
		unsigned int checkpoint_cnt = 0;
		long long t0 = 0;
		for(int i=0;i<src.size();i++)
		{
			s = src[i];
			d = dst[i];
			t = timestamp[i];
			if (s == d)
				continue;
			if (t0 == 0)
				t0 = t;
			t = t - t0;
			gc->insert_edge(s, d, t);
			if (t/checpoint_length>checkpoint_cnt)
			{
				checkpoint_cnt = t/checpoint_length;
				unsigned long long golden_cnt = gc->weighted_count();
				cout << checkpoint_cnt << " checkpoints have been processed "<< golden_cnt << endl;
				fout << golden_cnt << endl;
			}
		}
	}
	if (algorithm_type == 1) //SWTC
	{
		for (int i = 0; i < random_round; i++) {
			unsigned int seed = i + random_offset;
			SWTC* as = new SWTC(sample_size, window_size, 10, seed);
			ofstream fout((prefix + "SWTC-" + my_to_string(seed) + postfix).c_str());
			unsigned int checkpoint_cnt = 0;
			long long t0 = 0;
			for (int j = 0; j < src.size(); j++)
			{
				s = src[j];
				d = dst[j];
				t = timestamp[j]; 
				if (s == d)
					continue;
				if (t0 == 0)
					t0 = t;
				t = t - t0;
				as-> proceed(s, d, t);
				if (t/checpoint_length>checkpoint_cnt)
				{
					checkpoint_cnt = t/checpoint_length;
					unsigned long long trcount = as->count();
					cout << checkpoint_cnt << " checkpoints have been processed " << trcount << endl;
					fout << trcount << endl;
				}
			}
			cout<<"round "<<i<<" finished"<<endl;
			cout<<endl;
		}
	}

	if (algorithm_type == 2) //CBS
	{
		for (int i = 0; i < random_round; i++) {
			unsigned int seed = i + random_offset;
			CBS* as = new CBS(sample_size, window_size, slice_num, seed);
			ofstream fout((prefix + "CBS-" + my_to_string(slice_num) + "-" + my_to_string(seed) + postfix).c_str());
			unsigned int checkpoint_cnt = 0;
			long long t0 = 0;
			for (int j = 0; j < src.size(); j++)
			{
				s = src[j];
				d = dst[j];
				t = timestamp[j];
				if (s == d)
					continue;
				if (t0 == 0)
					t0 = t;
				t = t - t0;
				as-> proceed(s, d, t);
				if (t / checpoint_length > checkpoint_cnt)
				{
					checkpoint_cnt = t / checpoint_length;
					unsigned long long trcount = as->count();
					cout << checkpoint_cnt << " checkpoints have been processed " << trcount << endl;
					fout << trcount << endl;
				}
			}
				cout<<"round "<<i<<" finished"<<endl;
				cout<<endl;
		}
	}
}


