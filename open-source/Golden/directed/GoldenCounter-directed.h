#include<iostream>
#include<string>
#include<vector> 
#ifndef n_type
#define n_type unsigned int
#endif
#include "Graph-directed.h"
using namespace std;

struct timed_edge
{
	long long timestamp;
	n_type s; 
	n_type d;
	bool bi_direction;
	timed_edge* next;
	timed_edge(n_type s_, n_type d_, bool bi_direct, long long t)
	{
		s = s_;
		d = d_;
		bi_direction = bi_direct;
		timestamp = t;
		next = NULL;
	}
};

class GoldenCounter
{
	private:
		int windowsize;
		long long current_time;
		timed_edge* tsl_head;
		timed_edge* tsl_tail;
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
			timed_edge* cur = tsl_head;
			tsl_head = NULL;
			tsl_tail = NULL;
			timed_edge* next = cur;
			while (cur)
			{
				next = cur->next;
				delete cur;
				cur = next;
			}
			delete graph; 
		}
		int get_edgenum()
		{
			return graph->get_edgenum();
		}
		void insert_edge(n_type s, n_type d, bool bi_direction, long long time)
		{
			//if(!bi_direction)
			//cout << "inserting " << s << ' ' << d << ' ' << bi_direction << endl;
			duplicated_edgenum++;
			current_time = time;
			timed_edge* e = new timed_edge(s, d, bi_direction, current_time);
			if (!tsl_head)
				tsl_head = e;
			if (tsl_tail)
				tsl_tail->next = e;
			tsl_tail = e;
			graph->insert_edge(s, d, bi_direction);
			
			timed_edge* cur = tsl_head;
			timed_edge* next;
			while (cur->timestamp < current_time - windowsize)
			{
				duplicated_edgenum--;
				next = cur->next;
			//	cout << "delete " << cur->s << ' ' << cur->d <<' '<< cur->bi_direction << endl;
				graph->delete_edge(cur->s, cur->d, cur->bi_direction);
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
		
		int triangle_count(int direction)
		{
			return graph->count_triangle(direction);
		}
		unsigned long long weighted_triangle_count(int direction)
		{
			return graph->weighted_count(direction);
		} 
		
		void local_count(unordered_map<unsigned int, int> &cr, int direction)
		{
			graph->local_count(cr, direction);
		}
		
		void weighted_local(unordered_map<unsigned int, unsigned long long>& cr, int direction)
		{
			graph->weighted_local(cr, direction);
		}
		void print_graph()
		{
			graph->print_graph();
		}
		
};
