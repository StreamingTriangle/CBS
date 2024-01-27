#include<iostream>
#include<string>
#include<vector> 
#include<assert.h>
#include<unordered_map>
#ifndef setting 
#include "../../Common/undirected/undirected-setting.h"
#define setting
#endif
using namespace std;

class CBS_fp
{
public:
	unordered_map<unsigned int, node_info> graph; // 8+8+8 = 24 bytes per node, max node num <= 2* max edge num
	double p;
	int w;
	unsigned int slice_cnt;
	unsigned int slice_size;
	long long* sliced_trcount;
	long long expired_trcount;
	unsigned int frontier;
	unsigned int max_landmark;
	timed_edge* tsl_head;
	timed_edge* tsl_tail;
	long long current_time;
	int random_seed;
	CBS_fp(double probability, int window_size, int slice_num, int randomseed)
	{
		w = window_size;
		p = probability;
		tsl_head = NULL;
		tsl_tail = NULL;
		slice_cnt = slice_num;
		slice_size = w / slice_cnt;
		random_seed = randomseed;
		srand(randomseed);
		sliced_trcount = new long long [slice_num+1];
		for (int i = 0; i < slice_num + 1; i++)
				sliced_trcount[i] = 0;
		expired_trcount = 0;
		max_landmark = 0;
		current_time = 0;
		frontier = 0;
	}
	~CBS_fp()
	{
		unordered_map<unsigned int, node_info>::iterator it;
		for (it = graph.begin(); it != graph.end(); it++)
		{
			timed_edge* tmp = (it->second).first_edge;
			timed_edge* next = tmp;
			while (tmp)
			{
				next = tmp->src_next;
				delete tmp;
				tmp = next;
			}
		}
		graph.clear();
		delete[]sliced_trcount;
	}

	void print()
	{
		cout << "printing graph: " << endl;
		unordered_map<unsigned int, node_info>::iterator it;
		for (it = graph.begin(); it != graph.end(); it++)
		{
			cout << "node " << it->first << endl;
			timed_edge* tmp = (it->second).first_edge;
			while (tmp)
			{
				cout << tmp->s << ' ' << tmp->d << endl;
				if (tmp->s == it->first)
					tmp = tmp->src_next;
				else
					tmp = tmp->dst_next;
			}
			cout << endl;
		}

		cout << "printing time list:" << endl;
		timed_edge* tmp = tsl_head;
		while (tmp)
		{
			cout << tmp->s << ' ' << tmp->d << ' ' << tmp->timestamp << endl;
			tmp = tmp->tsl_next;
		}
		return;
	}


