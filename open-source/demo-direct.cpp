#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<map>
#include<iostream>
#include"Common\directed\directed-algorithm.h"
using namespace std;

int main()
{
	//ifstream fin("D://学习相关文档//论文研究//Data//out.lkml-reply");
	ifstream fin("./stackoverflow.txt");
	unsigned int algorithm_type = 0;
	double sample_rate = 0.04;
	unsigned int s, d, w;
	long long t;
	unsigned int window_length = 4000000;
	double time_unit = 3.775; // average time span
	unsigned int window_size = window_length * time_unit;
	unsigned int sample_size = window_length * sample_rate;
	unsigned int random_round = 10;
	unsigned int random_offset = 0;
	vector<unsigned int> src;
	vector<unsigned int> dst;
	vector<long long> timestamp;
	vector<bool> bidirection;
	unordered_set<unsigned long long> edges;
	int check_num = 50;
	int checpoint_length = window_size / check_num;
	unsigned int slice_num = 10;
	while (fin >> s >> d >> w >> t)
	{
		src.push_back(s);
		dst.push_back(d);
		timestamp.push_back(t);
		if (w == 0)
			bidirection.push_back(true);
		else
			bidirection.push_back(false);
	}
	for (unsigned int triangle_direction = 0; triangle_direction < 7; triangle_direction++) {
			string prefix = "./result/";
	string postfix = "-" + my_to_string(triangle_direction) + "-direct-count.txt";
		if (algorithm_type == 0) //golden triangle
		{
			GoldenCounter* gc = new GoldenCounter(window_size);
			ofstream fout((prefix + "golden" + postfix).c_str());
			unsigned int checkpoint_cnt = 0;
			long long t0 = 0;
			for (int i = 0; i < src.size(); i++)
			{
				s = src[i];
				d = dst[i];
				t = timestamp[i];
				//t = i;
				if (s == d)
					continue;
				if (t0 == 0)
					t0 = t;
				t = t - t0;
				gc->insert_edge(s, d, bidirection[i], t);
				if (t / checpoint_length > checkpoint_cnt)
				{
					checkpoint_cnt = t / checpoint_length;
					unsigned long long golden_cnt = gc->weighted_triangle_count(triangle_direction);
					cout << checkpoint_cnt << " checkpoints have been processed " << golden_cnt << endl;
					fout << golden_cnt << endl;
				}
			}
		}
		if (algorithm_type == 1) //SWTC
		{
			for (int i = 0; i < random_round; i++) {
				unsigned int seed = i + random_offset;
				SWTC* as = new SWTC(sample_size, window_size, 10, triangle_direction, seed);
				ofstream fout((prefix + "SWTC-" + my_to_string(seed) + postfix).c_str());
				unsigned int checkpoint_cnt = 0;
				long long t0 = 0;
				for (int j = 0; j < src.size(); j++)
				{
					s = src[j];
					d = dst[j];
					t = timestamp[j];
					//t = j;
					if (s == d)
						continue;
					if (t0 == 0)
						t0 = t;
					t = t - t0;
					as->proceed(s, d, bidirection[j], t);
					if (t / checpoint_length > checkpoint_cnt)
					{
						checkpoint_cnt = t / checpoint_length;
						as->prepare();
						unsigned long long trcount = as->count();
						cout << checkpoint_cnt << " checkpoints have been processed " << as->sample_prob<<' '<<as->ss->trcount<<' '<<trcount << endl;
						fout << trcount << endl;
					}
				}
				cout << "round " << i << " finished" << endl;
				cout << endl;
			}
		}

		if (algorithm_type == 2) //CBS
		{
			for (int i = 0; i < random_round; i++) {
				unsigned int seed = i + random_offset;
				CBS* as = new CBS(sample_size, window_size, slice_num, triangle_direction, seed);
				ofstream fout((prefix + "CBS-" + my_to_string(slice_num) + "-" + my_to_string(seed) + postfix).c_str());
				unsigned int checkpoint_cnt = 0;
				long long t0 = 0;
				for (int j = 0; j < src.size(); j++)
				{
					s = src[j];
					d = dst[j];
					t = timestamp[j];
					//t = j;
					if (s == d)
						continue;
					if (t0 == 0)
						t0 = t;
					t = t - t0;
					as->proceed(s, d, bidirection[j], t);
					if (t / checpoint_length > checkpoint_cnt)
					{
						checkpoint_cnt = t / checpoint_length;
						unsigned long long trcount = as->count();
						cout << checkpoint_cnt << " checkpoints have been processed " << trcount << endl;
						fout << trcount << endl;
					}
				}
				cout << "round " << i << " finished" << endl;
				cout << endl;
			}
		}
	}
}


