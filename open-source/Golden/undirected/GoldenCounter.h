#include<iostream>
#include<string>
#include<vector> 
#include "Graph.h"
using namespace std;


struct golden_timed_edge
{
	long long timestamp;
	unsigned int s; 
	unsigned int d;
	golden_timed_edge* next;
	golden_timed_edge(unsigned int s_, unsigned int d_, long long t)
	{
		s = s_;
		d = d_;
		timestamp = t;
		next = NULL;
	}
};

class GoldenCounter
{
	private:
		int windowsize;
		long long current_time;
		golden_timed_edge* tsl_head;
		golden_timed_edge* tsl_tail;
		int duplicated_edgenum;
		Graph* graph;
	 
	public:
		GoldenCounter(int w)
		{
			windowsize = w;
			current_time = 0;
			graph = new Graph;
			duplicated_edgenum = 0;
			tsl_head = NULL;
			tsl_tail = NULL;
		}
		~GoldenCounter()
		{
			golden_timed_edge* cur = tsl_head;
			tsl_head = NULL;
			tsl_tail = NULL;
			golden_timed_edge* next = cur;
			while (cur)
			{
				next = cur->next;
				delete cur;
				cur = next;
			}
			delete graph; 
		}
		void local_edge_count(unordered_map<unsigned long long, int> &cr)
		{
			graph->local_edge_count(cr);
		}
		int get_edgenum()
		{
			return graph->get_edgenum();
		}
		void insert_edge(unsigned int s, unsigned int d, long long time)
		{
		/*	if(s == d)
				return;*/
			duplicated_edgenum++;
			if(s < d)
			{
				unsigned int tmp = s;
				s = d;
				d = tmp;
			}
			///cout<<"inserting : "<<s<<' '<<d<<' ';
			current_time = time;
			golden_timed_edge* e = new golden_timed_edge(s, d, current_time);
			if (!tsl_head)
				tsl_head = e;
			if (tsl_tail)
				tsl_tail->next = e;
			tsl_tail = e;
			graph->insert_edge(s, d);
			
			golden_timed_edge* cur = tsl_head;
			golden_timed_edge* next;
			while (cur->timestamp < current_time - windowsize)
			{
				duplicated_edgenum--;
				next = cur->next;
				graph->delete_edge(cur->s, cur->d);
				delete cur;
				cur = next;
				tsl_head = next;
				if(!cur)
				    return;
			}
			//cout<<"insert finished"<<endl;
			return; 
		}
		
		int duplicate_count()
		{
			return duplicated_edgenum;
		}
		int edge_count()
		{
			return graph->get_edgenum();
		}
		
		int triangle_count()
		{
			return graph->count_triangle();
		 } 
		 unsigned long long weighted_count()
		 {
		 	return graph->weighted_count_triangle();
		 }
		void local_count(unordered_map<unsigned int, int> &cr)
		{
		  	graph->local_count(cr);	
		}
		void weighted_local(unordered_map<unsigned int, unsigned long long> &cr)
		{
			graph->weighted_local_count(cr);
		}	
	
		
};
