#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#include<assert.h>
#include "SWTC-sampletable.h"

using namespace std;

class SWTC
{
public:
	SWTC_sampletable* ss;
	int window_size;
	int group_num;
	long long current_time;
	long long* land_mark;
	long long* last_mark;
	int edge_estimate;
	int sample_size;
	double sample_prob;
	int randomseed;
	int gswitch_iter; // use a iterator to mark the group with largest group ID and has switched slice. This iterator increase from 0 to group_num and return to 0;

	SWTC(int size, int w, int g, int random_seed)
	{
		ss = new SWTC_sampletable(size, g);
		randomseed = random_seed;
		srand(randomseed);
		group_num = g;
		window_size = w;
		current_time = 0;
		land_mark = new long long[g];
		last_mark = new long long[g];
		for(int i=0;i<g;i++)
		{
			land_mark[i] = -(g-i)*(window_size/g);
		}
		gswitch_iter = 0;
		sample_size = 0;
		edge_estimate = 0;
		sample_prob = 0;

	}
	~SWTC()
	{
		delete ss;
		delete []land_mark;
		delete []last_mark;
	}

	void proceed(unsigned int s, unsigned int d, long long time)
	{
		if (s < d)
		{
			unsigned int tmp = s;
			s = d;
			d = tmp;
		}
		double p = (rand()%10000+1)/double(10000+2);
		int group  = rand()% group_num;
		current_time = time;
		ss->update(time - window_size);
		while (time - land_mark[gswitch_iter] >= window_size)
		{
			assert(time - land_mark[gswitch_iter] < 2 * window_size);
			last_mark[gswitch_iter] = land_mark[gswitch_iter];
			land_mark[gswitch_iter] = land_mark[gswitch_iter] + window_size;
			ss->slice_switch(last_mark[gswitch_iter], gswitch_iter);
			gswitch_iter++;
			if(gswitch_iter==group_num)
				gswitch_iter = 0;
		}
		ss->insert(s, d, group, p, time, land_mark[group], last_mark[group]);
	}

	void prepare()
	{
		sample_size = 0;
		for(int i=0;i<group_num;i++)
			sample_size += ss->valid_num[i];
		
		edge_estimate = 0;
		for(int i=0;i<group_num;i++)
		{
		int m = ss->group_size;
		double alpha = 0.7213 / (1 + (1.079 / m));
		int group_card = (double(alpha * m * m) / (ss->q_count[i]));
		if (group_card < 2.5*m)
			 group_card = -log(1 - double(ss->edge_count[i]) / m)*m;
		if(ss->edge_count[i]>0)
			edge_estimate += group_card *(double(ss->valid_num[i]) / ss->edge_count[i]);
		}

		if (sample_size > 3 && edge_estimate > 3)
			sample_prob = (double(sample_size) / edge_estimate) * (double(sample_size - 1) / (edge_estimate - 1)) * (double(sample_size - 2) / (edge_estimate - 2));
		else
			sample_prob = 0xFFFFFFFF;

	}
	unsigned long long count()	
	{
		return (ss->trcount) / sample_prob;
	}


	int valid_count()
	{
		int total_count = 0;
		for(int i=0;i<group_num;i++)
			total_count += ss->valid_num[i];
		return total_count;
	}
};
