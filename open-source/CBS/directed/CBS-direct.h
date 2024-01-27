#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<unordered_map>
#include<assert.h>
#include "CBS-sampletable-direct.h"

using namespace std;

class CBS
{
public:
	CBS_sampletable* ss;
	int window_size;
	long long current_time;
	double increase_prob; // probability for computation of incremental of the 
	double expired_prob;
	int random_seed; // random seed, used to carry out experiments multiple times with different random values.
	long long* sliced_trcount; // triangle counter for the last k+1 slices.
	long long expired_trcount; // triangle counter for the difference area between the last k+1 slices and the sliding window.
	unsigned int slice_num; // number of slices to monitor, namely k.
	unsigned int slice_size; // size of each slice, namley windowsize / slice_num
	long long max_landmark; //the maximum landmark below current time, used to check which slice a traingle falls into
	unsigned int frontier; // record the position of the counter for the last slice in array sliced_trcount.
	bool unbiased_correction;
	vector<int> direction_parameters[7];
	vector<int> bi_parameters[7];
	int triangle_direction;


	CBS(int sample_size, int window_size_, int slice_num_, int triangle_direction_, int random_seed_, bool correction = true, int group_num = 10)
	{
		window_size = window_size_;
		ss = new CBS_sampletable(sample_size, window_size, group_num);
		random_seed = random_seed_;
		srand(random_seed);
		current_time = 0;
		expired_prob = 0;
		increase_prob = 0;
		slice_num = slice_num_;
		sliced_trcount = new long long[slice_num + 1];
		expired_trcount = 0;
		slice_size = window_size / slice_num;
		frontier = 0;
		max_landmark = 0;
		unbiased_correction = correction;
		for (int i = 0; i < slice_num + 1; i++)
			sliced_trcount[i] = 0;
		triangle_direction = triangle_direction_;
		// type A
		direction_parameters[0].resize(6);
		direction_parameters[0] = { out, out, out, in, in, in };
		bi_parameters[0].resize(0);

		// type B
		direction_parameters[1].resize(2);
		direction_parameters[1] = { in, out };
		bi_parameters[1].resize(0);

		// type C
		direction_parameters[2].resize(4);
		direction_parameters[2] = { bi, out, in, bi };
		bi_parameters[2].resize(4);
		bi_parameters[2] = { out, in, in, out };

		// type D
		direction_parameters[3].resize(2);
		direction_parameters[3] = { out, bi };
		bi_parameters[3].resize(2);
		bi_parameters[3] = { in, in };

		// type E
		direction_parameters[4].resize(2);
		direction_parameters[4] = { bi, in };
		bi_parameters[4].resize(2);
		bi_parameters[4] = { out, out };

		// type F
		direction_parameters[5].resize(2);
		direction_parameters[5] = { bi, bi };
		bi_parameters[5].resize(8);
		bi_parameters[5] = { in, bi, bi, in, bi, out, out, bi };


		// type G
		direction_parameters[6].resize(0);
		bi_parameters[6].resize(2);
		bi_parameters[6] = { bi, bi };
	}
	~CBS()
	{
		delete ss;
		delete[]sliced_trcount;
	}
	void compute_probability() // compute increase probability and decrease probability based on the sample set.
	{
		unsigned int sample_size = ss->valid_sample_num();
		unsigned int edge_estimate = ss->cardinality_estimate();
		increase_prob = (double(sample_size) / edge_estimate) * (double(sample_size - 1) / (edge_estimate - 1));
		expired_prob = (double(sample_size) / edge_estimate) * (double(sample_size - 1) / (edge_estimate - 1)) * (double(sample_size - 2) / (edge_estimate - 2));
	}

