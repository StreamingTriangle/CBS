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

class fp_sampler
{
	public:
		unordered_map<unsigned int, node_info> graph; // 4+8+8 = 20 bytes per node, max node num <= 2* max edge num
		double p;
		int random_seed;
		long long trcount;
		int w; 
		int edge_cnt;
		timed_edge* tsl_head;
		timed_edge* tsl_tail;
		fp_sampler(double probability, int window_size, int randomseed, int max_edge_num = 1000)
		{
			w = window_size;
			p = probability;
			random_seed = randomseed;
			srand(randomseed);
			trcount = 0;
			edge_cnt = 0;
			tsl_head = NULL;
			tsl_tail = NULL;
			graph.reserve(2*max_edge_num);
		}
		~fp_sampler()
		{
			unordered_map<unsigned int, node_info>::iterator it;
			for(it = graph.begin();it!=graph.end();it++)
			{
				timed_edge* tmp = (it->second).first_edge;
				timed_edge* next = tmp;
				while(tmp)
				{
					next = tmp->src_next;
					delete tmp;
					tmp = next; 
				}
			}
			graph.clear();
		}
		
		void print()
		{
			cout<<"printing graph: "<<endl;
			unordered_map<unsigned int, node_info>::iterator it;
			for(it = graph.begin();it!=graph.end();it++)
			{
				cout<<"node "<<it->first<<endl;
				timed_edge* tmp = (it->second).first_edge;
				while(tmp)
				{
					cout<<tmp->s<<' '<<tmp->d<<endl;
					if(tmp->s==it->first)
						tmp = tmp->src_next;
					else
						tmp = tmp->dst_next;
				}
				cout<<endl;	 
			}
			
			cout<<"printing time list:"<<endl;
			timed_edge* tmp = tsl_head;
			while(tmp)
			{
				cout<<tmp->s<<' '<<tmp->d<<' '<<tmp->timestamp<<endl;
				tmp = tmp->tsl_next;
			}
			return;
		}
		
		void count_triangle(unsigned int s, unsigned int d, int op)
		{
			unordered_map<unsigned int, node_info>::iterator it1 = graph.find(s);
			if (it1 == graph.end())
				return;
			timed_edge* s_cur = (it1->second).first_edge;

			unordered_map<unsigned int, node_info>::iterator it2 = graph.find(d);
			if (it2 == graph.end())
				return;
			timed_edge* d_cur = (it2->second).first_edge;

			unordered_map<unsigned int, int> um;
			vector<unsigned int> vec;
			while (s_cur)
			{
				if (s_cur->s == s)
				{
					if (um.find(s_cur->d) == um.end())
						um[s_cur->d] = 1;
					else
						um[s_cur->d]++;
					s_cur = s_cur->src_next;
				}
				else if (s_cur->d == s)
				{
					if (um.find(s_cur->s) == um.end())
						um[s_cur->s] = 1;
					else
						um[s_cur->s]++;
					s_cur = s_cur->dst_next;
				}
			}

			while (d_cur)
			{
				if (d_cur->s == d)
				{
					vec.push_back(d_cur->d);
					d_cur = d_cur->src_next;
				}
				else if (d_cur->d == d)
				{
					vec.push_back(d_cur->s);
					d_cur = d_cur->dst_next;
				}
			}
			for (int i = 0; i < vec.size(); i++)
			{
				unsigned int id = vec[i];
				if (um.find(id) != um.end()) {
					if(op==1)
						trcount += um[id];
					else
						trcount -= um[id];
				}
			}

		}
		void insert_edge(unsigned int s, unsigned int d, long long time)
		{
			double prob = double(rand() % 1000 + 1) / 1001;
			if(prob>p)	
				return;

			edge_cnt++;
				
			
			count_triangle(s, d, 1);	
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
		void update(long long ex_time)
		{
			if(!tsl_head)
				return;
			while(tsl_head->timestamp<ex_time)
			{
				unsigned int s = tsl_head->s;
				unsigned int d = tsl_head->d;
				count_triangle(s, d, -1); // delete triangle 
				
				// split the edge from the graph
				unordered_map<unsigned int, node_info>::iterator it1 = graph.find(s);
				assert(it1!=graph.end());
				unordered_map<unsigned int, node_info>::iterator it2 = graph.find(d);
				assert(it2!=graph.end());
				
				timed_edge* tmp = (it1->second).first_edge;
				timed_edge* parent = tmp;
				while(tmp)
				{
					if(tmp == tsl_head)
					{
						if(parent!=tmp){		// if not the first edge in the edge list of s
							if(parent->s==s)
								parent->src_next = tmp->src_next;
							else if(parent->d==s)
								parent->dst_next = tmp->src_next;
							else
								assert(false);
							}
						else
						{
							if(tmp->src_next==NULL)	// no edges left in the edge list of s
								graph.erase(it1); // delete the node
							else
								(it1->second).first_edge = tmp->src_next; 
						}
						break;	
					}
					parent = tmp;
					if(tmp->s==s)
						tmp = tmp->src_next;
					else if(tmp->d==s)
						tmp = tmp->dst_next;
					else
						assert(false);
				}
				
				
				tmp = (it2->second).first_edge;
				parent = tmp;
				while(tmp)
				{
					if(tmp == tsl_head)
					{
						if(parent!=tmp){		// if not the first edge in the edge list of s
							if(parent->s==d)
								parent->src_next = tmp->dst_next;
							else if(parent->d==d)
								parent->dst_next = tmp->dst_next;
							else
								assert(false);
							}
						else
						{
							if(tmp->dst_next==NULL)	// no edges left in the edge list of s
								graph.erase(it2);
							else
								(it2->second).first_edge = tmp->dst_next; 
						}
						break;	
					}
					parent = tmp;
					if(tmp->s==d)
						tmp = tmp->src_next;
					else if(tmp->d==d)
						tmp = tmp->dst_next;
					else
						assert(false);
				}
				
				
				// delete the edge and move the tsl_head pointer
				tmp = tsl_head;
				tsl_head = tsl_head->tsl_next;
				delete tmp;
				edge_cnt--;
			}
		}
		void process_edge(unsigned int s, unsigned int d, long long time)
		{
			if(s<d)
			{
				unsigned int tmp =s;
				s = d;
				d = tmp;
			}
			update(time-w);
			insert_edge(s, d, time);
		}
		long long count()
		{
			//cout << trcount << ' ' << (p * p * p) << endl;
			return trcount/(p*p*p);
		}

};