	void count_triangle(unsigned int s, unsigned int d, long long timestamp, int op)
	{
		unordered_map<unsigned int, node_info>::iterator it1 = graph.find(s);
		if (it1 == graph.end())
			return;
		timed_edge* s_cur = (it1->second).first_edge;

		unordered_map<unsigned int, node_info>::iterator it2 = graph.find(d);
		if (it2 == graph.end())
			return;
		timed_edge* d_cur = (it2->second).first_edge;

		unordered_map<unsigned int, vector<long long> > um;
		vector<pair<unsigned int, long long>> vec;
		while (s_cur)
		{
			if (s_cur->s == s)
			{
				um[s_cur->d].push_back(s_cur->timestamp);
				s_cur = s_cur->src_next;
			}
			else if (s_cur->d == s)
			{
				um[s_cur->s].push_back(s_cur->timestamp);
				s_cur = s_cur->dst_next;
			}
		}

		while (d_cur)
		{
			if (d_cur->s == d)
			{
				vec.push_back(make_pair(d_cur->d, d_cur->timestamp));
				d_cur = d_cur->src_next;
			}
			else if (d_cur->d == d)
			{
				vec.push_back(make_pair(d_cur->s, d_cur->timestamp));
				d_cur = d_cur->dst_next;
			}
		}

	//	cout << "get neighbor finished " << endl;
		for (int i = 0; i < vec.size(); i++)
		{
			unsigned int neighbor_id = vec[i].first;
			long long neighbor_timestamp = vec[i].second;
			if (um.find(neighbor_id) != um.end())
			{
				for (int j = 0; j < um[neighbor_id].size(); j++) {
					long long time = min(um[neighbor_id][j], neighbor_timestamp);
					unsigned int pos;
					if (max_landmark < slice_size) // if the first slice has not finished yet, there is only 1 effective triangle counter.
						pos = 0;
					unsigned int slice_shift;
					if (time > max_landmark)
							slice_shift = 0;
					else
							slice_shift = (max_landmark - time) / slice_size + 1;
					if (frontier >= slice_shift)
							pos = frontier - slice_shift;
					else
							pos = slice_cnt + 1 - (slice_shift - frontier);
				//	cout <<"pos "<< pos <<' '<<slice_shift<<' '<<slice_cnt<<' '<<frontier<<' '<<max_landmark<<' '<<time<<endl;
					if(op==1)
							sliced_trcount[pos]+= double(1)/(p*p);
					else {
							expired_trcount += double(1) / (p * p * p);
						}
					}
				}
		}
	}
	void insert_edge(unsigned int s, unsigned int d, long long time)
	{
	//	cout << "inserting " << s << ' ' << d << ' ' << time << endl;
		count_triangle(s, d, time, 1);
	//	cout << "count triangle finished" << endl;
		double prob = double(rand() % 1000 + 1) / 1001;
		if (prob > p)
			return;
    // 	cout << "prob computing finished" << endl;

		unordered_map<unsigned int, node_info>::iterator it1 = graph.find(s);
		if (it1 == graph.end())
		{
			node_info new_info;
			graph[s] = new_info;
			it1 = graph.find(s);
		}

		unordered_map<unsigned int, node_info>::iterator it2 = graph.find(d);
		if (it2 == graph.end())
		{
			node_info new_info;
			graph[d] = new_info;
			it2 = graph.find(d);
		}
		timed_edge* new_edge = new timed_edge(s, d, time);
		new_edge->src_next = (it1->second).first_edge;
		(it1->second).first_edge = new_edge;

		new_edge->dst_next = (it2->second).first_edge;
		(it2->second).first_edge = new_edge;

		if (tsl_tail)
			tsl_tail->tsl_next = new_edge;
		new_edge->tsl_prev = tsl_tail;
		new_edge->tsl_next = NULL;
		tsl_tail = new_edge;
		if (!tsl_head)
			tsl_head = new_edge;
	}
	void update(long long current_time)
	{
		while (current_time - max_landmark > slice_size) {
				max_landmark += slice_size;
				frontier++;
				if (frontier > slice_cnt)
					frontier = 0;
				sliced_trcount[frontier] = 0;
				expired_trcount = 0;
		}
		if (!tsl_head)
			return;
		long long ex_time = current_time - w;
		while (tsl_head->timestamp < ex_time)
		{
			unsigned int s = tsl_head->s;
			unsigned int d = tsl_head->d;

			count_triangle(s, d, tsl_head->timestamp, -1);

			// split the edge from the graph
			unordered_map<unsigned int, node_info>::iterator it1 = graph.find(s);
			assert(it1 != graph.end());
			unordered_map<unsigned int, node_info>::iterator it2 = graph.find(d);
			assert(it2 != graph.end());

			timed_edge* tmp = (it1->second).first_edge;
			timed_edge* parent = tmp;
			while (tmp)
			{
				if (tmp == tsl_head)
				{
					if (parent != tmp) {		// if not the first edge in the edge list of s
						if (parent->s == s)
							parent->src_next = tmp->src_next;
						else if (parent->d == s)
							parent->dst_next = tmp->src_next;
						else
							assert(false);
					}
					else
					{
						if (tmp->src_next == NULL)	// no edges left in the edge list of s
							graph.erase(it1); // delete the node
						else
							(it1->second).first_edge = tmp->src_next;
					}
					break;
				}
				parent = tmp;
				if (tmp->s == s)
					tmp = tmp->src_next;
				else if (tmp->d == s)
					tmp = tmp->dst_next;
				else
					assert(false);
			}


			tmp = (it2->second).first_edge;
			parent = tmp;
			while (tmp)
			{
				if (tmp == tsl_head)
				{
					if (parent != tmp) {		// if not the first edge in the edge list of s
						if (parent->s == d)
							parent->src_next = tmp->dst_next;
						else if (parent->d == d)
							parent->dst_next = tmp->dst_next;
						else
							assert(false);
					}
					else
					{
						if (tmp->dst_next == NULL)	// no edges left in the edge list of s
							graph.erase(it2);
						else
							(it2->second).first_edge = tmp->dst_next;
					}
					break;
				}
				parent = tmp;
				if (tmp->s == d)
					tmp = tmp->src_next;
				else if (tmp->d == d)
					tmp = tmp->dst_next;
				else
					assert(false);
			}


			// delete the edge and move the tsl_head pointer
			tmp = tsl_head;
			tsl_head = tsl_head->tsl_next;
			delete tmp;
		}
	}
	void process_edge(unsigned int s, unsigned int d, long long time)
	{
	//	cout << "processing " << s << ' ' << d << ' ' << time << endl;
		if (s < d)
		{
			unsigned int tmp = s;
			s = d;
			d = tmp;
		}
		current_time = time;
		update(time);
	//	cout << "update finished" << endl;
		insert_edge(s, d, time);
	//	cout << "insert finished" << endl;
	}
	unsigned long long count()
	{

		long long sum = 0;
		for (int i = 0; i < slice_cnt + 1; i++)
			sum += sliced_trcount[i];
		if (sum > expired_trcount)
			return sum - expired_trcount;
		else
			return 0;
	}
};