	void modify_triangle(unsigned int s, unsigned int d, long long time, bool bi_direction, int op) // source node id, destination node id and timestamp of and edge, op means if the edge is deleted or expired.
	{
		compute_probability();
		unordered_map<unsigned int, vector<long long> > um;  // note that for each neighbor, the edge timestamps are organized as a vector, as there may be multiple occurences of the same edge with different timestamps
		vector<pair<unsigned int, long long> >vec; // store the neighbor set of s and d with unordered_map and vector, respectively. In convenient for the later join
		int direction_round = direction_parameters[triangle_direction].size() / 2;
		int bi_round = bi_parameters[triangle_direction].size() / 2;

		//cout << "modify triangle " << s << ' ' << d << " " << bi_direction << endl;
		int round = bi_direction ? bi_round : direction_round;
		int direction_s;
		int direction_d;
		int last_direction_s = -1;
		int last_direction_d = -1;

		for (int i = 0; i < round; i++) {

			direction_s = bi_direction ? bi_parameters[triangle_direction][i * 2] : direction_parameters[triangle_direction][i * 2];
			direction_d = bi_direction ? bi_parameters[triangle_direction][i * 2 + 1] : direction_parameters[triangle_direction][i * 2 + 1];


			if (direction_s != last_direction_s)
			{
				um.clear();
				ss->get_neighbor_list(s, direction_s, um);
			}
			if (direction_d != last_direction_d)
			{
				vec.clear();
				ss->get_neighbor_list(d, direction_d, vec);
			}
			//	cout << um.size() << " " << vec.size() << endl;

			for (int j = 0; j < vec.size(); j++)
			{
				unsigned int neighbor_id = vec[j].first;
				long long neighbor_timestamp = vec[j].second;
				if (um.find(neighbor_id) != um.end())
				{
					for (int k = 0; k < um[neighbor_id].size(); k++) {
						if (op == -1) //  if this is an edge expiration, only modify window_trcount;
						{
							expired_trcount += double(1) / expired_prob;
						}
						else {
							long long tr_time = min(um[neighbor_id][k], neighbor_timestamp);
							tr_time = min(time, tr_time);
							unsigned int pos;
							unsigned int slice_shift;
							if (tr_time > max_landmark)
								slice_shift = 0;
							else
								slice_shift = (max_landmark - tr_time) / slice_size + 1; // slice_shift indicates the gap between the target slice and the last slice.
							if (slice_shift > slice_num)
								continue;
							if (frontier >= slice_shift) // according to the slice shift, we compute the position of the triangle counter for the target slice.
								pos = frontier - slice_shift;
							else
								pos = slice_num + 1 - (slice_shift - frontier);
							sliced_trcount[pos] += double(1) / increase_prob;
						}
					}
				}
			}
		}
		um.clear();
		vec.clear();
	}

	void update(long long time) // given current time, update the sample set and counters to this time
	{
		current_time = time; // record current time
		while (time - max_landmark > slice_size) { // in an exteme case, update may be called after a long period, and multiple slices have passed.
			max_landmark += slice_size;
			frontier++; // push the fontier pointer forward, and reset triangle counter of the eldest slice, in order to make space for the new slice.
			if (frontier > slice_num)
				frontier = 0;
			sliced_trcount[frontier] = 0;
			expired_trcount = 0;
		}
		edge_info tmp = ss->expire_one_edge(time - window_size); // delete expired edges.
		while (tmp.priority > 0) // for each expired edge, we need to count the number of triangles containing this edge, and compute approximation of expired triangles based on it.  
		{
			if (unbiased_correction && tmp.timestamp > max_landmark - window_size) // not that edges before max_landmark-window_size should not influence the triangle count. The triangle decrease induced by their expiration is covered by seting window_trcount to the sum of sliced counters.
				modify_triangle(tmp.src, tmp.dst, tmp.timestamp, tmp.bidirection, -1);
			tmp = ss->expire_one_edge(time - window_size);
		}
	}
	void proceed(unsigned int s, unsigned int d, bool bidirection, long long time)
	{
		if (s < d&&bidirection)
		{
			unsigned int tmp = s;
			s = d;
			d = tmp;
		}
		update(time);
		ss->insert(s, d, time, bidirection);
		modify_triangle(s, d, time, bidirection,1);
	}

	long long count()
	{
		int cnt = 0;
		int idx = frontier;
		long long sum = 0;
		while (cnt <= slice_num)
		{
			sum += sliced_trcount[idx];
		//	cout << "sliced trcount " << sliced_trcount[idx] << endl;
			idx--;
			if (idx < 0)
				idx = slice_num;
			cnt++;
		}
		if (unbiased_correction)
			return sum - expired_trcount;
		else
			return sum;
	}

};
